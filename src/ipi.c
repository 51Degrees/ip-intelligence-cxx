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

#include "ipi.h"
#include "fiftyone.h"
#include "common-cxx/config.h"
#include "constantsIpi.h"
#include "ip-graph-cxx/graph.h"

MAP_TYPE(Collection)

/**
 * GENERAL MACROS TO IMPROVE READABILITY
 */

/** Offset used for a null profile. */
#define NULL_PROFILE_OFFSET UINT32_MAX
/** Offset does not have value. */
#define NULL_VALUE_OFFSET UINT32_MAX
/** Dynamic component */
#define DYNAMIC_COMPONENT_OFFSET UINT32_MAX

/** Default value and percentage separator */
#define DEFAULT_VALUE_PERCENTAGE_SEPARATOR ":"
/** Default values separator */
#define DEFAULT_VALUES_SEPARATOR "|"

#define COMPONENT(d, i) i < d->componentsList.count ? \
(Component*)d->componentsList.items[i].data.ptr : NULL

#define MAX_CONCURRENCY(t) if (config->t.concurrency > concurrency) { \
concurrency = config->t.concurrency; }

#define COLLECTION_CREATE_MEMORY(t) \
dataSet->t = CollectionCreateFromMemory( \
reader, \
dataSet->header.t); \
if (dataSet->t == NULL) { \
	return INVALID_COLLECTION_CONFIG; \
}

#define COLLECTION_CREATE_FILE(t,f) \
dataSet->t = CollectionCreateFromFile( \
	file, \
	&dataSet->b.b.filePool, \
	&dataSet->config.t, \
	dataSet->header.t, \
	f); \
if (dataSet->t == NULL) { \
	return INVALID_COLLECTION_CONFIG; \
}

/** Get min value */
#define MIN(a,b) a < b ? a : b
/** Get max value */
#define MAX(a,b) a > b ? a : b

/**
 * PRIVATE DATA STRUCTURES
 */

/**
 * Used to pass a data set pointer and an exception to methods that require a
 * callback method and a void pointer for state used by the callback method.
 */
typedef struct state_with_exception_t {
	void* state; /* Pointer to the data set or other state information */
	Exception* exception; /* Pointer to the exception structure */
} stateWithException;

typedef struct state_with_percentage_t {
	void* subState; /* Pointer to a data set or other information */
	uint16_t rawWeighting;
} stateWithPercentage;

/**
 * Used to pass a state together with an unique header index which
 * might be used to compared against evidence.
 */
typedef struct state_with_unique_header_index_t {
	void* subState; /* Pointer to the data set or other state information */
	uint32_t headerIndex; /* An unique header index to use */
} stateWithUniqueHeaderIndex;

/**
 * Used to represent the structure within a profile groups item
 */
#pragma pack(push, 2)
typedef struct profile_combination_component_index_t {
	uint16_t index; /* Index to the first profile of the component
					in the profiles list */
	uint16_t count; /* The number of profiles presents for that
					component */
} componentIndex;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct offset_percentage_t {
	uint32_t offset; /* Offset to a collection item */
	uint16_t rawWeighting; /* The weight of the item in the matched IP range, out of 65535 */
} offsetPercentage;
#pragma pack(pop)

/**
 * All profile weightings in a groups should add up to exactly this number.
 */
static const uint16_t FULL_RAW_WEIGHTING = 0xFFFFU;

/**
 * PRESET IP INTELLIGENCE CONFIGURATIONS
 */

/* The expected version of the data file */
#define FIFTYONE_DEGREES_IPI_TARGET_VERSION_MAJOR 4
#define FIFTYONE_DEGREES_IPI_TARGET_VERSION_MINOR 4

#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY true
fiftyoneDegreesConfigIpi fiftyoneDegreesIpiInMemoryConfig = {
	{FIFTYONE_DEGREES_CONFIG_DEFAULT_WITH_INDEX},
	{0,0,0}, // Strings
	{0,0,0}, // Components
	{0,0,0}, // Maps
	{0,0,0}, // Properties
	{0,0,0}, // Values
	{0,0,0}, // Profiles
	{0,0,0}, // Graphs
	{0,0,0}, // ProfileGroups
	{0,0,0}, // ProfileOffsets
	{0,0,0}  // Graph
};
#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY \
FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY_DEFAULT

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiHighPerformanceConfig = {
	{ FIFTYONE_DEGREES_CONFIG_DEFAULT_WITH_INDEX },
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Graphs
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileGroups
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }  // Graph
};

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiLowMemoryConfig = {
	{ FIFTYONE_DEGREES_CONFIG_DEFAULT_NO_INDEX },
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Graphs
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileGroups
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }  // Graph
};

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiSingleLoadedConfig = {
	{ FIFTYONE_DEGREES_CONFIG_DEFAULT_NO_INDEX },
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Graphs
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileGroups
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // ProfileOffsets
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }  // Graph
};

#define FIFTYONE_DEGREES_IPI_CONFIG_BALANCED \
{ FIFTYONE_DEGREES_CONFIG_DEFAULT_WITH_INDEX }, \
{ FIFTYONE_DEGREES_STRING_LOADED, FIFTYONE_DEGREES_STRING_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Strings */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Components */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Maps */ \
{ FIFTYONE_DEGREES_PROPERTY_LOADED, FIFTYONE_DEGREES_PROPERTY_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Properties */ \
{ FIFTYONE_DEGREES_VALUE_LOADED, FIFTYONE_DEGREES_VALUE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Values */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Profiles */ \
{ FIFTYONE_DEGREES_IP_GRAPHS_LOADED, FIFTYONE_DEGREES_IP_GRAPHS_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Graphs */ \
{ FIFTYONE_DEGREES_PROFILE_GROUPS_LOADED, FIFTYONE_DEGREES_PROFILE_GROUPS_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* ProfileGroups */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* ProfileOffsets */ \
{ FIFTYONE_DEGREES_IP_GRAPH_LOADED, FIFTYONE_DEGREES_IP_GRAPH_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY } /* Graph */

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiBalancedConfig = {
	FIFTYONE_DEGREES_IPI_CONFIG_BALANCED
};

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiDefaultConfig = {
	FIFTYONE_DEGREES_IPI_CONFIG_BALANCED
};

#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE true
fiftyoneDegreesConfigIpi fiftyoneDegreesIpiBalancedTempConfig = {
	FIFTYONE_DEGREES_IPI_CONFIG_BALANCED
};
#undef FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE
#define FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE \
FIFTYONE_DEGREES_CONFIG_USE_TEMP_FILE_DEFAULT

/**
 * IP INTELLIGENCE METHODS
 */

static void resultIpiReset(ResultIpi* result) {
	memset(result->targetIpAddress.value, 0, FIFTYONE_DEGREES_IPV6_LENGTH);
	result->targetIpAddress.length = 0;
	result->targetIpAddress.type = IP_TYPE_INVALID;
}

static int compareIpAddresses(
	const byte* address1,
	const byte* address2,
	int length) {
	for (int i = 0; i < length; i++) {
		int difference = (int)address1[i] - (int)address2[i];
		if (difference != 0) return difference;
	}
	return 0;
}

static int compareToIpv4Range(
	void* state,
	Item* item,
	long curIndex,
	Exception* exception) {
	int result = 0;
	fiftyoneDegreesIpAddress target = *((fiftyoneDegreesIpAddress*)state);
	if (target.length != FIFTYONE_DEGREES_IPV4_LENGTH)
	{
		// Length mismatched
		EXCEPTION_SET(INCORRECT_IP_ADDRESS_FORMAT);
	}
	else {
		// We will terminate if IP address is within the range between the current item and the next item
		int tempResult = compareIpAddresses(((Ipv4Range*)item->data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV4_LENGTH);
		if (tempResult < 0) {
			Item nextItem;
			DataReset(&nextItem.data);
			if ((uint32_t)curIndex + 1 < item->collection->count &&
				item->collection->get(
					item->collection,
					++curIndex,
					&nextItem,
					exception) != NULL && EXCEPTION_OKAY) {
				if (compareIpAddresses(((Ipv4Range*)nextItem.data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV4_LENGTH) <= 0) {
					result = -1;
				}
				COLLECTION_RELEASE(item->collection, &nextItem);
			}
		}
		else if (tempResult > 0 && curIndex > 0) {
			// The IP address is out of range
			// NOTE: If the current index is 0
			// There is no more item lower so return the current
			result = 1;
		}
	}
	return result;
}

static int compareToIpv6Range(
	void* state,
	Item* item,
	long curIndex,
	Exception* exception) {
	int result = 0;
	fiftyoneDegreesIpAddress target = *((fiftyoneDegreesIpAddress*)state);
	if (target.length != FIFTYONE_DEGREES_IPV6_LENGTH)
	{
		// Length mismatched
		EXCEPTION_SET(INCORRECT_IP_ADDRESS_FORMAT);
	}
	else {
		// We will terminate if IP address is within the range between the current item and the next item
		int tempResult = compareIpAddresses(((Ipv6Range*)item->data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV6_LENGTH);
		if (tempResult < 0) {
			Item nextItem;
			DataReset(&nextItem.data);
			if ((uint32_t)curIndex + 1 < item->collection->count &&
				item->collection->get(
					item->collection,
					++curIndex,
					&nextItem,
					exception) != NULL && EXCEPTION_OKAY) {
				if (compareIpAddresses(((Ipv6Range*)nextItem.data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV6_LENGTH) <= 0) {
					// The IP address is not within the range
					result = -1;
				}
				COLLECTION_RELEASE(item->collection, &nextItem);
			}
		}
		else if (tempResult > 0 && curIndex > 0) {
			// The IP address is out of range
			// NOTE: There is no more item lower
			// so return the current
			result = 1;
		}
	}
	return result;
}

static void setResultFromIpAddress(
	ResultIpi* result,
	DataSetIpi* dataSet,
	Exception* exception) {
	const uint32_t profileOffsetIndex = fiftyoneDegreesIpiGraphEvaluate(
		dataSet->graphsArray,
		1, 
		result->targetIpAddress, 
		exception);
	if (profileOffsetIndex != NULL_PROFILE_OFFSET && EXCEPTION_OKAY) {
		result->profileOffsetIndex = profileOffsetIndex;
	}
}

/**
 * DATA INITIALISE AND RESET METHODS
 */

static void resetDataSet(DataSetIpi* dataSet) {
	DataSetReset(&dataSet->b.b);
	ListReset(&dataSet->componentsList);
	dataSet->componentsAvailable = NULL;
	dataSet->components = NULL;
	dataSet->maps = NULL;
	dataSet->graphs = NULL;
	dataSet->profileGroups = NULL;
	dataSet->profileOffsets = NULL;
	dataSet->profiles = NULL;
	dataSet->properties = NULL;
	dataSet->strings = NULL;
	dataSet->values = NULL;
}

static void freeDataSet(void* dataSetPtr) {
	DataSetIpi* dataSet = (DataSetIpi*)dataSetPtr;

	// Free the common data set fields.
	DataSetFree(&dataSet->b.b);

	// Free the resources associated with the graphs.
	fiftyoneDegreesIpiGraphFree(dataSet->graphsArray);

	// Free the memory used for the lists and collections.
	ListFree(&dataSet->componentsList);
	Free(dataSet->componentsAvailable);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->strings);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->components);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->properties);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->maps);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->values);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profiles);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->graphs);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profileOffsets);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profileGroups);
	
	// Finally free the memory used by the resource itself as this is always
	// allocated within the IP Intelligence init manager method.
	Free(dataSet);
}

