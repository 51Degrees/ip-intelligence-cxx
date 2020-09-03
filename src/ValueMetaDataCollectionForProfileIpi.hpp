#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_FOR_PROFILE_IPI_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_FOR_PROFILE_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "common-cxx/ProfileMetaData.hpp"
#include "ValueMetaDataCollectionBaseIpi.hpp"
#include "ValueMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of value meta data relating to a profile contained
		 * in an IP Intelligence engine.
		 */
		class ValueMetaDataCollectionForProfileIpi
			: public ValueMetaDataCollectionBaseIpi {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Constructs a new instance of the collection from the data
			 * set managed by the manager provided.
			 * @param manager pointer to the manager which manages the
			 * data set
			 * @param profile pointer to the profile which the values must
			 * relate to
			 */
			ValueMetaDataCollectionForProfileIpi(
				fiftyoneDegreesResourceManager *manager,
				ProfileMetaData *profile);

			/**
			 * Releases the data set being referenced by the collection.
			 */
			~ValueMetaDataCollectionForProfileIpi();

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			ValueMetaData* getByIndex(uint32_t index);

			ValueMetaData* getByKey(ValueMetaDataKey key);

			uint32_t getSize();

			/**
			 * @}
			 */
		private:
			/**
			 * Get the underlying profile which the values all relate to.
			 * @return pointer to the profile
			 */
			fiftyoneDegreesProfile* getProfile();

			/** The underlying profile collection item */
			fiftyoneDegreesCollectionItem profileItem;

			/**
			 * Filter used when iterating over values. This returns only
			 * values which relate to the profile in this instance.
			 * @param state pointer to a #fiftyoneDegreesFilterResult to
			 * use
			 * @param item pointer to the item to store the values in
			 */
			static bool valueFilter(
				void *state,
				fiftyoneDegreesCollectionItem *item);
		};
	}
}

#endif