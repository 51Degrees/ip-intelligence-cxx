/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
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

#include "EngineIpIntelligenceTests.hpp"

class EngineIpiTestsMemory : public EngineIpIntelligenceTests {
public:
	EngineIpiTestsMemory(
		ConfigIpi *config,
		RequiredPropertiesConfig *properties,
		const char *dataDirectory,
		const char **ipiFileNames,
		int ipiFileNamesLength,
		const char *ipAddressesFileName)
		: EngineIpIntelligenceTests(
			config,
			properties,
			dataDirectory,
			ipiFileNames,
			ipiFileNamesLength,
			ipAddressesFileName) {
	};
	void SetUp() override {
		EngineIpIntelligenceTests::SetUp();
		if (fileReadToByteArray()) {
			engine = new EngineIpi(
				data.current,
				data.length, 
				config, 
				requiredProperties);
		}
		ASSERT_NE(engine, nullptr);
	};
	void TearDown() override {
		EngineIpIntelligenceTests::TearDown();
	}
	void reload() {
		// Skip this test on Ubuntu ARM64 due to performance limitations.
		// The reloadMemory() test loads the 2.3GB enterprise data file into
		// memory twice (initial load + reload), which exceeds the 25-minute
		// per-test timeout on ARM64 GitHub runners due to slower I/O performance.
		// The test passes successfully on x86_64 platforms.
#if defined(__linux__) && (defined(__aarch64__) || defined(__arm64__))
		GTEST_SKIP() << "Skipping memory reload test on Ubuntu ARM64 due to "
			"timeout issues with large data files (2.3GB x2 loads).";
#else
		reloadMemory();
#endif
	}
	void metaDataReload() {}
	void size() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesIpiSizeManagerFromMemory(
			&config->getConfig(),
			requiredProperties->getConfig(),
			data.startByte, 
			data.length,
			exception), (size_t)0) << "Size method should always return more than 0 "
			"bytes";
		if (FIFTYONE_DEGREES_EXCEPTION_FAILED) {
			FAIL() << "Getting the manager size failed with: " <<
				fiftyoneDegreesExceptionGetMessage(exception);
		}
	}
};

ENGINE_MEMORY_TESTS(Ipi)