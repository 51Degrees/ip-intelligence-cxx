# Proposal: Support WEIGHTED_STRING as a Stored Value Type

## Overview

This document describes the implementation plan for reading property values that have `WEIGHTED_STRING` as their `storedValueType` in the IPI dataset. Until now, `WEIGHTED_STRING` has only been used as a `valueType` — the type of data returned by the API — to represent a probability distribution of property values across multiple profiles in a profile group. In this project, `WEIGHTED_STRING` is used as a `storedValueType` — the format in which the value is physically stored in the data file within a single profile.

## Background

### Property Type System

Properties in the IPI dataset have two type descriptors:

- **`valueType`** (in `property_t`): The conceptual type of the property value as returned by the API. Inherited from Device Detection.
- **`storedValueType`** (in `property_type_record_t`): The physical format of the value in the data file. Stored in the separate `propertyTypes` collection, which extends `property_t` for IPI-specific needs. The `property_type_record_t` also contains a `nameOffset` for lookup by name when the property index is not known.

Until now, all stored values in the `strings` collection (which stores all binary values, not just strings) have been **self-contained** — strings, integers, floats, byte arrays (IP addresses, WKB geometry), etc. None of them contain references to other items in the same collection.

### The Challenge

A `WEIGHTED_STRING` stored value breaks this invariant: it is a byte array containing `uint32_t` offsets that point to other strings within the same `strings` collection. These sub-references must be **eagerly materialized** (resolved into actual string values) when the value is read from the collection.

This is distinct from the existing weighted values mechanism in profile groups. Profile groups produce weighted values by combining multiple profiles, each contributing one value per property, with profile-level weights. A stored WEIGHTED_STRING, by contrast, is a **single compound value** within a single profile that internally contains multiple string+weight pairs.

## Binary Format

The format in which `WEIGHTED_STRING` stored property values are written in the data file:

```
+-------------------+-------------------------------------------+
| uint16_t count    | N items, each:                            |
| (2 bytes)         |   uint32_t stringOffset (4 bytes)         |
|                   |   uint16_t weight       (2 bytes)         |
+-------------------+-------------------------------------------+
Total size: 2 + (count * 6) bytes
```

| Field | Type | Description |
|-------|------|-------------|
| `count` | `uint16_t` | Number of string+weight pairs (0–65,535) |
| `stringOffset` | `uint32_t` | Offset into the `strings` collection for the string value |
| `weight` | `uint16_t` | Raw weight (0–65,535), converts to float (0.0–1.0) by dividing by 65,535.0 |

## Architecture Decisions

### Decision 1: Materialization in the Collection Getter (Decorator)

The `strings` collection's `get` method is decorated so that when the key type is `WEIGHTED_STRING`, the raw blob (containing offsets) is read, and each string offset is resolved against the same underlying `strings` collection. The returned item contains a materialized buffer with actual string content, not raw offsets. This is purely a data-reading concern and is transparent to all consumers.

**Rationale:** Materialization is a collection-level responsibility. Callers of `StoredBinaryValueGet` should not need to know that the stored data contains sub-references. The decorator resolves them before the data leaves the collection layer.

### Decision 2: Single Compound Value in Results

The `addWeightedValue` callback (the renamed `addValueWithPercentage`) treats a stored WEIGHTED_STRING as a single value — one `WeightedItem` entry in the results list with one profile-level weight. The compound nature (multiple strings with sub-weights) is interpreted by the C++ accessor layer (`getValuesAsWeightedStringList` and friends). This keeps the two axes of weighting — profile-level (from profile groups) and value-level (from stored data) — cleanly separated.

**Rationale:** The stored weighted string is a single property value within a single profile. It should not be expanded into multiple profile-level result items. The sub-weights are an internal structure of the value, not a result of multi-profile matching.

### Decision 3: Rename Profile-Specific Types to Generic Names

The existing `ProfilePercentage` and `IpiList` types were originally named for profile-level weighting from profile groups. These are generic containers for weighted collection items and should be named accordingly.

## Type Renames

