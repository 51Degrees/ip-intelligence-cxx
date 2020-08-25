#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_FOR_COMPONENT_IPI_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_FOR_COMPONENT_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "common-cxx/PropertyMetaData.hpp"
#include "common-cxx/ComponentMetaData.hpp"
#include "ComponentMetaDataBuilderIpi.hpp"
#include "PropertyMetaDataCollectionIpi.hpp"
#include "Ipi.h"
#include <memory>

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of property meta data relating to a component
		 * contained in a Hash engine.
		 */
		class PropertyMetaDataCollectionForComponentIpi
			: public Collection<string, PropertyMetaData> {
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
			 * @param component pointer to the component which the
			 * properties must relate to
			 */
			PropertyMetaDataCollectionForComponentIpi(
				fiftyoneDegreesResourceManager *manager,
				ComponentMetaData *component);

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			 /**
			  * Releases the data set being referenced by the collection.
			  */
			~PropertyMetaDataCollectionForComponentIpi();

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