static long initGetHttpHeaderString(
	void *state,
	uint32_t index,
	Item *nameItem) {
	DataSetIpi *dataSet =
		(DataSetIpi*)((stateWithException*)state)->state;
	Exception *exception = ((stateWithException*)state)->exception;
	uint32_t i = 0, c = 0;
	Component *component = COMPONENT(dataSet, c);
	c++;
	while (component != NULL) {
		if (index < i + component->keyValuesCount) {
			const ComponentKeyValuePair *keyValue =
				ComponentGetKeyValuePair(
					component,
					(uint16_t)(index - i),
					exception);
			nameItem->collection = NULL;
			dataSet->strings->get(
				dataSet->strings,
				keyValue->key,
				nameItem,
				exception);
			return keyValue->key;
		}
		i += component->keyValuesCount;
		component = COMPONENT(dataSet, c);
		c++;
	}
	return -1;
}

static String* initGetPropertyString(
	void* state,
	uint32_t index,
	Item* item) {
	String* name = NULL;
	Item propertyItem;
	Property* property;
	DataSetIpi* dataSet = (DataSetIpi*)((stateWithException*)state)->state;
	Exception* exception = ((stateWithException*)state)->exception;
	uint32_t propertiesCount = CollectionGetCount(dataSet->properties);
	DataReset(&item->data);
	if (index < propertiesCount) {
		DataReset(&propertyItem.data);
		item->collection = NULL;
		item->handle = NULL;
		property = (Property*)dataSet->properties->get(
			dataSet->properties,
			index,
			&propertyItem,
			exception);
		if (property != NULL && EXCEPTION_OKAY) {
			name = PropertyGetName(
				dataSet->strings,
				property,
				item,
				exception);
			if (EXCEPTION_OKAY) {
				COLLECTION_RELEASE(dataSet->properties, &propertyItem);
			}
		}
	}
	return name;
}

static StatusCode initComponentsAvailable(
	DataSetIpi* dataSet,
	Exception* exception) {
	uint32_t i;
	Property* property;
	Item item;
	DataReset(&item.data);

	for (i = 0;
		i < dataSet->b.b.available->count;
		i++) {
		property = PropertyGet(
			dataSet->properties,
			dataSet->b.b.available->items[i].propertyIndex,
			&item,
			exception);
		if (property == NULL || EXCEPTION_FAILED) {
			return COLLECTION_FAILURE;
		}
		dataSet->componentsAvailable[property->componentIndex] = true;
		COLLECTION_RELEASE(dataSet->properties, &item);
	}
	return SUCCESS;
}

static int findPropertyIndexByName(
	Collection *properties,
	Collection *strings,
	char *name,
	Exception *exception) {
	int index;
	bool found = false;
	Property *property;
	String *propertyName;
	Item propertyItem, nameItem;
	int count = CollectionGetCount(properties);
	DataReset(&propertyItem.data);
	DataReset(&nameItem.data);
	for (index = 0; index < count && found == false; index++) {
		property = PropertyGet(
			properties,
			index,
			&propertyItem,
			exception);
		if (property != NULL &&
			EXCEPTION_OKAY) {
			propertyName = PropertyGetName(
				strings,
				property,
				&nameItem,
				exception);
			if (propertyName != NULL && EXCEPTION_OKAY) {
				if (StringCompare(name, &propertyName->value) == 0) {
					found = true;
				}
				COLLECTION_RELEASE(strings, &nameItem);
			}
			COLLECTION_RELEASE(properties, &propertyItem);
		}
	}
	return found ? index : -1;
}

static void initGetEvidencePropertyRelated(
	DataSetIpi* dataSet,
	PropertyAvailable* availableProperty,
	EvidenceProperties* evidenceProperties,
	int* count,
	char* suffix,
	Exception* exception) {
	Property* property;
	String* name;
	String* availableName = (String*)availableProperty->name.data.ptr;
	int requiredLength = ((int)strlen(suffix)) + availableName->size - 1;
	Item propertyItem, nameItem;
	DataReset(&propertyItem.data);
	DataReset(&nameItem.data);
	int propertiesCount = CollectionGetCount(dataSet->properties);
	for (int propertyIndex = 0; 
		propertyIndex < propertiesCount && EXCEPTION_OKAY; 
		propertyIndex++) {
		property = PropertyGet(
			dataSet->properties,
			propertyIndex,
			&propertyItem,
			exception);
		if (property != NULL && EXCEPTION_OKAY) {
			name = StringGet(
				dataSet->strings,
				property->nameOffset,
				&nameItem,
				exception);
			if (name != NULL && EXCEPTION_OKAY) {
				if (requiredLength == name->size -1 &&
					// Check that the available property matches the start of
					// the possible related property.
					StringCompareLength(
						&availableName->value,
						&name->value,
						(size_t)availableName->size - 1) == 0 && 
					// Check that the related property has a suffix that 
					// matches the one provided to the method.
					StringCompare(
						&name->value + availableName->size - 1, 
						suffix) == 0) {
					if (evidenceProperties != NULL) {
						evidenceProperties->items[*count] = propertyIndex;
					}
					(*count)++;
				}
				COLLECTION_RELEASE(dataSet->strings, &nameItem);
			}
			COLLECTION_RELEASE(dataSet->properties, &propertyItem);
		}
	}
}

