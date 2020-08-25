#include <string>
#include <iostream>
#include <thread>
#include "../../../src/EngineIpi.hpp"
#include "ExampleBase.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;
using namespace FiftyoneDegrees::Examples::IpIntelligence;

/**
@example IpIntelligence/ReloadFromFile.cpp
Reload from file example of using 51Degrees IP intelligence.

This example illustrates how to use a single reference to the engine to use
51Degrees on-premise IP intelligence and invoke the reload functionality
instead of maintaining a reference to the dataset directly.

This example is available in full on [GitHub](https://github.com/51Degrees/ip-intelligence-cxx/blob/master/examples/ReloadFromFile.cpp).

@include{doc} example-ipi-require-datafile.txt

@include{doc} example-ipi-how-to-run.txt

Expected output:

```
Result
```

In detail, this example shows how to:

1. Only maintain a reference to a single EngineIpi instance and use the
reference to process data.

2. Use the EngineIpi->refreshData() function to reload the dataset from the
same location and with the same set of properties.

3. Retrieve a results instance from the engine and release it when done with
detecting current IP address.
	
4. Use the reload functionality in a single threaded environment.

5. Use the reload functionality in a multi threaded environment.

The EngineIpi->refreshData() function requires an existing engine with the
initialized dataset. Function reloads the dataset from the same location and
with the same parameters as the original dataset. 
	
Please keep in mind that even if the current dataset was constructed with
all available properties this does not guarantee that the new dataset will
be initialized with the same set of properties. If the new data file
contains properties that were not part of the original data file, the new
extra property(ies) will not be initialized. If the new data file does not
contain one or more property that were previously available, then these
property(ies) will not be initialized.

Each successful data file reload should be accompanied by the integrity
check to verify that the properties you want have indeed been loaded. This
can be achieved by simply comparing the number of properties before and
after the reload as the number can not go up but it can go down.

The reload functionality works both with the single threaded as well as the
multi threaded modes. To try the reload functionality in single threaded
mode build with FIFTYONE_DEGREES_NO_THREADING defined. Or build without
FIFTYONE_DEGREES_NO_THREADING for multi threaded example.

In a single threaded environment the reload function is executed as part of
the normal flow of the program execution and will prevent any other actions
until the reload is complete.

*/

namespace FiftyoneDegrees {
	namespace Examples {
		namespace IpIntelligence {
			/**
			 * IP Intelligence Reload From File Example.
			 */
			class ReloadFromFile : public ExampleBase {
			public:
				/**
				 * @copydoc ExampleBase::ExampleBase(string, IpIntelligence::ConfigIpi*)
				 * @param ipAddressFilePath path to the CSV file containing the
				 * IP Addresses to process
				 */
				ReloadFromFile(
					string dataFilePath,
					string ipAddressFilePath,
					ConfigIpi *config)
					: ExampleBase(dataFilePath, config) {
					this->ipAddressFilePath = ipAddressFilePath;
				};

				/**
				 * @copydoc ExampleBase::run
				 */
				void run() {
					int numberOfReloads = 0;
					int numberOfReloadFails = 0;
					ExampleBase::SharedState state(engine, ipAddressFilePath);
					
					if (fiftyoneDegreesThreadingGetIsThreadSafe()) {
						printf("** Multi Threaded Reload Example **\r\n");
						state.startThreads();
						while (state.threadsFinished < THREAD_COUNT) {
							try {
								engine->refreshData();
								numberOfReloads++;
							}
							catch (StatusCodeException e) {
								numberOfReloadFails++;
							}
#ifdef _MSC_VER
							Sleep(2000); // milliseconds
#else
							usleep(20000000); // microseconds
#endif
						}
						state.joinThreads();
					}
					else {
						printf("** Single Threaded Reload Example **\r\n");
						state.processIpAddressesSingle();
						try {
							engine->refreshData();
							numberOfReloads++;
						}
						catch (StatusCodeException e) {
							numberOfReloadFails++;
						}
						state.processIpAddressesSingle();
					}

					// Report the number of reloads.
					printf("Reloaded '%i' times.\r\n", numberOfReloads);
					printf("Failed to reload '%i' times.\r\n", numberOfReloadFails);
					printf("Program execution complete. Press Return to exit.");
				}

			private:
				string ipAddressFilePath;
			};
		}
	}
}

/**
 * Main entry point.
 */
int main(int argc, char* argv[]) {
	fiftyoneDegreesStatusCode status = FIFTYONE_DEGREES_STATUS_SUCCESS;
	char dataFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
	char ipAddressFilePath[FIFTYONE_DEGREES_FILE_MAX_PATH];
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

	if (argc > 2) {
		strcpy(dataFilePath, argv[2]);
	}
	else {
		status = fiftyoneDegreesFileGetPath(
			dataDir,
			ipAddressFileName,
			ipAddressFilePath,
			sizeof(ipAddressFilePath));
	}
	if (status != FIFTYONE_DEGREES_STATUS_SUCCESS) {
		ExampleBase::reportStatus(status, ipAddressFileName);
		fgetc(stdin);
		return 1;
	}

#ifdef _DEBUG
#ifndef _MSC_VER
	dmalloc_debug_setup("log-stats,log-non-free,check-fence,log=dmalloc.log");
#endif
#endif
	ConfigIpi *config = new ConfigIpi();
	config->setConcurrency(THREAD_COUNT);
	ReloadFromFile *reloadFromFile = new ReloadFromFile(
		dataFilePath,
		ipAddressFilePath,
		config);
	reloadFromFile->run();
	delete reloadFromFile;

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