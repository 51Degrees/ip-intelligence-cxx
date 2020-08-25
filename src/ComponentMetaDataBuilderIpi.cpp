#include "ComponentMetaDataBuilderIpi.hpp"

#include "fiftyone.h"
#include "common-cxx/Exceptions.hpp"

using namespace FiftyoneDegrees::IpIntelligence;

ComponentMetaData* ComponentMetaDataBuilderIpi::build(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesComponent *component) {
	EXCEPTION_CREATE;
	ComponentMetaData *result = nullptr;
	Item item;
	Profile *profile;
	DataReset(&item.data);
	profile = (Profile*)dataSet->profiles->get(
		dataSet->profiles, 
		component->defaultProfileOffset,
		&item,
		exception);
	EXCEPTION_THROW;
	if (profile != nullptr) {
		result = new ComponentMetaData(
			component->componentId,
			getString(dataSet->strings, component->nameOffset),
			profile->profileId);
		COLLECTION_RELEASE(dataSet->profiles, &item);
	}
	return result;
}
