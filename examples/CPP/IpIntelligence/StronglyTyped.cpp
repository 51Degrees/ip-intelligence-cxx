#include <string>
#include <iostream>
#include "../../../src/EngineIpi.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;
using namespace FiftyoneDegrees::Examples::IpIntelligence;

/**
@example IpIntelligence/StronglyTyped.cpp
Strongly typed example of using 51Degrees IP intelligence.

The example shows how to extract the strongly typed value from the
returned results of the 51Degrees on-premise IP intelligence.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/blob/master/examples/StronglyTyped.cpp).

@include{doc} example-ipi-require-datafile.txt

@include{doc} example-ipi-how-to-run.txt

Expected output:

```
Result
```

In detail, the example shows how to:

1. Specify the name of the data file, properties the data set should be
initialised with, and the configuration.
```
using namespace FiftyoneDegrees;

string fileName = "51Degrees-V4.1.ipi";
string propertiesString = "RangeStart,RangeEnd,Country,City,AverageLocation";
Common::RequiredPropertiesConfig *properties =
	new Common::RequiredPropertiesConfig(&propertiesString);
IpIntelligence::ConfigIpi *config =
	new IpIntelligence::ConfigIpi();
```

2. Construct a new engine from the specified data file with the required
properties and the specified configuration.
```
using namespace FiftyoneDegrees;

IpIntelligence::EngineIpi *engine =
	new IpIntelligence::EngineIpi(
		dataFilePath,
		config,
		properties);
```

3. Create a evidence instance and add a single IP address byte array to be
processed.
```
using namespace FiftyoneDegrees;

IpIntelligence::EvidenceIpi *evidence =
	new IpIntelligence::EvidenceIpi();
evidence->operator[]("ipv4.ip") = ipAddress;
```

4. Process the evidence using the engine to retrieve the values associated
with the IP address for the selected properties.
```
using namespace FiftyoneDegrees;

IpIntelligence::ResultsIpi *results = engine->process(evidence);
```

5. Extract the value of a property as a weighted pair of floats from the results.
```
Value<vector<WeightedValue<pair<float, float>>>> value = 
	results->getValuesAsWeightedCoordinateList("AverageLocation");
for (WeightedValue<pair<float, float>> w : value.getValue()) {
	cout << "   AverageLocation: " <<
		w.getValue().first <<
		"," <<
		w.getValue().second << "\n";
}
```

6. Release the memory used by the results and the evidence.
```
delete results;
delete evidence;
```

7. Finally release the memory used by the engine.
```
delete engine;
```

 */

namespace FiftyoneDegrees {
	namespace Examples {
		namespace IpIntelligence {
			/**
			 * Hash Strongly Typed Example
			 */
			class StronglyTyped : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				StronglyTyped(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					try {
						ResultsIpi *results;

						// Create an evidence instance to store and process Ip Addresses.
						EvidenceIpi *evidence = new EvidenceIpi();

						IpAddress ipv4(ipv4Address);
						IpAddress ipv6(ipv6Address);

						cout << "Starting Strongly Typed Example.\n";

						// Carries out a match for a ipv4 address.
						cout << "\nIpv4 Address: " << ipv4Address << "\n";
						evidence->operator[]("ipv4.ip")
								= ipv4;
						results = engine->process(evidence);
						Common::Value<pair<float, float>> ipv4Value = results->getValueAsCoordinate("AverageLocation");
						cout << "   AverageLocation: " <<
							ipv4Value.getValue().first <<
							"," <<
							ipv4Value.getValue().second << "\n";
						delete results;

						// Carries out a match for a ipv6 address.
						cout << "\nIpv6 Address: " << ipv6Address << "\n";
						evidence->operator[]("ipv6.ip")
								= ipv6;
						results = engine->process(evidence);
						Common::Value<pair<float, float>> ipv6Value = results->getValuesAsCoordinate("AverageLocation");
						cout << "   AverageLocation: " <<
							ipv4Value.getValue().first <<
							"," <<
							ipv4Value.getValue().second << "\n";
						delete results;

						// Free the evidence.
						delete evidence;
					}
					catch (bad_alloc& e) {
						cout << "Failed to create IpAddress class\n";
						cout << e.what() << "\n";
					}
				}
			};
		}
	}
}

int main(int argc, char* argv[]) {
	fiftyoneDegreesStatusCode status = FIFTYONE_DEGREES_STATUS_SUCCESS;
	char dataFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	if (argc > 1) {
		strcpy(dataFilePath, argv[1]);
	}
	else {
		status = fiftyoneDegreesFileGetPath(
			dataDir,
			dataFileName,
			dataFilePath,
			sizeof(dataFilePath));
	}
	if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
		ExampleBase::reportStatus(status, dataFileName);
		fgetc(stdin);
		return 1;
	}


#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif

	StronglyTyped *stronglyTyped = new StronglyTyped(dataFilePath);
	stronglyTyped->run();
	delete stronglyTyped;

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