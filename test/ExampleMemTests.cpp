#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/MemIpi.c"

class ExampleTestMem : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        fiftyoneDegreesMemIpiRun(dataFilePath.c_str(), ipAddressFilePath.c_str(),
                                  config);

        // Don't print the stdout
        std::string output = testing::internal::GetCapturedStdout();
    }
};

EXAMPLE_TESTS(ExampleTestMem)