#include "ValueMetaDataCollectionIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ValueMetaDataCollectionIpi::ValueMetaDataCollectionIpi(
	fiftyoneDegreesResourceManager *manager)
	: ValueMetaDataCollectionBaseIpi(manager) {
}

ValueMetaDataCollectionIpi::~ValueMetaDataCollectionIpi() {}

ValueMetaData* ValueMetaDataCollectionIpi::getByIndex(uint32_t index) {
	EXCEPTION_CREATE;
	Item item;
	Value *value;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	value = ValueGet(dataSet->values, index, &item, exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderIpi::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

ValueMetaData* ValueMetaDataCollectionIpi::getByKey(ValueMetaDataKey key) {
	return ValueMetaDataCollectionBaseIpi::getByKey(key);
}

uint32_t ValueMetaDataCollectionIpi::getSize() {
	return CollectionGetCount(dataSet->values);
}