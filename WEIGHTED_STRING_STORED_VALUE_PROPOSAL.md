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

A `WEIGHTED_STRING` stored value breaks this invariant: it is a byte array containing `uint32_t` offsets that point to other strings within the same `strings` collection. These sub-references must be **eagerly resolved** (loaded as individual collection items) when the value is read from the collection, using a zero-copy approach that avoids duplicating string data in memory mode.

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

### Decision 1: Zero-Copy Resolution in the Collection Getter (Decorator)

The `strings` collection's `get` method is decorated so that when the key type is `WEIGHTED_STRING`, the raw blob (containing offsets + weights) is loaded, and each string offset is resolved against the same underlying `strings` collection. The returned item contains a `StoredListOfStrings` envelope holding references to the raw weights item and an array of individually-loaded `String` items — **no string data is copied**. In memory mode, all pointers go directly into memory-mapped data. This is purely a data-reading concern and is transparent to all consumers.

**Rationale:** Resolution is a collection-level responsibility. Callers of `StoredBinaryValueGet` should not need to know that the stored data contains sub-references. The decorator resolves them before the data leaves the collection layer. The zero-copy design avoids unnecessary allocations in memory mode — the only overhead is the small envelope struct and the pointer array.

### Decision 2: Single Compound Value in Results

The `addWeightedValue` callback (the renamed `addValueWithPercentage`) treats a stored WEIGHTED_STRING as a single value — one `WeightedItem` entry in the results list with one profile-level weight. The compound nature (multiple strings with sub-weights) is interpreted by the C++ accessor layer (`getValuesAsWeightedStringList` and friends). This keeps the two axes of weighting — profile-level (from profile groups) and value-level (from stored data) — cleanly separated.

**Rationale:** The stored weighted string is a single property value within a single profile. It should not be expanded into multiple profile-level result items. The sub-weights are an internal structure of the value, not a result of multi-profile matching.

### Decision 3: Rename Profile-Specific Types to Generic Names and Lift to common-cxx

The existing `ProfilePercentage` and `IpiList` types were originally named for profile-level weighting from profile groups. These are generic containers for weighted collection items and should be named accordingly. Since they are general-purpose data structures (a collection item + a weight, and a resizable list of those), they belong in **common-cxx**, not ip-intelligence-cxx. This also enables the collection decorator (which lives in common-cxx) to work with these types directly.

### Decision 4: Collection Decorator Lives in common-cxx

The zero-copy `WEIGHTED_STRING` resolution decorator is a **collection-level** concern — it wraps a `Collection` and intercepts `get` calls based on `CollectionKeyType`. It has no dependency on IPI-specific types. Moving it to `collection.c`/`collection.h` in common-cxx keeps the collection abstraction self-contained. ip-intelligence-cxx merely calls the creation function after initializing its strings collection.

## Type Renames

| Current Name | New Name | New Location | Scope |
|---|---|---|---|
| `fiftyoneDegreesProfilePercentage` | `fiftyoneDegreesWeightedItem` | **common-cxx** (`collection.h` or new `weightedItem.h`) | Public API |
| `fiftyone_degrees_profile_percentage_t` | `fiftyone_degrees_weighted_item_t` | common-cxx | Struct tag |
| `fiftyoneDegreesIpiList` | `fiftyoneDegreesWeightedItemList` | **common-cxx** (`collection.h` or new `weightedItem.h`) | Public API |
| `fiftyone_degrees_ipi_list_t` | `fiftyone_degrees_weighted_item_list_t` | common-cxx | Struct tag |
| `addValueWithPercentage` | `addWeightedValue` | ip-intelligence-cxx (stays in ipi.c) | Static in ipi.c |
| `stateWithPercentage` | `stateWithWeighting` | ip-intelligence-cxx (stays in ipi.c) | Static in ipi.c |
| `profilePercentageItem` (local vars) | `weightedItem` | ip-intelligence-cxx | Local variables |
| `ProfilePercentage` (MAP_TYPE) | `WeightedItem` | ip-intelligence-cxx (fiftyone.h) | MAP_TYPE macro |
| `ResultsIpiGetValues` return type | `const fiftyoneDegreesWeightedItem*` | ip-intelligence-cxx | Public API |

