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

/**
@example IpIntelligence/GettingStarted.c
Getting started example of using 51Degrees IP intelligence.

The example shows how to use 51Degrees on-premise IP intelligence to
determine the country of a given IP address.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/blob/master/examples/C/GettingStarted.c).

@include{doc} example-require-datafile-ipi.txt

@include{doc} example-how-to-run-ipi.txt

In detail, the example shows how to

1. Specify the name of the data file and properties the data set should be
initialised with.
```
const char* fileName = argv[1];
fiftyoneDegreesPropertiesRequired properties =
	fiftyoneDegreesPropertiesDefault;
properties.string = "IpRangeStart,IpRangeEnd,CountryCode,AverageLocation";
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

4. Process a single IP address string to retrieve the values associated
with the IP address for the selected properties.
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

Expected output:
```
...
Ipv4 Address: 185.28.167.77
IpRangeStart: "185.28.167.0":"1.000000"
IpRangeEnd: "185.28.167.127":"1.000000"
CountryCode: "gb":"1.000000"
CityName: "Moffat":"0.535714"|"Reading":"0.357143"|"Swindon":"0.107143"
AverageLocation: "53.576283,-2.328108":"1.000000"
LocationBoundNorthWest: "55.378052,-3.435973":"1.000000"
LocationBoundSouthEast: "51.457577,-0.976019":"1.000000"

Ipv6 Address: fdaa:bbcc:ddee:0:995f:d63a:f2a1:f189
IpRangeStart: "2c0f:ff30:0100:0000:0000:0000:0000:0000":"1.000000"
IpRangeEnd: "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff":"1.000000"
CountryCode: "ZZ":"1.000000"
CityName: "Unknown":"1.000000"
AverageLocation: "0.000000,0.000000":"1.000000"
LocationBoundNorthWest: "0.000000,0.000000":"1.000000"
LocationBoundSouthEast: "0.000000,0.000000":"1.000000"
...
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

static const char* dataFileName = "51Degrees-LiteV41.ipi";

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

static void printPropertyValueFromResults(ResultsIpi *results) {
	if (results != NULL && results->count > 0) {
		printf("IpRangeStart: %s\n", getPropertyValueAsString(results, "IpRangeStart"));
		printf("IpRangeEnd: %s\n", getPropertyValueAsString(results, "IpRangeEnd"));
		printf("CountryCode: %s\n", getPropertyValueAsString(results, "CountryCode"));
		printf("AccuracyRadius: %s\n", getPropertyValueAsString(results, "AccuracyRadius"));
		printf("RegisteredCountry: %s\n", getPropertyValueAsString(results, "RegisteredCountry"));
		printf("RegisteredName: %s\n", getPropertyValueAsString(results, "RegisteredName"));
		printf("Longitude: %s\n", getPropertyValueAsString(results, "Longitude"));
		printf("Latitude: %s\n", getPropertyValueAsString(results, "Latitude"));
		printf("Areas: %s\n", getPropertyValueAsString(results, "Areas"));
		printf("Mcc: %s\n", getPropertyValueAsString(results, "Mcc"));
	}
	else {
		printf("No results.");
	}
}

void fiftyoneDegreesIpiGettingStarted(
	const char* dataFilePath,
	ConfigIpi* config) {
	ResourceManager manager;
	EXCEPTION_CREATE;

	// Set the properties to be returned for each ip
	PropertiesRequired properties = PropertiesDefault;
	properties.string = "IpRangeStart,IpRangeEnd,CountryCode,AccuracyRadius,RegisteredCountry,RegisteredName,Longitude,Latitude,Areas,Mcc";

	StatusCode status = IpiInitManagerFromFile(
		&manager,
		config,
		&properties,
		dataFilePath,
		exception);
	EXCEPTION_THROW;
	if (status != SUCCESS) {
		reportStatus(status, dataFilePath);
#ifndef TEST
		fgetc(stdin);
#endif
		return;
	}

	// Create a results instance to store and process ip
	ResultsIpi* results = ResultsIpiCreate(&manager);

	// an ipv4 address string
	const char* ipv4Address = "185.28.167.77";

	// an ipv6 address string
	const char* ipv6Address = "fdaa:bbcc:ddee:0:995f:d63a:f2a1:f189";

	printf("Starting Getting Started Example.\n");

	char nextIpV4[64] = { 0 };
	const int64_t dx    = 0x01020304LL;
	const int64_t maxIp = 0xFFFFFFFFLL;
	for (int64_t i = 0; i < maxIp; i += dx) {
		int64_t tk = i;
		StringBuilder s = { nextIpV4, sizeof(nextIpV4) };
		StringBuilderInit(&s);
		StringBuilderAddInteger(&s, tk & 0xFFLL);
		StringBuilderAddChar(&s, '.');
		tk /= 256;
		StringBuilderAddInteger(&s, tk & 0xFFLL);
		StringBuilderAddChar(&s, '.');
		tk /= 256;
		StringBuilderAddInteger(&s, tk & 0xFFLL);
		StringBuilderAddChar(&s, '.');
		tk /= 256;
		StringBuilderAddInteger(&s, tk & 0xFFLL);
		StringBuilderComplete(&s);

		// Carries out a match for the ipv4 address
		printf("\nIpv4 Address: %s\n", nextIpV4);
		ResultsIpiFromIpAddressString(
			results,
			nextIpV4,
			strlen(nextIpV4),
			exception);
		if (EXCEPTION_FAILED) {
			printf("%s\n", ExceptionGetMessage(exception));
		}
		printPropertyValueFromResults(results);
	}

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
	printPropertyValueFromResults(results);

	// Ensure the results are freed to avoid memory leaks.
	ResultsIpiFree(results);

	// Free the resources used by the manager
	ResourceManagerFree(&manager);
}

#ifndef TEST

int main(int argc, char* argv[]) {
	StatusCode status = SUCCESS;
	// ConfigIpi config = IpiDefaultConfig;
	ConfigIpi config = IpiInMemoryConfig;
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