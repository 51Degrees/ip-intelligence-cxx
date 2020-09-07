/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2020 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
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
/** Default valus separator */
#define DEFAULT_VALUES_SEPARATOR "|"

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
	Float percentage;
} stateWithPercentage;

typedef struct state_with_ip_version_t {
	void* subState; /* Pointer to a data set or other information */
	fiftyoneDegreesEvidenceIpType type; /* Version of IP */
} stateWithIpVersion;

typedef struct profile_combination_component_index_t {
	uint16_t index; /* Index to the first profile of the component
					in the profiles list */
	uint16_t count; /* The number of profiles presents for that
					component */
} componentIndex;

typedef struct offset_percentage_t {
	uint32_t offset; /* Offset to a collection item */
	Float percentage; /* The weight of the item in the matched IP range */
} offsetPercentage;

/*
 * The structure of a string item containing
 * a pair of float values
 */
typedef struct float_pair_t {
	uint16_t length; /* Length of the data block */
	unsigned char type; /* Type of the data block */
	Float first; /* First float value */
	Float second; /* Second float value */
} floatPair;

/**
 * PRESET IP INTELLIGENCE CONFIGURATIONS
 */

/* The expected version of the data file */
// #define FIFTYONE_DEGREES_IPI_TARGET_VERSION_MAJOR 4
// #define FIFTYONE_DEGREES_IPI_TARGET_VERSION_MINOR 1
#define FIFTYONE_DEGREES_IPI_TARGET_VERSION_MAJOR 1
#define FIFTYONE_DEGREES_IPI_TARGET_VERSION_MINOR 2

#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY true
fiftyoneDegreesConfigIpi fiftyoneDegreesIpiInMemoryConfig = {
	{FIFTYONE_DEGREES_CONFIG_DEFAULT},
	{0,0,0}, // Strings
	{0,0,0}, // Components
	{0,0,0}, // Maps
	{0,0,0}, // Properties
	{0,0,0}, // Values
	{0,0,0}, // Profiles
	{0,0,0}, // Ipv4 Ranges
	{0,0,0}, // Ipv6 Ranges
	{0,0,0}, // Profile Combinations
	{0,0,0}  // ProfileOffsets
};
#undef FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY
#define FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY \
FIFTYONE_DEGREES_CONFIG_ALL_IN_MEMORY_DEFAULT

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiHighPerformanceConfig = {
	{ FIFTYONE_DEGREES_CONFIG_DEFAULT },
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Ipv4 Ranges
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Ipv6 Ranges
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profile Combinations
	{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }  // ProfileOffsets
};

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiLowMemoryConfig = {
	{ FIFTYONE_DEGREES_CONFIG_DEFAULT },
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Ipv4 Ranges
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Ipv6 Ranges
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profile Combinations
	{ 0, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }  // ProfileOffsets
};

fiftyoneDegreesConfigIpi fiftyoneDegreesIpiSingleLoadedConfig = {
	{ FIFTYONE_DEGREES_CONFIG_DEFAULT },
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Strings
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Components
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Maps
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Properties
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Values
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profiles
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Ipv4 Ranges
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Ipv6 Ranges
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, // Profile Combinations
	{ 1, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }  // ProfileOffsets
};

