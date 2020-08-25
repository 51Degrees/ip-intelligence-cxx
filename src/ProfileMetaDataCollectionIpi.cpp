#include "ProfileMetaDataCollectionIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ProfileMetaDataCollectionIpi::ProfileMetaDataCollectionIpi(
	fiftyoneDegreesResourceManager *manager)
	: Collection<uint32_t, ProfileMetaData>() {
	dataSet = DataSetIpiGet(manager);
	if (dataSet == nullptr) {
		throw runtime_error("Data set pointer can not be null");
	}
	profiles = dataSet->profiles;
	profileOffsets = dataSet->profileOffsets;
}

ProfileMetaDataCollectionIpi::~ProfileMetaDataCollectionIpi() {
	DataSetIpiRelease(dataSet);
}

ProfileMetaData* ProfileMetaDataCollectionIpi::getByIndex(uint32_t index) {
	EXCEPTION_CREATE;
	Item item;
	ProfileMetaData *result = nullptr;
	Profile *profile;
	DataReset(&item.data);
	profile = ProfileGetByIndex(
		profileOffsets,
		profiles,
		index,
		&item,
		exception);
	EXCEPTION_THROW
	if (profile != nullptr) {
		result = ProfileMetaDataBuilderIpi::build(dataSet, profile);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

ProfileMetaData* ProfileMetaDataCollectionIpi::getByKey(uint32_t key) {
	EXCEPTION_CREATE;
	Item item;
	ProfileMetaData *result = nullptr;
	Profile *profile;
	DataReset(&item.data);
	profile = ProfileGetByProfileId(
		profileOffsets,
		profiles,
		key,
		&item,
		exception);
	EXCEPTION_THROW;
	if (profile != nullptr) {
		result = ProfileMetaDataBuilderIpi::build(dataSet, profile);
		COLLECTION_RELEASE(item.collection, &item);
	}
	return result;
}

uint32_t ProfileMetaDataCollectionIpi::getSize() {
	return CollectionGetCount(profileOffsets);
}