| Current Name | New Name | Scope |
|---|---|---|
| `fiftyoneDegreesProfilePercentage` | `fiftyoneDegreesWeightedItem` | Public API (ipi.h) |
| `fiftyone_degrees_profile_percentage_t` | `fiftyone_degrees_weighted_item_t` | Struct tag |
| `fiftyoneDegreesIpiList` | `fiftyoneDegreesWeightedItemList` | Public API (ipi.h) |
| `fiftyone_degrees_ipi_list_t` | `fiftyone_degrees_weighted_item_list_t` | Struct tag |
| `addValueWithPercentage` | `addWeightedValue` | Static in ipi.c |
| `stateWithPercentage` | `stateWithWeighting` | Static in ipi.c |
| `profilePercentageItem` (local vars) | `weightedItem` | Local variables |
| `ProfilePercentage` (MAP_TYPE) | `WeightedItem` | fiftyone.h |
| `ResultsIpiGetValues` return type | `const fiftyoneDegreesWeightedItem*` | Public API |

**`rawWeighting`** is kept as-is — it is already generic enough.

Updated struct definition:

```c
/**
 * A weighted collection item. Represents a value retrieved from the
 * dataset together with a weight indicating the proportion or
 * confidence of this value (out of 65535). Used for profile-level
 * weighting (from profile groups) where the weight indicates the
 * proportion of the matched IP range attributable to this profile.
 */
typedef struct fiftyone_degrees_weighted_item_t {
    fiftyoneDegreesCollectionItem item;
    uint16_t rawWeighting;
} fiftyoneDegreesWeightedItem;
```

**Files affected by rename:**
- `ipi.h` — type definitions, `FIFTYONE_DEGREES_RESULTS_IPI_MEMBERS` macro, `ResultsIpiGetValues` signature
- `ipi.c` — all internal usage (~20 references)
- `ResultsIpi.cpp` — C++ layer consuming the C types (~10 references)
- `ipi_weighted_results.c` — weighted value construction (~3 references)
- `fiftyone.h` — `MAP_TYPE` macro
- `ResultsIpi.i` — SWIG interface (if it references the C type directly)

## Implementation Plan

### Step 1: New CollectionKeyType for WEIGHTED_STRING (common-cxx)

**`collectionKeyTypes.h`** — Add size computation function and key type constant:

```c
#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
static uint32_t fiftyoneDegreesGetFinalWeightedStringSize(
    const void * const initial,
    fiftyoneDegreesException * const exception) {
#   ifdef _MSC_VER
    UNREFERENCED_PARAMETER(exception);
#   endif
    return (uint32_t)(sizeof(uint16_t) + (*(uint16_t*)initial) * 6);
}
#else
#define fiftyoneDegreesGetFinalWeightedStringSize NULL
#endif

static const fiftyoneDegreesCollectionKeyType CollectionKeyType_WeightedString_raw = {
    FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING,
    sizeof(uint16_t),                                    // Read count first
    fiftyoneDegreesGetFinalWeightedStringSize,            // Total = 2 + count * 6
};
static const fiftyoneDegreesCollectionKeyType * const CollectionKeyType_WeightedString =
    &CollectionKeyType_WeightedString_raw;
```

**`collectionKeyTypes.c`** — Add case in `GetCollectionKeyTypeForStoredValueType` switch:

```c
case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING:
    return CollectionKeyType_WeightedString;
```

This allows `StoredBinaryValueGet` to read the correct number of bytes from the collection for `WEIGHTED_STRING` entries.

### Step 2: Struct Definitions for Stored WEIGHTED_STRING (common-cxx)

**`storedBinaryValue.h`** — Add packed structs for the on-disk format and the materialized format:

**On-disk format (raw, before materialization):**

```c
#pragma pack(push, 1)
typedef struct fiftyone_degrees_stored_weighted_string_item_raw_t {
    uint32_t stringOffset;  /**< Offset into strings collection */
    uint16_t weight;        /**< Raw weight (0–65535) */
} fiftyoneDegreesStoredWeightedStringItemRaw;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fiftyone_degrees_stored_weighted_string_raw_t {
    uint16_t count;                                           /**< Number of items */
    fiftyoneDegreesStoredWeightedStringItemRaw items[1];      /**< Variable-length array */
} fiftyoneDegreesStoredWeightedStringRaw;
#pragma pack(pop)
```

**Materialized format (after decorator resolves offsets):**

