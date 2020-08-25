#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_FOR_PROPERTY_IPI_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_FOR_PROPERTY_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "common-cxx/PropertyMetaData.hpp"
#include "PropertyMetaDataCollectionIpi.hpp"
#include "ipi.h"
#include <memory>

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		class PropertyMetaDataCollectionForPropertyIpi
			: public Collection<string, PropertyMetaData> {
		public:
			/**
			 * @name Constructor
			 * @{
			 */

			PropertyMetaDataCollectionForPropertyIpi(
				fiftyoneDegreesResourceManager* manager,
				PropertyMetaData *property);

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			 /**
			  * Releases the data set being referenced by the collection.
			  */
			~PropertyMetaDataCollectionForPropertyIpi();

			PropertyMetaData* getByIndex(uint32_t index);

			PropertyMetaData* getByKey(string name);

			uint32_t getSize();

			/**
			 * @}
			 */
		private:
			/**
			 * Vector of shared pointers to be handed out. This ensures
			 * they are all cleaned up.
			 */
			vector<shared_ptr<PropertyMetaData>> properties;
		};
	}
}

#endif