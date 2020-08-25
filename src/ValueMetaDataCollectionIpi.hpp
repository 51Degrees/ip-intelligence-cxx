#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_IPI_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_IPI_HPP

#include "ValueMetaDataCollectionBaseIpi.hpp"
#include "ValueMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of all value meta data contained in a Hash engine.
		 */
		class ValueMetaDataCollectionIpi
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
			  */
			ValueMetaDataCollectionIpi(
				fiftyoneDegreesResourceManager *manager);
			/**
			 * Releases the data set being referenced by the collection.
			 */
			~ValueMetaDataCollectionIpi();

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
		};
	}
}

#endif