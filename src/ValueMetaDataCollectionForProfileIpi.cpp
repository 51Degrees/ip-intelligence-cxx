#include "ValueMetaDataCollectionForProfileIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

struct FilterResult {
	DataSetIpi *dataSet = nullptr;
	string valueName;
	Value value {0, 0, 0, 0};
	bool found = false;
};

ValueMetaDataCollectionForProfileIpi::ValueMetaDataCollectionForProfileIpi(
	fiftyoneDegreesResourceManager *manager,
	ProfileMetaData *profile) : ValueMetaDataCollectionBaseIpi(manager) {
	EXCEPTION_CREATE;
	DataReset(&profileItem.data);
	ProfileGetByProfileId(
		dataSet->profileOffsets,
		dataSet->profiles,
		profile->getProfileId(),
		&profileItem,
		exception);
	EXCEPTION_THROW;
}

ValueMetaDataCollectionForProfileIpi::~ValueMetaDataCollectionForProfileIpi() {
	COLLECTION_RELEASE(dataSet->profiles, &profileItem);
}

ValueMetaData* ValueMetaDataCollectionForProfileIpi::getByIndex(uint32_t index) {
	EXCEPTION_CREATE;
	ValueMetaData *result = nullptr;
	Value *value;
	Item item;
	DataReset(&item.data);
	uint32_t valueIndex = ((uint32_t*)(getProfile() + 1))[index];
	value = ValueGet(
		dataSet->values,
		valueIndex,
		&item,
		exception);
	EXCEPTION_THROW;
	if (value != nullptr) {
		result = ValueMetaDataBuilderIpi::build(dataSet, value);
		COLLECTION_RELEASE(dataSet->values, &item);
	}
	return result;
}

bool ValueMetaDataCollectionForProfileIpi::valueFilter(
	void *state, 
	fiftyoneDegreesCollectionItem *valueItem) {
	EXCEPTION_CREATE;
	Item nameItem;
	String *name;
	Value *value;
	FilterResult *result = (FilterResult*)state;
	value = (Value*)valueItem->data.ptr;
	DataReset(&nameItem.data);
	name = ValueGetName(
		result->dataSet->strings, 
		value, 
		&nameItem,
		exception);
	EXCEPTION_THROW;
	if (name != nullptr) {
		if (strcmp(&name->value, result->valueName.c_str()) == 0) {
			memcpy(&result->value, value, sizeof(Value));
			result->found = true;
		}
		COLLECTION_RELEASE(result->dataSet->strings, &nameItem);
	}
	COLLECTION_RELEASE(result->dataSet->values, valueItem);
	return true;
}

ValueMetaData* ValueMetaDataCollectionForProfileIpi::getByKey(
	ValueMetaDataKey key) {
	EXCEPTION_CREATE;
	Item propertyItem;
	Property *property;
	uint32_t count;
	ValueMetaData *result = nullptr;
	FilterResult state;
	DataReset(&propertyItem.data);
	property = PropertyGetByName(
		dataSet->properties,
		dataSet->strings,
		key.getPropertyName().c_str(),
		&propertyItem,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		state.dataSet = dataSet;
		state.valueName = key.getValueName();
		count = ProfileIterateValuesForProperty(
			dataSet->values,
			getProfile(),
			property,
			&state,
			&valueFilter,
			exception);
		EXCEPTION_THROW;
		if (count > 0 && state.found == true) {
			result = ValueMetaDataBuilderIpi::build(dataSet, &state.value);
		}
	}
	return result;
}

uint32_t ValueMetaDataCollectionForProfileIpi::getSize() {
	return getProfile()->valueCount;
}

fiftyoneDegreesProfile* ValueMetaDataCollectionForProfileIpi::getProfile() {
	return (Profile*)profileItem.data.ptr;
}