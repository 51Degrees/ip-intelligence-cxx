#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/PerfIpi.c"

class ExampleTestPerf : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        fiftyoneDegreesPerfIpiRun(
            dataFilePath.c_str(),
            ipAddressFilePath.c_str(),
            config);

        // Don't print the stdout
        std::string output = testing::internal::GetCapturedStdout();
    }
};

EXAMPLE_TESTS(ExampleTestPerf)