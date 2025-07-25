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

#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_IPI_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_COLLECTION_IPI_HPP

#include "common-cxx/Collection.hpp"
#include "PropertyMetaDataBuilderIpi.hpp"
#include "ipi.h"


namespace FiftyoneDegrees {
	namespace IpIntelligence {
		using FiftyoneDegrees::Common::Collection;

		/**
		 * A collection of all property meta data contained in an IP 
		 * Intelligence engine.
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
			~PropertyMetaDataCollectionIpi() override;

			PropertyMetaData* getByIndex(uint32_t index) const override;

			PropertyMetaData* getByKey(string name) const override;

			uint32_t getSize() const override;
			
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