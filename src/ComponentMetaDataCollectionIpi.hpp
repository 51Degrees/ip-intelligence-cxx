#ifndef FIFTYONE_DEGREES_COMPONENT_META_DATA_COLLECTION_IPI_HPP
#define FIFTYONE_DEGREES_COMPONENT_META_DATA_COLLECTION_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "ComponentMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * A collection of component meta data contained in a IP Intelligence
		 * engine.
		 */
		class ComponentMetaDataCollectionIpi
			: public Collection<byte, ComponentMetaData> {
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
			ComponentMetaDataCollectionIpi(
				fiftyoneDegreesResourceManager *manager);

			/**
			 * Releases the data set being referenced by the collection.
			 */
			~ComponentMetaDataCollectionIpi();

			/**
			 * @}
			 * @name Common::Collection Implementation
			 * @{
			 */

			ComponentMetaData* getByIndex(uint32_t index);

			ComponentMetaData* getByKey(byte componentId);

			uint32_t getSize();

			/**
			 * @}
			 */
		private:
			/** The underlying data set containing the components. */
			fiftyoneDegreesDataSetIpi *dataSet;

			/** The underlying data set containing the components. */
			fiftyoneDegreesList *components;
		};
	}
}

#endif