```c
#pragma pack(push, 1)
typedef struct fiftyone_degrees_materialized_weighted_string_item_t {
    uint16_t weight;        /**< Raw weight (0–65535) */
    uint16_t stringSize;    /**< Size of string data including null terminator */
    char stringValue;       /**< First byte of string data (variable length follows) */
} fiftyoneDegreesStoredWeightedStringItem;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct fiftyone_degrees_stored_weighted_string_t {
    uint16_t count;         /**< Number of items */
    /**< Items follow as variable-length entries, accessed via iteration helper */
} fiftyoneDegreesStoredWeightedString;
#pragma pack(pop)
```

Add to the `StoredBinaryValue` union:

```c
typedef union fiftyone_degrees_stored_binary_value_t {
    fiftyoneDegreesString stringValue;
    fiftyoneDegreesVarLengthByteArray byteArrayValue;
    fiftyoneDegreesStoredWeightedString weightedStringValue;  /* NEW */
    fiftyoneDegreesFloat floatValue;
    int32_t intValue;
    int16_t shortValue;
    byte byteValue;
} fiftyoneDegreesStoredBinaryValue;
```

Provide an iteration helper to walk the variable-length materialized items:

```c
/**
 * Returns a pointer to the first materialized weighted string item
 * following the count field.
 */
EXTERNAL const fiftyoneDegreesStoredWeightedStringItem*
fiftyoneDegreesStoredWeightedStringGetFirst(
    const fiftyoneDegreesStoredWeightedString *ws);

/**
 * Advances to the next materialized weighted string item.
 */
EXTERNAL const fiftyoneDegreesStoredWeightedStringItem*
fiftyoneDegreesStoredWeightedStringGetNext(
    const fiftyoneDegreesStoredWeightedStringItem *current);
```

### Step 3: Collection Decorator for Materialization (ip-intelligence-cxx)

This is the core of the implementation. After the `strings` collection is created in `initWithMemory` (line 886) and `readDataSetFromFile` (line 1004), wrap it with a decorator that materializes `WEIGHTED_STRING` values.

**New decorator structure in `ipi.c`:**

```c
typedef struct weighted_string_decorator_state_t {
    fiftyoneDegreesCollection *inner;  /**< The underlying strings collection */
} WeightedStringDecoratorState;
```

**Decorator get method:**

```c
static void* getWithWeightedStringMaterialization(
    const Collection *collection,
    const CollectionKey *key,
    Item *item,
    Exception *exception) {

    WeightedStringDecoratorState *state =
        (WeightedStringDecoratorState *)collection->state;

    // For non-WEIGHTED_STRING keys, delegate directly to inner collection
    if (key->keyType->valueType !=
        FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING) {
        return state->inner->get(state->inner, key, item, exception);
    }

    // Read the raw WEIGHTED_STRING blob from the inner collection
    Item rawItem;
    DataReset(&rawItem.data);
    const StoredBinaryValue *raw = (const StoredBinaryValue *)
        state->inner->get(state->inner, key, &rawItem, exception);
    if (raw == NULL || EXCEPTION_FAILED) {
        return NULL;
    }

    const uint16_t count = raw->weightedStringValue.count;

    // First pass: resolve all strings to calculate total buffer size
    // Second pass: copy string data + weights into materialized buffer
    // (Implementation details: allocate buffer, populate, set item->data.ptr
    //  and item->handle for cleanup on release)

    // For each item in raw->weightedStringValue.items[0..count-1]:
    //   1. Read string from inner collection at items[i].stringOffset
    //   2. Copy string content + weight into materialized buffer
    //   3. Release the temporary string item

    // Release the raw blob
    COLLECTION_RELEASE(state->inner, &rawItem);

    // Set item->data.ptr to materialized buffer
    // Set item->handle to materialized buffer (for free on release)
    // Set item->collection to this decorator collection
    return item->data.ptr;
}
```

**Decorator release method:**

```c
static void releaseWeightedStringDecorator(Item *item) {
    if (item->handle != NULL) {
        Free(item->handle);  // Free the materialized buffer
        DataReset(&item->data);
        item->handle = NULL;
    }
}
```

**Decorator creation (called after strings collection is created):**

