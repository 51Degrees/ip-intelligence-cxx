/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2026 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#include <cstdio>
#include "ExampleIpIntelligenceTests.hpp"
#include "../examples/C/IpIntelligence/CountryOverlap.c"

/** CSV written by the test and removed afterwards. */
static const char testOutputPath[] = "country-overlap-test.csv";

/**
 * The country overlap example requires an enterprise data file with the
 * weighted country code properties, so the test skips rather than fails
 * when only the Lite data file is available. Two /8 chunks are swept with
 * two threads so the test completes quickly whilst still exercising real
 * data, the spot checks, and the CSV output.
 *
 * The tests are declared explicitly rather than via EXAMPLE_TESTS
 * because the low memory configuration is skipped. The sweep evaluates
 * millions of addresses and is only practical when the graph collection
 * is cached or loaded into memory. With the low memory configuration
 * every graph node read goes to the data file and a single test takes
 * upwards of an hour, so that configuration is not suitable for this
 * bulk analysis example.
 */
class ExampleTestCountryOverlap : public ExampleIpIntelligenceTest {
public:
    void run(fiftyoneDegreesConfigIpi config) {
        // Capture stdout for the test.
        testing::internal::CaptureStdout();

        const int result = fiftyoneDegreesIpiCountryOverlap(
            dataFilePath.c_str(),
            &config,
            testOutputPath,
            2,
            2,
            DEFAULT_MIN_SECONDARY_PERCENT);

        std::string output = testing::internal::GetCapturedStdout();

        if (result == COUNTRY_OVERLAP_PROPERTIES_MISSING) {
            GTEST_SKIP() <<
                "The data file does not include the weighted country "
                "code properties. An enterprise data file is required "
                "for the country overlap example.";
        }
        ASSERT_EQ(COUNTRY_OVERLAP_OK, result) <<
            "The country overlap example did not complete. Output: " <<
            output;

        // The spot checks compare the sweep against the normal lookup
        // process and every one must match.
        EXPECT_NE(
            output.find("spot checks match the normal lookup process"),
            std::string::npos) << output;
        EXPECT_EQ(output.find("MISMATCH"), std::string::npos) << output;

        // The CSV file must exist and have the expected header row.
        expectCsvWithHeader(
            testOutputPath,
            "PrimaryCountryCode,PrimaryCountry");

        std::remove(testOutputPath);
    }

private:
    static void expectCsvWithHeader(
        const char* path,
        const char* expectedStart) {
        FILE* file = fopen(path, "r");
        ASSERT_NE(nullptr, file) <<
            "Expected the example to write '" << path << "'.";
        char line[256] = "";
        const char* read = fgets(line, sizeof(line), file);
        fclose(file);
        ASSERT_NE(nullptr, read) <<
            "Expected '" << path << "' to contain a header row.";
        EXPECT_EQ(0, strncmp(line, expectedStart, strlen(expectedStart)))
            << "Unexpected header in '" << path << "': " << line;
    }
};

TEST_F(ExampleTestCountryOverlap, Default) {
    if (shouldSkipTempFileTestOnCI(fiftyoneDegreesIpiDefaultConfig)) {
        GTEST_SKIP() << "Skipping temp file test on CI";
    }
    if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
        run(fiftyoneDegreesIpiDefaultConfig);
    }
}
TEST_F(ExampleTestCountryOverlap, BalancedTemp) {
    if (shouldSkipTempFileTestOnCI(fiftyoneDegreesIpiBalancedTempConfig)) {
        GTEST_SKIP() << "Skipping temp file test on CI";
    }
    if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
        run(fiftyoneDegreesIpiBalancedTempConfig);
    }
}
TEST_F(ExampleTestCountryOverlap, Balanced) {
    if (shouldSkipTempFileTestOnCI(fiftyoneDegreesIpiBalancedConfig)) {
        GTEST_SKIP() << "Skipping temp file test on CI";
    }
    if (fiftyoneDegreesCollectionGetIsMemoryOnly() == false) {
        run(fiftyoneDegreesIpiBalancedConfig);
    }
}
TEST_F(ExampleTestCountryOverlap, LowMemory) {
    GTEST_SKIP() <<
        "The country overlap sweep evaluates millions of addresses "
        "and needs the graph collection cached or loaded into memory. "
        "The low memory configuration reads every graph node from the "
        "data file which takes upwards of an hour, so it is not "
        "suitable for this bulk analysis example.";
}
TEST_F(ExampleTestCountryOverlap, HighPerformance) {
    if (shouldSkipTempFileTestOnCI(
        fiftyoneDegreesIpiHighPerformanceConfig)) {
        GTEST_SKIP() << "Skipping temp file test on CI";
    }
    run(fiftyoneDegreesIpiHighPerformanceConfig);
}
TEST_F(ExampleTestCountryOverlap, InMemory) {
    if (shouldSkipTempFileTestOnCI(fiftyoneDegreesIpiInMemoryConfig)) {
        GTEST_SKIP() << "Skipping temp file test on CI";
    }
    run(fiftyoneDegreesIpiInMemoryConfig);
}
