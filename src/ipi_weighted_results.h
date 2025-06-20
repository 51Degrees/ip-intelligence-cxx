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

#ifndef FIFTYONE_DEGREES_IPI_WEIGHTED_RESULTS_INCLUDED
#define FIFTYONE_DEGREES_IPI_WEIGHTED_RESULTS_INCLUDED

#include "ipi.h"
#include "common-cxx/bool.h"
#include "common-cxx/data.h"

typedef struct fiftyone_degrees_weighted_value_header_t {
 fiftyoneDegreesPropertyValueType valueType; // "exposed" one
 int requiredPropertyIndex;
 uint16_t rawWeighting;
} fiftyoneDegreesWeightedValueHeader;


typedef struct fiftyone_degrees_weighted_int_t {
 fiftyoneDegreesWeightedValueHeader header;
 int32_t value;
} fiftyoneDegreesWeightedInt;

typedef struct fiftyone_degrees_weighted_double_t {
 fiftyoneDegreesWeightedValueHeader header;
 double value;
} fiftyoneDegreesWeightedDouble;

typedef struct fiftyone_degrees_weighted_bool_t {
 fiftyoneDegreesWeightedValueHeader header;
 bool value;
} fiftyoneDegreesWeightedBool;

typedef struct fiftyone_degrees_weighted_byte_t {
 fiftyoneDegreesWeightedValueHeader header;
 uint8_t value;
} fiftyoneDegreesWeightedByte;

typedef struct fiftyone_degrees_weighted_string_t {
 fiftyoneDegreesWeightedValueHeader header;
 fiftyoneDegreesData stringData; // owns `value` memory
 const char *value;
} fiftyoneDegreesWeightedString;


typedef struct fiftyone_degrees_weighted_values_collection_t {
 fiftyoneDegreesData valuesData; // owns "real" data
 fiftyoneDegreesData itemsData;  // owns `items` ToC
 fiftyoneDegreesWeightedValueHeader ** items;
 uint32_t itemsCount;
} fiftyoneDegreesWeightedValuesCollection;


EXTERNAL fiftyoneDegreesWeightedValuesCollection fiftyoneDegreesResultsIpiGetValuesCollection(
 fiftyoneDegreesResultsIpi *results,
 const int *requiredPropertyIndexes,
 uint32_t requiredPropertyIndexesLength,
 fiftyoneDegreesData *tempData,
 fiftyoneDegreesException* exception);

EXTERNAL void fiftyoneDegreesWeightedValuesCollectionRelease(
 fiftyoneDegreesWeightedValuesCollection *collection);

#endif //FIFTYONE_DEGREES_IPI_WEIGHTED_RESULTS_INCLUDED
