/**
@example IpIntelligence/OfflineProcessing.c
Offline processing example of using 51Degrees IP intelligence.

This example demonstrates one possible use of the 51Degrees on-premise IP intelligence
API and data for offline data processing. It also demonstrates that you can reuse the
retrieved results for multiple uses and only then release it.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/blob/master/examples/OfflineProcessing.c).

@include{doc} example-ipi-require-datafile.txt

@include{doc} example-ipi-how-to-run.txt

Expected output:

```
Result
```

In detail, the example shows how to:

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "Country";
```

2. Instantiate the 51Degrees data set within a resource manager from the
specified data file with the required properties and the specified
configuration.
```
fiftyoneDegreesStatusCode status =
	fiftyoneDegreesIpiInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);
```

3. Create a results instance ready to be populated by the data set.
```
fiftyoneDegreesResultsIpi* results =
	fiftyoneDegreesResultsIpiCreate(
		&manager);
```

4. Open an output file to write the results to.
```
	FILE* fout = fopen(outputFile, "w");
```

5. Write a header to the output file with the property names in '|'	separated
CSV format ('|' separated because some properties contain commas)
```
fprintf(fout, "IP Address");
for (i = 0; i < dataSet->b.b.available->count; i++) {
	fprintf(fout, ",\"%s\"",
		&((fiftyoneDegreesString*)
			dataSet->b.b.available->items[i].name.data.ptr)->value);
}
fprintf(fout, "\n");
```

6. Iterate over the IP Addresses in an input file writing the processing results
to the output file.
```
fiftyoneDegreesTextFileIterate(
	ipAddressFilePath,
	ipAddress,
	sizeof(ipAddress),
	&state,
	executeTest);
```

7. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

*/

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#else
#include "dmalloc.h"
#endif
#endif

#include "../../../src/ipi.h"
#include "../../../src/common-cxx/textfile.h"
#include "../../../src/fiftyone.h"

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV4.1.ipi";

static const char* ipAddressFileName = "20000 IP Addresses.csv";

static char valueBuffer[1024] = "";

static char* getPropertyValueAsString(
	ResultsIpi* results,
	uint32_t requiredPropertyIndex) {
	EXCEPTION_CREATE;
	ResultsIpiGetValuesStringByRequiredPropertyIndex(
		results,
		requiredPropertyIndex,
		valueBuffer,
		sizeof(valueBuffer),
		",",
		exception);
	EXCEPTION_THROW;
	return valueBuffer;
}

/**
 * CHOOSE THE DEFAULT MEMORY CONFIGURATION BY UNCOMMENTING ONE OF THE FOLLOWING
 * MACROS.
 */

// #define CONFIG fiftyoneDegreesIpiInMemoryConfig
// #define CONFIG fiftyoneDegreesIpiHighPerformanceConfig
// #define CONFIG fiftyoneDegreesIpiLowMemoryConfig
#define CONFIG fiftyoneDegreesIpihBalancedConfig
// #define CONFIG fiftyoneDegreesIpiBalancedTempConfig

 /**
  * State used for the offline processing operation.
  */
typedef struct t_offline_processing_state {
	FILE* output; /**< Output stream for the results */
	ResultsIpi* results; /**< The results used by the thread */
} offlineProcessState;

/**
 * Processes the IP Addresses provided, writing the results to the output file.
 * Called from the text file iterator.
 * @param ipAddress to be used for the test
 * @param state instance of offlineProcessState
 */
static void process(const char* ipAddress, void* state) {
	EXCEPTION_CREATE;
	uint32_t i;
	offlineProcessState* offline = (offlineProcessState*)state;
	ResultIpi* result;
	DataSetIpi* dataSet = (DataSetIpi*)offline->results->b.dataSet;
	ResultsIpiFromIpAddressString(
		offline->results,
		ipAddress,
		strlen(ipAddress),
		exception);
	result = (ResultIpi*)offline->results->items;

	// Write the IP Address.
	fprintf(offline->output, "\"%s\"", ipAddress);

	// Write all the available properties.
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		if (ResultsIpiGetValues(
			offline->results,
			i,
			exception) == NULL ||
			EXCEPTION_FAILED ||
			offline->results->values.count == 0) {

			// Write an empty value if one isn't available.
			fprintf(offline->output, ",\"\"");
		}
		else {

			// Write value(s) with comma separator.
			fprintf(offline->output, "|\"");
			fprintf(offline->output, "%s", getPropertyValueAsString(
				offline->results,
				i));
			fprintf(offline->output, "\"");
		}
	}
	fprintf(offline->output, "\n");
}

