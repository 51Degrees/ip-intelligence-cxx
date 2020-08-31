#include "ValueMetaDataKeyIpi.hpp"
#include "ValueMetaDataBuilderIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

BinaryValue ValueMetaDataBuilderIpi::getBinaryValue(
	fiftyoneDegreesCollection *stringsCollection,
	uint32_t offset) {
	FIFTYONE_DEGREES_EXCEPTION_CREATE;
	BinaryValue result;
	fiftyoneDegreesCollectionItem item;
	fiftyoneDegreesString *str;
	fiftyoneDegreesDataReset(&item.data);
	str = fiftyoneDegreesStringGet(
		stringsCollection,
		offset,
		&item,
		exception);
	FIFTYONE_DEGREES_EXCEPTION_THROW;
	if (str != nullptr) {
		result = BinaryValue((const byte *)str, str->size);
	}
	FIFTYONE_DEGREES_COLLECTION_RELEASE(
		stringsCollection,
		&item);
	return result;
}

ValueMetaData* ValueMetaDataBuilderIpi::build(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesValue *value) {
	EXCEPTION_CREATE;
	ValueMetaData *result = nullptr;
	Item item;
	Property *property;
	DataReset(&item.data);
	property = PropertyGet(
		dataSet->properties,
		value->propertyIndex, 
		&item,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = new ValueMetaData(
			ValueMetaDataKeyIpi(
				getString(dataSet->strings, property->nameOffset),
				getBinaryValue(dataSet->strings, property->nameOffset)),
			value->descriptionOffset == -1 ?
			"" :
			getString(dataSet->strings, value->descriptionOffset),
			value->urlOffset == -1 ?
			"" :
			getString(dataSet->strings, value->urlOffset));
		COLLECTION_RELEASE(dataSet->properties, &item);
	}
	return result;
}
