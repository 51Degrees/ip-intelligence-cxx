/* *********************************************************************
* This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL)
 * v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include "ipi_weighted_results.h"
#include "fiftyone.h"

MAP_TYPE(WeightedValueHeader);
MAP_TYPE(WeightedInt);
MAP_TYPE(WeightedDouble);
MAP_TYPE(WeightedBool);
MAP_TYPE(WeightedByte);
MAP_TYPE(WeightedString);
MAP_TYPE(WeightedValuesCollection);


typedef void (*PropValueInitFunc)(
    WeightedValueHeader *header,
    void *converterState);
typedef void (*PropValueSaveFunc)(
    WeightedValueHeader *header,
    const StoredBinaryValue *storedBinaryValue,
    PropertyValueType propertyValueType,
    void *converterState,
    Exception *exception);
typedef void (*PropValueFreeFunc)(
    WeightedValueHeader *header);

typedef struct {
    const char * const name;
    const PropertyValueType valueType;
    const PropValueInitFunc itemInitFunc;
    const PropValueSaveFunc itemSaveFunc;
    const PropValueFreeFunc itemFreeFunc;
    const size_t itemSize;
} PropValuesConverter;


typedef struct {
    int requiredPropertyIndex;
    Data data;
    uint32_t count;
    const PropValuesConverter *converter;
} PropValuesChunk;

typedef struct {
    uint32_t count;
    Data data;
    PropValuesChunk *items;
} PropValues;

static void PropValuesInit(PropValues * const values, const uint32_t count) {
    values->count = count;
    DataReset(&values->data);
    DataMalloc(&values->data, count * sizeof(PropValuesChunk));
    values->data.used = values->data.allocated;
    values->items = (PropValuesChunk *)values->data.ptr;
    for (uint32_t i = 0, n = values->count; i < n; i++) {
        DataReset(&values->items[i].data);
    }
}

static void PropValuesRelease(PropValues * const values) {
    if (values->items && values->count > 0) {
        for (uint32_t i = 0, n = values->count; i < n; i++) {
            Data * const nextChunkData = &(values->items[i].data);
            if (nextChunkData->allocated) {
                Free(nextChunkData->ptr);
                DataReset(nextChunkData);
            }
        }
    }
    if (values->data.allocated) {
        Free(values->data.ptr);
        DataReset(&values->data);
    }
    values->count = 0;
    values->items = NULL;
}

static void InitInt(
    WeightedValueHeader * const header,
    void * const converterState) {
    ((WeightedInt*)header)->value = *(int*)converterState;
}
static void SaveInt(
    WeightedValueHeader * const header,
    const StoredBinaryValue * const storedBinaryValue,
    const PropertyValueType propertyValueType,
    void * const converterState,
    Exception * const exception) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(exception);
#	endif

    ((WeightedInt*)header)->value = StoredBinaryValueToIntOrDefault(
        storedBinaryValue, propertyValueType, *(int*)converterState);
}

static void InitBool(
    WeightedValueHeader * const header,
    void * const converterState) {
    ((WeightedBool*)header)->value = *(bool*)converterState;
}
static void SaveBool(
    WeightedValueHeader * const header,
    const StoredBinaryValue * const storedBinaryValue,
    const PropertyValueType propertyValueType,
    void * const converterState,
    Exception * const exception) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(exception);
#	endif

    ((WeightedBool*)header)->value = StoredBinaryValueToBoolOrDefault(
        storedBinaryValue, propertyValueType, *(bool*)converterState);
}

static void InitDouble(
    WeightedValueHeader * const header,
    void * const converterState) {
    ((WeightedDouble*)header)->value = *(double*)converterState;
}
static void SaveDouble(
    WeightedValueHeader * const header,
    const StoredBinaryValue * const storedBinaryValue,
    const PropertyValueType propertyValueType,
    void * const converterState,
    Exception * const exception) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(exception);
#	endif

    ((WeightedDouble*)header)->value = StoredBinaryValueToDoubleOrDefault(
        storedBinaryValue, propertyValueType, *(double*)converterState);
}

static void InitByte(
    WeightedValueHeader * const header,
    void * const converterState) {
    ((WeightedByte*)header)->value = *(byte*)converterState;
}
static void SaveByte(
    WeightedValueHeader * const header,
    const StoredBinaryValue * const storedBinaryValue,
    const PropertyValueType propertyValueType,
    void * const converterState,
    Exception * const exception) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(exception);
#	endif

    ((WeightedByte*)header)->value = (byte)StoredBinaryValueToIntOrDefault(
        storedBinaryValue, propertyValueType, *(uint8_t*)converterState);
}

static void InitString(
    WeightedValueHeader * const header,
    void * const converterState) {
#	ifdef _MSC_VER
    UNREFERENCED_PARAMETER(converterState);
#	endif
    WeightedString * const wString = (WeightedString*)header;
    DataReset(&wString->stringData);
    wString->value = NULL;
}
typedef struct {
    const uint8_t decimalPlaces;
    Data * const tempData;
} StringConverterState;
static void SaveString(
    WeightedValueHeader * const header,
    const StoredBinaryValue * const storedBinaryValue,
    const PropertyValueType propertyValueType,
    void * const converterState,
    Exception * const exception) {

    StringConverterState * const state = (StringConverterState *)converterState;
    StringBuilder builder = {
        (char *)state->tempData->ptr,
        state->tempData->allocated,
    };
    StringBuilderInit(&builder);
    StringBuilderAddStringValue(
        &builder,
        storedBinaryValue,
        propertyValueType,
        state->decimalPlaces,
        exception);
    StringBuilderComplete(&builder);
    size_t added = builder.added;
    if (EXCEPTION_OKAY && builder.added > builder.length) {
        DataMalloc(state->tempData, builder.added + 2);
        StringBuilder builder2 = {
            (char *)state->tempData->ptr,
            state->tempData->allocated,
        };
        StringBuilderInit(&builder2);
        StringBuilderAddStringValue(
            &builder2,
            storedBinaryValue,
            propertyValueType,
            state->decimalPlaces,
            exception);
        StringBuilderComplete(&builder2);
        added = builder.added;
    }
    if (EXCEPTION_OKAY) {
        WeightedString * const wString = (WeightedString*)header;
        DataMalloc(&wString->stringData, added + 1);
        memcpy(
            wString->stringData.ptr,
            state->tempData->ptr,
            added + 1);
        wString->value = (char*)wString->stringData.ptr;
    }
}
static void FreeString(WeightedValueHeader * const header) {
    WeightedString * const wString = (WeightedString*)header;
    if (wString->stringData.allocated) {
        Free(wString->stringData.ptr);
        DataReset(&wString->stringData);
    }
    wString->value = NULL;
}

static const PropValuesConverter PropValuesConverter_Int = {
    "PropValuesConverter_Int",
    FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER,
    InitInt,
    SaveInt,
    NULL,
    sizeof(WeightedInt),
};
static const PropValuesConverter PropValuesConverter_Double = {
    "PropValuesConverter_Double",
    FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE,
    InitDouble,
    SaveDouble,
    NULL,
    sizeof(WeightedDouble),
};
static const PropValuesConverter PropValuesConverter_Bool = {
    "PropValuesConverter_Bool",
    FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN,
    InitBool,
    SaveBool,
    NULL,
    sizeof(WeightedBool),
};
static const PropValuesConverter PropValuesConverter_Byte = {
    "PropValuesConverter_Byte",
    FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE,
    InitByte,
    SaveByte,
    NULL,
    sizeof(WeightedByte),
};
static const PropValuesConverter PropValuesConverter_String = {
    "PropValuesConverter_String",
    FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
    InitString,
    SaveString,
    FreeString,
    sizeof(WeightedString),
};


static const PropValuesConverter * PropValuesConverterFor(
    const PropertyValueType valueType) {
    switch (valueType) {
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER:
            return &PropValuesConverter_Int;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE:
            return &PropValuesConverter_Double;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN:
            return &PropValuesConverter_Bool;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE:
            return &PropValuesConverter_Byte;
        default:
            assert(false);
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING:
            return &PropValuesConverter_String;
    }
}


typedef struct {
    PropValuesChunk * const chunk;
    const ProfilePercentage * const valuesItems;
    const uint32_t valuesCount;
    const PropertyValueType storedValueType;
    Exception * const exception;
} PropValuesChunkContext;

static void PropValuesChunkPopulate(
    const PropValuesChunkContext * const context,
    const PropValuesConverter * const converter,
    void * const converterState) {

    Exception * const exception = context->exception;

    context->chunk->converter = converter;
    DataMalloc(
        &context->chunk->data,
        context->chunk->count * converter->itemSize);
    context->chunk->data.used = context->chunk->data.allocated;
    uint8_t * const chunkDataPtr = context->chunk->data.ptr;

    for (uint32_t i = 0; i < context->valuesCount; i++) {
        WeightedValueHeader * const header = (WeightedValueHeader *)(
            converter->itemSize * i + chunkDataPtr);
        converter->itemInitFunc(header, converterState);
    }
    for (uint32_t i = 0; (i < context->valuesCount) && EXCEPTION_OKAY; i++) {
        WeightedValueHeader * const header = (WeightedValueHeader *)(
            converter->itemSize * i + chunkDataPtr);
        header->requiredPropertyIndex = context->chunk->requiredPropertyIndex;
        header->rawWeighting = context->valuesItems[i].rawWeighting;
        header->valueType = converter->valueType;
        const StoredBinaryValue * const binaryValue = (StoredBinaryValue *)(
            context->valuesItems[i].item.data.ptr);
        converter->itemSaveFunc(
            header,
            binaryValue,
            context->storedValueType,
            converterState,
            exception);
    }
}

static void PropValuesChunkPopulate_Int(
    const PropValuesChunkContext * const context,
    const int defaultValue) {
    PropValuesChunkPopulate(
        context,
        &PropValuesConverter_Int,
        (void *)&defaultValue);
}
static void PropValuesChunkPopulate_Double(
    const PropValuesChunkContext * const context,
    const double defaultValue) {
    PropValuesChunkPopulate(
        context,
        &PropValuesConverter_Double,
        (void *)&defaultValue);
}
static void PropValuesChunkPopulate_Byte(
    const PropValuesChunkContext * const context,
    const byte defaultValue) {
    PropValuesChunkPopulate(
        context,
        &PropValuesConverter_Byte,
        (void *)&defaultValue);
}
static void PropValuesChunkPopulate_Bool(
    const PropValuesChunkContext * const context,
    const bool defaultValue) {
    PropValuesChunkPopulate(
        context,
        &PropValuesConverter_Bool,
        (void *)&defaultValue);
}
static void PropValuesChunkPopulate_String(
    const PropValuesChunkContext * const context,
    fiftyoneDegreesData * const tempData,
    const uint8_t decimalPlaces) {
    StringConverterState state = {
        decimalPlaces,
        tempData,
    };
    PropValuesChunkPopulate(
        context,
        &PropValuesConverter_String,
        (void *)&state);
}

typedef struct {
    int intValue;
    double doubleValue;
    bool boolValue;
    uint8_t byteValue;
    uint8_t stringDecimalPlaces;
} PropValuesItemConversionDefaults;

static void PropValuesChunkInit(
    PropValuesChunk * const chunk,
    ResultsIpi * const results,
    const PropValuesItemConversionDefaults * const defaults,
    fiftyoneDegreesData * const tempData,
    Exception * const exception) {

    const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
    const uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
        dataSet->b.b.available,
        chunk->requiredPropertyIndex);

    // We should not have any undefined data type in the data file
    // If there is, the data file is not good to use so terminates.
    const PropertyValueType valueType = PropertyGetValueType(
        dataSet->properties, propertyIndex, exception);
    if (EXCEPTION_FAILED) {
        return;
    }

    const PropertyValueType storedValueType = PropertyGetStoredTypeByIndex(
        dataSet->propertyTypes,
        propertyIndex,
        exception);
    if (EXCEPTION_FAILED) {
        return;
    }

    // Get a pointer to the first value item for the property.
    const ProfilePercentage * const valuesItems = ResultsIpiGetValues(
        results,
        chunk->requiredPropertyIndex,
        exception);
    if (EXCEPTION_FAILED) {
        return;
    }
    if (valuesItems == NULL) {
        return;
    }

    chunk->count = results->values.count;
    const PropValuesChunkContext context = {
        chunk,
        valuesItems,
        chunk->count,
        storedValueType,
        exception,
    };

    switch (valueType) {
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER:
            PropValuesChunkPopulate_Int(&context, defaults->intValue);
            break;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_PRECISION_FLOAT:
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE:
            PropValuesChunkPopulate_Double(&context, defaults->doubleValue);
            break;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN:
            PropValuesChunkPopulate_Bool(&context, defaults->boolValue);
            break;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_SINGLE_BYTE:
            PropValuesChunkPopulate_Byte(&context, defaults->byteValue);
            break;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING:
        default:
            PropValuesChunkPopulate_String(
                &context,
                tempData,
                defaults->stringDecimalPlaces);
            break;
    }
    chunk->data.used = chunk->data.allocated;
}

static void PropValuesPopulate(
    const PropValues * const values,
    ResultsIpi * const results,
    const PropValuesItemConversionDefaults * const defaults,
    fiftyoneDegreesData * const tempData,
    Exception * const exception) {

    for (uint32_t i = 0, n = values->count; (i < n) && EXCEPTION_OKAY; i++) {
        PropValuesChunkInit(
            &(values->items[i]),
            results,
            defaults,
            tempData,
            exception);
    }
}

static void PropValuesMoveItems(
    const PropValues * const values,
    WeightedValuesCollection * const result) {

    size_t totalSize = 0;
    uint32_t totalCount = 0;
    for (uint32_t i = 0, n = values->count; i < n; i++) {
        totalSize += values->items[i].data.allocated;
        totalCount += values->items[i].count;
    }
    DataMalloc(&result->valuesData, totalSize);
    DataMalloc(
        &result->itemsData,
        totalCount * sizeof(WeightedValueHeader*));
    result->items = (WeightedValueHeader**)result->itemsData.ptr;
    uint8_t *nextChunkNew = (uint8_t *)result->valuesData.ptr;
    for (uint32_t i = 0, n = values->count; i < n; i++) {
        PropValuesChunk * const nextChunk = &(values->items[i]);
        const uint8_t *nextHeaderStart = (const uint8_t *)nextChunkNew;
        memcpy(nextChunkNew, nextChunk->data.ptr, nextChunk->data.allocated);
        result->valuesData.used += nextChunk->data.allocated;
        nextChunkNew += nextChunk->data.allocated;
        if (nextChunk->data.allocated) {
            Free(nextChunk->data.ptr);
            DataReset(&nextChunk->data);
        }
        for (uint32_t j = 0, m = nextChunk->count; j < m; j++) {
            WeightedValueHeader* const nextHeader =
                (WeightedValueHeader*)nextHeaderStart;
            result->items[result->itemsCount] = nextHeader;
            nextHeaderStart += nextChunk->converter->itemSize;
            result->itemsCount++;
        }
    }
}

WeightedValuesCollection fiftyoneDegreesResultsIpiGetValuesCollection(
    ResultsIpi * const results,
    const int * const requiredPropertyIndexes,
    const uint32_t requiredPropertyIndexesLength,
    fiftyoneDegreesData * const tempData,
    Exception * const exception) {

    const PropValuesItemConversionDefaults defaults = {
        0,
        0.0,
        false,
        0x00,
        MAX_DOUBLE_DECIMAL_PLACES,
    };

    WeightedValuesCollection result;
    DataReset(&result.valuesData);
    DataReset(&result.itemsData);
    result.items = NULL;
    result.itemsCount = 0;

    PropValues values;

    const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;
    if (requiredPropertyIndexes) {
        if (requiredPropertyIndexesLength <= 0) {
            EXCEPTION_SET(INVALID_INPUT);
            return result;
        }
        PropValuesInit(&values, requiredPropertyIndexesLength);
        for (uint32_t i = 0; i < requiredPropertyIndexesLength; i++) {
            values.items[i].requiredPropertyIndex = requiredPropertyIndexes[i];
        }
    } else {
        const uint32_t propsCount = dataSet->b.b.available->count;
        PropValuesInit(&values, propsCount);
        for (uint32_t i = 0; i < propsCount; i++) {
            values.items[i].requiredPropertyIndex = (int)i;
        }
    }
    {
        Data myTempData;
        Data * const theTempData = (tempData
            ? tempData
            : (DataReset(&myTempData), &myTempData));
        PropValuesPopulate(&values, results, &defaults, theTempData, exception);
        if ((theTempData == &myTempData) && myTempData.allocated) {
            Free(myTempData.ptr);
            DataReset(&myTempData);
        }
    }
    PropValuesMoveItems(&values, &result);
    PropValuesRelease(&values);
    if (EXCEPTION_FAILED) {
        fiftyoneDegreesWeightedValuesCollectionRelease(&result);
    }
    return result;
}

void fiftyoneDegreesWeightedValuesCollectionRelease(
    WeightedValuesCollection * const collection) {

    if (collection->items && collection->itemsCount > 0) {
        for (uint32_t i = 0, n = collection->itemsCount; i < n; i++) {
            WeightedValueHeader * const nextHeader = collection->items[i];
            const PropertyValueType valueType = nextHeader->valueType;
            const PropValuesConverter * const converter = (
                PropValuesConverterFor(valueType));
            const PropValueFreeFunc freeFunc = converter->itemFreeFunc;
            if (freeFunc) {
                freeFunc(collection->items[i]);
            }
        }
    }
    collection->items = NULL;
    collection->itemsCount = 0;
    if (collection->itemsData.allocated) {
        Free(collection->itemsData.ptr);
        DataReset(&collection->itemsData);
    }
    if (collection->valuesData.allocated) {
        Free(collection->valuesData.ptr);
        DataReset(&collection->valuesData);
    }
}