**`rawWeighting`** is kept as-is — it is already generic enough.

Updated struct definition (now in common-cxx):

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

/**
 * A resizable list of weighted items.
 */
typedef struct fiftyone_degrees_weighted_item_list_t {
    fiftyoneDegreesWeightedItem *items;  /**< Array of weighted items */
    uint32_t count;                       /**< Number of items in use */
    uint32_t capacity;                    /**< Allocated capacity */
} fiftyoneDegreesWeightedItemList;
```

**Files affected by rename:**
- **common-cxx** (new home):
  - `collection.h` or new `weightedItem.h` — type definitions for `WeightedItem` and `WeightedItemList`
  - `collection.c` or new `weightedItem.c` — list management functions (init, release, extend, add)
- **ip-intelligence-cxx** (consumer, update references):
  - `ipi.h` — remove old type definitions, include new common-cxx header, update `FIFTYONE_DEGREES_RESULTS_IPI_MEMBERS` macro
  - `ipi.c` — update all internal usage (~20 references), rename static functions
  - `ResultsIpi.cpp` — update C++ layer (~10 references)
  - `ipi_weighted_results.c` — update weighted value construction (~3 references)
  - `fiftyone.h` — update `MAP_TYPE` macro
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

**`storedBinaryValue.h`** — Add packed structs for the on-disk (raw) format and the zero-copy resolved format.

**On-disk format (raw, as stored in the data file):**

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

**Resolved format (after decorator resolves offsets — zero-copy):**

Instead of copying string data into a materialized buffer, the decorator returns a `StoredListOfStrings` that holds references to individually-loaded `CollectionItem`s. In memory mode, the string items are just pointers into mapped memory — no string data is copied. The only allocation is the `StoredListOfStrings` struct itself plus the `stringItems` array.

```c
#pragma pack(push, 1)
typedef struct fiftyone_degrees_stored_list_of_strings_t {
    fiftyoneDegreesCollectionItem weightingsItem;  /**< Loaded StoredWeightedStringRaw item
                                                        (raw offsets + weights from the
                                                        inner strings collection) */
    fiftyoneDegreesCollectionItem *stringItems;    /**< Array of count loaded String items,
                                                        each resolved from the corresponding
                                                        stringOffset in weightingsItem */
} fiftyoneDegreesStoredListOfStrings;
#pragma pack(pop)
```

The `StoredWeightedStringRaw` struct is **not** added to the `StoredBinaryValue` union — it is only used internally by the decorator to interpret the raw on-disk data. The decorator's output (`StoredListOfStrings`) is what callers see via `item->data.ptr`.

**Usage pattern (by C++ accessor or any consumer):**

```c
CollectionItem topResult;
DataReset(&topResult.data);

CollectionKey weightedValueKey = {
    weightedValueOffset,
    CollectionKeyType_WeightedString,
};

decoratedCollection->get(
    decoratedCollection,
    &weightedValueKey,
    &topResult,
    exception);

StoredListOfStrings * const listOfStrings = (StoredListOfStrings *)topResult.data.ptr;
StoredWeightedStringRaw * const rawWeights =
    (StoredWeightedStringRaw *)listOfStrings->weightingsItem.data.ptr;

// Iterate resolved strings + weights — no copies, just pointer chasing
for (uint16_t i = 0; i < rawWeights->count; i++) {
    String * const nextString =
        (String *)listOfStrings->stringItems[i].data.ptr;
    uint16_t const nextWeight = rawWeights->items[i].weight;
    // use nextString->value and nextWeight ...
}

