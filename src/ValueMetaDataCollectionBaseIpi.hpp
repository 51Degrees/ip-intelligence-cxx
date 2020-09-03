#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_BASE_IPI_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_BASE_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "ValueMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of all value meta data contained in an IP Intelligence
		 * engine.
		 */
		class ValueMetaDataCollectionBaseIpi
			: public Collection<ValueMetaDataKey, ValueMetaData> {
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
			  */
			ValueMetaDataCollectionBaseIpi(
				fiftyoneDegreesResourceManager *manager);

			/**
			 * Releases the data set being referenced by the collection.
			 */
			virtual ~ValueMetaDataCollectionBaseIpi();

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			ValueMetaData* getByKey(ValueMetaDataKey key);

			/**
			 * @}
			 */
		protected:
			/** Pointer to the underlying data set containing the values. */
			fiftyoneDegreesDataSetIpi *dataSet;
		};
	}
}

#endif