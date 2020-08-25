#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_IPI_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "PropertyMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of all property meta data contained in a Hash
		 * engine.
		 */
		class PropertyMetaDataCollectionIpi
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
			 */
			PropertyMetaDataCollectionIpi(
				fiftyoneDegreesResourceManager *manager);

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			 /**
			  * Releases the data set being referenced by the collection.
			  */
			~PropertyMetaDataCollectionIpi();

			PropertyMetaData* getByIndex(uint32_t index);

			PropertyMetaData* getByKey(string name);

			uint32_t getSize();
			
			/**
			 * @}
			 */
		private:
			/** Pointer to the data set managed by the resource manager */
			fiftyoneDegreesDataSetIpi *dataSet;

			/** Pointer to the underlying properties collection */
			fiftyoneDegreesCollection *properties;
		};
	}
}

#endif