```c
static fiftyoneDegreesCollection* createWeightedStringDecorator(
    fiftyoneDegreesCollection *innerStrings) {

    // Allocate the decorator collection + state
    Collection *decorator = (Collection *)Malloc(
        sizeof(Collection) + sizeof(WeightedStringDecoratorState));
    if (decorator == NULL) return NULL;

    WeightedStringDecoratorState *state =
        (WeightedStringDecoratorState *)(decorator + 1);
    state->inner = innerStrings;

    // Copy metadata from inner collection
    decorator->count = innerStrings->count;
    decorator->elementSize = innerStrings->elementSize;
    decorator->size = innerStrings->size;
    decorator->state = state;
    decorator->typeName = "CollectionWeightedStringDecorator";

    // Set decorator methods
    decorator->get = getWithWeightedStringMaterialization;
    decorator->release = releaseWeightedStringDecorator;
    decorator->freeCollection = freeWeightedStringDecorator; // frees inner too

    return decorator;
}
```

**Integration point in `initWithMemory` and `readDataSetFromFile`:**

```c
// After: COLLECTION_CREATE_MEMORY(strings)  (or COLLECTION_CREATE_FILE)
dataSet->strings = createWeightedStringDecorator(dataSet->strings);
if (dataSet->strings == NULL) {
    return INSUFFICIENT_MEMORY;
}
```

**Key design points:**

- The decorator is transparent to all callers. `StoredBinaryValueGet` calls `collection->get` which goes through the decorator. Non-WEIGHTED_STRING requests pass through unchanged.
- For memory-based collections, the inner `get` returns a pointer into mapped memory. The decorator allocates a new buffer for the materialized form. This is the only case where memory-mode strings collection items require allocation.
- For file/cache-based collections, the inner `get` already allocates memory. The decorator replaces that allocation with its own materialized buffer and releases the inner allocation.
- The decorator's `release` frees the materialized buffer. The inner collection's items (used temporarily for string resolution) are released immediately after copying.

### Step 4: Rename ProfilePercentage / IpiList (ip-intelligence-cxx)

