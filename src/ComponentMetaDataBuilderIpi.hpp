/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence
 * (EUPL) v.1.2 and is subject to its terms as set out below.
 *
 * If a copy of the EUPL was not distributed with this file, You can obtain
 * one at https://opensource.org/licenses/EUPL-1.2.
 *
 * The 'Compatible Licences' set out in the Appendix to the EUPL (as may be
 * amended by the European Commission) shall be deemed incompatible for
 * the purposes of the Work and the provisions of the compatibility
 * clause in Article 5 of the EUPL shall not apply.
 *
 * If using the Work as, or as part of, a network application, by
 * including the attribution notice(s) required under Article 5 of the EUPL
 * in the end user terms of the application under an appropriate heading,
 * such notice(s) shall fulfill the requirements of that article.
 * ********************************************************************* */

#ifndef FIFTYONE_DEGREES_COMPONENT_META_DATA_BUILDER_IPI_HPP
#define FIFTYONE_DEGREES_COMPONENT_META_DATA_BUILDER_IPI_HPP

#include <vector>
#include "common-cxx/ComponentMetaData.hpp"
#include "common-cxx/EntityMetaDataBuilder.hpp"
#include "ipi.h"

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		using Common::EntityMetaDataBuilder;
		using Common::ComponentMetaData;

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