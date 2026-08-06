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

/**
@example IpIntelligence/CountryOverlap.c
Country overlap analysis using 51Degrees IP intelligence.

Where IP addresses are used to restrict content to a country, sometimes
called geo fencing, an IP address whose likely area spans an international
border cannot be placed in a single country. This example measures how
often that happens by checking every IPv4 address in the data file, and
names the countries that share each country's address space. The results
can be reproduced with any enterprise IP Intelligence data file.

## What the example produces

A single CSV file and a console summary. Each CSV row pairs the most
probable, or primary, country with a secondary country for a location
confidence and connection type:

- PrimaryCountryCode, PrimaryCountry: ISO code and name of the most
  probable country for the addresses counted by the row.
- SecondaryCountryCode, SecondaryCountry: when these equal the primary
  country the row counts the addresses that resolve entirely to the
  primary country. Otherwise they name another country in the area
  weighted country list, and the row counts the addresses that can
  resolve to both the primary and the secondary country.
- LocationConfidence, ConnectionType: the values of the properties with
  these names. See the
  [property dictionary](https://51degrees.com/developers/property-dictionary?utm_source=code&utm_medium=example&utm_campaign=ip-intelligence-cxx&utm_content=examples-c-ipintelligence-countryoverlap.c&utm_term=property-dictionary)
  for their definitions.
- IpAddressCount: IPv4 addresses counted by the row.
- ProportionOfPrimaryCountryAddresses: IpAddressCount as a proportion,
  where 1 is 100%, of all the addresses whose primary country is the
  row's primary country, across every location confidence and
  connection type.

An address whose area overlaps several countries appears in one row for
each secondary country, so the proportions for a primary country do not
sum to 1.

## How it works

Every IPv4 address is checked individually because location results
change within network ranges. For each address the component graph is
evaluated directly, which takes a few microseconds. The full weighted
property values are only resolved through the standard results API when
the graph result changes from one address to the next, and resolutions
are cached by graph offset so each distinct location is resolved once.
A full sweep of the 4.3 billion IPv4 addresses completes in around an
hour on a modern multi core machine.

After the sweep, randomly selected addresses are compared against the
normal single address lookup process and the outcome of each comparison
is printed, so the method can be seen to produce the same values as
individual lookups would.

## Requirements

The example requires an enterprise IP Intelligence data file that
includes the weighted country code properties. It stops with a clear
message, and a distinct exit code, when the data file does not include
them, which is the case for the Lite data file. To obtain an enterprise
data file see our
[pricing](https://51degrees.com/pricing?utm_source=code&utm_medium=example&utm_campaign=ip-intelligence-cxx&utm_content=examples-c-ipintelligence-countryoverlap.c&utm_term=enterprise-data-file).

Only IPv4 is swept. IPv6 addresses are too numerous to check one by one,
although individual IPv6 addresses can be checked with the probe option.

@include{doc} example-require-datafile-ipi.txt

## Usage

```
CountryOverlap <data file> [output csv] [threads] [chunks]
CountryOverlap <data file> --probe <ip> [ip...]
```

- data file: path to an enterprise .ipi data file.
- output csv: path for the CSV, default country-overlap.csv.
- threads: number of worker threads, default 10.
- chunks: number of /8 chunks of the IPv4 space to process starting at
  0.0.0.0, default 256 which is the whole space. Small values are useful
  for testing.
- --probe: print the resolved values for the addresses provided so they
  can be compared against other 51Degrees APIs.
*/

#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"
#include "../../../src/ipi_weighted_results.h"

/** Properties the analysis needs. */
#define PROPERTY_COUNTRY_CODE "CountryCode"
#define PROPERTY_COUNTRY "Country"
#define PROPERTY_CONFIDENCE "LocationConfidence"
#define PROPERTY_CONNECTION "ConnectionType"
#define PROPERTY_GEO "CountryCodesGeographical"
#define PROPERTY_POP "CountryCodesPopulation"
static const char* countryOverlapProperties =
	PROPERTY_COUNTRY_CODE ","
	PROPERTY_COUNTRY ","
	PROPERTY_CONFIDENCE ","
	PROPERTY_CONNECTION ","
	PROPERTY_GEO ","
	PROPERTY_POP;

/** Return codes for fiftyoneDegreesIpiCountryOverlap. */
#define COUNTRY_OVERLAP_OK 0
#define COUNTRY_OVERLAP_FAILED 1
#define COUNTRY_OVERLAP_PROPERTIES_MISSING 2

/** Country slots: 26 x 26 two letter codes plus Unknown and Failed. */
#define COUNTRY_SLOTS (26 * 26 + 2)
#define COUNTRY_UNKNOWN (26 * 26)
#define COUNTRY_FAILED (26 * 26 + 1)

/** Maximum number of distinct values for the confidence and connection
type dimensions. */
#define VALUE_SLOTS 16

/** Maximum number of component graphs the required properties span. */
#define MAX_COMPONENTS 4

/** Maximum number of entries in a weighted list that are compared when
counting distinct countries. */
#define MAX_LIST_VALUES 64

/** Number of shared countries remembered for each distinct location,
kept in descending weighting order. Real areas rarely overlap more than
a handful of countries. Locations with more shared countries than this
are flagged and counted so truncation is visible in the output. */
#define SHARED_STORE 16

/** Number of top shared countries named for each country in the console
summary. */
#define TOP_SHARED 5

/** Size of each thread's cache of resolved graph offsets. Power of two. */
#define CACHE_SIZE (1u << 22)

/** Key used for failed evaluations. */
#define FAILED_KEY UINT64_MAX

/** Number of random addresses compared against the normal lookup process
after the sweep to validate the graph based method. */
#define SPOT_CHECKS 20

/** Total number of IPv4 addresses as a double for progress reporting. */
#define TOTAL_IPV4 4294967296.0

/** Resolved values for a combination of graph offsets. */
typedef struct country_overlap_resolved_t {
	uint64_t key; /**< Combined graph offsets plus one, zero when empty */
	uint16_t countryIndex; /**< Index into the country dimension */
	uint8_t confidenceIndex; /**< Index into the confidence dimension */
	uint8_t connectionIndex; /**< Index into the connection dimension */
	uint8_t flags; /**< Bit 0 geographic multi, bit 1 population multi,
				   bit 2 shared list truncated at SHARED_STORE */
	uint8_t sharedCount; /**< Number of entries in shared */
	uint16_t shared[SHARED_STORE]; /**< Country indexes that share the
								   area, highest weighted first */
} Resolved;

/** A cell of the output matrix. */
typedef struct country_overlap_cell_t {
	uint64_t segments; /**< Number of contiguous segments of addresses */
	uint64_t addresses;
	uint64_t singleGeo;
	uint64_t multiGeo;
	uint64_t singlePop;
	uint64_t multiPop;
} Cell;

/** Cells for all the combinations of the three dimensions. */
typedef Cell Matrix[COUNTRY_SLOTS][VALUE_SLOTS][VALUE_SLOTS];

/** Value names for a dimension, built dynamically from the data. */
typedef struct country_overlap_value_table_t {
	char* names[VALUE_SLOTS];
	int count;
} ValueTable;

/**
 * Addresses that can resolve to both a primary and a secondary country
 * for each combination of confidence and connection type. Each block is
 * a COUNTRY_SLOTS by COUNTRY_SLOTS matrix indexed
 * [primary * COUNTRY_SLOTS + secondary], allocated on first use because
 * only a few of the confidence and connection combinations occur in
 * real data.
 */
typedef struct country_overlap_shared_blocks_t {
	uint64_t* blocks[VALUE_SLOTS][VALUE_SLOTS];
} SharedBlocks;

/**
 * Returns the pair count block for the confidence and connection type,
 * allocating a zeroed block on first use. Returns null when the
 * allocation fails, in which case the pair counts are not recorded but
 * the sweep continues.
 */