uint32_t initGetEvidenceProperties(
	void* state,
	fiftyoneDegreesPropertyAvailable* availableProperty,
	fiftyoneDegreesEvidenceProperties* evidenceProperties) {
	int count = 0;
	DataSetIpi* dataSet =
		(DataSetIpi*)((stateWithException*)state)->state;
	Exception* exception = ((stateWithException*)state)->exception;

	// Any properties that have a suffix of JavaScript and are associated with
	// an available property should also be added. These are used to gather
	// evidence from JavaScript that might impact the value returned.
	initGetEvidencePropertyRelated(
		dataSet,
		availableProperty,
		evidenceProperties,
		&count,
		"JavaScript",
		exception);

	return (uint32_t)count;
}

static StatusCode initPropertiesAndHeaders(
	DataSetIpi* dataSet,
	PropertiesRequired* properties,
	Exception* exception) {
	stateWithException state;
	state.state = (void*)dataSet;
	state.exception = exception;
	return DataSetInitProperties(
		&dataSet->b.b,
		properties,
		&state,
		initGetPropertyString,
		initGetEvidenceProperties);
}

static StatusCode readHeaderFromMemory(
	MemoryReader* reader,
	const DataSetIpiHeader* header) {

	// Copy the bytes that make up the dataset header.
	if (memcpy(
		(void*)header,
		(const void*)reader->current,
		sizeof(DataSetIpiHeader)) != header) {
		return CORRUPT_DATA;
	}

	// Move the current pointer to the next data structure.
	return MemoryAdvance(reader, sizeof(DataSetIpiHeader)) == true ?
		SUCCESS : CORRUPT_DATA;
}

static StatusCode checkVersion(DataSetIpi* dataSet) {
	if (!(dataSet->header.versionMajor ==
		FIFTYONE_DEGREES_IPI_TARGET_VERSION_MAJOR &&
		dataSet->header.versionMinor ==
		FIFTYONE_DEGREES_IPI_TARGET_VERSION_MINOR)) {
		return INCORRECT_VERSION;
	}
	return SUCCESS;
}

static void initDataSetPost(
	DataSetIpi* dataSet,
	Exception* exception) {
	
	// Initialise the components lists
	ComponentInitList(
		dataSet->components,
		&dataSet->componentsList,
		dataSet->header.components.count,
		exception);
	if (EXCEPTION_FAILED) {
		return;
	}

	// Initialise the components which have required properties.
	dataSet->componentsAvailable = Malloc(
		sizeof(bool) * dataSet->componentsList.count);
	if (dataSet->componentsAvailable == NULL) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return;
	}
}

static StatusCode initWithMemory(
	DataSetIpi* dataSet,
	MemoryReader* reader,
	Exception* exception) {
	StatusCode status = SUCCESS;

	// Indicate that the data is in memory and there is no connection to the
	// source data file.
	dataSet->b.b.isInMemory = true;

	// Check that the reader is configured correctly.
	if (reader->current == NULL) {
		return NULL_POINTER;
	}

	// Copy the bytes that form the header from the start of the memory
	// location to the data set data.ptr provided.
	status = readHeaderFromMemory(reader, &dataSet->header);
	if (status != SUCCESS) {
		return status;
	}

	// Check the version.
	status = checkVersion(dataSet);
	if (status != SUCCESS) {
		return status;
	}

	// Create each of the collections.
	uint32_t stringsCount = dataSet->header.strings.count;
	*(uint32_t*)(&dataSet->header.strings.count) = 0;
	COLLECTION_CREATE_MEMORY(strings)
	*(uint32_t*)(&dataSet->header.strings.count) = stringsCount;

	// Override the header count so that the variable collection can work.
	uint32_t componentCount = dataSet->header.components.count;
	*(uint32_t*)(&dataSet->header.components.count) = 0;
	COLLECTION_CREATE_MEMORY(components)
	*(uint32_t*)(&dataSet->header.components.count) = componentCount;

	COLLECTION_CREATE_MEMORY(maps)
	COLLECTION_CREATE_MEMORY(properties)
	COLLECTION_CREATE_MEMORY(values)

	uint32_t profileCount = dataSet->header.profiles.count;
	*(uint32_t*)(&dataSet->header.profiles.count) = 0;
	COLLECTION_CREATE_MEMORY(profiles)
	*(uint32_t*)(&dataSet->header.profiles.count) = profileCount;

	COLLECTION_CREATE_MEMORY(graphs);

	COLLECTION_CREATE_MEMORY(profileGroups);
	COLLECTION_CREATE_MEMORY(profileOffsets);

	dataSet->graphsArray = fiftyoneDegreesIpiGraphCreateFromMemory(
		dataSet->graphs,
		reader,
		exception);

	/* Check that the current pointer equals the last byte */
	if (reader->lastByte != reader->current) {
		return POINTER_OUT_OF_BOUNDS;
	}

	initDataSetPost(dataSet, exception);

	return status;
}

static StatusCode initInMemory(
	DataSetIpi* dataSet,
	Exception* exception) {
	MemoryReader reader;

	// Read the data from the source file into memory using the reader to 
	// store the pointer to the first and last bytes.
	StatusCode status = DataSetInitInMemory(
		&dataSet->b.b,
		&reader);
	if (status != SUCCESS) {
		freeDataSet(dataSet);
		return status;
	}

	// Use the memory reader to initialize the IP Intelligence data set.
	status = initWithMemory(dataSet, &reader, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		freeDataSet(dataSet);
		return status;
	}
	return status;
}

