#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/ProcIpi.c"

class ExampleTestProc : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        string line;
        int lines = 0;

        // Associate the file of Ip Addresses with stdin.
        freopen(this->ipAddressFilePath.c_str(), "r", stdin);

        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        // Start to process Ip Addresses.
        int count =
            fiftyoneDegreesProcIpiRun(dataFilePath.c_str(), "Country", &config);

        // Get the output from the processing.
        stringstream output = stringstream(testing::internal::GetCapturedStdout());

        // Loop through the output lines.
        while (getline(output, line)) {
            lines++;
        }

        // Check lines in and out are the same.
        EXPECT_EQ(count, lines) << "Same number of IP addresses in and out required";
    }
};

EXAMPLE_TESTS(ExampleTestProc)