// Release when done — triggers decorator's custom release
COLLECTION_RELEASE(decoratedCollection, &topResult);
```

### Step 3: Collection Decorator for Zero-Copy Resolution (common-cxx)

This is the core of the implementation. The decorator lives in **common-cxx** (`collection.c` / `collection.h`) since it is a general-purpose collection wrapper with no IPI-specific dependencies. It intercepts `get` calls for `WEIGHTED_STRING` key types and resolves string offsets into individually-loaded `CollectionItem`s.

**New public API in `collection.h`:**

```c
/**
 * Creates a decorator collection that wraps the given inner collection
 * and intercepts get calls for WEIGHTED_STRING key types. For those
 * keys, the decorator loads the raw weighted string blob, resolves
 * each string offset against the inner collection, and returns a
 * StoredListOfStrings envelope via the output item.
 *
 * For all other key types, requests pass through to the inner
 * collection unchanged.
 *
 * @param innerStrings the collection to wrap
 * @return the decorator collection, or NULL on allocation failure.
 *         The caller owns the returned collection and must free it
 *         via collection->freeCollection (which also frees the inner).
 */
EXTERNAL fiftyoneDegreesCollection*
fiftyoneDegreesCollectionCreateWeightedStringDecorator(
    fiftyoneDegreesCollection *innerStrings);
