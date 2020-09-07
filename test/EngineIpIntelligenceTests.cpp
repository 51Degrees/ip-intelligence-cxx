/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2020 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
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

#ifdef _MSC_FULL_VER
#include <string.h>
#else
#include <strings.h>
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#endif
#include <regex>

#include "Constants.hpp"
#include "EngineIpIntelligenceTests.hpp"
#include "../src/EngineIpi.hpp"
#include "../src/common-cxx/file.h"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

EngineIpIntelligenceTests::EngineIpIntelligenceTests(
	ConfigIpi *config,
	RequiredPropertiesConfig *requiredProperties,
	const char *directory,
	const char **fileNames,
	int fileNamesLength,
	const char *ipAddressesFileName)
	: EngineTests(requiredProperties, directory, fileNames, fileNamesLength) {
	char ipAddressesFullName[FIFTYONE_DEGREES_FILE_MAX_PATH];
	const char ipAddress[40] = "";
	fiftyoneDegreesFileGetPath(
		directory,
		ipAddressesFileName,
		ipAddressesFullName,
		sizeof(ipAddressesFullName));
	fiftyoneDegreesTextFileIterate(
		ipAddressesFullName, 
		ipAddress, 
		sizeof(ipAddress), 
		this, 
		EngineIpIntelligenceTests::ipAddressRead);
	this->config = config;
	engine = nullptr;
}

EngineIpIntelligenceTests::~EngineIpIntelligenceTests() {
	while (ipAddresses.empty() == false) {
		ipAddresses.pop_back();
	}
}

EngineBase* EngineIpIntelligenceTests::getEngine() { return (EngineBase*)engine; }

void EngineIpIntelligenceTests::SetUp() {
	EngineTests::SetUp();
}

void EngineIpIntelligenceTests::TearDown() {
	if (engine != nullptr) {
		delete engine;
	}
	if (data.current != nullptr) {
		fiftyoneDegreesFree(data.current);
		data.current = nullptr;
		data.length = 0;
	}
	EngineTests::TearDown();
}

void EngineIpIntelligenceTests::metaData() {
	EngineTests::verifyMetaData(getEngine());
}
void EngineIpIntelligenceTests::availableProperties() {
	// TODO: This mainly check the evidence property in Device Detection
	// therefore probably not required for this test
}

string EngineIpIntelligenceTests::getExpectedFileType() {
	int i;
	for (i = 0; i < _IpiFileNamesLength; i++) {
		if (strcmp(fileName, _IpiFileNames[i]) == 0) {
			return _fileTypes[i];
		}
	}
	return nullptr;
}

void EngineIpIntelligenceTests::ipAddressRead(
	const char *ipAddress,
	void *state) {
	((EngineIpIntelligenceTests*)state)->ipAddresses.push_back(
		string(ipAddress));
}

void EngineIpIntelligenceTests::verifyPropertyValue(
	ResultsBase *results, 
	string property, 
	string value) {
	vector<string> props = results->getProperties();
	if (find(props.begin(), props.end(), property) != props.end()) {
		EXPECT_EQ(*results->getValueAsString(property), value);
	}
}

void EngineIpIntelligenceTests::validateIndex(
	ResultsBase *results,
	int index) {
	ResultsIpi *resultsIpi = (ResultsIpi *)results;
	Value<vector<string>> values = results->getValues(index);
	if (values.hasValue()) {
		EXPECT_NO_THROW(*resultsIpi->getValueAsIpAddress(index)) << "IP address value "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValueAsCoordinate(index)) << "Coordinate value for "
			"property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedBoolList(index)) << "Boolean list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedIntegerList(index)) << "Integer list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedDoubleList(index)) << "Double list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedStringList(index)) << "String list "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_NO_THROW(*resultsIpi->getValueAsString(index)) << "String value "
			"for property '" << resultsIpi->getPropertyName(index) << "' at "
			"index '" << index << "' can't throw exception";
		EXPECT_THROW(*resultsIpi->getValueAsBool(index), TooManyValuesException);
		EXPECT_THROW(*resultsIpi->getValueAsInteger(index), TooManyValuesException);
		EXPECT_THROW(*resultsIpi->getValueAsDouble(index), TooManyValuesException);
	}
}

