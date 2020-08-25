#include "MetaDataIpi.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

MetaDataIpi::MetaDataIpi(
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: MetaData(manager) {
}

MetaDataIpi::~MetaDataIpi() {
}

Collection<byte, ComponentMetaData>* MetaDataIpi::getComponents()
{
	return new ComponentMetaDataCollectionIpi(manager.get());
}

Collection<string, PropertyMetaData>* MetaDataIpi::getProperties()
{
	return new PropertyMetaDataCollectionIpi(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>* MetaDataIpi::getValues()
{
	return new ValueMetaDataCollectionIpi(manager.get());
}

Collection<uint32_t, ProfileMetaData>* MetaDataIpi::getProfiles()
{
	return new ProfileMetaDataCollectionIpi(manager.get());
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataIpi::getValuesForProperty(
	PropertyMetaData *property) {
	return new ValueMetaDataCollectionForPropertyIpi(
		manager.get(),
		property);
}

Collection<ValueMetaDataKey, ValueMetaData>*
MetaDataIpi::getValuesForProfile(
	ProfileMetaData *profile) {
	return new ValueMetaDataCollectionForProfileIpi(
		manager.get(),
		profile);
}

ComponentMetaData* MetaDataIpi::getComponentForProfile(
	ProfileMetaData *profile) {
	ComponentMetaData *result = nullptr;
	Collection<byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(profile->getComponentId());
		delete components;
	}
	return result;
}

ComponentMetaData* MetaDataIpi::getComponentForProperty(
	PropertyMetaData *property) {
	ComponentMetaData *result = nullptr;
	Collection<byte, ComponentMetaData> *components = getComponents();
	if (components != nullptr) {
		result = components->getByKey(property->getComponentId());
		delete components;
	}
	return result;
}

ProfileMetaData* MetaDataIpi::getDefaultProfileForComponent(
	ComponentMetaData *component) {
	ProfileMetaData *result = nullptr;
	Collection<uint32_t, ProfileMetaData> *profiles = getProfiles();
	if (profiles != nullptr) {
		result = profiles->getByKey(component->getDefaultProfileId());
		delete profiles;
	}
	return result;
}

ValueMetaData* MetaDataIpi::getDefaultValueForProperty(
	PropertyMetaData *property) {
	ValueMetaData *result = nullptr;
	Collection<ValueMetaDataKey, ValueMetaData> *values = getValues();
	if (values != nullptr) {
		result = values->getByKey(ValueMetaDataKey(
			property->getName(), 
			property->getDefaultValue()));
		delete values;
	}
	return result;
}

Collection<string, PropertyMetaData>*
MetaDataIpi::getPropertiesForComponent(
	ComponentMetaData *component) {
	return new PropertyMetaDataCollectionForComponentIpi(
		manager.get(),
		component);
}

Collection<string, PropertyMetaData>*
MetaDataIpi::getEvidencePropertiesForProperty(
	PropertyMetaData *property) {
	return new PropertyMetaDataCollectionForPropertyIpi(
		manager.get(),
		property);
}

PropertyMetaData* MetaDataIpi::getPropertyForValue(
	ValueMetaData *value) {
	PropertyMetaData *result = nullptr;
	Collection<string, PropertyMetaData> *properties = getProperties();
	if (properties != nullptr) {
		result = properties->getByKey(value->getKey().getPropertyName());
		delete properties;
	}
	return result;
}
