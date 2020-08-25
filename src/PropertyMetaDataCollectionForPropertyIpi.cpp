#include "PropertyMetaDataCollectionForPropertyIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

PropertyMetaDataCollectionForPropertyIpi::PropertyMetaDataCollectionForPropertyIpi(
	fiftyoneDegreesResourceManager* manager,
	PropertyMetaData *property) : Collection<string, PropertyMetaData>() {
	EXCEPTION_CREATE;
	Item item;
	Property* newProperty;
	DataSetIpi* dataSet = DataSetIpiGet(manager);
	if (dataSet != nullptr) {
		DataReset(&item.data);
		vector<uint32_t> indexes = property->getEvidenceProperties();
		for (uint32_t i = 0; i < indexes.size(); i++) {
			newProperty = (Property*)PropertyGet(
				dataSet->properties,
				indexes[i],
				&item,
				exception);
			EXCEPTION_THROW;
			if (newProperty != nullptr) {
				properties.push_back(shared_ptr<PropertyMetaData>(
					PropertyMetaDataBuilderIpi::build(
						dataSet,
						newProperty)));
				COLLECTION_RELEASE(dataSet->properties, &item);
			}
		}

		DataSetIpiRelease(dataSet);
	}
}

PropertyMetaDataCollectionForPropertyIpi::
~PropertyMetaDataCollectionForPropertyIpi() {
	properties.clear();
}

PropertyMetaData* PropertyMetaDataCollectionForPropertyIpi::getByIndex(
	uint32_t index) {
	return new PropertyMetaData(*properties.at(index));
}

PropertyMetaData* PropertyMetaDataCollectionForPropertyIpi::getByKey(
	string name) {
	PropertyMetaData* result = nullptr;
	for (vector<shared_ptr<PropertyMetaData>>::iterator i = properties.begin();
		i != properties.end();
		i++) {
		if (name == (*i)->getName()) {
			result = new PropertyMetaData(**i);
			break;
		}
	}
	return result;
}

uint32_t PropertyMetaDataCollectionForPropertyIpi::getSize() {
	return (uint32_t)properties.size();
}