static uint64_t* sharedBlockGet(SharedBlocks* sharedBlocks, int f, int n) {
	uint64_t* block = sharedBlocks->blocks[f][n];
	if (block == NULL) {
		const size_t size =
			sizeof(uint64_t) * COUNTRY_SLOTS * COUNTRY_SLOTS;
		block = (uint64_t*)Malloc(size);
		if (block != NULL) {
			memset(block, 0, size);
			sharedBlocks->blocks[f][n] = block;
		}
	}
	return block;
}

static void sharedBlocksFree(SharedBlocks* sharedBlocks) {
	for (int f = 0; f < VALUE_SLOTS; f++) {
		for (int n = 0; n < VALUE_SLOTS; n++) {
			if (sharedBlocks->blocks[f][n] != NULL) {
				Free(sharedBlocks->blocks[f][n]);
				sharedBlocks->blocks[f][n] = NULL;
			}
		}
	}
}

/** State shared by all threads. */
typedef struct country_overlap_shared_state_t {
	ResourceManager* manager;
	DataSetIpi* dataSet;
	byte componentIds[MAX_COMPONENTS]; /**< Distinct component ids of the
									   required properties */
	int componentCount;
	int reqIndexCountryCode;
	int reqIndexCountry;
	int reqIndexConfidence;
	int reqIndexConnection;
	int reqIndexGeo;
	int reqIndexPop;
	int totalChunks; /**< Number of /8 chunks to process */
	volatile long nextChunk; /**< Next chunk to be claimed by a thread */
	volatile long threadsDone;
} SharedState;

/** State private to a single worker thread. */
typedef struct country_overlap_thread_state_t {
	SharedState* shared;
	ResultsIpi* results;
	Resolved* cache;
	Matrix cells;
	SharedBlocks sharedBlocks;
	char* countryNames[COUNTRY_SLOTS];
	ValueTable confidence;
	ValueTable connection;
	volatile uint64_t addressesDone;
	uint64_t evaluations;
	uint64_t resolutions;
	uint64_t failures;
	uint64_t truncations; /**< Distinct locations whose shared country
						  list exceeded SHARED_STORE */
} ThreadState;

