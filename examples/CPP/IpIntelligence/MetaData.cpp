#include <string>
#include <iostream>
#include "../../../src/EngineIpi.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;
using namespace FiftyoneDegrees::Examples::IpIntelligence;

/**
@example IpIntelligence/MetaData.cpp
Meta data example of using 51Degrees IP intelligence.

The example shows how to retrieve meta data from 51Degrees on-premise
IP intelligence. This feature can be used to get information such as
the category that a property belons to or the possible values a property
can have.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/blob/master/examples/MetaData.cpp).

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

3. Fetch a collection containing the meta data of all the properties in the
engine's data set.
```
using namespace FiftyoneDegrees;

Common::Collection<string, Common::PropertyMetaData> *properties =
	engine->getMetaData()->getProperties();
```

4. Iterate over all the properties and print the name and description. Note
that getting a property meta data instance will return a new instance which
holds no reference to the engine nor any of its internals. Therefore it is the
responsibility of the caller to delete it once it is no longer needed.
```
using namespace FiftyoneDegrees;

for (uint32_t i = 0; i < properties->getSize(); i++){
	Common::PropertyMetaData *property = properties->getByIndex(i);
	cout << property->getName() << " - " << property->getDescription() << "\n";
	delete property;
}
```

5. Release the memory used by the properties collection, and its reference to
the engine's underlying data set.
```
delete properties;
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
			 * Hash Meta Data Example.
			 */
			class MetaDataExample : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string)
				 */
				MetaDataExample(string dataFilePath)
					: ExampleBase(dataFilePath)
				{};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					PropertyMetaData *property;
					Collection<string, PropertyMetaData> *properties;
					cout << "Starting MetaData Example.\n";
					properties = engine->getMetaData()->getProperties();
					for (uint32_t i = 0; i < properties->getSize(); i++){
						property = properties->getByIndex(i);
						cout << property->getName() << " - " << property->getDescription() << "\n";
                        delete property;
					}
					delete properties;
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

	MetaDataExample *metaData = new MetaDataExample(dataFilePath);
	metaData->run();
	delete metaData;

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