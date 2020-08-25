#include "PropertyMetaDataCollectionIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

PropertyMetaDataCollectionIpi::PropertyMetaDataCollectionIpi(
	fiftyoneDegreesResourceManager *manager)
	: Collection<string, PropertyMetaData>() {
	dataSet = DataSetIpiGet(manager);
	if (dataSet == nullptr) {
		throw runtime_error("Data set pointer can not be null");
	}
	properties = dataSet->properties;
}

PropertyMetaDataCollectionIpi::~PropertyMetaDataCollectionIpi() {
	DataSetIpiRelease(dataSet);
}

PropertyMetaData* PropertyMetaDataCollectionIpi::getByIndex(
	uint32_t index) {
	EXCEPTION_CREATE;
	Item item;
	Property *property;
	PropertyMetaData *result = nullptr;
	DataReset(&item.data);
	property = PropertyGet(
		dataSet->properties,
		index,
		&item,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = PropertyMetaDataBuilderIpi::build(dataSet, property);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

PropertyMetaData* PropertyMetaDataCollectionIpi::getByKey(string name) {
	EXCEPTION_CREATE;
	Item item;
	Property *property;
	PropertyMetaData *result = nullptr;
	DataReset(&item.data);
	property = PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		name.c_str(),
		&item,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = PropertyMetaDataBuilderIpi::build(dataSet, property);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

uint32_t PropertyMetaDataCollectionIpi::getSize() {
	return CollectionGetCount(properties);
}