static void countryOverlapReportStatus(
	StatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

static char* countryOverlapDuplicate(const char* value) {
	size_t length = strlen(value) + 1;
	char* copy = (char*)Malloc(length);
	if (copy != NULL) {
		memcpy(copy, value, length);
	}
	return copy;
}

/**
 * Returns the index in the value table for the name, adding the name if it
 * has not been seen before. The last slot is used when the table is full
 * which is not expected with real data.
 */
static int valueIndex(ValueTable* table, const char* name) {
	for (int i = 0; i < table->count; i++) {
		if (strcmp(table->names[i], name) == 0) {
			return i;
		}
	}
	if (table->count < VALUE_SLOTS) {
		table->names[table->count] = countryOverlapDuplicate(name);
		return table->count++;
	}
	return VALUE_SLOTS - 1;
}

static void valueTableFree(ValueTable* table) {
	for (int i = 0; i < table->count; i++) {
		Free(table->names[i]);
	}
	table->count = 0;
}

/**
 * Returns the country slot for a two letter ISO country code, or the
 * unknown slot when the code is not two upper case letters.
 */
static int countryIndex(const char* code) {
	if (code != NULL &&
		code[0] >= 'A' && code[0] <= 'Z' &&
		code[1] >= 'A' && code[1] <= 'Z' &&
		code[2] == '\0') {
		return (code[0] - 'A') * 26 + (code[1] - 'A');
	}
	return COUNTRY_UNKNOWN;
}

/** Returns the two letter code for a country slot into the buffer. */
static const char* countryCode(int index, char buffer[8]) {
	if (index == COUNTRY_UNKNOWN) {
		return "Unknown";
	}
	if (index == COUNTRY_FAILED) {
		return "Failed";
	}
	buffer[0] = (char)('A' + index / 26);
	buffer[1] = (char)('A' + index % 26);
	buffer[2] = '\0';
	return buffer;
}

/**
 * Counts the distinct string values for the required property index in
 * the weighted values collection.
 */
static int countDistinct(
	const WeightedValuesCollection* collection,
	int requiredPropertyIndex) {
	const char* seen[MAX_LIST_VALUES];
	int count = 0;
	for (uint32_t i = 0; i < collection->itemsCount; i++) {
		const fiftyoneDegreesWeightedValueHeader* header =
			collection->items[i];
		if (header->requiredPropertyIndex != requiredPropertyIndex ||
			header->valueType !=
			FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING) {
			continue;
		}
		const char* value =
			((const fiftyoneDegreesWeightedString*)header)->value;
		bool found = false;
		for (int s = 0; s < count && found == false; s++) {
			found = strcmp(seen[s], value) == 0;
		}
		if (found == false && count < MAX_LIST_VALUES) {
			seen[count++] = value;
		}
	}
	return count;
}

/**
 * Returns the string value with the highest weighting for the required
 * property index, or null when the property has no values.
 */
static const char* highestWeighted(
	const WeightedValuesCollection* collection,
	int requiredPropertyIndex) {
	const char* best = NULL;
	uint32_t bestWeight = 0;
	for (uint32_t i = 0; i < collection->itemsCount; i++) {
		const fiftyoneDegreesWeightedValueHeader* header =
			collection->items[i];
		if (header->requiredPropertyIndex != requiredPropertyIndex ||
			header->valueType !=
			FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING) {
			continue;
		}
		if (best == NULL || header->rawWeighting > bestWeight) {
			best = ((const fiftyoneDegreesWeightedString*)header)->value;
			bestWeight = header->rawWeighting;
		}
	}
	return best;
}

/**
 * Fills the shared country indexes of the resolved values with the
 * countries in the geographic weighted list other than the most probable
 * country, ordered with the highest weighted first.
 */
static void setSharedCountries(
	Resolved* resolved,
	const WeightedValuesCollection* collection,
	int requiredPropertyIndex) {
	uint16_t indexes[SHARED_STORE];
	uint32_t weights[SHARED_STORE];
	int count = 0;
	int qualifying = 0;
	for (uint32_t i = 0; i < collection->itemsCount; i++) {
		const fiftyoneDegreesWeightedValueHeader* header =
			collection->items[i];
		if (header->requiredPropertyIndex != requiredPropertyIndex ||
			header->valueType !=
			FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING) {
			continue;
		}
		const uint16_t index = (uint16_t)countryIndex(
			((const fiftyoneDegreesWeightedString*)header)->value);
		if (index == resolved->countryIndex ||
			index == COUNTRY_UNKNOWN) {
			continue;
		}
		// Skip countries already present keeping the higher weighting.
		bool present = false;
		for (int s = 0; s < count && present == false; s++) {
			present = indexes[s] == index;
		}
		if (present) {
			continue;
		}
		qualifying++;
		// Insert in descending weighting order.
		int position = count < SHARED_STORE ? count : SHARED_STORE;
		while (position > 0 &&
			weights[position - 1] < header->rawWeighting) {
			if (position < SHARED_STORE) {
				indexes[position] = indexes[position - 1];
				weights[position] = weights[position - 1];
			}
			position--;
		}
		if (position < SHARED_STORE) {
			indexes[position] = index;
			weights[position] = header->rawWeighting;
			if (count < SHARED_STORE) {
				count++;
			}
		}
	}
	resolved->sharedCount = (uint8_t)count;
	for (int s = 0; s < count; s++) {
		resolved->shared[s] = indexes[s];
	}
	if (qualifying > SHARED_STORE) {
		resolved->flags |= 4;
	}
}

/**
 * Resolves the values for the address using the normal lookup process.
 * The full lookup is only used once per distinct combination of graph
 * offsets because the hot loop detects repeats with the much cheaper
 * graph evaluations.
 */
static Resolved resolveAddress(ThreadState* state, uint32_t address) {
	SharedState* shared = state->shared;
	Resolved resolved;
	memset(&resolved, 0, sizeof(resolved));
	resolved.countryIndex = COUNTRY_FAILED;
	resolved.confidenceIndex = (uint8_t)valueIndex(
		&state->confidence,
		"Failed");
	resolved.connectionIndex = (uint8_t)valueIndex(
		&state->connection,
		"Failed");
	EXCEPTION_CREATE;

	byte bytes[4];
	bytes[0] = (byte)(address >> 24);
	bytes[1] = (byte)(address >> 16);
	bytes[2] = (byte)(address >> 8);
	bytes[3] = (byte)address;
	ResultsIpiFromIpAddress(
		state->results,
		bytes,
		sizeof(bytes),
		FIFTYONE_DEGREES_IP_TYPE_IPV4,
		exception);
	if (EXCEPTION_FAILED) {
		state->failures++;
		return resolved;
	}

	WeightedValuesCollection collection = ResultsIpiGetValuesCollection(
		state->results,
		NULL,
		0,
		NULL,
		exception);
	if (EXCEPTION_FAILED) {
		state->failures++;
		return resolved;
	}

	const char* code = highestWeighted(
		&collection,
		shared->reqIndexCountryCode);
	const char* confidence = highestWeighted(
		&collection,
		shared->reqIndexConfidence);
	const char* connection = highestWeighted(
		&collection,
		shared->reqIndexConnection);
	resolved.countryIndex = (uint16_t)countryIndex(code);
	resolved.confidenceIndex = (uint8_t)valueIndex(
		&state->confidence,
		confidence != NULL ? confidence : "Unknown");
	resolved.connectionIndex = (uint8_t)valueIndex(
		&state->connection,
		connection != NULL ? connection : "Unknown");
	if (countDistinct(&collection, shared->reqIndexGeo) > 1) {
		resolved.flags |= 1;
	}
	if (countDistinct(&collection, shared->reqIndexPop) > 1) {
		resolved.flags |= 2;
	}
	setSharedCountries(&resolved, &collection, shared->reqIndexGeo);
	if (resolved.flags & 4) {
		state->truncations++;
	}

	// Capture the display name for the country the first time the
	// country slot is seen by this thread.
	if (resolved.countryIndex < COUNTRY_UNKNOWN &&
		state->countryNames[resolved.countryIndex] == NULL) {
		const char* name = highestWeighted(
			&collection,
			shared->reqIndexCountry);
		if (name != NULL) {
			state->countryNames[resolved.countryIndex] =
				countryOverlapDuplicate(name);
		}
	}

	WeightedValuesCollectionRelease(&collection);
	state->resolutions++;
	return resolved;
}

/**
 * Returns the resolved values for the combined graph offsets, using the
 * thread's cache so that each distinct combination is resolved only once
 * per thread where cache capacity allows.
 */
static Resolved resolveKey(
	ThreadState* state,
	uint64_t key,
	uint32_t address) {
	if (key == FAILED_KEY) {
		Resolved failed;
		memset(&failed, 0, sizeof(failed));
		failed.key = FAILED_KEY;
		failed.countryIndex = COUNTRY_FAILED;
		failed.confidenceIndex = (uint8_t)valueIndex(
			&state->confidence,
			"Failed");
		failed.connectionIndex = (uint8_t)valueIndex(
			&state->connection,
			"Failed");
		return failed;
	}
	const uint64_t stored = key + 1;
	uint32_t slot = (uint32_t)(
		(key * 0x9E3779B97F4A7C15ull) >> 40) & (CACHE_SIZE - 1);
	for (uint32_t probes = 0; probes < 64; probes++) {
		Resolved* entry = &state->cache[slot];
		if (entry->key == stored) {
			return *entry;
		}
		if (entry->key == 0) {
			Resolved resolved = resolveAddress(state, address);
			resolved.key = stored;
			*entry = resolved;
			return resolved;
		}
		slot = (slot + 1) & (CACHE_SIZE - 1);
	}
	// The cache neighbourhood is full. Resolve without caching, which is
	// slower but still correct.
	return resolveAddress(state, address);
}

/** Adds a completed segment of addresses to the matrix. */
static void addSegment(
	ThreadState* state,
	const Resolved* resolved,
	uint64_t addresses) {
	Cell* cell = &state->cells
		[resolved->countryIndex]
		[resolved->confidenceIndex]
		[resolved->connectionIndex];
	cell->segments++;
	cell->addresses += addresses;
	if (resolved->flags & 1) {
		cell->multiGeo += addresses;
	}
	else {
		cell->singleGeo += addresses;
	}
	if (resolved->flags & 2) {
		cell->multiPop += addresses;
	}
	else {
		cell->singlePop += addresses;
	}
	if (resolved->sharedCount > 0) {
		uint64_t* block = sharedBlockGet(
			&state->sharedBlocks,
			resolved->confidenceIndex,
			resolved->connectionIndex);
		if (block != NULL) {
			uint64_t* row =
				block + (size_t)resolved->countryIndex * COUNTRY_SLOTS;
			for (int s = 0; s < resolved->sharedCount; s++) {
				row[resolved->shared[s]] += addresses;
			}
		}
	}
}

/**
 * Evaluates each of the required component graphs for the address and
 * combines the offsets into a single key. Returns FAILED_KEY when any
 * evaluation fails.
 */
static uint64_t evaluateKey(
	ThreadState* state,
	fiftyoneDegreesIpAddress address) {
	SharedState* shared = state->shared;
	const fiftyoneDegreesIpiCgArray* graphs =
		shared->dataSet->graphsArray;
	EXCEPTION_CREATE;
	uint64_t key = 0;
	for (int c = 0; c < shared->componentCount; c++) {
		fiftyoneDegreesIpiCgResult result =
			fiftyoneDegreesIpiGraphEvaluate(
				graphs,
				shared->componentIds[c],
				address,
				exception);
		state->evaluations++;
		if (EXCEPTION_FAILED) {
			EXCEPTION_CLEAR;
			state->failures++;
			return FAILED_KEY;
		}
		key = (key << 32) | (uint64_t)result.rawOffset;
	}
	return key;
}

/**
 * Sweeps a single /8 chunk of the IPv4 address space evaluating the
 * component graphs for every address. Full value resolution only happens
 * when the combined graph offsets change from one address to the next.
 */
static void sweepChunk(ThreadState* state, uint32_t chunk) {
	uint64_t start = (uint64_t)chunk << 24;
	uint64_t end = start + 0x00FFFFFF;

	fiftyoneDegreesIpAddress address;
	memset(&address, 0, sizeof(address));
	address.type = FIFTYONE_DEGREES_IP_TYPE_IPV4;

	uint64_t segmentKey = 0;
	bool segmentValid = false;
	uint64_t segmentStart = start;

	for (uint64_t ip = start; ip <= end; ip++) {
		address.value[0] = (byte)(ip >> 24);
		address.value[1] = (byte)(ip >> 16);
		address.value[2] = (byte)(ip >> 8);
		address.value[3] = (byte)ip;

		const uint64_t key = evaluateKey(state, address);

		if (segmentValid == false) {
			segmentKey = key;
			segmentValid = true;
			segmentStart = ip;
		}
		else if (key != segmentKey) {
			const Resolved resolved = resolveKey(
				state,
				segmentKey,
				(uint32_t)segmentStart);
			addSegment(state, &resolved, ip - segmentStart);
			segmentKey = key;
			segmentStart = ip;
		}

		// Periodically update the progress counter.
		if ((ip & 0xFFFFF) == 0xFFFFF) {
			state->addressesDone += 0x100000;
		}
	}
	if (segmentValid) {
		const Resolved resolved = resolveKey(
			state,
			segmentKey,
			(uint32_t)segmentStart);
		addSegment(state, &resolved, end - segmentStart + 1);
	}
}

/**
 * Validates the sweep's method for a single address by comparing the
 * values the sweep recorded for the address's segment against the values
 * the normal lookup process returns for the address itself.
 *
 * The sweep resolves values once at the start of each run of addresses
 * that share the same graph offsets. This check finds the start of the
 * run that contains the address, resolves both the run start and the
 * address with the normal lookup process, and compares the outcome. A
 * match shows the address was counted with the same values a single
 * lookup would have returned for it.
 */
static bool spotCheck(ThreadState* state, uint32_t address) {
	fiftyoneDegreesIpAddress ip;
	memset(&ip, 0, sizeof(ip));
	ip.type = FIFTYONE_DEGREES_IP_TYPE_IPV4;

	// Get the combined graph offsets for the address.
	ip.value[0] = (byte)(address >> 24);
	ip.value[1] = (byte)(address >> 16);
	ip.value[2] = (byte)(address >> 8);
	ip.value[3] = (byte)address;
	const uint64_t key = evaluateKey(state, ip);

	// Walk back to the start of the run of addresses sharing the
	// offsets, bounded so that very large runs do not slow the check.
	uint32_t start = address;
	for (int steps = 0; steps < 65536 && start > 0; steps++) {
		const uint32_t previous = start - 1;
		ip.value[0] = (byte)(previous >> 24);
		ip.value[1] = (byte)(previous >> 16);
		ip.value[2] = (byte)(previous >> 8);
		ip.value[3] = (byte)previous;
		if (evaluateKey(state, ip) != key) {
			break;
		}
		start = previous;
	}

	// Resolve both addresses with the normal lookup process and compare
	// all the values used by the analysis including the shared country
	// list.
	const Resolved atStart = resolveAddress(state, start);
	const Resolved atAddress = resolveAddress(state, address);
	bool match =
		atStart.countryIndex == atAddress.countryIndex &&
		atStart.confidenceIndex == atAddress.confidenceIndex &&
		atStart.connectionIndex == atAddress.connectionIndex &&
		atStart.flags == atAddress.flags &&
		atStart.sharedCount == atAddress.sharedCount;
	for (int s = 0; match && s < atStart.sharedCount; s++) {
		match = atStart.shared[s] == atAddress.shared[s];
	}
	char bufferA[8];
	char bufferB[8];
	printf(
		"Spot check %u.%u.%u.%u: sweep values from %u.%u.%u.%u "
		"(%s %s %s %s) vs normal lookup (%s %s %s %s) %s\n",
		address >> 24, (address >> 16) & 255,
		(address >> 8) & 255, address & 255,
		start >> 24, (start >> 16) & 255,
		(start >> 8) & 255, start & 255,
		countryCode(atStart.countryIndex, bufferA),
		state->confidence.names[atStart.confidenceIndex],
		state->connection.names[atStart.connectionIndex],
		(atStart.flags & 1) ? "multi-country" : "single-country",
		countryCode(atAddress.countryIndex, bufferB),
		state->confidence.names[atAddress.confidenceIndex],
		state->connection.names[atAddress.connectionIndex],
		(atAddress.flags & 1) ? "multi-country" : "single-country",
		match ? "MATCH" : "MISMATCH");
	return match;
}

/** Worker thread claiming chunks until none remain. */
static void sweepThread(void* statePointer) {
	ThreadState* state = (ThreadState*)statePointer;
	SharedState* shared = state->shared;
	for (;;) {
		long chunk =
			FIFTYONE_DEGREES_INTERLOCK_INC(&shared->nextChunk) - 1;
		if (chunk >= shared->totalChunks) {
			break;
		}
		sweepChunk(state, (uint32_t)chunk);
	}
	FIFTYONE_DEGREES_INTERLOCK_INC(&shared->threadsDone);
}

/**
 * Finds the component id that the property belongs to so the correct
 * graphs can be evaluated for each address. Distinct component ids are
 * accumulated in the shared state.
 */
static bool addComponentForProperty(
	SharedState* shared,
	const char* propertyName) {
	DataSetIpi* dataSet = shared->dataSet;
	EXCEPTION_CREATE;
	Item item;
	DataReset(&item.data);
	const int required = PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		propertyName);
	if (required < 0) {
		return false;
	}
	const uint32_t propertyIndex =
		dataSet->b.b.available->items[required].propertyIndex;
	const Property* property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&item,
		exception);
	if (property == NULL || EXCEPTION_FAILED) {
		return false;
	}
	const Component* component = (const Component*)
		dataSet->componentsList.items[property->componentIndex].data.ptr;
	const byte componentId = component->componentId;
	COLLECTION_RELEASE(dataSet->properties, &item);
	for (int c = 0; c < shared->componentCount; c++) {
		if (shared->componentIds[c] == componentId) {
			return true;
		}
	}
	if (shared->componentCount >= MAX_COMPONENTS) {
		return false;
	}
	shared->componentIds[shared->componentCount++] = componentId;
	return true;
}