void EngineIpIntelligenceTests::validateName(
	ResultsBase *results,
	string *name) {

	if (isNameAvailable(results, name) == false) {
		// If the name is not available in the properties then when it's 
		// requested an exception should be thrown. 
		EXPECT_THROW(*results->getValues(name),
			InvalidPropertyException) << "Property '" << *name << "' is "
			"missing and should throw exception";
	}
	else {
		ResultsIpi *resultsIpi = (ResultsIpi *)results;
		// If the name is available so the values should be retrieved with
		// out an exception.
		Value<vector<string>> values = results->getValues(name);

		if (values.hasValue() && values.getValue().size() > 0) {
			EXPECT_NO_THROW(*resultsIpi->getValueAsIpAddress(name)) << "IP address value "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValueAsCoordinate(name)) << "Coordinate value "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedBoolList(name)) << "Boolean list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedIntegerList(name)) << "Integer list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedDoubleList(name)) << "Double list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValuesAsWeightedStringList(name)) << "String list "
				"for property '" << *name << "' can't throw exception";
			EXPECT_NO_THROW(*resultsIpi->getValueAsString(name)) << "String value "
				"for property '" << *name << "' can't throw exception";
			EXPECT_THROW(*resultsIpi->ResultsBase::getValueAsBool(name), TooManyValuesException);
			EXPECT_THROW(*resultsIpi->ResultsBase::getValueAsInteger(name), TooManyValuesException);
			EXPECT_THROW(*resultsIpi->ResultsBase::getValueAsDouble(name), TooManyValuesException);
		}
		else {
			// There are no values returned. This is only allowed when:
			// 1. If there was no evidence provided. This means the results can
			//    not be determined as there was nothing to process.
			EXPECT_TRUE(values.getNoValueReason() ==
				FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_NO_RESULTS) <<
				L"Must get values for available property '" << *name << "'";
		}
	}
}

void EngineIpIntelligenceTests::verifyWithEvidence(EvidenceIpi *evidence) {
	ResultsIpi *results = ((EngineIpi*)getEngine())->process(evidence);
	validate(results);
	delete results;
}

void EngineIpIntelligenceTests::verifyWithEmptyEvidence() {
	EvidenceIpi evidence;
	verifyWithEvidence(&evidence);
}

void EngineIpIntelligenceTests::verifyWithEvidence() {
	EvidenceIpi evidence;
	IpAddress ipv4(ipv4Address);
	evidence["ipv4.ip"] = ipv4;
	verifyWithEvidence(&evidence);
}

void EngineIpIntelligenceTests::verifyWithIpAddressString(const char *ipAddress) {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results = engine->process(
		ipAddress);
	validate(results);
	delete results;
}

void EngineIpIntelligenceTests::verifyWithIpv4Address() {
	verifyWithIpAddressString(ipv4Address);
}

void EngineIpIntelligenceTests::verifyWithIpv6Address() {
	verifyWithIpAddressString(ipv6Address);
}

void EngineIpIntelligenceTests::verifyWithBadIpv4Address() {
	verifyWithIpAddressString(badIpv4Address);
}

void EngineIpIntelligenceTests::verifyWithBadIpv6Address() {
	verifyWithIpAddressString(badIpv6Address);
}

void EngineIpIntelligenceTests::verifyWithBoundIpv4Address() {
	verifyWithIpAddressString(lowerBoundIpv4Address);
	verifyWithIpAddressString(upperBoundIpv4Address);
}

void EngineIpIntelligenceTests::verifyWithBoundIpv6Address() {
	verifyWithIpAddressString(lowerBoundIpv6Address);
	verifyWithIpAddressString(upperBoundIpv6Address);
}

void EngineIpIntelligenceTests::verifyWithOutOfRangeIpv4Address() {
	verifyWithIpAddressString(outOfRangeIpv4Address);
}

void EngineIpIntelligenceTests::verifyWithOutOfRangeIpv6Address() {
	verifyWithIpAddressString(outOfRangeIpv6Address);
}


