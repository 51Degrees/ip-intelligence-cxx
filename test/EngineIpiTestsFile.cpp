#include "EngineIpIntelligenceTests.hpp"

class EngineIpiTestsFile : public EngineIpIntelligenceTests {
public:
	EngineIpiTestsFile(
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
	void SetUp() {
		EngineIpIntelligenceTests::SetUp();
		engine = new EngineIpi(fullName, config, requiredProperties);
	};
	void TearDown() {
		EngineIpIntelligenceTests::TearDown();
	};
	void reload() { 
		reloadFile();
	}
	void metaDataReload() { 
		verifyMetaDataReload(engine);
	};
	void size() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesIpiSizeManagerFromFile(
			config->getConfig(), 
			requiredProperties->getConfig(), 
			fullName,
			exception),
			(size_t)0) << "Size method should always return more than 0 "
			"bytes";
		if (FIFTYONE_DEGREES_EXCEPTION_FAILED) {
			FAIL() << "Getting the manager size failed with: " <<
				fiftyoneDegreesExceptionGetMessage(exception);
		}
	}
};

ENGINE_FILE_TESTS(Ipi)