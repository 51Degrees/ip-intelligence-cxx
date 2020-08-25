#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/OfflineProcessing.c"

class ExampleTestOfflineProcessing : public ExampleIpIntelligenceTest {
private:
    string getOutputFilePath() {
        stringstream output;
        uint32_t i = 0;
        while (ipAddressFilePath[i] != '.' && ipAddressFilePath[i] != '\0') {
            output << ipAddressFilePath[i++];
        }
        output << ".processed.csv";
        return output.str();
    }

public:
    void run(fiftyoneDegreesConfigIpi config) {
        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        fiftyoneDegreesOfflineProcessingRun(
            dataFilePath.c_str(), ipAddressFilePath.c_str(),
            getOutputFilePath().c_str(), "RangeStart,RangeEnd,AverageLocation", config);
        fiftyoneDegreesFileDelete(getOutputFilePath().c_str());

        // Don't print the stdout
        std::string output = testing::internal::GetCapturedStdout();
    }
};

EXAMPLE_TESTS(ExampleTestOfflineProcessing)