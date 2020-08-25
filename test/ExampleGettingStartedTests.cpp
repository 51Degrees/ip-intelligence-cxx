#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/GettingStarted.c"

class ExampleTestGettingStarted : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
      // Capture stdout for the test.
      testing::internal::CaptureStdout();

      fiftyoneDegreesIpiGettingStarted(dataFilePath.c_str(), &config);

      // Don't print the stdout
      std::string output = testing::internal::GetCapturedStdout();
    }
};

EXAMPLE_TESTS(ExampleTestGettingStarted)