/**
 * Returns the required property index for the name, reporting a message
 * when the property is not available in the data file.
 */
static int getRequiredIndex(DataSetIpi* dataSet, const char* name) {
	const int index = PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		name);
	if (index < 0) {
		printf(
			"The data file does not include the required property "
			"'%s'. An enterprise IP Intelligence data file that "
			"includes the weighted country code properties is "
			"required. To obtain one see "
			"https://51degrees.com/pricing?utm_source=code&utm_medium=example&utm_campaign=ip-intelligence-cxx&utm_content=examples-c-ipintelligence-countryoverlap.c&utm_term=missing-property\n",
			name);
	}
	return index;
}

/**
 * Writes the CSV pairing each primary country with the secondary
 * countries its addresses can also resolve to, for each combination of
 * location confidence and connection type. The row where the secondary
 * country equals the primary country counts the addresses that resolve
 * entirely to the primary country. The proportion, where 1 is 100%, is
 * of all the addresses whose primary country is the row's primary
 * country across every location confidence and connection type.
 */
static void writeCombinedCsv(
	FILE* file,
	Matrix cells,
	SharedBlocks* sharedBlocks,
	char* countryNames[COUNTRY_SLOTS],
	ValueTable* confidence,
	ValueTable* connection) {
	char buffer[8];
	char secondaryBuffer[8];

	// Total addresses for each primary country in any variant of
	// location confidence and connection type. Used as the denominator
	// for every percentage.
	static uint64_t primaryTotals[COUNTRY_SLOTS];
	memset(primaryTotals, 0, sizeof(primaryTotals));
	for (int c = 0; c < COUNTRY_SLOTS; c++) {
		for (int f = 0; f < confidence->count; f++) {
			for (int n = 0; n < connection->count; n++) {
				primaryTotals[c] += cells[c][f][n].addresses;
			}
		}
	}

	fprintf(
		file,
		"PrimaryCountryCode,PrimaryCountry,SecondaryCountryCode,"
		"SecondaryCountry,LocationConfidence,ConnectionType,"
		"IpAddressCount,ProportionOfPrimaryCountryAddresses\n");
	for (int c = 0; c < COUNTRY_SLOTS; c++) {
		const double total = (double)primaryTotals[c];
		for (int f = 0; f < confidence->count; f++) {
			for (int n = 0; n < connection->count; n++) {
				const Cell* cell = &cells[c][f][n];
				if (cell->addresses == 0) {
					continue;
				}
				// The row where the primary and secondary country are
				// the same counts the addresses that resolve entirely
				// to the primary country. The ISO code stands in for
				// a name the sweep never captured, which happens when
				// the country only ever appears as a secondary.
				char nameBuffer[8];
				const char* primaryName = countryNames[c] != NULL ?
					countryNames[c] : countryCode(c, nameBuffer);
				fprintf(
					file,
					"%s,\"%s\",%s,\"%s\",%s,%s,%llu,%.9g\n",
					countryCode(c, buffer),
					primaryName,
					countryCode(c, secondaryBuffer),
					primaryName,
					confidence->names[f],
					connection->names[n],
					(unsigned long long)cell->singleGeo,
					total > 0 ?
					(double)cell->singleGeo / total : 0);
				// One row per secondary country in descending order of
				// the addresses that can resolve to the combination.
				const uint64_t* block = sharedBlocks->blocks[f][n];
				if (block == NULL) {
					continue;
				}
				const uint64_t* row =
					block + (size_t)c * COUNTRY_SLOTS;
				bool used[COUNTRY_SLOTS];
				memset(used, 0, sizeof(used));
				for (;;) {
					int best = -1;
					uint64_t bestCount = 0;
					for (int s = 0; s < COUNTRY_SLOTS; s++) {
						if (used[s] == false && row[s] > bestCount) {
							bestCount = row[s];
							best = s;
						}
					}
					if (best < 0) {
						break;
					}
					used[best] = true;
					char secondaryNameBuffer[8];
					fprintf(
						file,
						"%s,\"%s\",%s,\"%s\",%s,%s,%llu,%.9g\n",
						countryCode(c, buffer),
						primaryName,
						countryCode(best, secondaryBuffer),
						countryNames[best] != NULL ?
						countryNames[best] :
						countryCode(best, secondaryNameBuffer),
						confidence->names[f],
						connection->names[n],
						(unsigned long long)bestCount,
						total > 0 ?
						(double)bestCount / total : 0);
				}
			}
		}
	}
}