```

**Decorator state (internal to `collection.c`):**

```c
typedef struct weighted_string_decorator_state_t {
    fiftyoneDegreesCollection *inner;  /**< The underlying strings collection */
} WeightedStringDecoratorState;
```

**Decorator get method (in `collection.c`):**

```c
static void* getWithWeightedStringResolution(
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

    // 1. Allocate the StoredListOfStrings envelope
    StoredListOfStrings *list =
        (StoredListOfStrings *)Malloc(sizeof(StoredListOfStrings));
    if (list == NULL) {
        EXCEPTION_SET(INSUFFICIENT_MEMORY);
        return NULL;
    }

    // 2. Load the raw weighted value blob (count + offset/weight pairs)
    //    from the inner collection. In memory mode this is just a pointer
    //    into mapped memory — no allocation.
    DataReset(&list->weightingsItem.data);
    state->inner->get(
        state->inner, key, &list->weightingsItem, exception);
    if (EXCEPTION_FAILED) {
        Free(list);
        return NULL;
    }

    const StoredWeightedStringRaw *raw =
        (const StoredWeightedStringRaw *)list->weightingsItem.data.ptr;
    const uint16_t count = raw->count;

    // 3. Allocate the array of CollectionItems for resolved strings
    list->stringItems = (CollectionItem *)Malloc(
        sizeof(CollectionItem) * count);
    if (list->stringItems == NULL && count > 0) {
        COLLECTION_RELEASE(state->inner, &list->weightingsItem);
        Free(list);
        EXCEPTION_SET(INSUFFICIENT_MEMORY);
        return NULL;
    }

    // 4. Resolve each string offset via the inner collection.
    //    In memory mode, each get returns a pointer into mapped memory.
    //    No string data is copied.
    for (uint16_t i = 0; i < count; i++) {
        DataReset(&list->stringItems[i].data);
        CollectionKey strKey = {
            raw->items[i].stringOffset,
            CollectionKeyType_String
        };
        state->inner->get(
            state->inner, &strKey, &list->stringItems[i], exception);
        if (EXCEPTION_FAILED) {
            // Release already-resolved items on failure
            for (uint16_t j = 0; j < i; j++) {
                COLLECTION_RELEASE(state->inner, &list->stringItems[j]);
            }
            COLLECTION_RELEASE(state->inner, &list->weightingsItem);
            Free(list->stringItems);
            Free(list);
            return NULL;
        }
    }

    // 5. Set the output item to point to the StoredListOfStrings.
    //    The collection pointer is set to THIS decorator so that
    //    COLLECTION_RELEASE dispatches to our custom release method.
    item->data.ptr = (byte *)list;
    item->data.used = sizeof(StoredListOfStrings);
    item->data.allocated = 0;  // we manage memory ourselves
    item->handle = list;       // used by release to identify our items
    item->collection = collection;  // points to decorator, NOT inner

    return item->data.ptr;
}
```

**Decorator release method (in `collection.c`):**

The release must unwind in reverse order: release each resolved string item, then release the raw weightings item, then free the allocated arrays and struct.

```c
static void releaseWeightedStringDecorator(Item *item) {
    if (item->handle == NULL) return;

    StoredListOfStrings *list = (StoredListOfStrings *)item->handle;

    // Get the inner collection from one of the loaded items
    // (the weightingsItem knows which collection it came from)
    const Collection *inner = list->weightingsItem.collection;

    // Get count from the raw data before releasing it
    const StoredWeightedStringRaw *raw =
        (const StoredWeightedStringRaw *)list->weightingsItem.data.ptr;
    const uint16_t count = raw->count;

    // Release each resolved string item back to the inner collection
    for (uint16_t i = 0; i < count; i++) {
        COLLECTION_RELEASE(inner, &list->stringItems[i]);
    }

    // Release the raw weightings item back to the inner collection
    COLLECTION_RELEASE(inner, &list->weightingsItem);

    // Free the allocated envelope
    Free(list->stringItems);
    Free(list);

    // Reset the output item
    DataReset(&item->data);
    item->handle = NULL;
}
```

**Decorator free method (in `collection.c`):**

```c
static void freeWeightedStringDecorator(Collection *collection) {
    WeightedStringDecoratorState *state =
        (WeightedStringDecoratorState *)collection->state;
    // Free the inner collection first
    state->inner->freeCollection(state->inner);
    // Then free the decorator itself (state is allocated inline after it)
    Free(collection);
}
```

**Public creation function (in `collection.c`):**

```c
fiftyoneDegreesCollection*
fiftyoneDegreesCollectionCreateWeightedStringDecorator(
    fiftyoneDegreesCollection *innerStrings) {

    // Allocate the decorator collection + state in one block
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

    // Set decorator methods
    decorator->get = getWithWeightedStringResolution;
    decorator->release = releaseWeightedStringDecorator;
    decorator->freeCollection = freeWeightedStringDecorator;

    return decorator;
}
```

**Key design points:**

- **Zero-copy in memory mode:** The inner collection's `getMemoryVariable` returns pointers into mapped memory (no allocation). The decorator does NOT copy any string data. Each `stringItems[i].data.ptr` points directly into the memory-mapped region. The raw `weightingsItem.data.ptr` also points into mapped memory. The only allocations are the `StoredListOfStrings` struct (fixed size) and the `stringItems` array (`count * sizeof(CollectionItem)`).
- **File/cache mode:** The inner collection allocates memory for each loaded item as usual. The decorator merely holds references to those allocations. On release, each item is released back to the inner collection which handles deallocation.
- **Transparent to all callers:** `StoredBinaryValueGet` calls `collection->get` which goes through the decorator. Non-WEIGHTED_STRING requests pass through unchanged.
- **Custom release dispatch:** `item->collection` is set to the decorator collection, so `COLLECTION_RELEASE(c, &item)` calls the decorator's release. The decorator then releases each sub-item back to the inner collection.
- **Lifetime contract:** The `StoredListOfStrings` and all its sub-items remain valid until `COLLECTION_RELEASE` is called on the top-level item. Consumers must not hold pointers to individual strings beyond that point.
- **No IPI dependencies:** The decorator only depends on common-cxx types (`Collection`, `CollectionItem`, `CollectionKey`, `CollectionKeyType`, `StoredWeightedStringRaw`, `StoredListOfStrings`). It can be reused by any product that needs `WEIGHTED_STRING` support, not just IPI.

### Step 4: Lift WeightedItem / WeightedItemList Types to common-cxx

Move the `WeightedItem` and `WeightedItemList` type definitions and their list management functions (`init`, `release`, `extend`, `add`) from `ipi.h`/`ipi.c` to common-cxx. These are generic data structures (a `CollectionItem` + weight, and a resizable array of those) with no IPI-specific logic.

**New header in common-cxx** (e.g., `weightedItem.h` or added to `collection.h`):

```c
#include "collection.h"

typedef struct fiftyone_degrees_weighted_item_t {
    fiftyoneDegreesCollectionItem item;
    uint16_t rawWeighting;
} fiftyoneDegreesWeightedItem;

