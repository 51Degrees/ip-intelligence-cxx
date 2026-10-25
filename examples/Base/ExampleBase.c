/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
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

#include "ExampleBase.h"
#include "../../src/fiftyone.h"
#include "../../src/common-cxx/collectionKeyTypes.h"
#include <string.h>

#define CONFIG_EQUALS(h) memcmp(&config, &h, sizeof(ConfigIpi)) == 0

const char* fiftyoneDegreesExampleGetConfigName(ConfigIpi config) {
	if (CONFIG_EQUALS(IpiInMemoryConfig)) {
		return "InMemory";
	}
	if (CONFIG_EQUALS(IpiHighPerformanceConfig)) {
		return "HighPerformance";
	}
	if (CONFIG_EQUALS(IpiLowMemoryConfig)) {
		return "LowMemory";
	}
	if (CONFIG_EQUALS(IpiBalancedConfig)) {
		return "Balanced";
	}
	if (CONFIG_EQUALS(IpiBalancedTempConfig)) {
		return "BalancedTemp";
	}
	return "Unknown";
}

void fiftyoneDegreesExampleMemCheck(
    fiftyoneDegreesExampleParameters *parameters,
    fiftyoneDegreesExampleRunPtr run) {
// Windows specific memory checking
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
	
// Use memory tracking for non Windows platforms. can be enabled on Windows
// platform via FORCE_MEMORY_TRACKING
#if (defined(_DEBUG) && !defined(_MSC_VER)) || defined(FORCE_MEMORY_TRACKING)
	fiftyoneDegreesSetUpMemoryTracking();
#endif

	// Call the actual function.
	run(parameters);

// Use memory tracking for non Windows platforms. can be enabled on Windows
// platform via FORCE_MEMORY_TRACKING
#if (defined(_DEBUG) && !defined(_MSC_VER)) || defined(FORCE_MEMORY_TRACKING)
	if (fiftyoneDegreesUnsetMemoryTracking() != 0) {
		printf("ERROR: There is memory leak. All allocated memory should "
			"be freed at the end of this test.\n");
		exit(1);
	}
#endif

// Windows specific memory checking
#if defined(_DEBUG) && defined(_MSC_VER)
	_CrtDumpMemoryLeaks();
#endif
}

