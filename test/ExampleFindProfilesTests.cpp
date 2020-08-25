#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/FindProfiles.c"

class ExampleTestFindProfiles : public ExampleIpIntelligenceTest {
public:
	void run(fiftyoneDegreesConfigIpi config) {
		// Capture stdout for the test.
		testing::internal::CaptureStdout();

		fiftyoneDegreesIpiFindProfiles(
			dataFilePath.c_str(),
			config);

		// Don't print the stdout
		std::string output = testing::internal::GetCapturedStdout();
	}
};

EXAMPLE_TESTS(ExampleTestFindProfiles)