Rename types as described in the [Type Renames](#type-renames) section above. This is a preparatory refactor with no behavioral changes.

### Step 5: C++ ResultsIpi Accessor for Stored WEIGHTED_STRING (ip-intelligence-cxx)

**`ResultsIpi.cpp`** — The `iterateWeightedValues` helper (line 370) iterates `WeightedItem` entries and passes each `StoredBinaryValue*` + `storedValueType` + `rawWeighting` to a callback. When `storedValueType == WEIGHTED_STRING`, the `StoredBinaryValue*` now points to a materialized `StoredWeightedString`.

The C++ accessors need to handle this compound structure:

**In `getValuesAsWeightedStringList`:** When `storedValueType == WEIGHTED_STRING`, instead of treating the `StoredBinaryValue` as a single string, iterate the materialized `StoredWeightedString` items:

```cpp
if (storedValueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING) {
    const StoredWeightedString *ws = &binaryValue->weightedStringValue;
    const StoredWeightedStringItem *wsItem =
        StoredWeightedStringGetFirst(ws);
    for (uint16_t j = 0; j < ws->count; j++) {
        WeightedValue<string> wv;
        wv.setValue(string(&wsItem->stringValue, wsItem->stringSize - 1));
        // Combine profile-level weight with sub-value weight
        uint16_t combinedWeight = (uint16_t)(
            ((uint32_t)rawWeighting * (uint32_t)wsItem->weight) / 65535);
        wv.setRawWeight(combinedWeight);
        values.push_back(wv);
        wsItem = StoredWeightedStringGetNext(wsItem);
    }
}
```

**Weight combination:** When a WEIGHTED_STRING property is in a profile that is part of a profile group, the profile-level weight (`rawWeighting` from the `WeightedItem`) is combined multiplicatively with each sub-value weight:

```
combinedWeight = profileWeight * subWeight / 65535
```

When the profile is singular (not part of a group), `profileWeight == 65535` (full weight), so `combinedWeight == subWeight` — the sub-weights pass through unchanged.

**In `getValuesInternal` (string formatting):** When `storedValueType == WEIGHTED_STRING`, format the compound value as a pipe-delimited list of weighted strings:

```
"value1:0.6|value2:0.3|value3:0.1"
```

Or delegate to the `StringBuilderAddStringValue` function if a C-level formatter is added (see Step 6).

**In `getValuesAsWeightedBoolList`, `getValuesAsWeightedIntegerList`, etc.:** These should skip or return an error for `WEIGHTED_STRING` stored type, since the sub-values are strings, not bools/ints.

### Step 6: String Builder Support (common-cxx)

**`storedBinaryValue.c` / `string_pp.cpp`** — Add handling in `StringBuilderAddStringValue` for `WEIGHTED_STRING`:

```c
case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING: {
    const StoredWeightedString *ws = &value->weightedStringValue;
    const StoredWeightedStringItem *item =
        StoredWeightedStringGetFirst(ws);
    for (uint16_t i = 0; i < ws->count; i++) {
        if (i > 0) StringBuilderAddChar(builder, '|');
        StringBuilderAddChars(builder, &item->stringValue, item->stringSize - 1);
        StringBuilderAddChar(builder, ':');
        StringBuilderAddFloat(builder,
            (float)item->weight / 65535.0f, decimalPlaces);
        item = StoredWeightedStringGetNext(item);
    }
    break;
}
```

This enables the C-level `getValueAsString` to return a human-readable representation of stored weighted strings without going through the C++ layer.

### Step 7: Debug/Logging Updates (ip-intelligence-cxx)

**`ipi.c`** — Add `"WeightedString"` case in the storedValueType name switch (~line 783):

```c
case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING: {
    propTypeText = "WeightedString";
    break;
}
```

### Step 8: SWIG Regeneration (ip-intelligence-cxx)

Run `RebuildSwig` with cmake to regenerate SWIG wrappers for both C# and Java. The existing SWIG templates already export:

```swig
%template(WeightedStringListSwig) std::vector<WeightedValue<std::string>>;
```

No changes to the `.i` file are needed — the C++ `getValuesAsWeightedStringList()` returns `vector<WeightedValue<string>>` regardless of whether the weighted strings came from profile group aggregation or from a stored WEIGHTED_STRING value.

### Step 9: .NET Layer (ip-intelligence-dotnet)

**Repository:** `ip-intelligence-dotnet`

1. **Update submodule pointer** to the new `ip-intelligence-cxx`.
2. **Copy regenerated SWIG output** — The SWIG-generated C# interop code in `FiftyOne.IpIntelligence.Engine.OnPremise` will be updated automatically from Step 8.
3. **No changes to managed wrappers** — The existing wrapper classes (`WeightedStringListSwigWrapper.cs`, `WeightedBoolListSwigWrapper.cs`, etc.) in `Wrappers/` consume SWIG types that are unchanged at the C++ template level.
4. **No changes to `IpDataOnPremise.cs`** — `GetValuesAsWeightedStringList()` already calls through to SWIG correctly.
5. **Rebuild native binaries** — Place rebuilt native libraries in `runtimes/{win-x64|linux-x64|osx-x64|osx-arm64}/native/`.
6. **Version bump** — Bump version in `.csproj` files for `FiftyOne.IpIntelligence.Engine.OnPremise` and dependent packages.

### Step 10: Java Layer (ip-intelligence-java)

**Repository:** `ip-intelligence-java`

1. **Update submodule pointer** to the new `ip-intelligence-cxx`.
2. **Copy regenerated SWIG output** — Java SWIG output in `ip-intelligence.engine.on-premise/src/main/java/fiftyone/ipintelligence/engine/onpremise/interop/swig/`.
3. **No changes to data layer** — `IPIntelligenceDataHashDefault.java` already handles `String` type via `results.getValuesAsWeightedStringList(propertyName)` and `populateWeightedValues()`.
4. **Rebuild JNI native library** — Bundle updated `.so`/`.dylib`/`.dll`.
5. **Version bump** — Update version in `pom.xml` (parent and `ip-intelligence.engine.on-premise` module).

### Step 11: Examples Updates

**ip-intelligence-dotnet-examples:**

```csharp
// Example: Reading a property with WEIGHTED_STRING storedValueType
var values = results.GetValuesAsWeightedStringList("TranslatedPropertyName");
if (values.HasValue)
{
    foreach (var wv in values.Value)
    {
        Console.WriteLine($"  {wv.Value}: {wv.Weighting():P1}");
    }
}
```

**ip-intelligence-java-examples:**

```java
// Example: Reading a property with WEIGHTED_STRING storedValueType
AspectPropertyValue<List<IWeightedValue<String>>> values =
    data.getValuesAsWeightedStringList("TranslatedPropertyName");
if (values.hasValue()) {
    for (IWeightedValue<String> wv : values.getValue()) {
        System.out.printf("  %s: %.1f%%%n",
            wv.getValue(), wv.getWeighting() * 100);
    }
}
```

## Data Flow Diagram

```
                         DATA FILE
                    ┌──────────────────┐
                    │  strings collection│
                    │  (all stored values│
                    │   including WS)   │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │   DECORATOR      │
  Step 3            │   (collection    │
                    │    getter)       │
                    │                  │
                    │  if WEIGHTED_STRING:
                    │   read raw blob  │
                    │   resolve offsets │◄── inner strings collection
                    │   → materialized │    (resolves each stringOffset)
                    │     buffer       │
                    │  else:           │
                    │   passthrough    │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │ StoredBinaryValue│
  Existing          │     Get()       │
  path              │  (constructs    │
                    │   CollectionKey) │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │  addWeightedValue │
  Step 4            │  (ipi.c callback)│
  (no changes       │                  │
   needed)          │  stores ONE      │
                    │  WeightedItem    │
                    │  with profile wt │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
  Step 5            │  C++ ResultsIpi  │
                    │  accessors       │
                    │                  │
                    │  if WEIGHTED_STRING:
                    │   iterate materialized
                    │   items, produce N│
                    │   WeightedValue<string>
                    │   with combined   │
                    │   weights         │
                    │  else:           │
                    │   existing logic  │
                    └────────┬─────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
     ┌────────▼───┐  ┌──────▼─────┐  ┌────▼────────┐
     │   SWIG     │  │   SWIG     │  │   C API     │
     │   C# / .NET│  │   Java     │  │  getString  │
     └────────────┘  └────────────┘  └─────────────┘
```

**Two axes of weighting:**

```
Profile Groups (profile-level weight):
  Profile A (60%) ──► value "foo" in profile A
  Profile B (40%) ──► value "bar" in profile B
  → WeightedItem list: [("foo", 0.6), ("bar", 0.4)]

Stored WEIGHTED_STRING (sub-value weight):
  Single Profile ──► stored value: [("English", 0.7), ("French", 0.3)]
  → ONE WeightedItem containing the compound materialized value
  → C++ accessor expands to: [("English", 0.7), ("French", 0.3)]

Combined (profile group + stored WEIGHTED_STRING):
  Profile A (60%) ──► stored: [("English", 0.7), ("French", 0.3)]
  Profile B (40%) ──► stored: [("Spanish", 1.0)]
  → WeightedItem list: [(materialized_A, 0.6), (materialized_B, 0.4)]
  → C++ accessor expands to:
       ("English",  0.6 * 0.7 = 0.42)
       ("French",   0.6 * 0.3 = 0.18)
       ("Spanish",  0.4 * 1.0 = 0.40)
```

## Package Cascade

```
common-cxx (Steps 1–2: CollectionKeyType + structs)
  └─► ip-intelligence-cxx (Steps 3–8: decorator, rename, C++ accessors, SWIG)
       ├─► ip-intelligence-dotnet (Step 9: submodule, SWIG copy, native rebuild, version bump)
       │    └─► ip-intelligence-dotnet-examples (Step 11)
       └─► ip-intelligence-java (Step 10: submodule, SWIG copy, native rebuild, version bump)
            └─► ip-intelligence-java-examples (Step 11)
```

## Memory Management

### Decorator Materialization Buffer

- The decorator allocates a buffer for the materialized form of each `WEIGHTED_STRING` value. This buffer contains the `count` field followed by variable-length `StoredWeightedStringItem` entries (each with weight, string size, and inline string data).
- `item->handle` is set to the allocated buffer, and `item->collection` is set to the decorator collection.
- The decorator's `release` method frees the buffer via `Free(item->handle)`.
- Temporary `CollectionItem` instances used to resolve individual string offsets from the inner collection are released immediately after the string content is copied into the materialized buffer.

### Memory-Mode Considerations

- For memory-based collections, the inner `getMemoryVariable` returns a pointer into mapped memory (no allocation). The decorator is the only code path that allocates memory for strings collection items in memory mode. This is acceptable because WEIGHTED_STRING items are expected to be infrequent relative to total string lookups.

### WeightedItem List Cleanup

- `releaseWeightedItemList` (formerly `releaseIpiList`) calls `COLLECTION_RELEASE` on each item's collection. For items whose stored value was a WEIGHTED_STRING, this calls the decorator's release, which frees the materialized buffer. For all other items, it calls through to the inner collection's release (which is a no-op for memory mode, or frees allocated memory for file mode).

## Thread Safety

- The decorator's `get` method is stateless (the `WeightedStringDecoratorState` only holds a pointer to the inner collection). All temporary state is stack-local.
- Each sub-string resolution via the inner collection's `get` uses its own stack-local `CollectionItem`, so concurrent access is safe.
- The inner `strings` collection is designed for concurrent reads across all collection modes (memory, file, cache).

## Backward Compatibility

- Existing data files without `WEIGHTED_STRING` stored values are unaffected — the decorator passes through all non-WEIGHTED_STRING requests unchanged.
- The `CollectionKeyType_Unsupported` default case still catches any truly unknown types.
- The type renames (`ProfilePercentage` → `WeightedItem`, `IpiList` → `WeightedItemList`) are source-breaking at the C level, but these types are internal to the IPI engine and not exposed to end users of .NET/Java packages. SWIG regeneration absorbs the rename.
- The C++ accessor `getValuesAsWeightedStringList` returns the same `vector<WeightedValue<string>>` type regardless of whether the strings came from profile-group aggregation or stored WEIGHTED_STRING. Consumers see no API change.

## Related PRs / Issues

- **common-cxx issue:** "Support WeightedValues derived from a WeightedPropertyValue in a Singular profile" (to be updated with these details)
- **common-metadata PR:** "Update Countries Metadata + Add new Translated properties" (adds properties that use `WEIGHTED_STRING` as `storedValueType`)

## Testing

- Unit tests in `ip-intelligence-cxx` for the decorator: verify materialization produces correct string content and weights for known test data
- Test iteration helpers (`GetFirst`, `GetNext`) on materialized buffers of varying sizes (0, 1, many items)
- Test weight combination: profile weight * sub-weight / 65535 with edge cases (both weights = 65535, one = 0, etc.)
- Verify `getValueAsString` (C-level) returns correct pipe-delimited weighted string format
- Verify `getValuesAsWeightedStringList` (C++ level) returns correct vector with combined weights
- Verify memory cleanup (no leaks) for all collection modes (memory, file, cache) using the decorator
- Integration tests in .NET and Java using an enterprise `.ipi` file containing WEIGHTED_STRING properties
- Verify end-to-end: query a translated property → get weighted string list → values and weights match expected data

## Files Modified (Summary)

| Repository | File | Change |
|---|---|---|
| common-cxx | `collectionKeyTypes.h` | Add `CollectionKeyType_WeightedString` + size function |
| common-cxx | `collectionKeyTypes.c` | Add `WEIGHTED_STRING` case in switch |
| common-cxx | `storedBinaryValue.h` | Add raw + materialized structs, iteration helpers, add to union |
| common-cxx | `storedBinaryValue.c` | Implement iteration helpers |
| common-cxx | `string_pp.cpp` or `storedBinaryValue.c` | Add `WEIGHTED_STRING` case in `StringBuilderAddStringValue` |
| ip-intelligence-cxx | `ipi.h` | Rename `ProfilePercentage` → `WeightedItem`, `IpiList` → `WeightedItemList` |
| ip-intelligence-cxx | `ipi.c` | Rename internals; add decorator creation + get/release/free methods; integrate decorator after strings collection init |
| ip-intelligence-cxx | `ipi.c` (debug) | Add `"WeightedString"` in type name switch |
| ip-intelligence-cxx | `ResultsIpi.cpp` | Handle `WEIGHTED_STRING` in accessors: iterate materialized items, combine weights |
| ip-intelligence-cxx | `ipi_weighted_results.c` | Rename `ProfilePercentage` references |
| ip-intelligence-cxx | `fiftyone.h` | Update `MAP_TYPE` macro |
| ip-intelligence-cxx | SWIG output | Regenerate via `RebuildSwig` |
| ip-intelligence-dotnet | Submodule + SWIG + native binaries | Update, rebuild, version bump |
| ip-intelligence-java | Submodule + SWIG + native binaries | Update, rebuild, version bump |
| ip-intelligence-dotnet-examples | Example files | Add/update WEIGHTED_STRING property example |
| ip-intelligence-java-examples | Example files | Add/update WEIGHTED_STRING property example |