#define FIFTYONE_DEGREES_IPI_CONFIG_BALANCED \
{ FIFTYONE_DEGREES_CONFIG_DEFAULT }, \
{ FIFTYONE_DEGREES_STRING_LOADED, FIFTYONE_DEGREES_STRING_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Strings */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Components */ \
{ INT_MAX, 0, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Maps */ \
{ FIFTYONE_DEGREES_PROPERTY_LOADED, FIFTYONE_DEGREES_PROPERTY_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Properties */ \
{ FIFTYONE_DEGREES_VALUE_LOADED, FIFTYONE_DEGREES_VALUE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Values */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Profiles */ \
{ FIFTYONE_DEGREES_IP_V4_RANGE_LOADED, FIFTYONE_DEGREES_IP_V4_RANGE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Ipv4 Ranges */ \
{ FIFTYONE_DEGREES_IP_V6_RANGE_LOADED, FIFTYONE_DEGREES_IP_V6_RANGE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Ipv6 Ranges */ \
{ FIFTYONE_DEGREES_PROFILE_COMBINATION_LOADED, FIFTYONE_DEGREES_PROFILE_COMBINATION_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY }, /* Profile Combinations */ \
{ FIFTYONE_DEGREES_PROFILE_LOADED, FIFTYONE_DEGREES_PROFILE_CACHE_SIZE, FIFTYONE_DEGREES_CACHE_CONCURRENCY } /* ProfileOffsets */ \

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
	result->targetIpAddress.type = FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_INVALID;
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
		EXCEPTION_SET(INCORRECT_FORMAT);
		result = 0;
	}
	else {
		// We will terminate if IP address is within the range between the current item and the next item
		int tempResult = compareIpAddresses(((Ipv4Range*)item->data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV4_LENGTH);
		if (tempResult == 0) {
			result = 0;
		}
		else if (tempResult < 0) {
			Item nextItem;
			DataReset(&nextItem.data);
			if ((uint32_t)curIndex > item->collection->count - 1) {
				// Target IP address is out of the maximum upper bound for and IP address
				EXCEPTION_SET(COLLECTION_INDEX_OUT_OF_RANGE);
				result = 0;
			}
			else {
				if ((uint32_t)curIndex + 1 == item->collection->count) {
					// There is no more item for the IP address
					// Use the current result instead
					result = 0;
				}
				else if (item->collection->get(
					item->collection,
					++curIndex,
					&nextItem,
					exception) != NULL && EXCEPTION_OKAY) {
					if (compareIpAddresses(((Ipv4Range*)nextItem.data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV4_LENGTH) > 0) {
						// The target IP address is within the range between the current and next items
						result = 0;
					}
					else {
						// The IP address is not within the range
						result = -1;
					}
					COLLECTION_RELEASE(item->collection, &nextItem);
				}
				else {
					// Error occured
					result = 0;
				}
			}
		}
		else if (curIndex > 0) {
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
		EXCEPTION_SET(INCORRECT_FORMAT);
		result = 0;
	}
	else {
		// We will terminate if IP address is within the range between the current item and the next item
		int tempResult = compareIpAddresses(((Ipv6Range*)item->data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV6_LENGTH);
		if (tempResult == 0) {
			result = 0;
		}
		else if (tempResult < 0) {
			Item nextItem;
			DataReset(&nextItem.data);
			if ((uint32_t)curIndex > item->collection->count - 1) {
				// Target IP address is out of the maximum upper bound for and IP address
				EXCEPTION_SET(COLLECTION_INDEX_OUT_OF_RANGE);
				result = 0;
			}
			else {
				if ((uint32_t)curIndex + 1 == item->collection->count) {
					// There is no more item for the IP address
					// Use the current result instead
					result = 0;
				}
				else if (item->collection->get(
					item->collection,
					++curIndex,
					&nextItem,
					exception) != NULL && EXCEPTION_OKAY) {
					if (compareIpAddresses(((Ipv6Range*)nextItem.data.ptr)->start, target.value, FIFTYONE_DEGREES_IPV6_LENGTH) > 0) {
						// The target IP address is within the range between the current and next items
						result = 0;
					}
					else {
						// The IP address is not within the range
						result = -1;
					}
					COLLECTION_RELEASE(item->collection, &nextItem);
				}
				else {
					// Error occured
					result = 0;
				}
			}
		}
		else if (curIndex > 0) {
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
	Item item;
	DataReset(&item.data);
	switch (result->targetIpAddress.type) {
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4:
		result->ipRangeOffset = CollectionBinarySearch(
			dataSet->ipv4Ranges,
			&item,
			0,
			CollectionGetCount(dataSet->ipv4Ranges) - 1,
			(void*)&result->targetIpAddress,
			compareToIpv4Range,
			exception);
		COLLECTION_RELEASE(dataSet->ipv4Ranges, &item);
		break;
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6:
	default:
		result->ipRangeOffset = CollectionBinarySearch(
			dataSet->ipv6Ranges,
			&item,
			0,
			CollectionGetCount(dataSet->ipv6Ranges) - 1,
			(void*)&result->targetIpAddress,
			compareToIpv6Range,
			exception);
		COLLECTION_RELEASE(dataSet->ipv6Ranges, &item);
		break;
		break;
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
	dataSet->ipv4Ranges = NULL;
	dataSet->ipv6Ranges = NULL;
	dataSet->profileCombinations = NULL;
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

	// Free the memory used for the lists and collections.
	ListFree(&dataSet->componentsList);
	Free(dataSet->componentsAvailable);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->strings);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->components);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->properties);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->maps);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->values);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profiles);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->ipv4Ranges);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->ipv6Ranges);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profileCombinations);
	FIFTYONE_DEGREES_COLLECTION_FREE(dataSet->profileOffsets);

	// Finally free the memory used by the resource itself as this is always
	// allocated within the IP Intelligence init manager method.
	Free(dataSet);
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

int findPropertyName(
	Collection* properties,
	Collection* strings,
	const char* name,
	Exception* exception) {
	int i;
	Property* property;
	String* propertyName;
	Item propertyItem, nameItem;
	int count = CollectionGetCount(properties);
	DataReset(&propertyItem.data);
	DataReset(&nameItem.data);
	for (i = 0; i < count; i++) {
		property = PropertyGet(
			properties,
			i,
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
				if (StringCompare(name, STRING(propertyName)) == 0) {
					COLLECTION_RELEASE(strings, &nameItem);
					COLLECTION_RELEASE(properties, &propertyItem);
					return i;
				}
				COLLECTION_RELEASE(strings, &nameItem);
			}
			COLLECTION_RELEASE(properties, &propertyItem);
		}
	}
	return -1;
}

uint32_t initGetEvidenceProperties(
	void* state,
	fiftyoneDegreesPropertyAvailable* availableProperty,
	fiftyoneDegreesEvidenceProperties* evidenceProperties) {
	int count = 0;
	int index;
	Item propertyItem, stringItem;
	Component* component;
	String* name;
	char* jsName;
	DataSetIpi* dataSet = (DataSetIpi*)((stateWithException*)state)->state;
	Exception* exception = ((stateWithException*)state)->exception;

	DataReset(&propertyItem.data);

	// First get the property to check its component.
	Property* property = PropertyGet(
		dataSet->properties,
		availableProperty->propertyIndex,
		&propertyItem,
		exception);
	if (property != NULL && EXCEPTION_OKAY) {
		// Get the name of the component which the property belongs to.
		component = (Component*)dataSet->componentsList.items[property->componentIndex].data.ptr;

		DataReset(&stringItem.data);
		name = StringGet(
			dataSet->strings,
			component->nameOffset,
			&stringItem,
			exception);
		if (name != NULL && EXCEPTION_OKAY) {
			// The property and its component exist
			COLLECTION_RELEASE(dataSet->strings, &stringItem);
		}
		COLLECTION_RELEASE(dataSet->properties, &propertyItem);
	}

	// Only carry on doing this if there was no exception from the section
	// above.
	if (EXCEPTION_OKAY) {
		name = (String*)availableProperty->name.data.ptr;
		// Allocate some space to set the name of a target property. This
		// follows the convension of a 'JavaScript' suffix. For example, a
		// property named 'CountryJavaScript' would contain JavaScript which
		// produced evidence for the 'Country' property.
		// For the scope of IP Intelligence this is not required. But it is
		// good to have its in place for future supports.
		jsName = (char*)Malloc(sizeof(char) * (name->size + strlen("javascript") + 1));
		if (jsName != NULL) {
			// Construct the name to look for.
			strcpy(jsName, &name->value);
			strcpy(jsName + name->size - 1, "javascript");
			// Do a sequential search for the property name just constructed.
			// Ideally this would be a binary search, but the properties are not
			// guaranteed to be in alphabetical order. Besides, this method only
			// runs at startup and the number of properties is relatively
			// small, so performance is not such an issue.
			index = findPropertyName(
				dataSet->properties,
				dataSet->strings,
				jsName,
				exception);
			if (EXCEPTION_OKAY) {
				if (index >= 0) {
					if (evidenceProperties != NULL) {
						// Add the index if the structure has been allocated.
						evidenceProperties->items[count] = index;
					}
					count++;
				}
			}
			Free(jsName);
		}
		else {
			EXCEPTION_SET(INSUFFICIENT_MEMORY);
		}
	}
	return count;
}

static StatusCode initProperties(
	DataSetIpi* dataSet,
	PropertiesRequired* properties,
	Exception* exception) {
	stateWithException state;
	state.state = (void*)dataSet;
	state.exception = exception;
	StatusCode status = DataSetInitProperties(
		&dataSet->b.b,
		properties,
		&state,
		initGetPropertyString,
		initGetEvidenceProperties);
	if (status != SUCCESS) {
		return status;
	}

	return status;
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

	COLLECTION_CREATE_MEMORY(ipv4Ranges);
	COLLECTION_CREATE_MEMORY(ipv6Ranges);

	uint32_t nodesCount = dataSet->header.profileCombinations.count;
	*(uint32_t*)(&dataSet->header.profileCombinations.count) = 0;
	COLLECTION_CREATE_MEMORY(profileCombinations)
	*(uint32_t*)(&dataSet->header.profileCombinations.count) = nodesCount;

	COLLECTION_CREATE_MEMORY(profileOffsets)

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

/**
 * Get the actual size of the profile combination by using
 * its intial value.
 * @param initial the intial value of a profile combination after read from file
 * @return the actual size of the profile combination
 */
static uint32_t getProfileCombinationFinalSize(void* initial) {
	return (uint32_t)(sizeof(int16_t) + (*(int16_t*)initial));
}

void* fiftyoneDegreesProfileCombinationReadFromFile(
	const fiftyoneDegreesCollectionFile* file,
	uint32_t offset,
	fiftyoneDegreesData* data,
	fiftyoneDegreesException* exception) {
	ProfileCombination profileCombination = { 0, 0, 0, 0 };
	return CollectionReadFileVariable(
		file,
		data,
		offset,
		&profileCombination,
		sizeof(uint16_t), // Read the item header
		getProfileCombinationFinalSize,
		exception);
}

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

	COLLECTION_CREATE_FILE(ipv4Ranges, CollectionReadFileFixed);
	COLLECTION_CREATE_FILE(ipv6Ranges, CollectionReadFileFixed);

	uint32_t profileCombinationCount = dataSet->header.profileCombinations.count;
	*(uint32_t*)(&dataSet->header.profileCombinations.count) = 0;
	COLLECTION_CREATE_FILE(profileCombinations, fiftyoneDegreesProfileCombinationReadFromFile);
	*(uint32_t*)(&dataSet->header.profileCombinations.count) = profileCombinationCount;

	COLLECTION_CREATE_FILE(profileOffsets, CollectionReadFileFixed);

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
	MAX_CONCURRENCY(ipv4Ranges);
	MAX_CONCURRENCY(ipv6Ranges);
	MAX_CONCURRENCY(profileCombinations);
	MAX_CONCURRENCY(profileOffsets);
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

	// Return the status code if comthing has gone wrong.
	if (status != SUCCESS || EXCEPTION_FAILED) {
		// Delete the temp file if one has been created.
		if (config->b.useTempFile == true) {
			FileDelete(dataSet->b.b.fileName);
		}
		return status;
	}

	// Initialise the required properties and headers and check the 
	// initialisation was successful.
	status = initProperties(dataSet, properties, exception);
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
	status = initProperties(dataSet, properties, exception);
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

static void resizeIpiList(
	fiftyoneDegreesIpiList* list,
	uint32_t newCapacity) {
	// Allocate new list
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

static void addIpiListItem(
	fiftyoneDegreesIpiList* list,
	fiftyoneDegreesProfilePercentage* item) {
	assert(list->count < list->capacity);
	assert(item->item.collection != NULL);
	list->items[list->count++] = *item;
	// Check if the list has reached its load factor
	if ((float)(list->count / list->capacity) > list->loadFactor) {
		// Get new capacity
		uint32_t newCapacity =
			(uint32_t)ceilf(list->capacity * IPI_LIST_RESIZE_FACTOR);

		resizeIpiList(list, newCapacity);
	}
}

//static fiftyoneDegreesString* fiftyoneDegreesListGetAsString(
//	fiftyoneDegreesList* list,
//	int index) {
//	return (String*)list->items[index].data.ptr;
//}

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
	fiftyoneDegreesEvidenceIpType type,
	fiftyoneDegreesException* exception) {
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	// Make sure the input is always in the correct format
	if (type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_INVALID
		|| (type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4 
			&& ipAddressLength < FIFTYONE_DEGREES_IPV4_LENGTH)
		|| (type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6 
			&& ipAddressLength < FIFTYONE_DEGREES_IPV6_LENGTH)) {
		EXCEPTION_SET(INCORRECT_FORMAT);
		return;
	}

	resultIpiReset(&results->items[0]);
	results->items[0].ipRangeOffset = NULL_PROFILE_OFFSET; // Default IP range offset
	results->items[0].targetIpAddress.type = type;
	results->items[0].type = type;

	memset(results->items[0].targetIpAddress.value, 0, FIFTYONE_DEGREES_IPV6_LENGTH);
	if (type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
		// We only get the exact length of ipv4
		memcpy(results->items[0].targetIpAddress.value, ipAddress, FIFTYONE_DEGREES_IPV4_LENGTH);
		// Make sure we only operate on the valid range
		results->items[0].targetIpAddress.length = FIFTYONE_DEGREES_IPV4_LENGTH;
	}
	else {
		// We only get the exact length of ipv6
		memcpy(results->items[0].targetIpAddress.value, ipAddress, FIFTYONE_DEGREES_IPV6_LENGTH);
		// Make sure we only operate on the valid range
		results->items[0].targetIpAddress.length = FIFTYONE_DEGREES_IPV6_LENGTH;
	}
	results->count = 1;

	if (results != (ResultsIpi*)NULL) {
		setResultFromIpAddress(
			&results->items[0],
			dataSet,
			exception);
		if (EXCEPTION_FAILED) {
			return;
		}
	}
}

void fiftyoneDegreesResultsIpiFromIpAddressString(
	fiftyoneDegreesResultsIpi* results,
	const char* ipAddress,
	size_t ipLength,
	fiftyoneDegreesException* exception) {
	fiftyoneDegreesEvidenceIpAddress *ip = 
		fiftyoneDegreesIpParseAddress(malloc, ipAddress, ipAddress + ipLength);
	// Check if the IP address was successfully created
	if (ip == NULL) {
		EXCEPTION_SET(INSUFFICIENT_MEMORY);
		return;
	}
	
	// Perform the search on the IP address byte array
	switch(ip->type) {
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4:
		fiftyoneDegreesResultsIpiFromIpAddress(
			results,
			ip->address,
			FIFTYONE_DEGREES_IPV4_LENGTH,
			FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4,
			exception);
		break;
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6:
		fiftyoneDegreesResultsIpiFromIpAddress(
			results,
			ip->address,
			FIFTYONE_DEGREES_IPV6_LENGTH,
			FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6,
			exception);
		break;
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_INVALID:
	default:
		EXCEPTION_SET(INCORRECT_FORMAT);
		break;
	}
	// Free parsed IP address
	free(ip);
}

static bool setResultFromEvidence(
	void* state,
	EvidenceKeyValuePair* pair) {
	stateWithIpVersion *ipState = (stateWithIpVersion *)((stateWithException*)state)->state;
	ResultIpi* result;
	ResultsIpi* results =
		(ResultsIpi*)ipState->subState;
	DataSetIpi* dataSet =
		(DataSetIpi*)results->b.dataSet;
	Exception* exception = ((stateWithException*)state)->exception;

	// Configure the next result in the array of results.
	result = &((ResultIpi*)results->items)[results->count];
	resultIpiReset(result);
	results->items[0].ipRangeOffset = NULL_PROFILE_OFFSET; // Default IP range offset
	result->targetIpAddress.length = 
		ipState->type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4 ? FIFTYONE_DEGREES_IPV4_LENGTH : FIFTYONE_DEGREES_IPV6_LENGTH;
	if (ipState->type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
		memset(result->targetIpAddress.value, 0, FIFTYONE_DEGREES_IPV6_LENGTH);
		// We only get the exact length of ipv4
		memcpy(result->targetIpAddress.value, pair->parsedValue, FIFTYONE_DEGREES_IPV4_LENGTH);
	}
	else {
		// We only get the exact length of ipv6
		memcpy(result->targetIpAddress.value, pair->parsedValue, FIFTYONE_DEGREES_IPV6_LENGTH);
	}
	result->targetIpAddress.type = ipState->type;
	result->type = ipState->type;
	results->count++;

	setResultFromIpAddress(
		result,
		dataSet,
		exception);
	if (EXCEPTION_FAILED) {
		return false;
	}

	return EXCEPTION_OKAY;
}

void fiftyoneDegreesResultsIpiFromEvidence(
	fiftyoneDegreesResultsIpi* results,
	fiftyoneDegreesEvidenceKeyValuePairArray* evidence,
	fiftyoneDegreesException* exception) {
	stateWithException state;
	stateWithIpVersion ipState;
	ipState.subState = results;
	state.state = &ipState;
	state.exception = exception;

	if (evidence != (EvidenceKeyValuePairArray*)NULL) {
		// Reset the results data before iterating the evidence.
		results->count = 0;

		// Check the Ipv4 prefixed evidence keys before the Ipv6 prefixed
		// evidence keys.
		ipState.type = FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4;
		EvidenceIterate(
			evidence,
			FIFTYONE_DEGREES_EVIDENCE_IPV4,
			&state,
			setResultFromEvidence);
		if (EXCEPTION_FAILED) {
			return;
		}

		// If no results were obtained from the Ipv4 evidence prefix then use
		// the Ipv6 prefix to populate the results.
		ipState.type = FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6;
		if (results->count == 0) {
			EvidenceIterate(
				evidence,
				FIFTYONE_DEGREES_EVIDENCE_IPV6,
				&state,
				setResultFromEvidence);
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
			profilePercentageItem.percentage = percentageState->percentage;
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
	Float percentage,
	Exception* exception) {
	uint32_t count;

	// Set the state for the callbacks.
	stateWithException state;
	stateWithPercentage percentageState;
	percentageState.subState = results;
	percentageState.percentage = percentage;
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

static uint32_t addValuesFromIpRanges(
	ResultsIpi *results,
	ResultIpi *result,
	const char *propertyName,
	Exception *exception) {
	uint32_t offset;
	uint32_t count = 0;
	Item ipRangeItem;
	ProfilePercentage profilePercentage;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;
	profilePercentage.percentage = NATIVE_TO_FLOAT(1);
	
	offset = result->ipRangeOffset;

	DataReset(&ipRangeItem.data);
	if (result->type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
		// If offset is the last item then don't increase
		if (StringCompare("RangeEnd", propertyName) == 0
			&& offset < dataSet->ipv4Ranges->count - 1) {
			offset++;
		}

		Ipv4Range* ipv4Range = dataSet->ipv4Ranges->get(
			dataSet->ipv4Ranges,
			offset,
			&ipRangeItem,
			exception);
		if (ipv4Range != NULL && EXCEPTION_OKAY) {
			// The capacity of the list should be at least 1
			profilePercentage.item = ipRangeItem;
			addIpiListItem(&results->values, &profilePercentage);
			count = 1;
		}
	}
	else {
		// If offset is the last item then don't increase
		if (StringCompare("RangeEnd", propertyName) == 0
			&& offset < dataSet->ipv6Ranges->count - 1) {
			offset++;
		}

		Ipv6Range* ipv6Range = dataSet->ipv6Ranges->get(
			dataSet->ipv6Ranges,
			offset,
			&ipRangeItem,
			exception);
		if (ipv6Range != NULL && EXCEPTION_OKAY) {
			// The capacity of the list should be at least 1
			profilePercentage.item = ipRangeItem;
			addIpiListItem(&results->values, &profilePercentage);
			count = 1;
		}
	}
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

/*
 * get the profile combination offset from the result
 * @param results the results array
 * @param result the matched result
 * @param exception the exception object which will be used
 * when an exception occurs
 * @return offset
 */
static uint32_t getProfileCombinationOffset(
	ResultsIpi* results,
	ResultIpi* result,
	Exception *exception) {
	uint32_t offset = 0;
	Item item;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	DataReset(&item.data);
	if (result->type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
		Ipv4Range* ipv4Range = dataSet->ipv4Ranges->get(
			dataSet->ipv4Ranges,
			result->ipRangeOffset,
			&item,
			exception);
		if (ipv4Range != NULL && EXCEPTION_OKAY) {
			offset = ipv4Range->profileCombinationOffset;
			COLLECTION_RELEASE(dataSet->ipv4Ranges, &item);
		}
	}
	else {
		Ipv6Range* ipv6Range = dataSet->ipv6Ranges->get(
			dataSet->ipv6Ranges,
			result->ipRangeOffset,
			&item,
			exception);
		if (ipv6Range != NULL && EXCEPTION_OKAY) {
			offset = ipv6Range->profileCombinationOffset;
			COLLECTION_RELEASE(dataSet->ipv4Ranges, &item);
		}
	}
	return offset;
}

static uint32_t addValuesFromLocations(
	ResultsIpi* results,
	ResultIpi* result,
	const char* propertyName,
	Exception* exception) {
	uint32_t offset;
	uint32_t count = 0;
	Item item;
	ProfileCombination* profileCombination;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	// Obtain the profile combination offset from the returned result
	offset = getProfileCombinationOffset(results, result, exception);

	// We will only execute this step if successfully obtained the
	// profile combination offset from the previous step
	if (EXCEPTION_OKAY) {
		DataReset(&item.data);
		profileCombination = dataSet->profileCombinations->get(
			dataSet->profileCombinations,
			offset,
			&item,
			exception);
		if (profileCombination != NULL && EXCEPTION_OKAY) {
			componentIndex* index;
			offsetPercentage* profiles;
			offsetPercentage* values;
			uint32_t firstIndex = 0;
			// Set pointer to the component index list in the profile combination block
			index = (componentIndex*)&profileCombination->firstByte;
			// Set pointer to the profiles list in the profile combination block
			profiles = (offsetPercentage*)(index + dataSet->componentsList.count);
			// Set pointer to the values list in the profile combination block
			values = profiles + profileCombination->profileCount;
			// Location is a pair of lat and long values
			// The Locations are grouped in a ordered set at
			// the end of the profile combinations
			// i.e. (AverageLocation, LocationBoundSouthEast, LocationBoundNortWest)
			if (StringCompare("AverageLocation", propertyName) == 0) {
				// AverageLocation
				firstIndex = 0;
			}
			else if (StringCompare("LocationBoundSouthEast", propertyName) == 0) {
				firstIndex = 1;
			}
			else if (StringCompare("LocationBoundNorthWest", propertyName) == 0) {
				firstIndex = 2;
			}

			// Get the number of locations group
			if (firstIndex < (uint32_t)profileCombination->valueCount) {
				Item coordinate = getStringItemByValueOffset(
					results,
					values[firstIndex].offset,
					exception);
				if (EXCEPTION_OKAY) {
					ProfilePercentage coordProfilePercentage;
					coordProfilePercentage.item = coordinate;
					coordProfilePercentage.percentage = values[firstIndex].percentage;
					addIpiListItem(&results->values, &coordProfilePercentage);
					count = 1;
				}
			}
			COLLECTION_RELEASE(dataSet->profileCombinations, &item);
		}
	}
	return count;
}

static uint32_t addValuesFromDynamicProperty(
	ResultsIpi* results,
	ResultIpi* result,
	Property* property,
	Exception* exception) {
	uint32_t count = 0;
	String* propertyName;
	Item stringItem;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	DataReset(&stringItem.data);
	propertyName = StringGet(
		dataSet->strings,
		property->nameOffset,
		&stringItem,
		exception);
	if (propertyName != NULL && EXCEPTION_OKAY) {
		if (StringCompareLength("Range", &propertyName->value, strlen("Range")) == 0) {
			count = addValuesFromIpRanges(
				results,
				result,
				&propertyName->value,
				exception);
		}
		else if (StringSubString(&propertyName->value, "Location") != NULL) {
			count = addValuesFromLocations(
				results,
				result,
				&propertyName->value,
				exception);
		}
		else {
			EXCEPTION_SET(CORRUPT_DATA);
		}
		COLLECTION_RELEASE(dataSet->strings, &stringItem);
	}
	return count;
}

static uint32_t addValuesFromResult(
	ResultsIpi* results,
	ResultIpi* result,
	Property* property,
	Exception* exception) {
	uint32_t count = 0;
	ProfileCombination* profileCombination;
	componentIndex* index;
	offsetPercentage* profiles;
	Item profileCombinationItem, profileItem;
	DataSetIpi* dataSet = (DataSetIpi*)results->b.dataSet;

	Component* component = (Component*)dataSet->componentsList.items[
		property->componentIndex].data.ptr;
	if (component->defaultProfileOffset == DYNAMIC_COMPONENT_OFFSET) {
		// This is a dynamic component so check property name
		// and get data from profileCombination instead
		count = addValuesFromDynamicProperty(
			results,
			result,
			property,
			exception);
	}
	else {
		// Get the profile combination
		DataReset(&profileCombinationItem.data);
		if (results->count > 0) {
			uint32_t profileCombinationOffset = getProfileCombinationOffset(
				results,
				result,
				exception);

			if (profileCombinationOffset != -1
				&& EXCEPTION_OKAY) {
				profileCombination = (ProfileCombination*)dataSet->profileCombinations->get(
					dataSet->profileCombinations,
					profileCombinationOffset,
					&profileCombinationItem,
					exception);

				if (profileCombination != NULL && EXCEPTION_OKAY) {
					// Get the profiles array and values array
					index = (componentIndex*)&profileCombination->firstByte;
					profiles = (offsetPercentage*)(index + dataSet->componentsList.count);

					// Add values from profiles
					Profile* profile;
					uint32_t profileOffset;
					uint16_t cur = index[property->componentIndex].index;
					uint16_t end = cur + index[property->componentIndex].count;
					// Loop through the list of profiles and their percentage
					DataReset(&profileItem.data);
					for (; cur < end; cur++) {
						profileOffset = profiles[cur].offset;
						profile = NULL;
						if (profileOffset != NULL_PROFILE_OFFSET) {
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
									profiles[cur].percentage,
									exception);
								COLLECTION_RELEASE(dataSet->profiles, &profileItem);
							}
						}
					}
					COLLECTION_RELEASE(dataSet->profileCombinations, &profileCombinationItem);
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

fiftyoneDegreesProfilePercentage* fiftyoneDegreesResultsIpiGetValues(
	fiftyoneDegreesResultsIpi* results,
	int requiredPropertyIndex,
	fiftyoneDegreesException* exception) {
	ResultIpi* result;
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

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);

	// There will only one result for each IP address
	result = &results->items[0];

	if (property != NULL && EXCEPTION_OKAY) {
		// Ensure there is a collection available to the property item so
		// that it can be freed when the results are freed.
		if (results->propertyItem.collection == NULL) {
			results->propertyItem.collection = dataSet->properties;
		}

		if (result != NULL && EXCEPTION_OKAY) {
			firstValue = getValuesFromResult(
				results,
				result,
				property,
				exception);
		}
	}

	if (firstValue == NULL) {
		// There are no values for the property requested. Reset the values 
		// list to zero count.
		releaseIpiList(&results->values);
	}
	return firstValue;
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

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	Property *property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);

	ProfilePercentage *profilePercentage = getValuesFromResult(
		results,
		&results->items[0],
		property,
		exception);

	if (profilePercentage == NULL) {
		// There is no value for the required property.
		return false;
	}

	// None of the checks have returned false, so there must be valid values.
	return true;
}

static bool resultGetHasValidPropertyValueOffset(
	fiftyoneDegreesResultsIpi* results,
	fiftyoneDegreesResultIpi* result,
	int requiredPropertyIndex,
	fiftyoneDegreesException* exception) {
	bool hasValidOffset = false;
	Item item;
	ProfileCombination *profileCombination;
	DataSetIpi *dataSet = (DataSetIpi*)results->b.dataSet;

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	Property *property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);

	const char *propertyName = STRING(
		PropertiesGetNameFromRequiredIndex(
			dataSet->b.b.available,
			requiredPropertyIndex));
	if (propertyName != NULL && EXCEPTION_OKAY) {
		if (StringCompareLength("Range", propertyName, strlen("Range")) == 0) {
			if (results->count == 0)
				return false;
			else
				return true;
		}
		else {
			// Obtain the profile combination offset from the returned result
			uint32_t offset = getProfileCombinationOffset(results, result, exception);

			// We will only execute this step if successfully obtained the
			// profile combination offset from the previous step
			if (EXCEPTION_OKAY) {
				DataReset(&item.data);
				profileCombination = dataSet->profileCombinations->get(
					dataSet->profileCombinations,
					offset,
					&item,
					exception);

				if (profileCombination != NULL && EXCEPTION_OKAY) {
					componentIndex* indices;
					offsetPercentage* profiles;
					offsetPercentage* values;
					uint32_t firstIndex = 0;
					// Set pointer to the component index list in the profile combination block
					indices = (componentIndex*)&profileCombination->firstByte;
					// Set pointer to the profiles list in the profile combination block
					profiles = (offsetPercentage*)(indices + dataSet->componentsList.count);
					// Set pointer to the values list in the profile combination block
					values = profiles + profileCombination->profileCount;
					if (StringSubString(propertyName, "Location") != NULL) {
						// Location is a pair of lat and long values
						// The Locations are grouped in a ordered set at
						// the end of the profile combinations
						// i.e. (AverageLocation, LocationBoundSouthEast, LocationBoundNortWest)
						if (StringCompare("AverageLocation", propertyName) == 0) {
							// AverageLocation
							firstIndex = 0;
						}
						else if (StringCompare("LocationBoundSouthEast", propertyName) == 0) {
							firstIndex = 1;
						}
						else if (StringCompare("LocationBoundNorthWest", propertyName) == 0) {
							firstIndex = 2;
						}
						if (values[firstIndex].offset != NULL_VALUE_OFFSET) {
							hasValidOffset = true;
						}
					}
					else {
						for (int i = 0; i < indices[property->componentIndex].count; i++) {
							if (profiles[indices[property->componentIndex].index + i].offset != NULL_PROFILE_OFFSET) {
								hasValidOffset = true;
							}
						}
					}
				}
				else {
					EXCEPTION_SET(CORRUPT_DATA);
				}
				COLLECTION_RELEASE(dataSet->profileCombinations, &item);
			}
		}
	}
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

	bool hasValidOffset = resultGetHasValidPropertyValueOffset(
		results,
		&results->items[0],
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
		return "The results are empty. This is probably because we don't have this data in our database.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NULL_PROFILE:
		return "The results contained a null profile for the component which "
			"the required property belongs to.";
	case FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_UNKNOWN:
	default:
		return "The reason for missing values is unknown.";
	}
}

static size_t getIpv4RangeString(
	Ipv4Range *ipRange,
	char *buffer,
	size_t bufferLength) {
	size_t charactersAdded = snprintf(
		buffer,
		bufferLength,
		"%d.%d.%d.%d",
		(int)ipRange->start[0],
		(int)ipRange->start[1],
		(int)ipRange->start[2],
		(int)ipRange->start[3]);
	return charactersAdded > 0 ? charactersAdded : 0;
}

static size_t getIpv6RangeString(
	Ipv6Range *ipRange,
	char *buffer,
	size_t bufferLength) {
	const char *separator = ":";
	const char *hex = "0123456789abcdef";
	const size_t charLen = 1;
	size_t charactersAdded = 0, separatorLen = strlen(separator);
	for (int i = 0; i < FIFTYONE_DEGREES_IPV6_LENGTH; i += 2) {
		for (int j = 0; j < 2; j++) {
			if (charactersAdded + charLen < bufferLength) {
				// Get the first character of hex representation of the byte
				memcpy(buffer + charactersAdded, &hex[(((int)ipRange->start[i + j]) >> 4) & 0x0F], charLen);
				charactersAdded += charLen;
			}
			if (charactersAdded + charLen < bufferLength) {
				// Get the second character of hex representation of the byte
				memcpy(buffer + charactersAdded, &hex[((int)ipRange->start[i + j]) & 0x0F], charLen);
				charactersAdded += charLen;
			}
		}
		if (i != FIFTYONE_DEGREES_IPV6_LENGTH - 2) {
			if (charactersAdded + separatorLen < bufferLength) {
				memcpy(buffer + charactersAdded, separator, separatorLen);
				charactersAdded += separatorLen;
			}
		}
	}

	// Terminate the string buffer if characters were added
	if (charactersAdded < bufferLength) {
		buffer[charactersAdded] = '\0';
	}
	return charactersAdded;
}

static size_t getRangeString(
	ProfilePercentage *profilePercentage,
	uint32_t count,
	fiftyoneDegreesEvidenceIpType type,
	char *buffer,
	size_t bufferLength) {
	size_t charactersAdded = 0, tempAdded = 0;
	const size_t quoteLen = strlen("\"");
	
	if (count > 0) {
		// Add the opening quote
		if (charactersAdded + quoteLen < bufferLength) {
			tempAdded = sprintf(buffer, "\"");
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
			}
		}
		if (type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
			Ipv4Range *ipRange = (Ipv4Range *)profilePercentage[0].item.data.ptr;
			charactersAdded += getIpv4RangeString(
				ipRange,
				buffer + charactersAdded,
				bufferLength - charactersAdded);
		}
		else {
			Ipv6Range *ipRange = (Ipv6Range *)profilePercentage[0].item.data.ptr;
			charactersAdded += getIpv6RangeString(
				ipRange,
				buffer + charactersAdded,
				bufferLength - charactersAdded);
		}
		// Add the closing quote
		if (charactersAdded + quoteLen < bufferLength) {
			tempAdded = sprintf(buffer + charactersAdded, "\"");
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
			}
		}
		if (charactersAdded < bufferLength) {
			tempAdded = snprintf(
				buffer + charactersAdded,
				bufferLength - charactersAdded,
				":\"%f\"",
				FLOAT_TO_NATIVE(profilePercentage[0].percentage));
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
				buffer[charactersAdded] = '\0';
			}
		}
	}
	return charactersAdded;
}

static size_t getLocationString(
	ProfilePercentage *profilePercentage,
	uint32_t count,
	char *buffer,
	size_t bufferLength) {
	size_t charactersAdded = 0, tempAdded = 0;
	const size_t quoteLen = strlen("\"");

	if (count > 0) {
		floatPair *location =
			(floatPair *)profilePercentage[0].item.data.ptr;
		
		// Add the opening quote
		if (charactersAdded + quoteLen < bufferLength) {
			tempAdded = sprintf(buffer, "\"");
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
			}
		}
		tempAdded = snprintf(
			buffer + charactersAdded,
			bufferLength - charactersAdded,
			"%f,%f",
			FLOAT_TO_NATIVE(location->first),
			FLOAT_TO_NATIVE(location->second));
		if (tempAdded > 0) {
			charactersAdded += tempAdded;
			// Add the closing quote
			if (charactersAdded + quoteLen < bufferLength) {
				tempAdded = sprintf(buffer + charactersAdded, "\"");
				if (tempAdded > 0) {
					charactersAdded += tempAdded;
				}
			}
			tempAdded = snprintf(
				buffer + charactersAdded,
				bufferLength - charactersAdded,
				":\"%f\"",
				FLOAT_TO_NATIVE(profilePercentage[0].percentage));
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
				buffer[charactersAdded] = '\0';
			}
		}
	}
	return charactersAdded;
}

static size_t getNormalString(
	ProfilePercentage *profilePercentage,
	uint32_t count,
	char *buffer,
	size_t bufferLength,
	const char *separator) {
	String *string;
	const size_t quoteLen = strlen("\"");
	size_t charactersAdded = 0,
		separatorLen = strlen(separator),
		stringLen,
		tempAdded = 0;

	// Loop through the values adding them to the string buffer.
	for (uint32_t i = 0; i < count;  i++) {
		// Append the separator
		if (i > 0 && charactersAdded + separatorLen < bufferLength) {
			memcpy(buffer + charactersAdded, separator, separatorLen);
			charactersAdded += separatorLen;
		}

		// Add the opening quote
		if (charactersAdded + quoteLen < bufferLength) {
			tempAdded = sprintf(buffer + charactersAdded, "\"");
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
			}
		}

		// Get the string for the value index.
		string = (String*)profilePercentage[i].item.data.ptr;

		// Add the string to the output buffer recording the number
		// of characters added.
		if (string != NULL) {
			stringLen = strlen(&string->value);
			if (charactersAdded + stringLen < bufferLength) {
				memcpy(
					buffer + charactersAdded,
					&string->value,
					stringLen);
			}
			charactersAdded += stringLen;
		}

		// Add the closing quote
		if (charactersAdded + quoteLen < bufferLength) {
			tempAdded = sprintf(buffer + charactersAdded, "\"");
			if (tempAdded > 0) {
				charactersAdded += tempAdded;
			}
		}

		// Append the percentage
		tempAdded = snprintf(
			buffer + charactersAdded,
			bufferLength - charactersAdded,
			":\"%f\"",
			FLOAT_TO_NATIVE(profilePercentage[i].percentage));
		if (tempAdded > 0) {
			charactersAdded += tempAdded;
		}
	}

	// Terminate the string buffer if characters were added.
	if (charactersAdded < bufferLength) {
		buffer[charactersAdded] = '\0';
	}
	return charactersAdded;
}

static size_t fiftyoneDegreesResultsIpiGetValuesStringInternal(
	fiftyoneDegreesResultsIpi* results,
	const char* propertyName,
	int requiredPropertyIndex,
	char* buffer,
	size_t bufferLength,
	const char* separator,
	fiftyoneDegreesException* exception) {
	ProfilePercentage *profilePercentage;
	size_t charactersAdded = 0;

	if (requiredPropertyIndex >= 0) {
		profilePercentage = fiftyoneDegreesResultsIpiGetValues(
			results,
			requiredPropertyIndex,
			exception);
		if (profilePercentage != NULL && EXCEPTION_OKAY) {
			if (StringCompareLength("Range", propertyName, strlen("Range")) == 0) {
				charactersAdded = getRangeString(
					profilePercentage,
					results->values.count,
					results->items[0].type,
					buffer,
					bufferLength);
			}
			else if(StringSubString(propertyName, "Location") != NULL) {
				charactersAdded = getLocationString(
					profilePercentage,
					results->values.count,
					buffer,
					bufferLength);
			}
			else {
				charactersAdded = getNormalString(
					profilePercentage,
					results->values.count,
					buffer,
					bufferLength,
					separator);
			}
		}
	}
	return charactersAdded;
}

size_t fiftyoneDegreesResultsIpiGetValuesString(
	fiftyoneDegreesResultsIpi* results,
	const char* propertyName,
	char* buffer,
	size_t bufferLength,
	const char* separator,
	fiftyoneDegreesException* exception) {
	DataSetIpi *dataSet = (DataSetIpi *)results->b.dataSet;
	int requiredPropertyIndex = PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		propertyName);

	return fiftyoneDegreesResultsIpiGetValuesStringInternal(
		results,
		propertyName,
		requiredPropertyIndex,
		buffer,
		bufferLength,
		separator,
		exception);
}

