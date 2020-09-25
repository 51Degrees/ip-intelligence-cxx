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

#include <iostream>
#include "EngineIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::IpIntelligence;

EngineIpi::EngineIpi(
	const char *fileName,
	IpIntelligence::ConfigIpi *config,
	Common::RequiredPropertiesConfig *properties)
	: EngineBase(config, properties) {
	EXCEPTION_CREATE;
	StatusCode status = IpiInitManagerFromFile(
		manager.get(),
		config->getConfig(),
		properties->getConfig(),
		fileName,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status, fileName);
		return;
	}
	EXCEPTION_THROW;
	init();
}

EngineIpi::EngineIpi(
	const string &fileName,
	IpIntelligence::ConfigIpi *config,
	Common::RequiredPropertiesConfig *properties)
	: EngineIpi(fileName.c_str(), config, properties) {
}

EngineIpi::EngineIpi(
	void *data,
	long length,
	IpIntelligence::ConfigIpi *config,
	Common::RequiredPropertiesConfig *properties) 
	: EngineBase(config, properties) {
	EXCEPTION_CREATE;

	// Copy the data and hand the responsibility for cleaning up to the C layer
	config->getConfig()->b.freeData = true;
	void *dataCopy = copyData(data, length);

	StatusCode status = IpiInitManagerFromMemory(
		manager.get(),
		config->getConfig(),
		properties->getConfig(),
		dataCopy,
		(size_t)length,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
	init();
}

EngineIpi::EngineIpi(
	unsigned char data[],
	long length,
	IpIntelligence::ConfigIpi *config,
	Common::RequiredPropertiesConfig *properties)
	: EngineIpi((void*)data, length, config, properties) {
}

void EngineIpi::init() {
	DataSetIpi *dataSet = DataSetIpiGet(manager.get());
	initHttpHeaderKeys(dataSet->b.b.uniqueHeaders);
	initMetaData();
	DataSetIpiRelease(dataSet);
}

void* EngineIpi::copyData(void *data, size_t length) {
	void *dataCopy = (void*)Malloc(length);
	if (dataCopy == nullptr) {
		throw StatusCodeException(INSUFFICIENT_MEMORY);
	}
	memcpy(dataCopy, data, length);
	return dataCopy;
}

/**
 * @return the name of the data set used contained in the source file.
 */
string EngineIpi::getProduct() {
	stringstream stream;
	DataSetIpi *dataSet = DataSetIpiGet(manager.get());
	appendString(stream, dataSet->strings, dataSet->header.nameOffset);
	DataSetIpiRelease(dataSet);
	return stream.str();
}

/**
 * Returns the string that represents the type of data file when requesting an
 * updated file.
 */
string EngineIpi::getType() {
	return string("IpiV41");
}

/**
 * @return the date that 51Degrees published the data file.
 */
Date EngineIpi::getPublishedTime() {
	DataSetIpi*dataSet = DataSetIpiGet(manager.get());
	Date date = Date(&dataSet->header.published);
	DataSetIpiRelease(dataSet);
	return date;
}

/**
 * @return the date that 51Degrees will publish an updated data file.
 */
Date EngineIpi::getUpdateAvailableTime() {
	DataSetIpi *dataSet = DataSetIpiGet(manager.get());
	Date date = Date(&dataSet->header.nextUpdate);
	DataSetIpiRelease(dataSet);
	return date;
}

string EngineIpi::getDataFilePath() {
	DataSetIpi *dataSet = DataSetIpiGet(manager.get());
	string path = string(dataSet->b.b.masterFileName);
	DataSetIpiRelease(dataSet);
	return path;
}

string EngineIpi::getDataFileTempPath() {
	string path;
	DataSetIpi *dataSet = DataSetIpiGet(manager.get());
	if (strcmp(
		dataSet->b.b.masterFileName,
		dataSet->b.b.fileName) == 0) {
		path = string("");
	}
	else {
		path = string(dataSet->b.b.fileName);
	}
	DataSetIpiRelease(dataSet);
	return path;
}

void EngineIpi::refreshData() {
	EXCEPTION_CREATE;
	StatusCode status = IpiReloadManagerFromOriginalFile(
		manager.get(),
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
}

void EngineIpi::refreshData(const char *fileName) {
	EXCEPTION_CREATE;
	StatusCode status = IpiReloadManagerFromFile(
		manager.get(),
		fileName,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
}

void EngineIpi::refreshData(void *data, long length) {
	EXCEPTION_CREATE;
	void *dataCopy = copyData(data, length);
	StatusCode status = IpiReloadManagerFromMemory(
		manager.get(),
		dataCopy,
		length,
		exception);
	if (status != SUCCESS) {
		throw StatusCodeException(status);
	}
	EXCEPTION_THROW;
}

void EngineIpi::refreshData(
	unsigned char data[], 
	long length) {
	refreshData((void*)data, length);
}

IpIntelligence::ResultsIpi* EngineIpi::process(
	IpIntelligence::EvidenceIpi *evidence) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsIpi *results = ResultsIpiCreate(
		manager.get());
	ResultsIpiFromEvidence(
		results, 
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	if (exception->status != 
		FIFTYONE_DEGREES_STATUS_INCORRECT_IP_ADDRESS_FORMAT) {
		EXCEPTION_THROW;
	}

	return new ResultsIpi(results, manager);
}

IpIntelligence::ResultsIpi* EngineIpi::process(
	const char *ipAddress) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsIpi *results = ResultsIpiCreate(
		manager.get());
	ResultsIpiFromIpAddressString(
		results,
		ipAddress,
		ipAddress == nullptr ? 0 : strlen(ipAddress),
		exception);
	if (exception->status != 
		FIFTYONE_DEGREES_STATUS_INCORRECT_IP_ADDRESS_FORMAT) {
		EXCEPTION_THROW;
	}
	return new ResultsIpi(results, manager);
}

IpIntelligence::ResultsIpi *EngineIpi::process(
	unsigned char ipAddress[], 
	long length,
	fiftyoneDegreesEvidenceIpType type) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsIpi *results = ResultsIpiCreate(manager.get());
	ResultsIpiFromIpAddress(
		results, 
		ipAddress,
		length,
        type,
		exception);
	if (exception->status != 
		FIFTYONE_DEGREES_STATUS_INCORRECT_IP_ADDRESS_FORMAT) {
		EXCEPTION_THROW;
	}
	return new ResultsIpi(results, manager);
}

Common::ResultsBase* EngineIpi::processBase(
	Common::EvidenceBase *evidence) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsIpi *results = ResultsIpiCreate(
		manager.get());
	ResultsIpiFromEvidence(
		results, 
		evidence == nullptr ? nullptr : evidence->get(),
		exception);
	if (exception->status != 
		FIFTYONE_DEGREES_STATUS_INCORRECT_IP_ADDRESS_FORMAT) {
		EXCEPTION_THROW;
	}
	return new ResultsIpi(results, manager);
}

void EngineIpi::initHttpHeaderKeys(fiftyoneDegreesHeaders *uniqueHeaders) {
	uint32_t i, p;
	const char *prefixes[] = { "query.", "server." };
	for (i = 0; i < uniqueHeaders->count; i++) {
		for (p = 0; p < sizeof(prefixes) / sizeof(const char*); p++) {
			string key = string(prefixes[p]);
			key.append(&((fiftyoneDegreesString*)
				uniqueHeaders->items[i].name.data.ptr)->value);
			addKey(key);
		}
	}
}

void EngineIpi::initMetaData() {
	metaData = new MetaDataIpi(manager);
}