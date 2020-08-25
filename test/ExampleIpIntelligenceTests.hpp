#ifndef FIFTYONE_DEGREES_EXAMPLE_IP_INTELLIGENCE_TESTS_HPP
#define FIFTYONE_DEGREES_EXAMPLE_IP_INTELLIGENCE_TESTS_HPP

#include "../src/common-cxx/file.h"
#include "../src/common-cxx/tests/ExampleTests.hpp"
#include "Constants.hpp"

class ExampleIpIntelligenceTest : public ExampleTests {
public:
    ExampleIpIntelligenceTest();

    ExampleIpIntelligenceTest(
        const char **dataFileNames,
        int dataFileNamesLength,
        const char *ipAddressFileName);

protected:
    string dataFilePath;
    string ipAddressFilePath;
    const char *requiredProperties;
};

#define EXAMPLE_TESTS(c)                                        \
    TEST_F(c, Default) {                                             \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiDefaultConfig);                      \
        }                                                              \
    }                                                                \
    TEST_F(c, BalancedTemp) {                                        \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiBalancedTempConfig);                 \
        }                                                              \
    }                                                                \
    TEST_F(c, Balanced) {                                            \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiBalancedConfig);                     \
        }                                                              \
    }                                                                \
    TEST_F(c, LowMemory) {                                           \
        if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {     \
            run(fiftyoneDegreesIpiLowMemoryConfig);                    \
        }                                                              \
    }                                                                \
    TEST_F(c, HighPerformance) {                                     \
        run(fiftyoneDegreesIpiHighPerformanceConfig);                \
    }                                                                \
    TEST_F(c, InMemory) { run(fiftyoneDegreesIpiInMemoryConfig); } \
    TEST_F(c, SingleLoaded) { run(fiftyoneDegreesIpiSingleLoadedConfig); }

#endif