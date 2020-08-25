#include "PropertyMetaDataBuilderIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

PropertyMetaData* PropertyMetaDataBuilderIpi::build(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesProperty *property) {
	string name = getString(dataSet->strings, property->nameOffset);
	return new PropertyMetaData(
		name,
		getPropertyMap(
			dataSet->strings,
			dataSet->maps,
			property),
		getPropertyType(property),
		(int)property->categoryOffset != -1 ?
		getString(dataSet->strings, property->categoryOffset) :
		string(),
		(int)property->urlOffset != -1 ?
		getString(dataSet->strings, property->urlOffset) :
		string(),
		propertyIsAvailable(dataSet, &name),
		property->displayOrder,
		property->isMandatory,
		property->isList,
		property->isObsolete,
		property->show,
		property->showValues,
		(int)property->descriptionOffset != -1 ?
		getString(dataSet->strings, property->descriptionOffset) :
		string(),
		getDefaultValue(dataSet, property->defaultValueIndex),
		getComponentId(dataSet, property),
		getEvidenceProperties(dataSet, property));
}

byte PropertyMetaDataBuilderIpi::getComponentId(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesProperty *property) {
	return ((Component*)dataSet->componentsList.items[
		property->componentIndex].data.ptr)->componentId;
}

vector<uint32_t> PropertyMetaDataBuilderIpi::getEvidenceProperties(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesProperty *property) {
	EXCEPTION_CREATE;
	vector<uint32_t> indexes;
	uint32_t i;
	int index;
	Item item;
	String* name;
	DataReset(&item.data);
	name = PropertyGetName(
		dataSet->strings,
		property,
		&item,
		exception);
	EXCEPTION_THROW;
	index = PropertiesGetRequiredPropertyIndexFromName(
		dataSet->b.b.available,
		&name->value);
	COLLECTION_RELEASE(dataSet->strings, &item);

	if (index >= 0) {
		fiftyoneDegreesEvidenceProperties* evidenceProperties =
			dataSet->b.b.available->items[index].evidenceProperties;

		indexes.reserve(evidenceProperties->count);
		for (i = 0;
			i < evidenceProperties->count;
			i++) {
			indexes.push_back(evidenceProperties->items[i]);
		}
	}
	return indexes;

}
string PropertyMetaDataBuilderIpi::getDefaultValue(
	fiftyoneDegreesDataSetIpi *dataSet,
	uint32_t valueIndex) {
	EXCEPTION_CREATE;
	string result;
	Item item;
	Value *value;
	if (valueIndex != UINT32_MAX) {
		DataReset(&item.data);
		value = ValueGet(
			dataSet->values,
			valueIndex,
			&item,
			exception);
		EXCEPTION_THROW;
			if (value != nullptr) {
				result = getString(dataSet->strings, value->nameOffset);
				COLLECTION_RELEASE(dataSet->values, &item);
			}
	}
	return result;
}

bool PropertyMetaDataBuilderIpi::propertyIsAvailable(
	fiftyoneDegreesDataSetIpi* dataSet,
	string *name) {
	return PropertiesGetPropertyIndexFromName(
		dataSet->b.b.available,
		name->c_str()) >= 0;
}

string PropertyMetaDataBuilderIpi::getPropertyType(
	fiftyoneDegreesProperty *property) {
	switch (property->valueType) {
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_INTEGER:
		return string("int");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN:
		return string("bool");
    case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE:
		return string("double");
    case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE:
		return string("coordinate");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_JAVASCRIPT:
		return string("javascript");
	case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING:
		if (property->isList != 0) {
			return string("string[]");
		}
		return string("string");
	default:
		return string("string");
	}
}

vector<string> PropertyMetaDataBuilderIpi::getPropertyMap(
	fiftyoneDegreesCollection *stringsCollection,
	fiftyoneDegreesCollection *mapsCollection,
	fiftyoneDegreesProperty *property) {
	EXCEPTION_CREATE;
	uint32_t i, offset;
	const char *name;
	vector<string> map;
	Item item;
	DataReset(&item.data);
	map.reserve(property->mapCount);
	for (i = property->firstMapIndex;
		i < (uint32_t)(property->firstMapIndex + property->mapCount);
		i++) {
		offset = CollectionGetInteger32(
			mapsCollection, 
			i, 
			exception);
		EXCEPTION_THROW;
		name = STRING(StringGet(
			stringsCollection,
			offset,
			&item,
			exception));
		EXCEPTION_THROW;
		if (name != nullptr) {
			map.push_back(string(name));
		}
		COLLECTION_RELEASE(stringsCollection, &item);
	}
	return map;
}