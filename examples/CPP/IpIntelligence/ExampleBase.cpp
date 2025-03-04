/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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

#include "ExampleBase.hpp"
#include "../../Base/ExampleBase.h"

using std::cout;
using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::Examples::IpIntelligence;

const char *ExampleBase::ipv4Address = "185.28.167.77";

const char *ExampleBase::ipv6Address = "2001:4860:4860::8888";

ExampleBase::ExampleBase(::byte *data, long length, FiftyoneDegrees::IpIntelligence::ConfigIpi *config) {
	this->config = config;
	
	// Set the properties to be returned for each Ip Address.
	string propertiesString = "IpRangeStart,IpRangeEnd,CountryCode,CityName,AccuracyRadius";
	properties = new RequiredPropertiesConfig(propertiesString);
	
	// Initialise the engine for IP intelligence.
	engine = new EngineIpi(data, length, config, properties);
}

ExampleBase::ExampleBase(string dataFilePath, FiftyoneDegrees::IpIntelligence::ConfigIpi *config) {
	this->config = config;
	
	// Set the properties to be returned for each Ip Address.
	string propertiesString = "IpRangeStart,IpRangeEnd,CountryCode,CityName,AccuracyRadius,RegisteredCountry";
	properties = new RequiredPropertiesConfig(propertiesString);

	// Initialise the engine for ip intelligence.
	engine = new EngineIpi(dataFilePath, config, properties);
}

ExampleBase::ExampleBase(string dataFilePath)
    : ExampleBase(dataFilePath, new FiftyoneDegrees::IpIntelligence::ConfigIpi()) {}

ExampleBase::~ExampleBase() {
	delete engine;
	delete config;
	delete properties;
}

void ExampleBase::reportStatus(fiftyoneDegreesStatusCode status,
                               const char *fileName) {
	const char *message = fiftyoneDegreesStatusGetMessage(status, fileName);
	cout << message;
	fiftyoneDegreesFree((void *)message);
}

unsigned long ExampleBase::generateHash(unsigned char *value) {
	unsigned long hashCode = 5381;
	int i = *value++;
	while (i != 0) {
		hashCode = ((hashCode << 5) + hashCode) + i;
		i = *value++;
	}
	return hashCode;
}

unsigned long ExampleBase::getHashCode(FiftyoneDegrees::IpIntelligence::ResultsIpi *results) {
	unsigned long hashCode = 0;
	uint32_t requiredPropertyIndex;
	string valueName;
	
	for (requiredPropertyIndex = 0;
		requiredPropertyIndex < (uint32_t)results->getAvailableProperties();
		requiredPropertyIndex++) {
		valueName = *results->getValueAsString(requiredPropertyIndex);
		hashCode ^= generateHash((unsigned char *)(valueName.c_str()));
	}
	return hashCode;
}

void ExampleBase::processIpAddress(const char *ipAddress, void *state) {
	ThreadState *thread = (ThreadState *)state;
	
	FiftyoneDegrees::IpIntelligence::ResultsIpi *results = thread->engine->process(ipAddress);
	
	thread->hashCode ^= getHashCode(results);
	
	delete results;
}

void ExampleBase::SharedState::processIpAddressesSingle() {
	char ipAddress[500] = "";
	ThreadState thread(engine);
	fiftyoneDegreesEvidenceFileIterate(ipAddressFilePath.c_str(), ipAddress,
								sizeof(ipAddress), &thread, processIpAddress);
	printf("Finished with hash code '%i'\r\n", thread.hashCode);
}

void ExampleBase::SharedState::processIpAddressesMulti(void *state) {
	SharedState *shared = (SharedState *)state;
	shared->processIpAddressesSingle();
	FIFTYONE_DEGREES_INTERLOCK_INC(&shared->threadsFinished);
}

void ExampleBase::SharedState::startThreads() {
	int i;
	for (i = 0; i < THREAD_COUNT; i++) {
		threads[i] = thread(processIpAddressesMulti, this);
	}
}

void ExampleBase::SharedState::joinThreads() {
	int i;
	for (i = 0; i < THREAD_COUNT; i++) {
		threads[i].join();
	}
}

ExampleBase::SharedState::SharedState(FiftyoneDegrees::IpIntelligence::EngineIpi *engine,
                                      string ipAddressFilePath) {
	this->engine = engine;
	this->threadsFinished = 0;
	this->ipAddressFilePath = ipAddressFilePath;
}

ExampleBase::ThreadState::ThreadState(
    FiftyoneDegrees::IpIntelligence::EngineIpi *engine) {
	this->engine = engine;
	this->hashCode = 0;
}