void run(
	ResourceManager* manager,
	const char* ipAddressFilePath,
	const char* outputFilePath) {
	uint32_t i;
	char ipAddress[40];
	offlineProcessState state;
	DataSetIpi* dataSet;

	// Open a fresh data file for writing the output to.
	FileDelete(outputFilePath);
	state.output = fopen(outputFilePath, "w");
	if (state.output == NULL) {
		printf("Could not open file %s for write\n", outputFilePath);
		return;
	}

	// Get the results and data set from the manager.
	state.results = ResultsIpiCreate(manager);
	dataSet = (DataSetIpi*)state.results->b.dataSet;

	printf("Starting Offline Processing Example.\n");

	// Print CSV headers to output file.
	fprintf(state.output, "\"IP Address\"");
	for (i = 0; i < dataSet->b.b.available->count; i++) {
		fprintf(state.output, "|\"%s\"",
			&((String*)dataSet->b.b.available->items[i].name.data.ptr)->value);
	}
	fprintf(state.output, "\n");

	// Perform offline processing.
	TextFileIterate(
		ipAddressFilePath,
		ipAddress,
		sizeof(ipAddress),
		&state,
		process);

	fclose(state.output);
	printf("Output Written to %s\n", outputFilePath);

	// Free the memory used by the results instance.
	ResultsIpiFree(state.results);
}

/**
 * Reports the status of the data file initialization.
 * @param status code to be displayed
 * @param fileName to be used in any messages
 */
static void reportStatus(
	StatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

/**
 * Start the offline processing with the files and configuration provided.
 * @param dataFilePath full file path to the ip intelligence data file
 * @param ipAddressFilePath full file path to the IP Address test data
 * @param config configuration to use for the memory test
 */
void fiftyoneDegreesOfflineProcessingRun(
	const char* dataFilePath,
	const char* ipAddressFilePath,
	const char* outputFilePath,
	const char* requiredProperties,
	ConfigIpi config) {
	EXCEPTION_CREATE;

	// Set concurrency to ensure sufficient shared resources available.
	config.ipv4Ranges.concurrency =
		config.ipv6Ranges.concurrency =
		config.profiles.concurrency =
		config.profileOffsets.concurrency =
		config.profileCombinations.concurrency =
		config.values.concurrency =
		config.strings.concurrency = 1;

	// Set the required properties for the output file.
	PropertiesRequired properties = PropertiesDefault;
	properties.string = requiredProperties;

	ResourceManager manager;
	StatusCode status = IpiInitManagerFromFile(
		&manager,
		&config,
		&properties,
		dataFilePath,
		exception);

	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
	}
	else {

		// Process the IP Addresses writing the results to the output path.
		run(&manager, ipAddressFilePath, outputFilePath);

		// Free the memory used by the data set.
		ResourceManagerFree(&manager);
	}
}

#ifndef TEST

/**
 * Only included if the example us being used from the console. Not included
 * when part of a test framework where the main method is not required.
 * @arg1 data file path
 * @arg2 IP Addresses file path
 */
int main(int argc, char* argv[]) {
	int i = 0;
	StatusCode status = SUCCESS;
	char dataFilePath[FILE_MAX_PATH];
	char ipAddressFilePath[FILE_MAX_PATH];
	char outputFilePath[FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = FileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, dataFileName);
		fgetc(stdin);
		return 1;
	}
	if (argc > 2) {
		strcpy(ipAddressFilePath, argv[2]);
	}
	else {
		status = FileGetPath(
			dataDir,
			ipAddressFileName,
			ipAddressFilePath,
			sizeof(ipAddressFilePath));
	}
	if (status != SUCCESS) {
		reportStatus(status, ipAddressFilePath);
		fgetc(stdin);
		return 1;
	}
	if (argc > 3) {
		strcpy(outputFilePath, argv[3]);
	}
	else {
		while (ipAddressFilePath[i] != '.' && ipAddressFilePath[i] != '\0') {
			outputFilePath[i] = ipAddressFilePath[i];
			i++;
		}
		strcpy(&outputFilePath[i], ".processed.csv");
	}

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#else
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif
#endif

	// Start the offline processing.
	fiftyoneDegreesOfflineProcessingRun(
		dataFilePath,
		ipAddressFilePath,
		outputFilePath,
		argc > 4 ? argv[4] : "Country,City,AverageLocation,"
		"RangeStart,RangeEnd",
		CONFIG);

#ifdef _DEBUG
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
#else
	printf("Log file is %s\r\n", dmalloc_logpath);
#endif
#endif

	// Wait for a character to be pressed.
	fgetc(stdin);

	return 0;
}

#endif