typedef struct fiftyone_degrees_weighted_item_list_t {
    fiftyoneDegreesWeightedItem *items;
    uint32_t count;
    uint32_t capacity;
} fiftyoneDegreesWeightedItemList;

EXTERNAL void fiftyoneDegreesWeightedItemListInit(
    fiftyoneDegreesWeightedItemList *list,
    uint32_t initialCapacity);

EXTERNAL void fiftyoneDegreesWeightedItemListRelease(
    fiftyoneDegreesWeightedItemList *list);

EXTERNAL fiftyoneDegreesStatusCode fiftyoneDegreesWeightedItemListExtend(
    fiftyoneDegreesWeightedItemList *list);

EXTERNAL fiftyoneDegreesWeightedItem* fiftyoneDegreesWeightedItemListAdd(
    fiftyoneDegreesWeightedItemList *list,
    fiftyoneDegreesException *exception);
```

### Step 5: Update ip-intelligence-cxx to Use common-cxx Types

**`ipi.h`:** Remove old `ProfilePercentage` and `IpiList` definitions. Include the new common-cxx header. Update `FIFTYONE_DEGREES_RESULTS_IPI_MEMBERS` macro and `ResultsIpiGetValues` to use `WeightedItem` / `WeightedItemList`.

**`ipi.c`:** Rename all internal references (~20 occurrences). Rename static functions (`addValueWithPercentage` → `addWeightedValue`, `stateWithPercentage` → `stateWithWeighting`). Replace calls to old list functions with new common-cxx equivalents.

### Step 6: Integrate Decorator in ip-intelligence-cxx

**`ipi.c`** — After strings collection is created, call the common-cxx decorator creation function:

```c
// After: COLLECTION_CREATE_MEMORY(strings)  (or COLLECTION_CREATE_FILE)
dataSet->strings =
    fiftyoneDegreesCollectionCreateWeightedStringDecorator(dataSet->strings);
