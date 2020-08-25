#include "PropertyMetaDataCollectionForComponentIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

PropertyMetaDataCollectionForComponentIpi::PropertyMetaDataCollectionForComponentIpi(
	fiftyoneDegreesResourceManager *manager,
	ComponentMetaData *component) : Collection<string, PropertyMetaData>() {
	EXCEPTION_CREATE;
	Item item;
	Property *property;
	Component *propertyComponent;
	DataSetIpi *dataSet = DataSetIpiGet(manager);
	if (dataSet != nullptr) {
		DataReset(&item.data);
		uint32_t propertiesCount = CollectionGetCount(dataSet->properties);
		for (uint32_t i = 0; i < propertiesCount; i++) {
			property = (Property*)PropertyGet(
				dataSet->properties,
				i,
				&item,
				exception);
			EXCEPTION_THROW;
			if (property != nullptr) {
				propertyComponent = (Component*)dataSet->componentsList.items[
					property->componentIndex].data.ptr;
				if (propertyComponent->componentId ==
					component->getComponentId()) {
					properties.push_back(shared_ptr<PropertyMetaData>(
						PropertyMetaDataBuilderIpi::build(
							dataSet,
							property)));
				}
				COLLECTION_RELEASE(dataSet->properties, &item);
			}
		}
		DataSetIpiRelease(dataSet);
	}
}

PropertyMetaDataCollectionForComponentIpi::
~PropertyMetaDataCollectionForComponentIpi() {
	properties.clear();
}

PropertyMetaData* PropertyMetaDataCollectionForComponentIpi::getByIndex(
	uint32_t index) {
	return new PropertyMetaData(*properties.at(index));
}

PropertyMetaData* PropertyMetaDataCollectionForComponentIpi::getByKey(
	string name) {
	PropertyMetaData *result = nullptr;
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

uint32_t PropertyMetaDataCollectionForComponentIpi::getSize() {
	return (uint32_t)properties.size();
}