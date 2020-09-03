#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_FOR_PROPERTY_IPI_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_COLLECTION_FOR_PROPERTY_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "common-cxx/PropertyMetaData.hpp"
#include "ValueMetaDataCollectionBaseIpi.hpp"
#include "ValueMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of value meta data relating to a property contained
		 * in an IP Intelligence engine.
		 */
		class ValueMetaDataCollectionForPropertyIpi
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
			 * @param property pointer to the property which the values
			 * must relate to
			 */
			ValueMetaDataCollectionForPropertyIpi(
				fiftyoneDegreesResourceManager *manager,
				PropertyMetaData *property);
		
			/**
			 * Releases the data set being referenced by the collection.
			 */
			~ValueMetaDataCollectionForPropertyIpi();

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
			 * Get the underlying property which the values all relate to.
			 * @return pointer to the property
			 */
			fiftyoneDegreesProperty* getProperty();
			
			/** The underlying property collection item */
			fiftyoneDegreesCollectionItem propertyItem;
		};
	}
}

#endif