/**
 * Prints the countries with the highest overlap share, optionally
 * restricted to a single connection type index, or -1 for all. When the
 * shared counts are provided the top shared countries are named for each
 * country in the table.
 */
static void printTop(
	Matrix cells,
	SharedBlocks* sharedBlocks,
	char* countryNames[COUNTRY_SLOTS],
	ValueTable* confidence,
	ValueTable* connection,
	int connectionFilter,
	int count) {
	char buffer[8];
	char sharedBuffer[8];
	printf(
		"%-5s %-30s %15s %15s %7s  %s\n",
		"Code",
		"Country",
		"Addresses",
		"Overlapping",
		"Share",
		sharedBlocks != NULL ? "Top shared countries" : "");
	bool printed[COUNTRY_UNKNOWN];
	memset(printed, 0, sizeof(printed));
	for (int rank = 0; rank < count; rank++) {
		int bestCountry = -1;
		double bestShare = -1;
		uint64_t bestAddresses = 0;
		uint64_t bestOverlapping = 0;
		for (int c = 0; c < COUNTRY_UNKNOWN; c++) {
			if (printed[c]) {
				continue;
			}
			uint64_t addresses = 0;
			uint64_t overlapping = 0;
			for (int f = 0; f < confidence->count; f++) {
				for (int n = 0; n < connection->count; n++) {
					if (connectionFilter >= 0 &&
						n != connectionFilter) {
						continue;
					}
					addresses += cells[c][f][n].addresses;
					overlapping += cells[c][f][n].multiGeo;
				}
			}
			if (addresses < 1000000) {
				continue;
			}
			const double share = (double)overlapping / (double)addresses;
			if (share > bestShare) {
				bestShare = share;
				bestCountry = c;
				bestAddresses = addresses;
				bestOverlapping = overlapping;
			}
		}
		if (bestCountry < 0) {
			break;
		}
		printed[bestCountry] = true;
		printf(
			"%-5s %-30s %15llu %15llu %6.2f%%  ",
			countryCode(bestCountry, buffer),
			countryNames[bestCountry] != NULL ?
			countryNames[bestCountry] : "",
			(unsigned long long)bestAddresses,
			(unsigned long long)bestOverlapping,
			100.0 * bestShare);
		if (sharedBlocks != NULL) {
			// Name the top shared countries for the country across all
			// the confidence values and connection types.
			bool used[COUNTRY_SLOTS];
			memset(used, 0, sizeof(used));
			for (int top = 0; top < TOP_SHARED; top++) {
				int best = -1;
				uint64_t bestCount = 0;
				for (int s = 0; s < COUNTRY_SLOTS; s++) {
					if (used[s]) {
						continue;
					}
					uint64_t value = 0;
					for (int f = 0; f < confidence->count; f++) {
						for (int n = 0; n < connection->count; n++) {
							const uint64_t* block =
								sharedBlocks->blocks[f][n];
							if (block != NULL) {
								value += block[
									(size_t)bestCountry *
									COUNTRY_SLOTS + s];
							}
						}
					}
					if (value > bestCount) {
						bestCount = value;
						best = s;
					}
				}
				if (best < 0) {
					break;
				}
				used[best] = true;
				printf(
					"%s%s %.1f%%",
					top > 0 ? ", " : "",
					countryCode(best, sharedBuffer),
					100.0 * (double)bestCount /
					(double)bestAddresses);
			}
		}
		printf("\n");
	}
}

/** Prints overall statistics and top countries per connection type. */
static void printSummary(
	Matrix cells,
	SharedBlocks* sharedBlocks,
	char* countryNames[COUNTRY_SLOTS],
	ValueTable* confidence,
	ValueTable* connection) {
	uint64_t total = 0;
	uint64_t multi = 0;
	for (int c = 0; c < COUNTRY_UNKNOWN; c++) {
		for (int f = 0; f < confidence->count; f++) {
			for (int n = 0; n < connection->count; n++) {
				total += cells[c][f][n].addresses;
				multi += cells[c][f][n].multiGeo;
			}
		}
	}
	printf(
		"\nIPv4 addresses that resolved to a country: %llu\n",
		(unsigned long long)total);
	printf(
		"Of which the area also overlaps one or more additional "
		"countries: %llu (%.2f%%)\n",
		(unsigned long long)multi,
		total > 0 ? 100.0 * (double)multi / (double)total : 0);

	// Overall share for each connection type.
	printf("\nBy connection type:\n");
	for (int n = 0; n < connection->count; n++) {
		uint64_t typeTotal = 0;
		uint64_t typeMulti = 0;
		for (int c = 0; c < COUNTRY_UNKNOWN; c++) {
			for (int f = 0; f < confidence->count; f++) {
				typeTotal += cells[c][f][n].addresses;
				typeMulti += cells[c][f][n].multiGeo;
			}
		}
		if (typeTotal == 0) {
			continue;
		}
		printf(
			"  %-22s %15llu addresses, %6.2f%% overlap additional "
			"countries\n",
			connection->names[n],
			(unsigned long long)typeTotal,
			100.0 * (double)typeMulti / (double)typeTotal);
	}

	printf(
		"\nCountries with at least 1 million addresses ordered by share "
		"of addresses that overlap additional countries (top 20, all "
		"connection types):\n");
	printTop(
		cells,
		sharedBlocks,
		countryNames,
		confidence,
		connection,
		-1,
		20);
	for (int n = 0; n < connection->count; n++) {
		if (strcmp(connection->names[n], "Cellular") == 0 ||
			strcmp(connection->names[n], "Broadband") == 0) {
			printf(
				"\nTop 10 for connection type '%s':\n",
				connection->names[n]);
			printTop(
				cells,
				NULL,
				countryNames,
				confidence,
				connection,
				n,
				10);
		}
	}
}

