#include "ExampleIpIntelligenceTests.hpp"

ExampleIpIntelligenceTest::ExampleIpIntelligenceTest(
    const char **dataFileNames,
    int dataFileNamesLength,
    const char *ipAddressFileName) : ExampleTests() {
    dataFilePath = "";
    for (int i = 0;
        i < dataFileNamesLength && strcmp("", dataFilePath.c_str()) == 0;
        i++) {
        dataFilePath = GetFilePath(_dataFolderName, dataFileNames[i]);
    }

  ipAddressFilePath = GetFilePath(_dataFolderName, ipAddressFileName);
  requiredProperties = "RangeStart,RangeEnd,Country,City,ContactEmail,AverageLocation";
};
ExampleIpIntelligenceTest::ExampleIpIntelligenceTest()
    : ExampleIpIntelligenceTest(
        _IpiFileNames,
        _IpiFileNamesLength,
        _ipAddressesFileName) {}