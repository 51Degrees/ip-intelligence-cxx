#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/ReloadFromFile.c"

class ExampleTestReloadFromFile : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        fiftyoneDegreesIpiReloadFromFileRun(
            dataFilePath.c_str(),
            ipAddressFilePath.c_str(),
            requiredProperties, config);

        // Check the results.
        string output = testing::internal::GetCapturedStdout();
        EXPECT_NE(output.find("Failed to reload '0' times"), std::string::npos)
            << "The data set should never fail to reload";
        EXPECT_EQ(output.find("Reloaded '0' times"), std::string::npos)
            << "The data set should reload at least once";
    }
};

EXAMPLE_TESTS(ExampleTestReloadFromFile)