/**
 * Runs the country overlap analysis.
 * @param dataFilePath path to the enterprise data file.
 * @param configProvided configuration to use, or null to use the
 * Balanced configuration falling back to LowMemory when memory is not
 * available.
 * @param outputPath path for the output CSV.
 * @param threadCount number of worker threads.
 * @param totalChunks number of /8 chunks of the IPv4 address space to
 * process starting at 0.0.0.0, up to 256 for the whole space.
 * @return COUNTRY_OVERLAP_OK on success,
 * COUNTRY_OVERLAP_PROPERTIES_MISSING when the data file does not include
 * the required properties, or COUNTRY_OVERLAP_FAILED on any other
 * failure.
 */
int fiftyoneDegreesIpiCountryOverlap(
	const char* dataFilePath,
	fiftyoneDegreesConfigIpi* configProvided,
	const char* outputPath,
	int threadCount,
	int totalChunks) {
	if (threadCount < 1) threadCount = 1;
	if (totalChunks < 1 || totalChunks > 256) totalChunks = 256;

	ResourceManager manager;
	EXCEPTION_CREATE;
	const uint16_t concurrency = (uint16_t)(threadCount + 2);
	PropertiesRequired properties = PropertiesDefault;
	properties.string = countryOverlapProperties;

	// The Balanced configuration loads the graph structures into memory
	// which gives the best sweep throughput but requires several GB of
	// committed memory for a large data file. When no configuration is
	// provided and that allocation fails, the example falls back to the
	// LowMemory configuration with enlarged caches. The operating system
	// file cache keeps the hot parts of the data file in physical memory
	// so the fallback remains fast for the sequential access pattern of
	// the sweep.
	StatusCode status = NOT_SET;
	const int attempts = configProvided != NULL ? 1 : 2;
	for (int attempt = 0; attempt < attempts; attempt++) {
		ConfigIpi config;
		if (configProvided != NULL) {
			config = *configProvided;
			// The sweep evaluates the graph for billions of addresses
			// and resolves hundreds of thousands of distinct locations,
			// so a configuration that neither loads nor caches the
			// collections involved would spend hours re-reading the
			// same records from disk. Enforce minimum caches on any
			// collection that has neither, so every standard
			// configuration remains practical for the sweep.
			bool adjusted = false;
#define COUNTRY_OVERLAP_MIN_CACHE(collection, size) \
			if (config.collection.loaded == false && \
				config.collection.capacity == 0) { \
				config.collection.capacity = size; \
				adjusted = true; \
			}
			COUNTRY_OVERLAP_MIN_CACHE(graph, 200000)
			COUNTRY_OVERLAP_MIN_CACHE(graphs, 1000)
			COUNTRY_OVERLAP_MIN_CACHE(strings, 50000)
			COUNTRY_OVERLAP_MIN_CACHE(values, 10000)
			COUNTRY_OVERLAP_MIN_CACHE(profiles, 10000)
			COUNTRY_OVERLAP_MIN_CACHE(profileGroups, 50000)
			COUNTRY_OVERLAP_MIN_CACHE(profileOffsets, 10000)
			COUNTRY_OVERLAP_MIN_CACHE(propertyTypes, 1000)
			COUNTRY_OVERLAP_MIN_CACHE(properties, 1000)
			COUNTRY_OVERLAP_MIN_CACHE(components, 100)
			COUNTRY_OVERLAP_MIN_CACHE(maps, 100)
#undef COUNTRY_OVERLAP_MIN_CACHE
			if (adjusted) {
				printf(
					"The provided configuration leaves collections "
					"without caches, using minimum caches for the "
					"sweep.\n");
			}
		}
		else if (attempt == 0) {
			config = IpiBalancedConfig;
		}
		else {
			printf(
				"Balanced configuration failed, falling back to the "
				"LowMemory configuration with enlarged caches.\n");
			config = IpiLowMemoryConfig;
			config.graph.capacity = 200000;
			config.graphs.capacity = 1000;
			config.profileGroups.capacity = 50000;
			config.profileOffsets.capacity = 10000;
			config.profiles.capacity = 10000;
			config.values.capacity = 10000;
			config.strings.capacity = 50000;
			config.properties.capacity = 1000;
			config.components.capacity = 100;
			config.maps.capacity = 100;
			config.propertyTypes.capacity = 1000;
		}
		// Size the caches for the expected concurrency.
		config.strings.concurrency = concurrency;
		config.components.concurrency = concurrency;
		config.maps.concurrency = concurrency;
		config.properties.concurrency = concurrency;
		config.values.concurrency = concurrency;
		config.profiles.concurrency = concurrency;
		config.graphs.concurrency = concurrency;
		config.profileGroups.concurrency = concurrency;
		config.profileOffsets.concurrency = concurrency;
		config.propertyTypes.concurrency = concurrency;
		config.graph.concurrency = concurrency;

		EXCEPTION_CLEAR;
		status = IpiInitManagerFromFile(
			&manager,
			&config,
			&properties,
			dataFilePath,
			exception);
		// Both the status and the exception must be clean. The status
		// can report success while the exception carries a failure from
		// the graph structures, in which case the manager must not be
		// used.
		if (status == SUCCESS && EXCEPTION_OKAY) {
			break;
		}
		if (status == SUCCESS) {
			status = EXCEPTION_FAILED ? exception->status : NOT_SET;
		}
		// When none of the required properties exist in the data file
		// there is no point retrying with another configuration. This
		// is the expected outcome for the Lite data file.
		if (status == REQ_PROP_NOT_PRESENT) {
			printf(
				"The data file does not include the properties required "
				"by this example (%s). An enterprise IP Intelligence "
				"data file is required. To obtain one see "
				"https://51degrees.com/pricing?utm_source=code&utm_medium=example&utm_campaign=ip-intelligence-cxx&utm_content=examples-c-ipintelligence-countryoverlap.c&utm_term=properties-missing\n",
				countryOverlapProperties);
			return COUNTRY_OVERLAP_PROPERTIES_MISSING;
		}
	}
	if (status != SUCCESS || EXCEPTION_FAILED) {
		countryOverlapReportStatus(status, dataFilePath);
		printf(
			"The data file could not be initialised. This can happen "
			"when the machine does not have enough free committed "
			"memory for the selected configuration.\n");
		return COUNTRY_OVERLAP_FAILED;
	}

	DataSetIpi* dataSet = DataSetIpiGet(&manager);

	// Stop when any required property is missing from the data file. The
	// analysis is meaningless without the weighted country code lists,
	// which the Lite data file does not include.
	SharedState shared;
	memset(&shared, 0, sizeof(shared));
	shared.manager = &manager;
	shared.dataSet = dataSet;
	shared.totalChunks = totalChunks;
	shared.reqIndexCountryCode = getRequiredIndex(
		dataSet, PROPERTY_COUNTRY_CODE);
	shared.reqIndexCountry = getRequiredIndex(dataSet, PROPERTY_COUNTRY);
	shared.reqIndexConfidence = getRequiredIndex(
		dataSet, PROPERTY_CONFIDENCE);
	shared.reqIndexConnection = getRequiredIndex(
		dataSet, PROPERTY_CONNECTION);
	shared.reqIndexGeo = getRequiredIndex(dataSet, PROPERTY_GEO);
	shared.reqIndexPop = getRequiredIndex(dataSet, PROPERTY_POP);
	if (shared.reqIndexCountryCode < 0 ||
		shared.reqIndexCountry < 0 ||
		shared.reqIndexConfidence < 0 ||
		shared.reqIndexConnection < 0 ||
		shared.reqIndexGeo < 0 ||
		shared.reqIndexPop < 0) {
		DataSetIpiRelease(dataSet);
		ResourceManagerFree(&manager);
		return COUNTRY_OVERLAP_PROPERTIES_MISSING;
	}

	// Determine the distinct component graphs that the dimension
	// properties depend on.
	if (addComponentForProperty(
		&shared, PROPERTY_COUNTRY_CODE) == false ||
		addComponentForProperty(&shared, PROPERTY_CONNECTION) == false) {
		printf("Could not determine the required components.\n");
		DataSetIpiRelease(dataSet);
		ResourceManagerFree(&manager);
		return COUNTRY_OVERLAP_FAILED;
	}

	// The segment key packs each component's 32 bit graph offset into a
	// 64 bit value, which is exact for up to two components. More would
	// silently discard offsets and corrupt the segment detection, so stop
	// rather than produce wrong counts.
	if (shared.componentCount > 2) {
		printf(
			"The required properties span %d component graphs which "
			"exceeds the two the segment key supports.\n",
			shared.componentCount);
		DataSetIpiRelease(dataSet);
		ResourceManagerFree(&manager);
		return COUNTRY_OVERLAP_FAILED;
	}

	printf(
		"Sweeping %d /8 chunks of the IPv4 address space with %d "
		"threads evaluating %d component graphs per address.\n",
		totalChunks,
		threadCount,
		shared.componentCount);

	// Create the thread states and start the workers.
	ThreadState* states = (ThreadState*)Malloc(
		sizeof(ThreadState) * threadCount);
	FIFTYONE_DEGREES_THREAD* threads = (FIFTYONE_DEGREES_THREAD*)Malloc(
		sizeof(FIFTYONE_DEGREES_THREAD) * threadCount);
	for (int t = 0; t < threadCount; t++) {
		ThreadState* state = &states[t];
		memset(state, 0, sizeof(ThreadState));
		state->shared = &shared;
		state->results = ResultsIpiCreate(&manager);
		state->cache = (Resolved*)Malloc(sizeof(Resolved) * CACHE_SIZE);
		memset(state->cache, 0, sizeof(Resolved) * CACHE_SIZE);
	}
	time_t started = time(NULL);
	for (int t = 0; t < threadCount; t++) {
		FIFTYONE_DEGREES_THREAD_CREATE(
			threads[t],
			(FIFTYONE_DEGREES_THREAD_ROUTINE)&sweepThread,
			&states[t]);
	}

	// Report progress until all the threads have finished.
	while (shared.threadsDone < threadCount) {
#ifdef _MSC_VER
		Sleep(30000);
#else
		sleep(30);
#endif
		if (shared.threadsDone >= threadCount) {
			break;
		}
		uint64_t done = 0;
		for (int t = 0; t < threadCount; t++) {
			done += states[t].addressesDone;
		}
		double progress = (double)done /
			(TOTAL_IPV4 * totalChunks / 256.0);
		double elapsed = difftime(time(NULL), started);
		double remaining = progress > 0 ?
			elapsed * (1 - progress) / progress : 0;
		printf(
			"Covered %.1f%% of the target address space in %.0fs, "
			"estimated %.0fs remaining.\n",
			progress * 100,
			elapsed,
			remaining);
		fflush(stdout);
	}
	for (int t = 0; t < threadCount; t++) {
		FIFTYONE_DEGREES_THREAD_JOIN(threads[t]);
		FIFTYONE_DEGREES_THREAD_CLOSE(threads[t]);
	}
	double elapsed = difftime(time(NULL), started);

	// Merge the thread results using global dimension tables so the
	// indexes align across threads. The merge tables are static because
	// they are too large for the stack.
	static Matrix totals;
	static char* names[COUNTRY_SLOTS];
	SharedBlocks totalShared;
	ValueTable confidence;
	ValueTable connection;
	memset(&confidence, 0, sizeof(confidence));
	memset(&connection, 0, sizeof(connection));
	memset(totals, 0, sizeof(totals));
	memset(&totalShared, 0, sizeof(totalShared));
	memset(names, 0, sizeof(names));
	uint64_t evaluations = 0;
	uint64_t resolutions = 0;
	uint64_t failures = 0;
	uint64_t truncations = 0;
	for (int t = 0; t < threadCount; t++) {
		ThreadState* state = &states[t];
		evaluations += state->evaluations;
		resolutions += state->resolutions;
		failures += state->failures;
		truncations += state->truncations;
		for (int c = 0; c < COUNTRY_SLOTS; c++) {
			if (names[c] == NULL && state->countryNames[c] != NULL) {
				names[c] = state->countryNames[c];
			}
			for (int f = 0; f < state->confidence.count; f++) {
				const int globalF = valueIndex(
					&confidence,
					state->confidence.names[f]);
				for (int n = 0; n < state->connection.count; n++) {
					const Cell* cell = &state->cells[c][f][n];
					if (cell->addresses == 0 && cell->segments == 0) {
						continue;
					}
					Cell* target = &totals
						[c]
						[globalF]
						[valueIndex(
							&connection,
							state->connection.names[n])];
					target->segments += cell->segments;
					target->addresses += cell->addresses;
					target->singleGeo += cell->singleGeo;
					target->multiGeo += cell->multiGeo;
					target->singlePop += cell->singlePop;
					target->multiPop += cell->multiPop;
				}
			}
		}
		// Merge the pair count blocks aligning the dimension indexes
		// with the global tables.
		for (int f = 0; f < state->confidence.count; f++) {
			const int globalF = valueIndex(
				&confidence,
				state->confidence.names[f]);
			for (int n = 0; n < state->connection.count; n++) {
				const uint64_t* source = state->sharedBlocks.blocks[f][n];
				if (source == NULL) {
					continue;
				}
				uint64_t* target = sharedBlockGet(
					&totalShared,
					globalF,
					valueIndex(&connection, state->connection.names[n]));
				if (target != NULL) {
					const size_t entries =
						(size_t)COUNTRY_SLOTS * COUNTRY_SLOTS;
					for (size_t i = 0; i < entries; i++) {
						target[i] += source[i];
					}
				}
			}
		}
	}

	printf(
		"\nCompleted %llu graph evaluations with %llu distinct "
		"combinations resolved and %llu failures in %.0f seconds.\n",
		(unsigned long long)evaluations,
		(unsigned long long)resolutions,
		(unsigned long long)failures,
		elapsed);
	if (truncations > 0) {
		printf(
			"%llu distinct locations overlapped more than %d other "
			"countries, so the pair counts for those locations exclude "
			"the lowest weighted countries.\n",
			(unsigned long long)truncations,
			SHARED_STORE);
	}

	FILE* file = fopen(outputPath, "w");
	if (file != NULL) {
		writeCombinedCsv(
			file,
			totals,
			&totalShared,
			names,
			&confidence,
			&connection);
		fclose(file);
		printf("Results written to '%s'.\n", outputPath);
	}
	else {
		printf("Could not open '%s' for writing.\n", outputPath);
	}

	// Spot check randomly selected addresses against the normal lookup
	// process so the method can be seen to produce the same values as a
	// single lookup would. The seed is fixed so runs are reproducible.
	printf(
		"\nSpot checking %d random addresses against the normal lookup "
		"process:\n",
		SPOT_CHECKS);
	srand(42);
	int matches = 0;
	for (int i = 0; i < SPOT_CHECKS; i++) {
		const uint32_t chunk = (uint32_t)(rand() % totalChunks);
		const uint32_t withinChunk =
			(((uint32_t)rand() << 9) | ((uint32_t)rand() & 0x1FF)) &
			0x00FFFFFF;
		if (spotCheck(&states[0], (chunk << 24) | withinChunk)) {
			matches++;
		}
	}
	printf(
		"%d of %d spot checks match the normal lookup process.\n",
		matches,
		SPOT_CHECKS);

	printSummary(totals, &totalShared, names, &confidence, &connection);

	// Free all the resources.
	for (int t = 0; t < threadCount; t++) {
		ThreadState* state = &states[t];
		ResultsIpiFree(state->results);
		Free(state->cache);
		sharedBlocksFree(&state->sharedBlocks);
		valueTableFree(&state->confidence);
		valueTableFree(&state->connection);
		// Country names moved into the global table are freed below, so
		// only free names that were not selected.
		for (int c = 0; c < COUNTRY_SLOTS; c++) {
			if (state->countryNames[c] != NULL &&
				state->countryNames[c] != names[c]) {
				Free(state->countryNames[c]);
			}
		}
	}
	for (int c = 0; c < COUNTRY_SLOTS; c++) {
		if (names[c] != NULL) {
			Free(names[c]);
			names[c] = NULL;
		}
	}
	valueTableFree(&confidence);
	valueTableFree(&connection);
	sharedBlocksFree(&totalShared);
	Free(states);
	Free(threads);
	DataSetIpiRelease(dataSet);
	ResourceManagerFree(&manager);
	return matches == SPOT_CHECKS ?
		COUNTRY_OVERLAP_OK : COUNTRY_OVERLAP_FAILED;
}

