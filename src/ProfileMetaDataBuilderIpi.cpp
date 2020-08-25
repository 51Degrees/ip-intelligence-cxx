#include "ProfileMetaDataBuilderIpi.hpp"

#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ProfileMetaData* ProfileMetaDataBuilderIpi::build(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesProfile *profile) {
	byte componentId = ((Component*)dataSet->componentsList
		.items[profile->componentIndex].data.ptr)->componentId;
	return new ProfileMetaData(
		profile->profileId,
		componentId);
}
