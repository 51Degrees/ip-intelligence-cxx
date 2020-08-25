#include "ValueMetaDataBuilderIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

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
			ValueMetaDataKey(
				getString(dataSet->strings, property->nameOffset),
				getString(dataSet->strings, value->nameOffset)),
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