void EngineIpIntelligenceTests::verifyWithInvalidInput() {
	int character;
	char userAgent[2];
	userAgent[1] = '\0';
	EngineIpi *engine = (EngineIpi*)getEngine();
	for (character = CHAR_MIN; character <= CHAR_MAX; character++) {
		userAgent[0] = (char)character;
		ResultsIpi *results = engine->process(
		userAgent);
		validate(results);
		delete results;
	}
}

void EngineIpIntelligenceTests::verifyWithNullEvidence() {
	EngineTests::verifyWithEvidence(nullptr);
}
 
void EngineIpIntelligenceTests::verifyWithNullIpAddress() {
	EngineIpi*engine = (EngineIpi*)getEngine();
	ResultsIpi *results = engine->process(
		(const char *)nullptr);
	validate(results);
	delete results;
}
 
void EngineIpIntelligenceTests::verifyWithEmptyIpAddress() {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results = engine->process(
		"");
	validate(results);
	delete results;
}


void EngineIpIntelligenceTests::verify() {
	EngineTests::verify();
	verifyWithEvidence();
	verifyWithIpv4Address();
	verifyWithIpv6Address();
	verifyWithBadIpv4Address();
	verifyWithBadIpv6Address();
	verifyWithBoundIpv4Address();
	verifyWithBoundIpv6Address();
	verifyWithOutOfRangeIpv4Address();
	verifyWithOutOfRangeIpv6Address();
	verifyWithEmptyEvidence();
	verifyWithEmptyIpAddress();
	verifyWithNullIpAddress();
	verifyWithNullEvidence();
	verifyWithInvalidInput();
}

bool EngineIpIntelligenceTests::validateIpAddressInternal(IpAddress ipAddress, int length) {
	const unsigned char *ip = ipAddress.getIpAddress();
	int octet;
	for (int i = 0; i < length; i++) {
		octet = (int)ip[i];
		if (octet < 0 || octet > 255) {
			return false;
		}
	}
	return true;
}

bool EngineIpIntelligenceTests::validateIpAddress(IpAddress ipAddress) {
	bool result = false;

	switch(ipAddress.getType()) {
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4:
		result = validateIpAddressInternal(ipAddress, FIFTYONE_DEGREES_IPV4_LENGTH);
		break;
	case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6:
		result = validateIpAddressInternal(ipAddress, FIFTYONE_DEGREES_IPV6_LENGTH);
		break;
	default:
		break;
	}
	return result;
}

void EngineIpIntelligenceTests::verifyIpAddressValue(
	const char *ipAddress, 
	Value<IpAddress> value) {
	EXPECT_EQ(true, value.hasValue()) << "Could not find an IP range that matches"
		"the IP address: " << ipAddress;

	EXPECT_EQ(true, validateIpAddress(value.getValue())) << "An invalid IP address has been"
		"returned, where it should be for IP address: " << ipAddress;
}

void EngineIpIntelligenceTests::ipAddressPresent(const char *ipAddress) {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results = engine->process(ipAddress);
	Value<IpAddress> rangeStart = results->getValueAsIpAddress("RangeStart");
	Value<IpAddress> rangeEnd = results->getValueAsIpAddress("RangeEnd");

	verifyIpAddressValue(ipAddress, rangeStart);
	verifyIpAddressValue(ipAddress, rangeEnd);

	delete results;
}

void EngineIpIntelligenceTests::boundIpAddressPresent(const char *ipAddress) {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results = engine->process(ipAddress);
	Value<IpAddress> rangeStart = results->getValueAsIpAddress("RangeStart");
	Value<IpAddress> rangeEnd = results->getValueAsIpAddress("RangeEnd");
	uint32_t addressLength = 
		rangeStart.getValue().getType() == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4 ?
		FIFTYONE_DEGREES_IPV4_LENGTH :
		FIFTYONE_DEGREES_IPV6_LENGTH;

	verifyIpAddressValue(ipAddress, rangeStart);
	verifyIpAddressValue(ipAddress, rangeEnd);

	EXPECT_EQ(0, 
		memcmp(rangeStart.getValue().getIpAddress(), 
			rangeEnd.getValue().getIpAddress(), 
			addressLength)) << "RangeStart are not the same as RangeEnd, "
		"where it should be at IP address: " << ipAddress;

	delete results;
}

