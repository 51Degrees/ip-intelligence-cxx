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

	if (component->defaultProfileOffset == -1) {
		// The component is a dynamic component
		// so create it with profileId = 0
		// This can then be check if a component
		// is dynamic through metadata class
		result = new ComponentMetaData(
			component->componentId,
			getString(dataSet->strings, component->nameOffset),
			0);
	}
	else {
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
	}
	return result;
}