size_t fiftyoneDegreesResultsIpiGetValuesStringByRequiredPropertyIndex(
	fiftyoneDegreesResultsIpi* results,
	const int requiredPropertyIndex,
	char* buffer,
	size_t bufferLength,
	const char* separator,
	fiftyoneDegreesException* exception) {
	DataSetIpi *dataSet = (DataSetIpi *)results->b.dataSet;
	const char *propertyName = STRING(
		PropertiesGetNameFromRequiredIndex(
			dataSet->b.b.available,
			requiredPropertyIndex));

	return fiftyoneDegreesResultsIpiGetValuesStringInternal(
		results,
		propertyName,
		requiredPropertyIndex,
		buffer,
		bufferLength,
		separator,
		exception);
}

/*
 * Supporting Macros to printout the NetworkId
 */
#define PRINT_PROFILE_SEP(d,b,s,f) d += snprintf(d, (b - d) + s, f)
#define PRINT_PROFILE_ID(d,b,s,f,v,p) d += snprintf(d, (b - d) + s, f, v, p)
#define PRINT_NULL_PROFILE_ID(d,b,s,p) PRINT_PROFILE_ID(d, b, s, "%i:%f", 0, p)

char* fiftyoneDegreesIpiGetNetworkIdFromResult(
	fiftyoneDegreesResultsIpi* results,
	fiftyoneDegreesResultIpi* result,
	char* destination,
	size_t size,
	fiftyoneDegreesException* exception) {
	uint32_t offset;
	Item profileCombinationItem, profileItem;
	Profile *profile;
	char *buffer = destination;
	ProfileCombination* profileCombination;
	DataSetIpi *dataSet = (DataSetIpi *)results->b.dataSet;
	// Obtain the profile combination offset from the returned result
	offset = getProfileCombinationOffset(results, result, exception);

	// We will only execute this step if successfully obtained the
	// profile combination offset from the previous step
	if (EXCEPTION_OKAY) {
		DataReset(&profileCombinationItem.data);
		profileCombination = dataSet->profileCombinations->get(
			dataSet->profileCombinations,
			offset,
			&profileCombinationItem,
			exception);
		if (profileCombination != NULL && EXCEPTION_OKAY) {
			componentIndex* index;
			offsetPercentage* profiles;
			// Set pointer to the component index list in the profile combination block
			index = (componentIndex*)&profileCombination->firstByte;
			// Set pointer to the profiles list in the profile combination block
			profiles = (offsetPercentage*)(index + dataSet->componentsList.count);
			DataReset(&profileItem.data);
			for (int i = 0; i < profileCombination->profileCount; i++) {
				// Print separator
				if (i != 0) {
					PRINT_PROFILE_SEP(destination, buffer, size, "|");
				}

				// Get profile offset
				offset = profiles[i].offset;
				profile = (Profile *)dataSet->profiles->get(
					dataSet->profiles,
					offset,
					&profileItem,
					exception);
				if (profile == NULL) {
					PRINT_NULL_PROFILE_ID(destination, buffer, size, FLOAT_TO_NATIVE(profiles[i].percentage));
				}
				else {
					PRINT_PROFILE_ID(
						destination,
						buffer,
						size,
						"%i:%f",
						profile->profileId,
						FLOAT_TO_NATIVE(profiles[i].percentage));
				}
				COLLECTION_RELEASE(dataSet->profiles, &profileItem);
			}
			COLLECTION_RELEASE(dataSet->profileCombinations, &profileCombinationItem);
		}
	}
	return destination;
}