static void initDataSet(DataSetIpi* dataSet, ConfigIpi** config) {
	// If no config has been provided then use the balanced configuration.
	if (*config == NULL) {
		*config = &IpiBalancedConfig;
	}

	// Reset the data set so that if a partial initialise occurs some memory
	// can freed.
	resetDataSet(dataSet);

	// Copy the configuration into the data set to ensure it's always
	// available in cases where the source configuration gets freed.
	memcpy((void*)&dataSet->config, *config, sizeof(ConfigIpi));
	dataSet->b.b.config = &dataSet->config;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode readHeaderFromFile(
	FILE* file,
	const DataSetIpiHeader* header) {
	// Read the bytes that make up the dataset header.
	if (fread(
		(void*)header,
		sizeof(DataSetIpiHeader),
		1,
		file) != 1) {
		return CORRUPT_DATA;
	}

	return SUCCESS;
}

static StatusCode readDataSetFromFile(
	DataSetIpi* dataSet,
	FILE* file,
	Exception* exception) {
	StatusCode status = SUCCESS;

	// Copy the bytes that form the header from the start of the memory
	// location to the data set data.ptr provided
	status = readHeaderFromFile(file, &dataSet->header);
	if (status != SUCCESS) {
		return status;
	}

	// Check the version.
	status = checkVersion(dataSet);
	if (status != SUCCESS) {
		return status;
	}

	// Create the strings collection.
	uint32_t stringsCount = dataSet->header.strings.count;
	*(uint32_t*)(&dataSet->header.strings.count) = 0;
	COLLECTION_CREATE_FILE(strings, fiftyoneDegreesStringRead);
	*(uint32_t*)(&dataSet->header.strings.count) = stringsCount;

	// Override the header count so that the variable collection can work.
	uint32_t componentCount = dataSet->header.components.count;
	*(uint32_t*)(&dataSet->header.components.count) = 0;
	COLLECTION_CREATE_FILE(components, fiftyoneDegreesComponentReadFromFile);
	*(uint32_t*)(&dataSet->header.components.count) = componentCount;

	COLLECTION_CREATE_FILE(maps, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(properties, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(values, CollectionReadFileFixed);

	uint32_t profileCount = dataSet->header.profiles.count;
	*(uint32_t*)(&dataSet->header.profiles.count) = 0;
	COLLECTION_CREATE_FILE(profiles, fiftyoneDegreesProfileReadFromFile);
	*(uint32_t*)(&dataSet->header.profiles.count) = profileCount;

	COLLECTION_CREATE_FILE(graphs, CollectionReadFileFixed);

	COLLECTION_CREATE_FILE(profileGroups, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(profileOffsets, CollectionReadFileFixed);

	dataSet->graphsArray = fiftyoneDegreesIpiGraphCreateFromFile(
		dataSet->graphs,
		file,
		&dataSet->b.b.filePool,
		// This is not the configuration for the collection of all graphs, but
		// the configuration for each individual graph.
		dataSet->config.graph,
		exception);

	initDataSetPost(dataSet, exception);

	return status;
}

#endif

/**
 * Calculates the highest concurrency value to ensure sufficient file reader
 * handles are generated at initialisation to service the maximum number of
 * concurrent operations.
 * @param config being used for initialisation.
 * @return the highest concurrency value from the configuration, or 1 if no
 * concurrency values are available.
 */
static uint16_t getMaxConcurrency(const ConfigIpi* config) {
	uint16_t concurrency = 1;
	MAX_CONCURRENCY(strings);
	MAX_CONCURRENCY(components);
	MAX_CONCURRENCY(maps);
	MAX_CONCURRENCY(properties);
	MAX_CONCURRENCY(values);
	MAX_CONCURRENCY(profiles);
	MAX_CONCURRENCY(graphs);
	MAX_CONCURRENCY(profileOffsets);
	MAX_CONCURRENCY(profileGroups);
	MAX_CONCURRENCY(graph);
	return concurrency;
}

#ifndef FIFTYONE_DEGREES_MEMORY_ONLY

static StatusCode initWithFile(DataSetIpi* dataSet, Exception* exception) {
	StatusCode status;
	FileHandle handle;

	// Initialise the file read for the dataset. Create as many readers as
	// there will be concurrent operations.
	status = FilePoolInit(
		&dataSet->b.b.filePool,
		dataSet->b.b.fileName,
		getMaxConcurrency(&dataSet->config),
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}

	// Create a new file handle for the read operation. The file handle can't
	// come from the pool of handles because there may only be one available
	// in the pool and it will be needed for some initialisation activities.
	status = FileOpen(dataSet->b.b.fileName, &handle.file);
	if (status != SUCCESS) {
		return status;
	}

	// Read the data set from the source.
	status = readDataSetFromFile(dataSet, handle.file, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		fclose(handle.file);
		return status;
	}

	// Before closing the file handle, clean up any other temp files which are
	// not in use.
#ifndef __APPLE__
	if (dataSet->config.b.useTempFile == true) {
		FileDeleteUnusedTempFiles(
			dataSet->b.b.masterFileName,
			dataSet->config.b.tempDirs,
			dataSet->config.b.tempDirCount,
			sizeof(DataSetIpiHeader));
	}
#endif
	// Close the file handle.
	fclose(handle.file);

	return status;
}

#endif

static StatusCode initDataSetFromFile(
	void* dataSetBase,
	const void* configBase,
	PropertiesRequired* properties,
	const char* fileName,
	Exception* exception) {
	DataSetIpi* dataSet = (DataSetIpi*)dataSetBase;
	ConfigIpi* config = (ConfigIpi*)configBase;
	StatusCode status = NOT_SET;

	// Common data set initialisation actions.
	initDataSet(dataSet, &config);

	// Initialise the super data set with the filename and configuration
	// provided.
	status = DataSetInitFromFile(
		&dataSet->b.b,
		fileName,
		sizeof(DataSetIpiHeader));
	if (status != SUCCESS) {
		return status;
	}

	// If there is no collection configuration the the entire data file should
	// be loaded into memory. Otherwise use the collection configuration to
	// partially load data into memory and cache the rest.
	if (config->b.allInMemory == true) {
		status = initInMemory(dataSet, exception);
	}
	else {
#ifndef FIFTYONE_DEGREES_MEMORY_ONLY
		status = initWithFile(dataSet, exception);
#else
		status = INVALID_CONFIG;
#endif
	}

	// Return the status code if something has gone wrong.
	if (status != SUCCESS || EXCEPTION_FAILED) {
		// Delete the temp file if one has been created.
		if (config->b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the required properties and headers and check the 
	// initialisation was successful.
	status = initPropertiesAndHeaders(dataSet, properties, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		// Delete the temp file if one has been created.
		if (config->b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the components available to flag which components have 
	// properties which are to be returned (i.e. available properties).
	status = initComponentsAvailable(dataSet, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		if (config->b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Check there are properties available for retrieval.
	if (dataSet->b.b.available->count == 0) {
		// Delete the temp file if one has been created.
		if (config->b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}
	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesIpiInitManagerFromFile(
	fiftyoneDegreesResourceManager* manager,
	fiftyoneDegreesConfigIpi* config,
	fiftyoneDegreesPropertiesRequired* properties,
	const char* fileName,
	fiftyoneDegreesException* exception) {

	DataSetIpi* dataSet = (DataSetIpi*)Malloc(sizeof(DataSetIpi));
	if (dataSet == NULL) {
		return INSUFFICIENT_MEMORY;
	}

	StatusCode status = initDataSetFromFile(
		dataSet,
		config,
		properties,
		fileName,
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}
	ResourceManagerInit(manager, dataSet, &dataSet->b.b.handle, freeDataSet);
	if (dataSet->b.b.handle == NULL) {
		freeDataSet(dataSet);
		status = INSUFFICIENT_MEMORY;
	}
	return status;
}

size_t fiftyoneDegreesIpiSizeManagerFromFile(
	fiftyoneDegreesConfigIpi* config,
	fiftyoneDegreesPropertiesRequired* properties,
	const char* fileName,
	fiftyoneDegreesException* exception) {
	size_t allocated;
	ResourceManager manager;
#ifdef _DEBUG 
	StatusCode status;
#endif

	// Set the memory allocation and free methods for tracking.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	MallocAligned = MemoryTrackingMallocAligned;
	Free = MemoryTrackingFree;
	FreeAligned = MemoryTrackingFreeAligned;

	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated.
#ifdef _DEBUG 
	status =
#endif
		IpiInitManagerFromFile(
			&manager,
			config,
			properties,
			fileName,
			exception);
#ifdef _DEBUG
	assert(status == SUCCESS);
#endif
	assert(EXCEPTION_OKAY);

	// Free the manager and get the total maximum amount of allocated memory
	// needed for the manager and associated resources.
	ResourceManagerFree(&manager);
	allocated = MemoryTrackingGetMax();

	// Check that all the memory has been freed.
	assert(MemoryTrackingGetAllocated() == 0);

	// Return the malloc and free methods to standard operation.
	Malloc = MemoryStandardMalloc;
	MallocAligned = MemoryStandardMallocAligned;
	Free = MemoryStandardFree;
	FreeAligned = MemoryStandardFreeAligned;
	MemoryTrackingReset();

	return allocated;
}

static StatusCode initDataSetFromMemory(
	void* dataSetBase,
	const void* configBase,
	PropertiesRequired* properties,
	void* memory,
	long size,
	Exception* exception) {
	StatusCode status = SUCCESS;
	MemoryReader reader;
	DataSetIpi* dataSet = (DataSetIpi*)dataSetBase;
	ConfigIpi* config = (ConfigIpi*)configBase;

	// Common data set initialisation actions.
	initDataSet(dataSet, &config);

	// If memory is to be freed when the data set is freed then record the 
	// pointer to the memory location for future reference.
	if (dataSet->config.b.freeData == true) {
		dataSet->b.b.memoryToFree = memory;
	}

	// Set up the reader.
	reader.startByte = reader.current = (byte*)memory;
	reader.length = size;
	reader.lastByte = reader.current + size;

	// Initialise the data set from the memory reader.
	status = initWithMemory(dataSet, &reader, exception);

	// Return the status code if something has gone wrong.
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}

	// Initialise the required properties and headers.
	status = initPropertiesAndHeaders(dataSet, properties, exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		return status;
	}

	// Initialise the components available to flag which components have
	// properties which are to be returned (i.e. available properties).
	status = initComponentsAvailable(dataSet, exception);

	return status;
}

fiftyoneDegreesStatusCode fiftyoneDegreesIpiInitManagerFromMemory(
	fiftyoneDegreesResourceManager* manager,
	fiftyoneDegreesConfigIpi* config,
	fiftyoneDegreesPropertiesRequired* properties,
	void* memory,
	long size,
	fiftyoneDegreesException* exception) {
	DataSetIpi* dataSet = (DataSetIpi*)Malloc(sizeof(DataSetIpi));
	if (dataSet == NULL) {
		return INSUFFICIENT_MEMORY;
	}

	StatusCode status = initDataSetFromMemory(
		dataSet,
		config,
		properties,
		memory,
		size,
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		Free(dataSet);
		return status;
	}
	ResourceManagerInit(manager, dataSet, &dataSet->b.b.handle, freeDataSet);
	if (dataSet->b.b.handle == NULL)
	{
		freeDataSet(dataSet);
		status = INSUFFICIENT_MEMORY;
	}
	return status;
}

size_t fiftyoneDegreesIpiSizeManagerFromMemory(
	fiftyoneDegreesConfigIpi* config,
	fiftyoneDegreesPropertiesRequired* properties,
	void* memory,
	long size,
	fiftyoneDegreesException* exception) {
	size_t allocated;
	ResourceManager manager;
#ifdef _DEBUG
	StatusCode status;
#endif
	// Set the memory allocation and free methods for tracking.
	MemoryTrackingReset();
	Malloc = MemoryTrackingMalloc;
	MallocAligned = MemoryTrackingMallocAligned;
	Free = MemoryTrackingFree;
	FreeAligned = MemoryTrackingFreeAligned;

	// Ensure that the memory used is not freed with the data set.
	ConfigIpi sizeConfig = *config;
	sizeConfig.b.freeData = false;

	// Initialise the manager with the tracking methods in use to determine
	// the amount of memory that is allocated.
#ifdef _DEBUG
	status =
#endif
		IpiInitManagerFromMemory(
			&manager,
			&sizeConfig,
			properties,
			memory,
			size,
			exception);
#ifdef _DEBUG
	assert(status == SUCCESS);
#endif
	assert(EXCEPTION_OKAY);

	// Free the manager and get the total maximum amount of allocated memory
	// needed for the manager and associated resources.
	ResourceManagerFree(&manager);
	allocated = MemoryTrackingGetMax();

	// Check that all the memory has been freed.
	assert(MemoryTrackingGetAllocated() == 0);

	// Return the malloc and free methods to standard operation.
	Malloc = MemoryStandardMalloc;
	MallocAligned = MemoryStandardMallocAligned;
	Free = MemoryStandardFree;
	FreeAligned = MemoryStandardFreeAligned;
	MemoryTrackingReset();

	return allocated;
}

fiftyoneDegreesDataSetIpi* fiftyoneDegreesDataSetIpiGet(fiftyoneDegreesResourceManager* manager) {
	return (DataSetIpi*)DataSetGet(manager);
}

void fiftyoneDegreesDataSetIpiRelease(fiftyoneDegreesDataSetIpi* dataSet) {
	DataSetRelease(&dataSet->b.b);
}

/**
 * Definition of the reload methods from the data set macro.
 */
FIFTYONE_DEGREES_DATASET_RELOAD(Ipi)


/**
 * IP INTELLIGENCE RESULTS METHODS
 */


/**
 * Methods to manipulate the profile percentage list
 * This is mainly used in the values returned in the results
 * for particular property.
 */
static fiftyoneDegreesIpiList* initIpiList(
	fiftyoneDegreesIpiList* list,
	uint32_t capacity,
	const float loadFactor) {
	list->items = (ProfilePercentage*)Malloc(capacity * sizeof(ProfilePercentage));
	if (list->items == NULL) {
		return NULL;
	}
	list->capacity = capacity;
	list->count = 0;
	list->loadFactor = loadFactor;
	return list;
}

static void releaseIpiList(fiftyoneDegreesIpiList* list) {
	uint32_t i;
	for (i = 0; i < list->count; i++) {
		COLLECTION_RELEASE(list->items[i].item.collection, &list->items[i].item);
	}
	list->count = 0;
}

static void freeIpiList(fiftyoneDegreesIpiList* list) {
	releaseIpiList(list);
	if (list->items != NULL) {
		Free(list->items);
	}
	list->items = NULL;
	list->capacity = 0;
}

/*
 * Extend the size of the current list.
 * This should ever be used by the addIpiListItem
 * @param list the current list
 * @param newCapacity which should be bigger
 * than the current capacity else now
 * change will be made
 */
static void extendIpiList(
	fiftyoneDegreesIpiList* list,
	uint32_t newCapacity) {
	// Allocate new list
	if (newCapacity > list->capacity) {
		ProfilePercentage *newItems 
			= (ProfilePercentage*)Malloc(newCapacity * sizeof(ProfilePercentage));
		if (newItems == NULL) {
			return;
		}

		if (list->items != NULL) {
			memcpy(newItems, list->items, list->count * sizeof(ProfilePercentage));
			Free(list->items);
		}
		list->items = newItems;
		list->capacity = newCapacity;
	}
}

static void addIpiListItem(
	fiftyoneDegreesIpiList* const list,
	const fiftyoneDegreesProfilePercentage* const item) {
	assert(list->count < list->capacity);
	assert(item->item.collection != NULL);
	list->items[list->count++] = *item;
	// Check if the list has reached its load factor
	if ((float)(list->count / list->capacity) > list->loadFactor) {
		// Get new capacity
		uint32_t newCapacity =
			(uint32_t)ceilf(list->capacity * IPI_LIST_RESIZE_FACTOR);

		extendIpiList(list, newCapacity);
	}
}

static void resetIpiList(fiftyoneDegreesIpiList* list) {
	list->capacity = 0;
	list->count = 0;
	list->items = NULL;
}

/**
 * Results methods
 */

fiftyoneDegreesResultsIpi* fiftyoneDegreesResultsIpiCreate(
	fiftyoneDegreesResourceManager* manager) {
	ResultsIpi* results;
	DataSetIpi* dataSet;

	// Create a new instance of results.
	FIFTYONE_DEGREES_ARRAY_CREATE(ResultIpi, results, 1);

	if (results != NULL) {

		// Increment the inUse counter for the active data set so that we can
		// track any results that are created.
		dataSet = (DataSetIpi*)DataSetGet(manager);

		// Initialise the results.
		fiftyoneDegreesResultsInit(&results->b, (void*)(&dataSet->b));

		// Reset the property and values list ready for first use sized for 
		// a single value to be returned.
		initIpiList(&results->values, 1, IPI_LIST_DEFAULT_LOAD_FACTOR);
		DataReset(&results->propertyItem.data);
	}

	return results;
}

static void resultsIpiRelease(ResultsIpi* results) {
	if (results->propertyItem.data.ptr != NULL &&
		results->propertyItem.collection != NULL) {
		COLLECTION_RELEASE(
			results->propertyItem.collection,
			&results->propertyItem);
	}
	releaseIpiList(&results->values);
}

void fiftyoneDegreesResultsIpiFree(fiftyoneDegreesResultsIpi* results) {
	resultsIpiRelease(results);
	freeIpiList(&results->values);
	DataSetRelease((DataSetBase*)results->b.dataSet);
	Free(results);
}

void fiftyoneDegreesResultsIpiFromIpAddress(
	fiftyoneDegreesResultsIpi* results,
	const unsigned char* ipAddress,
	size_t ipAddressLength,
	fiftyoneDegreesIpType type,
	fiftyoneDegreesException* exception) {
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	// Make sure the input is always in the correct format
	if (type == IP_TYPE_INVALID
		|| (type == IP_TYPE_IPV4
			&& ipAddressLength < IPV4_LENGTH)
		|| (type == IP_TYPE_IPV6
			&& ipAddressLength < IPV6_LENGTH)) {
		EXCEPTION_SET(INCORRECT_IP_ADDRESS_FORMAT);
		return;
	}

	resultIpiReset(&results->items[0]);
	// Default IP range offset
	results->items[0].profileOffsetIndex = NULL_PROFILE_OFFSET;
	results->items[0].targetIpAddress.type = type;
	results->items[0].type = type;

	memset(results->items[0].targetIpAddress.value, 0, IPV6_LENGTH);
	if (type == IP_TYPE_IPV4) {
		// We only get the exact length of ipv4
		memcpy(results->items[0].targetIpAddress.value, ipAddress, IPV4_LENGTH);
		// Make sure we only operate on the valid range
		results->items[0].targetIpAddress.length = IPV4_LENGTH;
	}
	else {
		// We only get the exact length of ipv6
		memcpy(results->items[0].targetIpAddress.value, ipAddress, IPV6_LENGTH);
		// Make sure we only operate on the valid range
		results->items[0].targetIpAddress.length = IPV6_LENGTH;
	}
	results->count = 1;

	setResultFromIpAddress(
		&results->items[0],
		dataSet,
		exception);
	if (EXCEPTION_FAILED) {
		return;
	}
}

void fiftyoneDegreesResultsIpiFromIpAddressString(
	fiftyoneDegreesResultsIpi* results,
	const char* ipAddress,
	size_t ipLength,
	fiftyoneDegreesException* exception) {
	IpAddress ip;
	const bool parsed =
		IpAddressParse(ipAddress, ipAddress + ipLength, &ip);
	// Check if the IP address was successfully created
	if (!parsed) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return;
	}
	
	// Perform the search on the IP address byte array
	switch(ip.type) {
	case IP_TYPE_IPV4:
		fiftyoneDegreesResultsIpiFromIpAddress(
			results,
			ip.value,
			IPV4_LENGTH,
			IP_TYPE_IPV4,
			exception);
		break;
	case IP_TYPE_IPV6:
		fiftyoneDegreesResultsIpiFromIpAddress(
			results,
			ip.value,
			IPV6_LENGTH,
			IP_TYPE_IPV6,
			exception);
		break;
	case IP_TYPE_INVALID:
	default:
		EXCEPTION_SET(INCORRECT_IP_ADDRESS_FORMAT);
		break;
	}
}

static bool setResultFromEvidence(
	void* state,
	EvidenceKeyValuePair* pair) {
	stateWithUniqueHeaderIndex* indexState = (stateWithUniqueHeaderIndex*)state;
	stateWithException* exceptionState = (stateWithException*)indexState->subState;
	ResultsIpi* results = (ResultsIpi*)exceptionState->state;
	Exception* exception = exceptionState->exception;
	// We should not look further if a 
	// result has already been found
	if (results->count == 0) {
		ResultIpi* result;
		DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;
		uint32_t curHeaderIndex = indexState->headerIndex;
		int headerIndex = HeaderGetIndex(
			dataSet->b.b.uniqueHeaders,
			pair->header->name,
			pair->header->nameLength);
		// Only the current header index should be considered
		if (headerIndex >= 0 && headerIndex == (int)curHeaderIndex) {
			// Get the parsed Value
			const char *ipAddressString = (const char *)pair->parsedValue;
			// Obtain the byte array first
			IpAddress ipAddress;
			const bool parsed =
				fiftyoneDegreesIpAddressParse(ipAddressString, ipAddressString + strlen(ipAddressString), &ipAddress);
			// Check if the IP address was successfully created
			if (!parsed) {
				EXCEPTION_SET(INSUFFICIENT_MEMORY);
				return false;
			}
			else if(ipAddress.type == IP_TYPE_INVALID) {
				EXCEPTION_SET(INCORRECT_IP_ADDRESS_FORMAT);
				return false;
			}

			// Obtain the correct IP address
			int ipLength = 
				ipAddress.type == IP_TYPE_IPV4 ?
				FIFTYONE_DEGREES_IPV4_LENGTH : 
				FIFTYONE_DEGREES_IPV6_LENGTH;
			// Configure the next result in the array of results.
			result = &((ResultIpi*)results->items)[results->count];
			resultIpiReset(result);
			results->items[0].profileOffsetIndex = NULL_PROFILE_OFFSET; // Default IP range offset
			result->targetIpAddress.length = ipLength;
			memset(result->targetIpAddress.value, 0, IPV6_LENGTH);
			memcpy(result->targetIpAddress.value, ipAddress.value, ipLength);
			memcpy(result->targetIpAddress.value, ipAddress.value, IPV6_LENGTH);
			result->targetIpAddress.type = ipAddress.type;
			result->type = ipAddress.type;
			results->count++;

			setResultFromIpAddress(
				result,
				dataSet,
				exception);
			if (EXCEPTION_FAILED) {
				return false;
			}
		}
	}

	return EXCEPTION_OKAY;
}

static void fiftyoneDegreesIterateHeadersWithEvidence(
	ResultsIpi* const results,
	EvidenceKeyValuePairArray* evidence,
	int prefixes,
	stateWithUniqueHeaderIndex *state) {

	const DataSetIpi * const dataSet = (DataSetIpi *)results->b.dataSet;
	const uint32_t headersCount = dataSet->b.b.uniqueHeaders ? dataSet->b.b.uniqueHeaders->count : 0;

	// Each unique header is checked against the evidence
	// in the order that its added to the headers array.
	// The order represents the prioritis of the headers.
	for (uint32_t i = 0;
		i < headersCount && results->count == 0;
		i++) {
		state->headerIndex = i;
		EvidenceIterate(
			evidence,
			prefixes,
			state,
			setResultFromEvidence);
	}
}

void fiftyoneDegreesResultsIpiFromEvidence(
	fiftyoneDegreesResultsIpi* results,
	fiftyoneDegreesEvidenceKeyValuePairArray* evidence,
	fiftyoneDegreesException* exception) {
	stateWithException subState;
	stateWithUniqueHeaderIndex state;
	subState.state = results;
	subState.exception = exception;
	state.subState = &subState;

	if (evidence != (EvidenceKeyValuePairArray*)NULL) {
		// Reset the results data before iterating the evidence.
		results->count = 0;

		fiftyoneDegreesIterateHeadersWithEvidence(
			results,
			evidence,
			FIFTYONE_DEGREES_EVIDENCE_QUERY,
			&state);
		if (EXCEPTION_FAILED) {
			return;
		}

		// If no results were obtained from the query evidence prefix then use
		// the HTTP headers to populate the results.
		if (results->count == 0) {
			fiftyoneDegreesIterateHeadersWithEvidence(
				results,
				evidence,
				FIFTYONE_DEGREES_EVIDENCE_SERVER,
				&state);
			if (EXCEPTION_FAILED) {
				return;
			}
		}
	}
}

static bool addValueWithPercentage(void* state, Item* item) {
	Item stringItem;
	ProfilePercentage profilePercentageItem;
	/**
	 * The results values are a list of collection items and their percentage
	 * The percentage cannot be passed along with Item as this is the profile
	 * standard in common-cxx. Thus the percentage is passed along with the state
	 */
	stateWithPercentage* percentageState = (stateWithPercentage*)((stateWithException*)state)->state;
	ResultsIpi* results =
		(ResultsIpi*)percentageState->subState;
	Exception* exception = ((stateWithException*)state)->exception;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;
	Value* value = (Value*)item->data.ptr;
	if (value != NULL && results->values.count < results->values.capacity) {
		DataReset(&stringItem.data);
		if (StringGet(
			dataSet->strings,
			value->nameOffset,
			&stringItem,
			exception) != NULL && EXCEPTION_OKAY) {
			profilePercentageItem.item = stringItem;
			profilePercentageItem.rawWeighting = percentageState->rawWeighting;
			addIpiListItem(&results->values, &profilePercentageItem);
		}
	}
	COLLECTION_RELEASE(dataSet->values, item);
	return EXCEPTION_OKAY;
}

static uint32_t addValuesFromProfile(
	DataSetIpi* dataSet,
	ResultsIpi* results,
	Profile* profile,
	Property* property,
	uint16_t rawWeighting,
	Exception* exception) {
	uint32_t count;

	// Set the state for the callbacks.
	stateWithException state;
	stateWithPercentage percentageState;
	percentageState.subState = results;
	percentageState.rawWeighting = rawWeighting;
	state.state = &percentageState;
	state.exception = exception;

	// Iterate over the values associated with the property adding them
	// to the list of values. Get the number of values available as 
	// this will be used to increase the size of the list should there
	// be insufficient space.
	count = ProfileIterateValuesForProperty(
		dataSet->values,
		profile,
		property,
		&state,
		addValueWithPercentage,
		exception);
	EXCEPTION_THROW;

	// The count of values should always be lower or the same as the profile
	// count. i.e. there can never be more values counted than there are values
	// against the profile.
	assert(count <= profile->valueCount);

	return count;
}

/*
 * Caller should check for EXCEPTION_OKAY before accessing
 * the returned item
 * @param results result array
 * @param valueOffset the offset in the values collection
 * @param exception the exception object that will be used
 * if an exception occurs
 * @return the item from the strings collection
 */
static Item getStringItemByValueOffset(
	ResultsIpi* results,
	uint32_t valueOffset,
	Exception* exception) {
	Item valueItem, stringItem;
	Item returnedItem;
	Value* value;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	// Initialise the returned item
	DataReset(&returnedItem.data);
	returnedItem.collection = NULL;
	returnedItem.handle = NULL;

	DataReset(&valueItem.data);
	if ((value = dataSet->values->get(
		dataSet->values,
		valueOffset,
		&valueItem,
		exception)) != NULL && EXCEPTION_OKAY) {
		DataReset(&stringItem.data);
		if (StringGet(
			dataSet->strings,
			value->nameOffset,
			&stringItem,
			exception) != NULL && EXCEPTION_OKAY) {
			returnedItem = stringItem;
		}
		COLLECTION_RELEASE(dataSet->values, &valueItem);
	}
	return returnedItem;
}

static uint32_t addValuesFromSingleProfile(
	ResultsIpi* results,
	Property *property,
	uint32_t profileOffset,
	uint16_t rawWeighting,
	Exception* exception) {
	uint32_t count = 0;
	Item profileItem;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	// Add values from profiles
	Profile *profile = NULL;
	if (profileOffset != NULL_PROFILE_OFFSET) {
		DataReset(&profileItem.data);
		profile = (Profile*)dataSet->profiles->get(
			dataSet->profiles,
			profileOffset,
			&profileItem,
			exception);
		// If profile is found
		if (profile != NULL && EXCEPTION_OKAY) {
			count += addValuesFromProfile(
				dataSet,
				results,
				profile,
				property,
				rawWeighting,
				exception);
			COLLECTION_RELEASE(dataSet->profiles, &profileItem);
		}
	}
	return count;
}

static uint32_t addValuesFromProfileGroup(
	ResultsIpi* results,
	Property *property,
	uint32_t profileGroupOffset,
	Exception* exception) {
	uint32_t count = 0;
	offsetPercentage *weightedProfileOffset = NULL;
	Item profileGroupItem;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	if (profileGroupOffset != NULL_PROFILE_OFFSET) {
		DataReset(&profileGroupItem.data);
		weightedProfileOffset = (offsetPercentage*)dataSet->profileGroups->get(
			dataSet->profileGroups,
			profileGroupOffset,
			&profileGroupItem,
			exception);
		if (weightedProfileOffset != NULL && EXCEPTION_OKAY) {
			for (uint32_t totalWeight = 0;
				totalWeight < FULL_RAW_WEIGHTING;
				++weightedProfileOffset) {
				totalWeight += weightedProfileOffset->rawWeighting;
				if (totalWeight > FULL_RAW_WEIGHTING) {
					EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_CORRUPT_DATA);
					break;
				}
				count += addValuesFromSingleProfile(
					results,
					property,
					weightedProfileOffset->offset,
					weightedProfileOffset->rawWeighting,
					exception);
			}
			COLLECTION_RELEASE(dataSet->profiles, &profileGroupItem);
		}
	}

	return count;
}

static uint32_t addValuesFromResult(
	ResultsIpi* results,
	ResultIpi* result,
	Property* property,
	Exception* exception) {
	uint32_t count = 0;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	if (results->count > 0) {
		if (result->profileOffsetIndex != NULL_PROFILE_OFFSET) {
			const int32_t profileOffsetValue = CollectionGetInteger32(
				dataSet->profileOffsets, result->profileOffsetIndex, exception);

			if (EXCEPTION_OKAY) {
				if (profileOffsetValue >= 0) {
					count += addValuesFromSingleProfile(
						results,
						property,
						profileOffsetValue,
						FULL_RAW_WEIGHTING,
						exception);
				} else {
					const uint32_t groupOffset = -1LL - profileOffsetValue;
					count += addValuesFromProfileGroup(
						results,
						property,
						groupOffset,
						exception);
				}
			}
		}
	}
	return count;
}

static ProfilePercentage* getValuesFromResult(
	ResultsIpi* results,
	ResultIpi* result,
	Property* property,
	Exception* exception) {
	// There is a profile available for the property requested. 
	// Use this to add the values to the results.
	addValuesFromResult(results, result, property, exception);

	// Return the first value in the list of items.
	return results->values.items;
}

typedef fiftyoneDegreesPropertyValueType PropertyValueType;

// TODO: Remove or deduplicate from `ResultsIpi::getPropertyValueType`
static PropertyValueType
getPropertyValueType(
	const DataSetIpi * const dataSet,
	ResultsIpi* const results,
	const int requiredPropertyIndex,
	fiftyoneDegreesException * const exception) {
	// Default to string type. Consumers of
	// this function should always check for exception status
	PropertyValueType valueType
		= FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING;

	// Work out the property index from the required property index.
	const uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure.
	// This may also be needed to work out which of a selection of results
	// are used to obtain the values.
	const Property * const property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);
	if (property != NULL && EXCEPTION_OKAY) {
		valueType = (PropertyValueType)property->valueType;
	}
	return valueType;
}

const fiftyoneDegreesProfilePercentage* fiftyoneDegreesResultsIpiGetValues(
	fiftyoneDegreesResultsIpi* const results,
	int const requiredPropertyIndex,
	fiftyoneDegreesException* const exception) {
	Property* property;
	DataSetIpi* dataSet;
	ProfilePercentage* firstValue = NULL;

	// Ensure any previous uses of the results to get values are released.
	resultsIpiRelease(results);

	dataSet = (DataSetIpi*)results->b.dataSet;

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	if (propertyIndex >= 0) {
		// Set the property that will be available in the results structure. 
		// This may also be needed to work out which of a selection of results 
		// are used to obtain the values.
		property = PropertyGet(
			dataSet->properties,
			propertyIndex,
			&results->propertyItem,
			exception);

		if (property != NULL && EXCEPTION_OKAY) {
			// Ensure there is a collection available to the property item so
			// that it can be freed when the results are freed.
			if (results->propertyItem.collection == NULL) {
				results->propertyItem.collection = dataSet->properties;
			}

			// There will only be one result
			if (results->count > 0 && EXCEPTION_OKAY) {
				firstValue = getValuesFromResult(
					results,
					results->items,
					property,
					exception);
			}
		}
	}

	if (firstValue == NULL) {
		// There are no values for the property requested. Reset the values 
		// list to zero count.
		releaseIpiList(&results->values);
	}
	return firstValue;
}

static bool visitProfilePropertyValue(
	void *state,
	fiftyoneDegreesCollectionItem *item) {
	(void)item; // suppress C4100 "unused formal parameter"

	*((bool *)state) = true; // found
	return false; // break
}

static bool profileHasValidPropertyValue(
	const DataSetIpi * const dataSet,
	const uint32_t profileOffset,
	Property * const property,
	Exception * const exception) {
	Item profileItem;
	Profile *profile = NULL;
	bool valueFound = false;

	if (profileOffset != NULL_PROFILE_OFFSET) {
		DataReset(&profileItem.data);
		profile = (Profile*)dataSet->profiles->get(
			dataSet->profiles,
			profileOffset,
			&profileItem,
			exception);
		// If profile is found
		if (profile != NULL && EXCEPTION_OKAY) {
			ProfileIterateValuesForProperty(
				dataSet->values,
				profile,
				property,
				&valueFound,
				visitProfilePropertyValue,
				exception);
			COLLECTION_RELEASE(dataSet->profiles, &profileItem);
		}
	}
	return valueFound;
}

static bool resultGetHasValidPropertyValueOffset(
	fiftyoneDegreesResultsIpi* const results,
	const fiftyoneDegreesResultIpi* const result,
	const int requiredPropertyIndex,
	fiftyoneDegreesException* const exception) {
	bool hasValidOffset = false;
	Item item;
	DataReset(&item.data);
	const DataSetIpi * const dataSet = (DataSetIpi*)results->b.dataSet;

	// Work out the property index from the required property index.
	const int32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	if (propertyIndex >= 0) {
		// Set the property that will be available in the results structure.
		// This may also be needed to work out which of a selection of results
		// are used to obtain the values.
		Property * const property = PropertyGet(
			dataSet->properties,
			propertyIndex,
			&results->propertyItem,
			exception);

		const char * const propertyName = STRING(
			PropertiesGetNameFromRequiredIndex(
				dataSet->b.b.available,
				requiredPropertyIndex));
		if (propertyName != NULL && EXCEPTION_OKAY) {
			// We will only execute this step if successfully obtained the
			// profile groups offset from the previous step
			if (result->profileOffsetIndex != NULL_PROFILE_OFFSET) {
				const int32_t profileOffsetValue = CollectionGetInteger32(
					dataSet->profileOffsets, result->profileOffsetIndex, exception);

				if (profileOffsetValue != NULL_PROFILE_OFFSET && EXCEPTION_OKAY) {
					if (profileOffsetValue >= 0) {
						hasValidOffset = profileHasValidPropertyValue(
							dataSet, profileOffsetValue, property, exception);
					} else {
						const uint32_t profileGroupOffset = 0LL - profileOffsetValue;
						offsetPercentage *weightedProfileOffset =
							(offsetPercentage*)dataSet->profileGroups->get(
								dataSet->profileGroups,
								profileGroupOffset,
								&item,
								exception);
						if (weightedProfileOffset && EXCEPTION_OKAY) {
							for (uint32_t totalWeight = 0;
								!hasValidOffset && totalWeight < FULL_RAW_WEIGHTING;
								++weightedProfileOffset) {
								totalWeight += weightedProfileOffset->rawWeighting;
								if (totalWeight > FULL_RAW_WEIGHTING) {
									EXCEPTION_SET(FIFTYONE_DEGREES_STATUS_CORRUPT_DATA);
									break;
								}
								hasValidOffset = profileHasValidPropertyValue(
									dataSet, weightedProfileOffset->offset, property, exception);
							}
							COLLECTION_RELEASE(dataSet->profiles, &item);
						}
					}
				}
				else {
					EXCEPTION_SET(CORRUPT_DATA);
				}
			}
		}
	}
	return hasValidOffset;
}

bool fiftyoneDegreesResultsIpiGetHasValues(
	fiftyoneDegreesResultsIpi* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException* exception) {
	DataSetIpi *dataSet = (DataSetIpi*)results->b.dataSet;
	// Ensure any previous uses of the results to get values are released.
	resultsIpiRelease(results);

	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		// The property index is not valid.
		return false;
	}

	if (results->count == 0)
		// There is no result
		return false;

	// There will only be one result
	bool hasValidOffset = resultGetHasValidPropertyValueOffset(
		results,
		results->items,
		requiredPropertyIndex,
		exception);
	if (!hasValidOffset || EXCEPTION_FAILED) {
		return false;
	}

	// None of the checks have returned false, so there must be valid values.
	return true;
}

fiftyoneDegreesResultsNoValueReason fiftyoneDegreesResultsIpiGetNoValueReason(
	fiftyoneDegreesResultsIpi* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException* exception) {
	DataSetIpi *dataSet = (DataSetIpi*)results->b.dataSet;
	// Ensure any previous uses of the results to get values are released.
	resultsIpiRelease(results);

	if (requiredPropertyIndex < 0 ||
		requiredPropertyIndex >= (int)dataSet->b.b.available->count) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_INVALID_PROPERTY;
	}

	if (results->count == 0) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS;
	}

	// There will only be one result
	bool hasValidOffset = resultGetHasValidPropertyValueOffset(
		results,
		results->items,
		requiredPropertyIndex,
		exception);
	if (EXCEPTION_OKAY && !hasValidOffset) {
		return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE;
	}

	return FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN;
}

const char* fiftyoneDegreesResultsIpiGetNoValueReasonMessage(
	fiftyoneDegreesResultsNoValueReason reason) {
	switch (reason) {
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS:
		return "The results are empty. This is probably because we don't "
			"have this data in our database.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE:
		return "The results contained a null profile for the component which "
			"the required property belongs to.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN:
	default:
		return "The reason for missing values is unknown.";
	}
}

static void pushValues(
	const ProfilePercentage * const profilePercentage,
	const uint32_t count,
	StringBuilder * const builder,
	const char * const separator,
	const uint8_t decimalPlaces,
	Exception * const exception) {

	const size_t sepLen = strlen(separator);

	// Loop through the values adding them to the string buffer.
	for (uint32_t i = 0; i < count;  i++) {
		// Append the separator
		if (i) {
			StringBuilderAddChars(builder, separator, sepLen);
		}

		// Add the opening quote
		StringBuilderAddChar(builder, '"');

		// Get the string for the value index.
		const String * const string =
			(const String*)profilePercentage[i].item.data.ptr;

		// Add the string to the output buffer recording the number
		// of characters added.
		StringBuilderAddStringValue(builder, string, decimalPlaces, exception);

		// Add the closing quote
		StringBuilderAddChar(builder, '"');
		StringBuilderAddChar(builder, ':');
		StringBuilderAddDouble(
			builder,
			(float)profilePercentage[i].rawWeighting / 65535.f,
			decimalPlaces);
	}
}

static void fiftyoneDegreesResultsIpiGetValuesStringInternal(
	fiftyoneDegreesResultsIpi* results,
	int requiredPropertyIndex,
	StringBuilder * const builder,
	const char* separator,
	fiftyoneDegreesException* exception) {
	const ProfilePercentage *profilePercentage;
	Item propertyItem;
	Property *property;
	DataSetIpi *dataSet = (DataSetIpi *)results->b.dataSet;

	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	if (propertyIndex >= 0) {
		DataReset(&propertyItem.data);
		property = (Property*)dataSet->properties->get(
				dataSet->properties,
				propertyIndex,
				&propertyItem,
				exception);
		if (property != NULL && EXCEPTION_OKAY) {
			if (requiredPropertyIndex >= 0) {
				profilePercentage = fiftyoneDegreesResultsIpiGetValues(
					results,
					requiredPropertyIndex,
					exception);
				if (profilePercentage != NULL && EXCEPTION_OKAY) {
					pushValues(
							profilePercentage,
							results->values.count,
							builder,
							separator,
							DefaultWktDecimalPlaces,
							exception);
				}
			}
			COLLECTION_RELEASE(dataSet->properties, &propertyItem);
		}
	}
}

void fiftyoneDegreesResultsIpiAddValuesString(
	fiftyoneDegreesResultsIpi* results,
	const char* propertyName,
	fiftyoneDegreesStringBuilder *builder,
	const char* separator,
	fiftyoneDegreesException* exception) {
	const DataSetIpi * const dataSet = (DataSetIpi *)results->b.dataSet;
	const int requiredPropertyIndex = PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		propertyName);

	if (requiredPropertyIndex >= 0) {
		fiftyoneDegreesResultsIpiGetValuesStringInternal(
			results,
			requiredPropertyIndex,
			builder,
			separator,
			exception);
	}
}

size_t fiftyoneDegreesResultsIpiGetValuesString(
	fiftyoneDegreesResultsIpi* results,
	const char* propertyName,
	char* buffer,
	size_t bufferLength,
	const char* separator,
	fiftyoneDegreesException* exception) {

	StringBuilder builder = { buffer, bufferLength };
	StringBuilderInit(&builder);

	fiftyoneDegreesResultsIpiAddValuesString(
		results,
		propertyName,
		&builder,
		separator,
		exception);

	StringBuilderComplete(&builder);

	return builder.added;
}

size_t fiftyoneDegreesResultsIpiGetValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsIpi* results,
	const int requiredPropertyIndex,
	char* buffer,
	size_t bufferLength,
	const char* separator,
	fiftyoneDegreesException* exception) {

	StringBuilder builder = { buffer, bufferLength };
	StringBuilderInit(&builder);

	fiftyoneDegreesResultsIpiGetValuesStringInternal(
		results,
		requiredPropertyIndex,
		&builder,
		separator,
		exception);

	StringBuilderComplete(&builder);

	return builder.added;
}

/*
 * Supporting Macros to printout the NetworkId
 */
#define PRINT_PROFILE_SEP(d,b,s,f) printProfileSeparator(&d, (b - d) + s, f)
#define PRINT_PROFILE_ID(d,b,s,f,v,p) printProfileId(&d, (b - d) + s, f, v, p)
#define PRINT_NULL_PROFILE_ID(d,b,s,p) PRINT_PROFILE_ID(d, b, s, "%i:%f", 0, p)

size_t fiftyoneDegreesIpiGetIpAddressAsString(
	const fiftyoneDegreesCollectionItem * const item,
	const fiftyoneDegreesIpType type,
	char * const buffer,
	const uint32_t bufferLength,
	fiftyoneDegreesException * const exception) {

	StringBuilder builder = { buffer, bufferLength };
	StringBuilderInit(&builder);

	StringBuilderAddIpAddress(
		&builder,
		(const String *)item->data.ptr,
		type,
		exception);

	return builder.added;
}

uint32_t fiftyoneDegreesIpiIterateProfilesForPropertyAndValue(
	fiftyoneDegreesResourceManager* manager,
	const char* propertyName,
	const char* valueName,
	void* state,
	fiftyoneDegreesProfileIterateMethod callback,
	fiftyoneDegreesException* exception) {
	uint32_t count = 0;
	DataSetIpi* dataSet = DataSetIpiGet(manager);
	count = ProfileIterateProfilesForPropertyAndValue(
		dataSet->strings,
		dataSet->properties,
		dataSet->values,
		dataSet->profiles,
		dataSet->profileOffsets,
		propertyName,
		valueName,
		state,
		callback,
		exception);
	DataSetIpiRelease(dataSet);
	return count;
}