void EngineIpIntelligenceTests::randomIpAddressPresent(int count) {
	EngineIpi *engine = (EngineIpi*)getEngine();

	for (int i = 0; i < count; i++) {
		string ipAddress = ipAddresses[rand() % ipAddresses.size()];
		ResultsIpi *results = engine->process(
			ipAddress.c_str());

		Value<IpAddress> rangeStart = results->getValueAsIpAddress("RangeStart");
		Value<IpAddress> rangeEnd = results->getValueAsIpAddress("RangeEnd");
		uint32_t addressLength = 
			rangeStart.getValue().getType() == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4 ?
			FIFTYONE_DEGREES_IPV4_LENGTH :
			FIFTYONE_DEGREES_IPV6_LENGTH;

		verifyIpAddressValue(ipAddress.c_str(), rangeStart);
		verifyIpAddressValue(ipAddress.c_str(), rangeEnd);

		delete results;
	}
}

void EngineIpIntelligenceTests::verifyNetworkId(const char *ipAddress) {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results = engine->process(ipAddress);
	string networkId = results->getNetworkId();
	// By default regex use ECMAScript regex
	// For special pattern character such as '\d'
	// the '\' need to be escaped. eg. '\\d'
	regex matchPattern = regex("^(\\d+\\:\\d+.\\d+)(\\|(\\d+\\:\\d+.\\d+))*");
	
	EXPECT_EQ(true, regex_match(networkId, matchPattern)) << "An "
		"invalid network ID has been returned, where it should be for IP address: "
		<< ipAddress;

	delete results;
}

void EngineIpIntelligenceTests::verifyCoordinate() {
	EngineIpi *engine = (EngineIpi*)getEngine();
	uint32_t defaultCount = 0;
	// Use a constant to make sure it is always
	// testing a decent number of IP addresses
	const int count = 50;

	for (int i = 0; i < count; i++) {
		string ipAddress = ipAddresses[rand() % ipAddresses.size()];
		ResultsIpi *results = engine->process(
			ipAddress.c_str());
		Value<pair<float, float>> value = results->getValueAsCoordinate("AverageLocation");
		pair<float, float> coordinate;

		EXPECT_EQ(true, value.hasValue()) << "Could not find an IP range that matches"
			"the IP address: " << ipAddress;

		coordinate = value.getValue();
		EXPECT_EQ(true, (coordinate.first >= -90.0f && coordinate.first <= 90.0f)) << "An "
			"invalid latitude has been returned, where it should be for IP address: " 
			<< ipAddress;

		EXPECT_EQ(true, (coordinate.second >= -180.0f && coordinate.second <= 180.0f)) << "An "
			"invalid longitude has been returned, where it should be for IP address: "
			<< ipAddress;

		if (coordinate.first == 0 && coordinate.second == 0) {
			// Counting the number of default coordinate returned
			// This is to ensure we won't run into the case where
			// the default value is always returned.
			defaultCount++;
		}

		delete results;
	}

	EXPECT_EQ(true, defaultCount < count) << "A special case has occurs where all test "
		"addresses have returned a default coordinate of (0, 0). This need to be verified "
		"that it is not a coincidence.";
}

void EngineIpIntelligenceTests::randomWithIpAddress(int count) {
	EngineIpi *engine = (EngineIpi*)getEngine();
	for (int i = 0; i < count; i++) {
		string ipAddress = ipAddresses[rand() % ipAddresses.size()];
		ResultsIpi *results = engine->process(
			ipAddress.c_str());
		validateQuick(results);
		delete results;
	}
}

string EngineIpIntelligenceTests::getRandomKeyWithMatchingPrefix(
	vector<string> *keys,
	string prefix) {
	string key;
	do {
		key = keys->at(rand() % (keys->size() - 1));
	} while (fiftyoneDegreesEvidenceMapPrefix(key.c_str())->prefixEnum !=
		fiftyoneDegreesEvidenceMapPrefix(prefix.c_str())->prefixEnum);
	return key;
}