#ifndef TEST

static const char* dataDir = "ip-intelligence-data";

/** The enterprise data file is searched for first because the analysis
requires properties the Lite file does not include. The Lite name is
still searched so a clear message about the missing properties can be
given rather than a file not found error. */
static const char* dataFileNames[] = {
	"51Degrees-EnterpriseIpiV41.ipi",
	"51Degrees-LiteV41.ipi",
};

/**
 * Prints the resolved values for a single IPv4 address so the results
 * can be compared against other 51Degrees APIs for verification.
 */
static void probeAddress(
	SharedState* shared,
	ResultsIpi* results,
	ValueTable* confidence,
	ValueTable* connection,
	const char* ip) {
	unsigned a, b, c, d;
	(void)confidence;
	(void)connection;
	if (sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
		printf("'%s' is not an IPv4 address.\n", ip);
		return;
	}
	EXCEPTION_CREATE;
	byte bytes[4] = {
		(byte)a, (byte)b, (byte)c, (byte)d };
	ResultsIpiFromIpAddress(
		results,
		bytes,
		sizeof(bytes),
		FIFTYONE_DEGREES_IP_TYPE_IPV4,
		exception);
	EXCEPTION_THROW;
	WeightedValuesCollection collection = ResultsIpiGetValuesCollection(
		results,
		NULL,
		0,
		NULL,
		exception);
	EXCEPTION_THROW;
	const char* code = highestWeighted(
		&collection, shared->reqIndexCountryCode);
	const char* conf = highestWeighted(
		&collection, shared->reqIndexConfidence);
	const char* conn = highestWeighted(
		&collection, shared->reqIndexConnection);
	printf(
		"%s cc=%s lc=%s ct=%s geo=[",
		ip,
		code != NULL ? code : "Unknown",
		conf != NULL ? conf : "Unknown",
		conn != NULL ? conn : "Unknown");
	// Weights are displayed as each value's share of the total weight
	// for the property, which is stable across the raw weighting scales
	// used by different versions of the API.
	double totalWeight = 0;
	for (uint32_t i = 0; i < collection.itemsCount; i++) {
		const fiftyoneDegreesWeightedValueHeader* header =
			collection.items[i];
		if (header->requiredPropertyIndex == shared->reqIndexGeo) {
			totalWeight += (double)header->rawWeighting;
		}
	}
	for (uint32_t i = 0; i < collection.itemsCount; i++) {
		const fiftyoneDegreesWeightedValueHeader* header =
			collection.items[i];
		if (header->requiredPropertyIndex == shared->reqIndexGeo &&
			header->valueType ==
			FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING &&
			totalWeight > 0) {
			printf(
				"%s:%.3f|",
				((const fiftyoneDegreesWeightedString*)header)->value,
				(double)header->rawWeighting / totalWeight);
		}
	}
	printf("] geoDistinct=%d popDistinct=%d\n",
		countDistinct(&collection, shared->reqIndexGeo),
		countDistinct(&collection, shared->reqIndexPop));
	WeightedValuesCollectionRelease(&collection);
}

