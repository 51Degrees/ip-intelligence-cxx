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

			Collection<byte, ComponentMetaData>* getComponents();

			Collection<string, PropertyMetaData>* getProperties();

			Collection<uint32_t, ProfileMetaData>* getProfiles();

			Collection<ValueMetaDataKey, ValueMetaData>* getValues();

			Collection<ValueMetaDataKey, ValueMetaData>*
				getValuesForProperty(PropertyMetaData *property);

			Collection<ValueMetaDataKey, ValueMetaData>*
				getValuesForProfile(ProfileMetaData *profile);

			ComponentMetaData* getComponentForProfile(
				ProfileMetaData *profile);

			ComponentMetaData* getComponentForProperty(
				PropertyMetaData *property);

			ProfileMetaData* getDefaultProfileForComponent(
				ComponentMetaData *component);

			ValueMetaData* getDefaultValueForProperty(
				PropertyMetaData *property);

			Collection<string, PropertyMetaData>*
				getPropertiesForComponent(ComponentMetaData *component);

			Collection<string, PropertyMetaData>*
				getEvidencePropertiesForProperty(PropertyMetaData *property);

			PropertyMetaData* getPropertyForValue(ValueMetaData *value);
			/**
			 * @}
			 */
		};
	}
}

#endif
