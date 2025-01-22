/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2025 51 Degrees Mobile Experts Limited, Davidson House,
 * Forbury Square, Reading, Berkshire, United Kingdom RG1 3EU.
 *
 * This Original Work is licensed under the European Union Public Licence (EUPL) 
 * v.1.2 and is subject to its terms as set out below.
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

#ifndef FIFTYONE_DEGREES_METADATA_IPI_HPP
#define FIFTYONE_DEGREES_METADATA_IPI_HPP

#include "common-cxx/Exceptions.hpp"
#include "common-cxx/MetaData.hpp"
#include "PropertyMetaDataCollectionIpi.hpp"
#include "ValueMetaDataCollectionIpi.hpp"
#include "ValueMetaDataCollectionForPropertyIpi.hpp"
#include "ValueMetaDataCollectionForProfileIpi.hpp"
#include "ComponentMetaDataCollectionIpi.hpp"
#include "ProfileMetaDataCollectionIpi.hpp"
#include "PropertyMetaDataCollectionForComponentIpi.hpp"
#include "PropertyMetaDataCollectionForPropertyIpi.hpp"
#include "PropertyMetaDataBuilderIpi.hpp"
#include "ipi.h"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * @copydoc MetaData
		 */
		class MetaDataIpi : public MetaData
		{
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			 /**
			  * @copydoc Common::MetaData::MetaData
			  */
			MetaDataIpi(
				shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * @copydoc Common::MetaData::~MetaData
			 */
			~MetaDataIpi();

			/**
			 * @}
			 * @name Common::MetaData Implementation
			 * @{
			 */

			Collection<::byte, ComponentMetaData>* getComponents() const;

			Collection<string, PropertyMetaData>* getProperties() const;

			Collection<uint32_t, ProfileMetaData>* getProfiles() const;

			Collection<ValueMetaDataKey, ValueMetaData>* getValues() const;

			Collection<ValueMetaDataKey, ValueMetaData>*
				getValuesForProperty(PropertyMetaData *property) const;

			Collection<ValueMetaDataKey, ValueMetaData>*
				getValuesForProfile(ProfileMetaData *profile) const;

			ComponentMetaData* getComponentForProfile(
				ProfileMetaData *profile) const;

			ComponentMetaData* getComponentForProperty(
				PropertyMetaData *property) const;

			ProfileMetaData* getDefaultProfileForComponent(
				ComponentMetaData *component) const;

			ValueMetaData* getDefaultValueForProperty(
				PropertyMetaData *property) const;

			Collection<string, PropertyMetaData>*
				getPropertiesForComponent(ComponentMetaData *component) 
				const;

			Collection<string, PropertyMetaData>*
				getEvidencePropertiesForProperty(PropertyMetaData *property)
				const;

			PropertyMetaData* getPropertyForValue(ValueMetaData *value)
				const;

			/**
			 * @}
			 */
		};
	}
}

#endif
