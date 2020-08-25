/**
@example IpIntelligence/GettingStarted.c
Getting started example of using 51Degrees IP intelligence.

The example shows how to use 51Degrees on-premise IP intelligence to
determine the country of a given IP address.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/blob/master/examples/GettingStarted.c).

@include{doc} example-ipi-require-datafile.txt

@include{doc} example-ipi-how-to-run.txt

Expected output:

```
Result
```

In detail, the example shows how to

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "RangeStart,RangeEnd,Country,City,AverageLocation";
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
fiftyoneDegreesResultsIpi *results =
	fiftyoneDegreesResultsIpiCreate(
		&manager);
```

4. Process a single IP addess string to retrieve the values associated
with the IP addess for the selected properties.
```
fiftyoneDegreesResultsIpiFromIpAddressString(
	results,
	ipv4Address,
	strlen(ipv4Address),
	exception);
```

5. Extract the value of a property as a string from the results.
```
fiftyoneDegreesResultsIpiGetValuesString(
	results,
	propertyName,
	valueBuffer,
	sizeof(valueBuffer),
	"|",
	exception);
```

6. Release the memory used by the results.
```
fiftyoneDegreesResultsIpiFree(results);
```

7. Finally release the memory used by the data set resource.
```
fiftyoneDegreesResourceManagerFree(&manager);
```

*/

#ifdef _DEBUG
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include "dmalloc.h"
#endif
#endif

#include <stdio.h>
#include "../../../src/ipi.h"
#include "../../../src/fiftyone.h"

static const char* dataDir = "ip-intelligence-data";

static const char* dataFileName = "51Degrees-LiteV4.1.ipi";

static char valueBuffer[1024] = "";

static const char* getPropertyValueAsString(
	ResultsIpi* results,
	const char* propertyName) {
	EXCEPTION_CREATE;
	ResultsIpiGetValuesString(
		results,
		propertyName,
		valueBuffer,
		sizeof(valueBuffer),
		"|",
		exception);
	EXCEPTION_THROW;
	return valueBuffer;
}

/**
 * Reports the status of the data file initialization.
 */
static void reportStatus(StatusCode status,
	const char* fileName) {
	const char* message = StatusGetMessage(status, fileName);
	printf("%s\n", message);
	Free((void*)message);
}

void fiftyoneDegreesIpiGettingStarted(
	const char* dataFilePath,
	ConfigIpi* config) {
	ResourceManager manager;
	EXCEPTION_CREATE;

	// Set the properties to be returned for each ip
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "RangeStart,RangeEnd,Country,City,AverageLocation";

	StatusCode status = IpiInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
		fgetc(stdin);
		return;
	}

	// Create a results instance to store and process ip
	ResultsIpi* results = ResultsIpiCreate(&manager);

	// an ipv4 address string
	const char* ipv4Address = "8.8.8.8";

	// an ipv6 address string
	const char* ipv6Address = "2001:4860:4860::8888::2001:4860:4860::8844";

	printf("Starting Getting Started Example.\n");

	// Carries out a match for the ipv4 address
	printf("\nIpv4 Address: %s\n", ipv4Address);
	ResultsIpiFromIpAddressString(
		results,
		ipv4Address,
		strlen(ipv4Address),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printf("Countries: %s\n", getPropertyValueAsString(results, "Country"));

	// Carries out a match for the ipv6 address
	printf("\nIpv6 Address: %s\n", ipv6Address);
	ResultsIpiFromIpAddressString(
		results,
		ipv6Address,
		strlen(ipv6Address),
		exception);
	if (EXCEPTION_FAILED) {
		printf("%s\n", ExceptionGetMessage(exception));
	}
	printf("Countries: %s\n", getPropertyValueAsString(results, "Country"));

	// Ensure the results are freed to avoid memory leaks.
	ResultsIpiFree(results);

	// Free the resources used by the manager
	ResourceManagerFree(&manager);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	ConfigIpi config = IpiDefaultConfig;
	// ConfigIpi config = IpiInMemoryConfig;
	// ConfigIpi config = IpiHighPerformanceConfig;
	// ConfigIpi config = IpiLowMemoryConfig;
	// ConfigIpi config = IpiBalancedConfig;
	char dataFilePath[FILE_MAX_PATH];
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

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif

	fiftyoneDegreesIpiGettingStarted(
		dataFilePath,
		&config);

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