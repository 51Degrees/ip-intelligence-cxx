#include "ValueMetaDataCollectionForPropertyIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ValueMetaDataCollectionForPropertyIpi::ValueMetaDataCollectionForPropertyIpi(
	fiftyoneDegreesResourceManager *manager,
	PropertyMetaData *property) : ValueMetaDataCollectionBaseIpi(manager) {
	EXCEPTION_CREATE;
	DataReset(&propertyItem.data);
	PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		property->getName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForPropertyIpi::~ValueMetaDataCollectionForPropertyIpi() {
	COLLECTION_RELEASE(dataSet->properties, &propertyItem);
}

ValueMetaData* ValueMetaDataCollectionForPropertyIpi::getByIndex(
	uint32_t index) {
	EXCEPTION_CREATE;
	Item item;
	Value *value;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	value = ValueGet(
		dataSet->values, 
		getProperty()->firstValueIndex + index, 
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderIpi::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

ValueMetaData* ValueMetaDataCollectionForPropertyIpi::getByKey(
	ValueMetaDataKey key) {
	EXCEPTION_CREATE;
	Item item;
	ValueMetaData *result = nullptr;
	DataReset(&item.data);
	String *name = PropertyGetName(
		dataSet->strings, 
		getProperty(), 
		&item, 
		exception);
	EXCEPTION_THROW;
	if (name != nullptr) {
		result = ValueMetaDataCollectionBaseIpi::getByKey(key);
		if (strcmp(
			result->getKey().getPropertyName().c_str(), 
			&name->value) != 0) {
			delete result;
			result = nullptr;
		}
		dataSet->strings->release(&item);
	}
	return result;
}

uint32_t ValueMetaDataCollectionForPropertyIpi::getSize() {
	return (int)getProperty()->firstValueIndex == -1 ?
		0 :
		getProperty()->lastValueIndex - getProperty()->firstValueIndex + 1;
}

fiftyoneDegreesProperty* ValueMetaDataCollectionForPropertyIpi::getProperty() {
	return (Property*)propertyItem.data.ptr;
}