/**
 * Initialises the manager and prints the values for each of the
 * addresses provided on the command line.
 */
static int runProbe(const char* dataFilePath, int argc, char* argv[]) {
	ResourceManager manager;
	EXCEPTION_CREATE;
	ConfigIpi config = IpiLowMemoryConfig;
	PropertiesRequired properties = PropertiesDefault;
	properties.string = countryOverlapProperties;
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);
	if (status != SUCCESS || EXCEPTION_FAILED) {
		countryOverlapReportStatus(status, dataFilePath);
		return COUNTRY_OVERLAP_FAILED;
	}
	DataSetIpi* dataSet = DataSetIpiGet(&manager);
	SharedState shared;
	memset(&shared, 0, sizeof(shared));
	shared.manager = &manager;
	shared.dataSet = dataSet;
	shared.reqIndexCountryCode = getRequiredIndex(
		dataSet, PROPERTY_COUNTRY_CODE);
	shared.reqIndexCountry = getRequiredIndex(dataSet, PROPERTY_COUNTRY);
	shared.reqIndexConfidence = getRequiredIndex(
		dataSet, PROPERTY_CONFIDENCE);
	shared.reqIndexConnection = getRequiredIndex(
		dataSet, PROPERTY_CONNECTION);
	shared.reqIndexGeo = getRequiredIndex(dataSet, PROPERTY_GEO);
	shared.reqIndexPop = getRequiredIndex(dataSet, PROPERTY_POP);
	if (shared.reqIndexCountryCode < 0 ||
		shared.reqIndexCountry < 0 ||
		shared.reqIndexConfidence < 0 ||
		shared.reqIndexConnection < 0 ||
		shared.reqIndexGeo < 0 ||
		shared.reqIndexPop < 0) {
		DataSetIpiRelease(dataSet);
		ResourceManagerFree(&manager);
		return COUNTRY_OVERLAP_PROPERTIES_MISSING;
	}
	ResultsIpi* results = ResultsIpiCreate(&manager);
	ValueTable confidence;
	ValueTable connection;
	memset(&confidence, 0, sizeof(confidence));
	memset(&connection, 0, sizeof(connection));
	for (int i = 3; i < argc; i++) {
		probeAddress(&shared, results, &confidence, &connection, argv[i]);
	}
	ResultsIpiFree(results);
	valueTableFree(&confidence);
	valueTableFree(&connection);
	DataSetIpiRelease(dataSet);
	ResourceManagerFree(&manager);
	return COUNTRY_OVERLAP_OK;
}

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	dataFilePath[0] = '\0';
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		for (int i = 0;
			i < (int)(sizeof(dataFileNames) / sizeof(dataFileNames[0]));
			i++) {
			status = FileGetPath(
				dataDir,
				dataFileNames[i],
				dataFilePath,
				sizeof(dataFilePath));
			if (status == SUCCESS) {
				break;
			}
		}
		if (status != SUCCESS) {
			countryOverlapReportStatus(status, dataFileNames[0]);
			printf(
				"Provide the path to an enterprise IP Intelligence "
				"data file as the first argument.\n");
			return 1;
		}
	}

	// Probe mode prints the values for the supplied addresses so
	// results can be verified against other 51Degrees APIs.
	// Usage: CountryOverlap <data file> --probe <ip> [ip...]
	if (argc > 3 && strcmp(argv[2], "--probe") == 0) {
		return runProbe(dataFilePath, argc, argv);
	}

	const char* outputPath = argc > 2 ? argv[2] :
		"country-overlap.csv";
	const int threadCount = argc > 3 ? atoi(argv[3]) : 10;
	const int totalChunks = argc > 4 ? atoi(argv[4]) : 256;

	return fiftyoneDegreesIpiCountryOverlap(
		dataFilePath,
		NULL,
		outputPath,
		threadCount,
		totalChunks);
}

#endif