char* fiftyoneDegreesIpiGetNetworkIdFromResults(
	fiftyoneDegreesResultsIpi* results,
	char* destination,
	size_t size,
	fiftyoneDegreesException* exception) {
	return fiftyoneDegreesIpiGetNetworkIdFromResult(
		results,
		&results->items[0],
		destination,
		size,
		exception);
}

fiftyoneDegreesCoordinate fiftyoneDegreesIpiGetCoordinate(
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesException *exception) {
	floatPair *fPair = (floatPair *)item->data.ptr;
	fiftyoneDegreesCoordinate coordinate = { 0, 0 };
	if (fPair->type == 1) {
		coordinate.lat = FLOAT_TO_NATIVE(fPair->first);
		coordinate.lon = FLOAT_TO_NATIVE(fPair->second);
	}
	else {
		EXCEPTION_SET(INCORRECT_FORMAT);
	}
	return coordinate;
}

size_t fiftyoneDegreesIpiGetIpRangeAsString(
	fiftyoneDegreesCollectionItem *item,
	fiftyoneDegreesEvidenceIpType type,
	char *buffer,
	uint32_t bufferLength,
	fiftyoneDegreesException *exception) {
	size_t charactersAdded =  0;
	if (type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
		Ipv4Range *ipRange = (Ipv4Range *)item->data.ptr;
		charactersAdded += getIpv4RangeString(
			ipRange,
			buffer,
			bufferLength);
	}
	else if (type ==  FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6) {
		Ipv6Range *ipRange = (Ipv6Range *)item->data.ptr;
		charactersAdded += getIpv6RangeString(
			ipRange,
			buffer,
			bufferLength);
	}
	else {
		EXCEPTION_SET(INCORRECT_FORMAT);
	}
	return charactersAdded;
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