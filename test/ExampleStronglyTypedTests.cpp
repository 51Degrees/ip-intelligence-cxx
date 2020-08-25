#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/StronglyTyped.c"

class ExampleStronglyTypedTests : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        fiftyoneDegreesIpiStronglyTyped(dataFilePath.c_str(), &config);

        // Don't print the stdout
        std::string output = testing::internal::GetCapturedStdout();
    }
};

EXAMPLE_TESTS(ExampleStronglyTypedTests)