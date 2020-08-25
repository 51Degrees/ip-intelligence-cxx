#include "ComponentMetaDataCollectionIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ComponentMetaDataCollectionIpi::ComponentMetaDataCollectionIpi(
	fiftyoneDegreesResourceManager *manager)
	: Collection<byte, ComponentMetaData>() {
	dataSet = DataSetIpiGet(manager);
	if (dataSet == nullptr) {
		throw runtime_error("Data set pointer can not be null");
	}
	components = &dataSet->componentsList;
}

ComponentMetaDataCollectionIpi::~ComponentMetaDataCollectionIpi() {
	DataSetIpiRelease(dataSet);
}

ComponentMetaData* ComponentMetaDataCollectionIpi::getByIndex(
	uint32_t index) {
	ComponentMetaData *component = nullptr;
	if (index < components->count) {
		component = ComponentMetaDataBuilderIpi::build(
			dataSet, 
			(Component*)components->items[index].data.ptr);
	}
	return component;
}

ComponentMetaData* ComponentMetaDataCollectionIpi::getByKey(
	byte componentId) {
	ComponentMetaData *result = nullptr;
	Component *component;
	uint32_t i;
	for (i = 0; i < dataSet->componentsList.count; i++) {
		component = (Component*)dataSet->componentsList.items[i].data.ptr;
		if (component->componentId == componentId) {
			result = ComponentMetaDataBuilderIpi::build(dataSet, component);
		}
	}
	return result;
}

uint32_t ComponentMetaDataCollectionIpi::getSize() {
	return components->count;
}