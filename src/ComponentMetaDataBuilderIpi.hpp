#ifndef FIFTYONE_DEGREES_COMPONENT_META_DATA_BUILDER_IPI_HPP
#define FIFTYONE_DEGREES_COMPONENT_META_DATA_BUILDER_IPI_HPP

#include <vector>
#include "common-cxx/ComponentMetaData.hpp"
#include "common-cxx/EntityMetaDataBuilder.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Meta data builder class contains static helper methods used when
		 * building component meta data instances from an IP Intelligence
		 * data set.
		 */
		class ComponentMetaDataBuilderIpi : EntityMetaDataBuilder {
		public:
			/**
			 * Build a new instance of ComponentMetaData from the
			 * underlying component provided. The instance returned does
			 * not hold a reference to the data set or component, and
			 * contains a copy of all the meta data.
			 * @param dataSet pointer to the data set containing the
			 * component
			 * @param component pointer to the underlying component to
			 * create the meta data from
			 */
			static ComponentMetaData* build(
				fiftyoneDegreesDataSetIpi *dataSet,
				fiftyoneDegreesComponent *component);
		};
	}
}

#endif