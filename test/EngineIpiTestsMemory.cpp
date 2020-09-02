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
	void SetUp() {
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
	void TearDown() {
		EngineIpIntelligenceTests::TearDown();
	}
	void reload() { reloadMemory(); }
	void metaDataReload() {}
	void size() {
		FIFTYONE_DEGREES_EXCEPTION_CREATE;
		EXPECT_GT(fiftyoneDegreesIpiSizeManagerFromMemory(
			config->getConfig(),
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