void EngineIpIntelligenceTests::randomWithEvidence(int count) {
	string ipv4Key = "ipv4.ip";
	string ipv6Key = "ipv6.ip";
	IpAddress ipAddress1;
	IpAddress ipAddress2;
	EngineIpi *engine = (EngineIpi*)getEngine();
	for (int i = 0; i < count; i++) {
		EvidenceIpi evidence;
		try{
			ipAddress1 = IpAddress(
				ipAddresses[rand() % ipAddresses.size()].c_str());
			ASSERT_NE(FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_INVALID, ipAddress1.getType());

			if (ipAddress1.getType() == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
				evidence[ipv4Key] = ipAddress1;
			} 
			else {
				evidence[ipv6Key] = ipAddress1;
			}
			
			ResultsIpi *results = engine->process(&evidence);
			validateQuick(results);
			delete results;
		}
		catch (bad_alloc& e) {
			cout << "Failed to create IpAddress object\n";
			cout << e.what() << "\n";
			FAIL();
		}
	}
}

void EngineIpIntelligenceTests::multiThreadRandomRunThread(void* state) {
	((EngineIpIntelligenceTests*)state)->randomWithIpAddress(200);
	((EngineIpIntelligenceTests*)state)->randomWithEvidence(200);
	FIFTYONE_DEGREES_THREAD_EXIT;
}

/**
 * Check that multiple threads can fetch items from the cache safely.
 * NOTE: it is important that 'number of threads' <=
 * 'number of values' / 'number of threads'. This prevents null from being
 * returned by the cache.
 * @param concurrency number of threads to run the test with
 */
void EngineIpIntelligenceTests::multiThreadRandom(uint16_t concurrency) {
	if (fiftyoneDegreesThreadingGetIsThreadSafe() == false) {
		return;
	}
	ASSERT_NE(nullptr, getEngine());
	runThreads(
		concurrency,
		(FIFTYONE_DEGREES_THREAD_ROUTINE)multiThreadRandomRunThread);
}

 void EngineIpIntelligenceTests::compareResults(
 	ResultsIpi *a, 
 	ResultsIpi *b) {
 	EXPECT_NE(a->results->b.dataSet, b->results->b.dataSet) <<
 		"The data set was not reloaded.";
 	EXPECT_EQ(a->getAvailableProperties(), b->getAvailableProperties()) <<
 		"Number of properties available does not match";
 	for (size_t i = 0; i < a->getProperties().size(); i++) {
 		vector<string> av = *a->getValues((int)i);
 		vector<string> bv = *b->getValues((int)i);
 		EXPECT_EQ(av.size(), bv.size()) << "Expected same number of values";
 		for (size_t v = 0; v < av.size(); v++) {
 			EXPECT_STREQ(av[v].c_str(), bv[v].c_str()) <<
 				"Values for the new data set should be the same.";
 		}
 	}
 }
 
bool EngineIpIntelligenceTests::fileReadToByteArray() {
	fiftyoneDegreesStatusCode status = fiftyoneDegreesFileReadToByteArray(
		fullName,
		&data);
	return status == FIFTYONE_DEGREES_STATUS_SUCCESS;
}
 
void EngineIpIntelligenceTests::reloadFile() {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results1 = engine->process(
		ipv4Address);
	engine->refreshData();
	ResultsIpi *results2 = engine->process(
		ipv4Address);
	compareResults(results1, results2);
	delete results1;
	delete results2;
}
 
void EngineIpIntelligenceTests::reloadMemory() {
	EngineIpi *engine = (EngineIpi*)getEngine();
	ResultsIpi *results1 = engine->process(
		ipv4Address);
	fiftyoneDegreesMemoryReader newData;
	fiftyoneDegreesStatusCode status = fiftyoneDegreesFileReadToByteArray(
		fullName,
		&newData);
	EXPECT_EQ(status, FIFTYONE_DEGREES_STATUS_SUCCESS) << "New data could "
		"not be loaded into memory from '" << fullName << "'";
	EXPECT_NE(newData.current, nullptr) << "New data could "
		"not be loaded into memory from '" << fullName << "'";
	engine->refreshData(newData.current, newData.length);
	ResultsIpi *results2 = engine->process(
		ipv4Address);
	compareResults(results1, results2);
	delete results1;
	delete results2;

	// Now that the results1 has been deleted free the memory used by the 
	// now replaced original dataset. Set the data that will be freed 
	// during tear down to that which is now active in memory.
	fiftyoneDegreesFree(data.current);
	data = newData;
}

ENGINE_TEST_CONFIGS(Ipi)