void fiftyoneDegreesExampleCheckDataFile(
	fiftyoneDegreesDataSetIpi *dataset) {
	Item item;
	DataReset(&item.data);

	EXCEPTION_CREATE
	StoredBinaryValueGet(
		dataset->strings,
		dataset->header.nameOffset,
		FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING, // result is unused anyway
		&item,
		exception);
	EXCEPTION_THROW

	const char* dataTier = STRING(item.data.ptr); // product is string

	fiftyoneDegreesDate date = dataset->header.published;
	struct tm gt;
	gt.tm_year = (date.year >= 1900) ? date.year - 1900 : date.year;
	gt.tm_mday = date.day;
	gt.tm_mon = (int)date.month - 1;
	gt.tm_hour = 0;
	gt.tm_min = 0;
	gt.tm_sec = 0;
	gt.tm_isdst = 0;
	gt.tm_wday = 0;
	gt.tm_yday = 0;
	time_t published = mktime(&gt);
	time_t now = time(NULL);

	char timeStr[500] = "";
#if defined( _MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
	if (asctime_s(timeStr, 500, (const struct tm*)&gt) != 0) {
#else
	if (asctime_r((const struct tm*)&gt, timeStr) != timeStr) {
#endif
		printf("\033[0;33m");
		printf(("Failed to obtain textual representation of the data file "
			"published date\n"));
		printf("\033[0m");
	}

	printf("Using a %s data file created %s from location %s\n",
		dataTier, timeStr, dataset->b.b.fileName);

#define DATA_FILE_AGE_WARNING 30
	if ((now - published) / (24 * 60 * 60) > DATA_FILE_AGE_WARNING) {
		printf("\033[0;33m");
		printf(("This example is using a data file "
			"that is more than %d days old. "
			"A more recent data file may be needed to "
			"correctly detect the IP ranges."
			"The latest lite data file is available from "
			"the ip-intelligence-data repository on GitHub "
			"https://github.com/51Degrees/ip-intelligence-data. "
			"Find out about the Enterprise data file, which "
			"includes automatic daily updates, on our pricing "
			"page: https://51degrees.com/pricing?utm_source=code&utm_medium=example&utm_campaign=ip-intelligence-cxx&utm_content=examples-base-examplebase.c&utm_term=data-file-age-warning\n"), DATA_FILE_AGE_WARNING);
		printf("\033[0m");
	}

	if (strncmp(dataTier, "Lite", strlen("Lite")) == 0) {
		printf(("This example is using the \"Lite\" "
			"data file. This is used for illustration, and "
			"has limited accuracy and capabilities. Find "
			"out about the Enterprise data file on our "
			"pricing page: https://51degrees.com/pricing?utm_source=code&utm_medium=example&utm_campaign=ip-intelligence-cxx&utm_content=examples-base-examplebase.c&utm_term=lite-data-file\n"));
	}
	if (!CollectionGetIsMemoryOnly()) {
		COLLECTION_RELEASE(dataset->strings, &item);
	}
}

// Bytes of the profiles collection, or of a range within it, from a length or
// offset in the units the data file header declares.
static uint64_t profileUnitsToBytes(
	const fiftyoneDegreesDataSetIpi *dataset,
	uint64_t units) {
	return units << dataset->header.profilesOffsetShift;
}

// Byte position within the profiles collection of the profile at the index in
// the profile offsets collection, or the end of the collection for an index
// one past the last.
static uint64_t profilePosition(
	fiftyoneDegreesDataSetIpi *dataset,
	uint32_t index,
	fiftyoneDegreesException *exception) {
	if (index >= dataset->header.profileOffsets.count) {
		return profileUnitsToBytes(dataset, dataset->header.profiles.length);
	}
	Item item;
	DataReset(&item.data);
	const CollectionKey key = { index, CollectionKeyType_Integer };
	const uint32_t *offset = (const uint32_t*)dataset->profileOffsets->get(
		dataset->profileOffsets,
		&key,
		&item,
		exception);
	uint64_t position = 0;
	if (offset != NULL && EXCEPTION_OKAY) {
		position = profileUnitsToBytes(dataset, *offset);
		EXAMPLE_COLLECTION_RELEASE(dataset->profileOffsets, item);
	}
	return position;
}

// Bytes of the four collections that hold a graph's data.
static uint64_t graphBytes(const fiftyoneDegreesIpiCgInfo *info) {
	return (uint64_t)info->spanBytes.length +
		info->spans.length +
		info->clusters.length +
		info->nodes.collection.length;
}

static void printLayoutLine(
	const char *name,
	uint64_t bytes,
	uint64_t total) {
	printf("  %-16s %12.1f MB %6.1f%%\n",
		name,
		(double)bytes / (1024.0 * 1024.0),
		total > 0 ? (double)bytes * 100.0 / (double)total : 0.0);
}

void fiftyoneDegreesExamplePrintDataFileLayout(
	fiftyoneDegreesDataSetIpi *dataset) {
	EXCEPTION_CREATE;
	const fiftyoneDegreesDataSetIpiHeader *header = &dataset->header;
	const fiftyoneDegreesIpiCgArray *graphs = dataset->graphsArray;
	const uint32_t componentCount = dataset->componentsList.count;

	// Sum the graph data, which the graphs collection only points to.
	uint64_t graphDataBytes = 0;
	for (uint32_t i = 0; i < graphs->count; i++) {
		graphDataBytes += graphBytes(&graphs->items[i].info);
	}

	const uint64_t profilesBytes = profileUnitsToBytes(
		dataset,
		header->profiles.length);
	const uint64_t total = sizeof(fiftyoneDegreesDataSetIpiHeader) +
		header->strings.length +
		header->components.length +
		header->maps.length +
		header->properties.length +
		header->values.length +
		profilesBytes +
		header->graphs.length +
		header->profileGroups.length +
		header->propertyTypes.length +
		header->profileOffsets.length +
		graphDataBytes;

	printf("Data file of %.1f MB by collection:\n",
		(double)total / (1024.0 * 1024.0));
	printLayoutLine("strings", header->strings.length, total);
	printLayoutLine("values", header->values.length, total);
	printLayoutLine("profiles", profilesBytes, total);
	printLayoutLine("profileOffsets", header->profileOffsets.length, total);
	printLayoutLine("graph data", graphDataBytes, total);
	printLayoutLine("other", total - header->strings.length -
		header->values.length - profilesBytes -
		header->profileOffsets.length - graphDataBytes, total);

	// Profiles and graph data belong to one component each. Strings and
	// values are shared, so they are not attributed.
	printf("Profiles and graph data by component:\n");
	for (uint32_t c = 0; c < componentCount; c++) {
		const fiftyoneDegreesComponent *component =
			(const fiftyoneDegreesComponent*)
			dataset->componentsList.items[c].data.ptr;
		uint64_t componentGraphBytes = 0;
		uint64_t profilesStart = UINT64_MAX;
		uint64_t profilesEnd = 0;
		for (uint32_t i = 0; i < graphs->count; i++) {
			const fiftyoneDegreesIpiCgInfo *info = &graphs->items[i].info;
			if (info->componentId != component->componentId) {
				continue;
			}
			componentGraphBytes += graphBytes(info);
			const uint64_t start = profilePosition(
				dataset,
				info->firstProfileIndex,
				exception);
			const uint64_t end = profilePosition(
				dataset,
				info->firstProfileIndex + info->profileCount,
				exception);
			EXCEPTION_THROW;
			if (start < profilesStart) {
				profilesStart = start;
			}
			if (end > profilesEnd) {
				profilesEnd = end;
			}
		}
		const uint64_t componentProfilesBytes =
			profilesEnd > profilesStart ? profilesEnd - profilesStart : 0;

		Item nameItem;
		DataReset(&nameItem.data);
		const char *name = "";
		if (StoredBinaryValueGet(
			dataset->strings,
			component->nameOffset,
			FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING,
			&nameItem,
			exception) != NULL && EXCEPTION_OKAY) {
			name = STRING(nameItem.data.ptr);
		}
		EXCEPTION_THROW;
		printf("  %-16s profiles %10.1f MB %6.1f%%   graph data %8.1f MB %6.1f%%\n",
			name,
			(double)componentProfilesBytes / (1024.0 * 1024.0),
			(double)componentProfilesBytes * 100.0 / (double)total,
			(double)componentGraphBytes / (1024.0 * 1024.0),
			(double)componentGraphBytes * 100.0 / (double)total);
		if (nameItem.data.ptr != NULL) {
			EXAMPLE_COLLECTION_RELEASE(dataset->strings, nameItem);
		}
	}
}

uint32_t fiftyoneDegreesIterateFakeIPv4s(
	uint32_t const rangeStart,
	uint32_t const rangeEnd,
	uint32_t const increment,
	fiftyoneDegreesIpAddressHandlerPtr const ipAddressHandler,
	void * const state) {

	char buffer[16];
	uint32_t count = 0;
	for (uint64_t i = rangeStart; i <= rangeEnd; i += increment, ++count) {
		{
			StringBuilder builder = { buffer, sizeof(buffer) };
			StringBuilderInit(&builder);
			StringBuilderAddInteger(&builder, (uint8_t)((i >> 24) & 0xFF));
			StringBuilderAddChar(&builder, '.');
			StringBuilderAddInteger(&builder, (uint8_t)((i >> 16) & 0xFF));
			StringBuilderAddChar(&builder, '.');
			StringBuilderAddInteger(&builder, (uint8_t)((i >> 8) & 0xFF));
			StringBuilderAddChar(&builder, '.');
			StringBuilderAddInteger(&builder, (uint8_t)(i & 0xFF));
			StringBuilderComplete(&builder);
		}
		ipAddressHandler(buffer, state);
	}
	return count;
}

// Parameters used for allocating memory when reading evidence.
#define SIZE_OF_KEY 500
#define SIZE_OF_VALUE 1000
#define MAX_EVIDENCE 20

/**
 * Data for TextFileIterate-to-YamlFileIterate callback adapter.
 */
typedef struct {
	char * const ipAddressBuffer;
	const int bufferLength;
	void * const state;
	void(* const callback)(const char*, void *);
} YamlCallbackState;

/**
 * Adapter of TextFileIterate-compatible callback
 * to YamlFileIterate-compatible one.
 *
 * @param pairs Pairs of YAML document
 * @param size Number of pairs in the YAML document
 * @param callbackState Wrapped callback and parameters
 */
static void yamlCallback(
	fiftyoneDegreesKeyValuePair * const pairs,
	const uint16_t size,
	void * const callbackState) {

	const YamlCallbackState * const state =
		(const YamlCallbackState *)callbackState;
	for (uint16_t i = 0; i < size; i++) {
		const size_t writeableBufferLength = (state->bufferLength > 0
			? (size_t)(state->bufferLength - 1) : 0);
		size_t const length = (writeableBufferLength < pairs[i].valueLength
			? writeableBufferLength : pairs[i].valueLength);
		memcpy(state->ipAddressBuffer, pairs[i].value, length);
		state->ipAddressBuffer[length] = '\0';
		state->callback(state->ipAddressBuffer, state->state);
	}
}

void fiftyoneDegreesEvidenceFileIterate(
	const char * const fileName,
	char * const buffer,
	const int length,
	void * const state,
	void(* const callback)(const char*, void *)) {

	// Allocate working memory for iterating over the YAML evidence source.
	char evidenceBuffer[MAX_EVIDENCE * (SIZE_OF_KEY + SIZE_OF_VALUE)];
	KeyValuePair pair[MAX_EVIDENCE];
	char key[MAX_EVIDENCE][SIZE_OF_KEY];
	char value[MAX_EVIDENCE][SIZE_OF_VALUE];
	for (int i = 0; i < MAX_EVIDENCE; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = SIZE_OF_KEY;
		pair[i].value = value[i];
		pair[i].valueLength = SIZE_OF_VALUE;
	}

	YamlCallbackState callbackState = {
		buffer,
		length,
		state,
		callback,
	};

	YamlFileIterate(
		fileName,
		evidenceBuffer,
		sizeof(evidenceBuffer),
		pair,
		MAX_EVIDENCE,
		&callbackState,
		yamlCallback);
}

void fiftyoneDegreesEvidenceFileIterateWithLimit(
	const char * const fileName,
	char * const buffer,
	const int length,
	int limit,
	void * const state,
	void(* const callback)(const char*, void *)) {

	// Allocate working memory for iterating over the YAML evidence source.
	char evidenceBuffer[MAX_EVIDENCE * (SIZE_OF_KEY + SIZE_OF_VALUE)];
	KeyValuePair pair[MAX_EVIDENCE];
	char key[MAX_EVIDENCE][SIZE_OF_KEY];
	char value[MAX_EVIDENCE][SIZE_OF_VALUE];
	for (int i = 0; i < MAX_EVIDENCE; i++) {
		pair[i].key = key[i];
		pair[i].keyLength = SIZE_OF_KEY;
		pair[i].value = value[i];
		pair[i].valueLength = SIZE_OF_VALUE;
	}

	YamlCallbackState callbackState = {
		buffer,
		length,
		state,
		callback,
	};

	YamlFileIterateWithLimit(
		fileName,
		evidenceBuffer,
		sizeof(evidenceBuffer),
		pair,
		MAX_EVIDENCE,
		limit,
		&callbackState,
		yamlCallback);
}
