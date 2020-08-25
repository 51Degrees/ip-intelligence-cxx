#include "ValueMetaDataCollectionBaseIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ValueMetaDataCollectionBaseIpi::ValueMetaDataCollectionBaseIpi(
	fiftyoneDegreesResourceManager *manager)
	: Collection<ValueMetaDataKey, ValueMetaData>() {
	dataSet = DataSetIpiGet(manager);
	if (dataSet == nullptr) {
		throw runtime_error("Data set pointer can not be null");
	}
}

ValueMetaDataCollectionBaseIpi::~ValueMetaDataCollectionBaseIpi() {
	DataSetIpiRelease(dataSet);
}

ValueMetaData* ValueMetaDataCollectionBaseIpi::getByKey(
	ValueMetaDataKey key) {
	EXCEPTION_CREATE;
	Item propertyItem, valueItem;
	Value *value;
	Property *property;
	ValueMetaData *result = nullptr;
	DataReset(&propertyItem.data);
	property = PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		key.getPropertyName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		DataReset(&valueItem.data);
		value = ValueGetByName(
			dataSet->values,
			dataSet->strings,
			property,
			key.getValueName().c_str(),
			&valueItem,
			exception);
		EXCEPTION_THROW;
		if (value != nullptr) {
			result = ValueMetaDataBuilderIpi::build(dataSet, value);
			COLLECTION_RELEASE(dataSet->values, &valueItem);
		}
		COLLECTION_RELEASE(dataSet->properties, &propertyItem);
	}
	return result;
}