#ifndef FIFTYONE_DEGREES_PROFILE_META_DATA_BUILDER_HASH_HPP
#define FIFTYONE_DEGREES_PROFILE_META_DATA_BUILDER_HASH_HPP

#include <vector>
#include "common-cxx/ProfileMetaData.hpp"
#include "common-cxx/EntityMetaDataBuilder.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Meta data builder class contains static helper methods used when
		 * building profile meta data instances from a Hash data set.
		 */
		class ProfileMetaDataBuilderIpi : EntityMetaDataBuilder {
		public:
			/**
			 * Build a new instance of ProfileMetaData from the underlying
			 * profile provided. The instance returned does not hold a
			 * reference to the data set or profile, and contains a copy of
			 * all the meta data.
			 * @param dataSet pointer to the data set containing the
			 * component
			 * @param profile pointer to the underlying profile to create
			 * the meta data from
			 */
			static ProfileMetaData* build(
				fiftyoneDegreesDataSetIpi *dataSet,
				fiftyoneDegreesProfile *profile);
		};
	}
}

#endif