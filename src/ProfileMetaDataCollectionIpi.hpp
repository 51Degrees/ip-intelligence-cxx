#ifndef FIFTYONE_DEGREES_PROFILE_META_DATA_COLLECTION_IPI_HPP
#define FIFTYONE_DEGREES_PROFILE_META_DATA_COLLECTION_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "ProfileMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of all profile meta data contained in an IP
		 * Intelligence engine.
		 */
		class ProfileMetaDataCollectionIpi
			: public Collection<uint32_t, ProfileMetaData> {
		public:
			/**
			 * @name Constructor
			 * @{
			 */
			 
			 /**
			 * Constructs a new instance of the collection from the data
			 * set managed by the manager provided.
			 * @param manager pointer to the manager which manages the data
			 * set
			 */
			ProfileMetaDataCollectionIpi(
				fiftyoneDegreesResourceManager *manager);

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			/**
			 * Releases the data set being referenced by the collection.
			 */
			~ProfileMetaDataCollectionIpi();

			ProfileMetaData* getByIndex(uint32_t index);

			ProfileMetaData* getByKey(uint32_t profileId);

			uint32_t getSize();

			/**
			 * @}
			 */
		private:
			/** Pointer to the data set managed by the resource manager */
			fiftyoneDegreesDataSetIpi *dataSet;

			/** Pointer to the profiles collection in the data set */
			fiftyoneDegreesCollection *profiles;

			/** Pointer to the profile offsets collection in the data set */
			fiftyoneDegreesCollection *profileOffsets;
		};
	}
}

#endif