if (dataSet->strings == NULL) {
    return INSUFFICIENT_MEMORY;
}
```

This is the **only** ip-intelligence-cxx code for the decorator — one function call at each init path.

### Step 7: C++ ResultsIpi Accessor for Stored WEIGHTED_STRING (ip-intelligence-cxx)

**`ResultsIpi.cpp`** — The `iterateWeightedValues` helper (line 370) iterates `WeightedItem` entries and passes each `StoredBinaryValue*` + `storedValueType` + `rawWeighting` to a callback. When `storedValueType == WEIGHTED_STRING`, the `item->data.ptr` now points to a `StoredListOfStrings` (not a `StoredBinaryValue`), because the decorator intercepts the get and returns the resolved envelope.

The C++ accessors need to handle this compound structure:

**In `getValuesAsWeightedStringList`:** When `storedValueType == WEIGHTED_STRING`, cast to `StoredListOfStrings` and iterate the resolved string items + raw weights:

```cpp
if (storedValueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING) {
    // The decorator returned a StoredListOfStrings, not a StoredBinaryValue
    const StoredListOfStrings *list =
        (const StoredListOfStrings *)weightedItem->item.data.ptr;
    const StoredWeightedStringRaw *rawWeights =
        (const StoredWeightedStringRaw *)list->weightingsItem.data.ptr;

    for (uint16_t j = 0; j < rawWeights->count; j++) {
        const String *str =
            (const String *)list->stringItems[j].data.ptr;
        uint16_t subWeight = rawWeights->items[j].weight;

        WeightedValue<string> wv;
        wv.setValue(string(&str->value, str->size - 1));

        // Combine profile-level weight with sub-value weight
        uint16_t combinedWeight = (uint16_t)(
            ((uint32_t)weightedItem->rawWeighting * (uint32_t)subWeight)
            / 65535);
        wv.setRawWeight(combinedWeight);
        values.push_back(wv);
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

### Step 8: String Builder Support (common-cxx)

Since `StoredListOfStrings` now lives in common-cxx, the string builder can handle `WEIGHTED_STRING` directly.

**`storedBinaryValue.c`** — Add handling in `StringBuilderAddStringValue` for `WEIGHTED_STRING`:

```c
case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING: {
    // The value pointer is actually a StoredListOfStrings*
    const StoredListOfStrings *list = (const StoredListOfStrings *)value;
    const StoredWeightedStringRaw *raw =
        (const StoredWeightedStringRaw *)list->weightingsItem.data.ptr;
    for (uint16_t i = 0; i < raw->count; i++) {
        if (i > 0) StringBuilderAddChar(builder, '|');
        const String *str =
            (const String *)list->stringItems[i].data.ptr;
        StringBuilderAddChars(builder, &str->value, str->size - 1);
        StringBuilderAddChar(builder, ':');
        StringBuilderAddFloat(builder,
            (float)raw->items[i].weight / 65535.0f, decimalPlaces);
    }
    break;
}
```

This enables the C-level `getValueAsString` to return a human-readable representation of stored weighted strings without going through the C++ layer.

### Step 9: Debug/Logging Updates (ip-intelligence-cxx)

**`ipi.c`** — Add `"WeightedString"` case in the storedValueType name switch (~line 783):

```c
case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WEIGHTED_STRING: {
    propTypeText = "WeightedString";
    break;
}
```

### Step 10: SWIG Regeneration (ip-intelligence-cxx)

Run `RebuildSwig` with cmake to regenerate SWIG wrappers for both C# and Java. The existing SWIG templates already export:

```swig
%template(WeightedStringListSwig) std::vector<WeightedValue<std::string>>;
```

No changes to the `.i` file are needed — the C++ `getValuesAsWeightedStringList()` returns `vector<WeightedValue<string>>` regardless of whether the weighted strings came from profile group aggregation or from a stored WEIGHTED_STRING value.

### Step 11: .NET Layer (ip-intelligence-dotnet)

**Repository:** `ip-intelligence-dotnet`

1. **Update submodule pointer** to the new `ip-intelligence-cxx`.
2. **Copy regenerated SWIG output** — The SWIG-generated C# interop code in `FiftyOne.IpIntelligence.Engine.OnPremise` will be updated automatically from Step 8.
3. **No changes to managed wrappers** — The existing wrapper classes (`WeightedStringListSwigWrapper.cs`, `WeightedBoolListSwigWrapper.cs`, etc.) in `Wrappers/` consume SWIG types that are unchanged at the C++ template level.
4. **No changes to `IpDataOnPremise.cs`** — `GetValuesAsWeightedStringList()` already calls through to SWIG correctly.
5. **Rebuild native binaries** — Place rebuilt native libraries in `runtimes/{win-x64|linux-x64|osx-x64|osx-arm64}/native/`.
6. **Version bump** — Bump version in `.csproj` files for `FiftyOne.IpIntelligence.Engine.OnPremise` and dependent packages.

### Step 12: Java Layer (ip-intelligence-java)

**Repository:** `ip-intelligence-java`

1. **Update submodule pointer** to the new `ip-intelligence-cxx`.
2. **Copy regenerated SWIG output** — Java SWIG output in `ip-intelligence.engine.on-premise/src/main/java/fiftyone/ipintelligence/engine/onpremise/interop/swig/`.
3. **No changes to data layer** — `IPIntelligenceDataHashDefault.java` already handles `String` type via `results.getValuesAsWeightedStringList(propertyName)` and `populateWeightedValues()`.
4. **Rebuild JNI native library** — Bundle updated `.so`/`.dylib`/`.dll`.
5. **Version bump** — Update version in `pom.xml` (parent and `ip-intelligence.engine.on-premise` module).

### Step 13: Examples Updates

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
  Step 3 (common)   │   (collection    │
                    │    getter)       │
                    │                  │
                    │  if WEIGHTED_STRING:
                    │   load raw blob  │
                    │   → weightingsItem│
                    │   resolve offsets │◄── inner strings collection
                    │   → stringItems[]│    (resolves each stringOffset)
                    │   return         │
                    │   StoredListOf   │    Zero-copy in memory mode:
                    │   Strings        │    all pointers into mapped
                    │  else:           │    memory, no string copies
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
  Step 6            │  (ipi.c callback)│
  (no changes       │                  │
   needed)          │  stores ONE      │
                    │  WeightedItem    │
                    │  with profile wt │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
  Step 7            │  C++ ResultsIpi  │
                    │  accessors       │
                    │                  │
                    │  if WEIGHTED_STRING:
                    │   cast to StoredList│
                    │   OfStrings, iterate│
                    │   stringItems[] +  │
                    │   rawWeights,      │
                    │   produce N        │
                    │   WeightedValue<   │
                    │   string> with     │
                    │   combined weights │
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
common-cxx (Steps 1–4, 8: CollectionKeyType, structs, decorator, WeightedItem types, string builder)
  └─► ip-intelligence-cxx (Steps 5–7, 9–10: consume types, integrate decorator, C++ accessors, SWIG)
       ├─► ip-intelligence-dotnet (Step 11: submodule, SWIG copy, native rebuild, version bump)
       │    └─► ip-intelligence-dotnet-examples (Step 13)
       └─► ip-intelligence-java (Step 12: submodule, SWIG copy, native rebuild, version bump)
            └─► ip-intelligence-java-examples (Step 13)
```

## Memory Management

### Zero-Copy Decorator Approach

The decorator allocates only the **envelope** — not the string data:

| Allocation | Size | Lifetime |
|---|---|---|
| `StoredListOfStrings` struct | `sizeof(StoredListOfStrings)` = fixed (~40 bytes) | Until `COLLECTION_RELEASE` on the top-level item |
| `stringItems` array | `count * sizeof(CollectionItem)` | Same as above |
| String data (memory mode) | **0 — zero-copy** | Backed by memory-mapped region |
| String data (file mode) | Allocated by inner collection's `getFile` | Released back to inner collection by decorator's release |

- `item->handle` is set to the `StoredListOfStrings*`, and `item->collection` is set to the decorator collection.
- The decorator's `release` method: releases each `stringItems[i]` and `weightingsItem` back to the inner collection, then frees `stringItems` array and the `StoredListOfStrings` struct.

### Memory-Mode Considerations

- In memory mode, the inner collection's `get` returns pointers into memory-mapped data with no allocation. The resolved `stringItems[i].data.ptr` and `weightingsItem.data.ptr` are just pointers into the mapped region.
- The **only** allocations in memory mode are the `StoredListOfStrings` struct and the `stringItems` array — both small, fixed-overhead structures.
- In memory mode, releasing the inner items (`COLLECTION_RELEASE` on each `stringItems[i]` and `weightingsItem`) is a no-op since memory-mode items don't allocate.
- This is a significant improvement over the materialization approach, which would have copied all string content into a new buffer even in memory mode.

### File/Cache Mode Considerations

- In file mode, the inner collection's `get` allocates memory for each loaded item. The decorator holds references to these allocations.
- On release, `COLLECTION_RELEASE` on each sub-item returns the allocated memory to the inner collection (file mode frees it, cache mode returns it to the LRU cache).

### WeightedItem List Cleanup

- `releaseWeightedItemList` (formerly `releaseIpiList`) calls `COLLECTION_RELEASE` on each item's collection. For items whose stored value was a WEIGHTED_STRING, `item->collection` points to the decorator, so `COLLECTION_RELEASE` dispatches to the decorator's custom release method. The decorator then cascades release to all sub-items. For all other items, release goes directly to the inner collection as before.

## Thread Safety

- The decorator's `get` method is stateless (the `WeightedStringDecoratorState` only holds a pointer to the inner collection). All temporary state is stack-local.
- Each sub-string resolution via the inner collection's `get` uses its own stack-local `CollectionItem`, so concurrent access is safe.
- The inner `strings` collection is designed for concurrent reads across all collection modes (memory, file, cache).

## Backward Compatibility

- Existing data files without `WEIGHTED_STRING` stored values are unaffected — the decorator passes through all non-WEIGHTED_STRING requests unchanged.
- The `CollectionKeyType_Unsupported` default case still catches any truly unknown types.
- The type renames (`ProfilePercentage` → `WeightedItem`, `IpiList` → `WeightedItemList`) are source-breaking at the C level and the types move from ip-intelligence-cxx to common-cxx. However, these types are internal to the IPI engine and not exposed to end users of .NET/Java packages. SWIG regeneration absorbs the rename. Other products consuming common-cxx gain access to these generic types for future use.
- The C++ accessor `getValuesAsWeightedStringList` returns the same `vector<WeightedValue<string>>` type regardless of whether the strings came from profile-group aggregation or stored WEIGHTED_STRING. Consumers see no API change.

## Related PRs / Issues

- **common-cxx issue:** "Support WeightedValues derived from a WeightedPropertyValue in a Singular profile" (to be updated with these details)
- **common-metadata PR:** "Update Countries Metadata + Add new Translated properties" (adds properties that use `WEIGHTED_STRING` as `storedValueType`)

## Testing

- Unit tests in `ip-intelligence-cxx` for the decorator: verify `StoredListOfStrings` contains correct string pointers and weights for known test data
- Test the decorator with varying counts (0, 1, many items) and verify all `stringItems[i].data.ptr` point to valid `String` structs
- Test weight combination: profile weight * sub-weight / 65535 with edge cases (both weights = 65535, one = 0, etc.)
- Verify `getValueAsString` (C-level) returns correct pipe-delimited weighted string format
- Verify `getValuesAsWeightedStringList` (C++ level) returns correct vector with combined weights
- Verify memory cleanup (no leaks) for all collection modes (memory, file, cache) — ensure decorator release cascades to all sub-items and frees the envelope
- Integration tests in .NET and Java using an enterprise `.ipi` file containing WEIGHTED_STRING properties
- Verify end-to-end: query a translated property → get weighted string list → values and weights match expected data

## Files Modified (Summary)

| Repository | File | Change |
|---|---|---|
| **common-cxx** | `collectionKeyTypes.h` | Add `CollectionKeyType_WeightedString` + size function |
| **common-cxx** | `collectionKeyTypes.c` | Add `WEIGHTED_STRING` case in switch |
| **common-cxx** | `storedBinaryValue.h` | Add `StoredWeightedStringRaw` (on-disk format) and `StoredListOfStrings` (zero-copy resolved format) structs |
| **common-cxx** | `collection.h` | Add `fiftyoneDegreesCollectionCreateWeightedStringDecorator` declaration |
| **common-cxx** | `collection.c` | Add decorator implementation (get/release/free/create) |
| **common-cxx** | `weightedItem.h` (new) or `collection.h` | Add `WeightedItem` and `WeightedItemList` type definitions |
| **common-cxx** | `weightedItem.c` (new) or `collection.c` | Add list management functions (init, release, extend, add) |
| **common-cxx** | `storedBinaryValue.c` | Add `WEIGHTED_STRING` case in `StringBuilderAddStringValue` |
| ip-intelligence-cxx | `ipi.h` | Remove old type defs, include common-cxx header, update macros/signatures |
| ip-intelligence-cxx | `ipi.c` | Rename internals; call `CollectionCreateWeightedStringDecorator` after strings init |
| ip-intelligence-cxx | `ipi.c` (debug) | Add `"WeightedString"` in type name switch |
| ip-intelligence-cxx | `ResultsIpi.cpp` | Handle `WEIGHTED_STRING` in accessors: iterate `StoredListOfStrings`, combine weights |
| ip-intelligence-cxx | `ipi_weighted_results.c` | Update to use renamed `WeightedItem` types |
| ip-intelligence-cxx | `fiftyone.h` | Update `MAP_TYPE` macro |
| ip-intelligence-cxx | SWIG output | Regenerate via `RebuildSwig` |
| ip-intelligence-dotnet | Submodule + SWIG + native binaries | Update, rebuild, version bump |
| ip-intelligence-java | Submodule + SWIG + native binaries | Update, rebuild, version bump |
| ip-intelligence-dotnet-examples | Example files | Add/update WEIGHTED_STRING property example |
| ip-intelligence-java-examples | Example files | Add/update WEIGHTED_STRING property example |
