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

#ifndef FIFTYONE_DEGREES_CONFIG_IPI_HPP
#define FIFTYONE_DEGREES_CONFIG_IPI_HPP

#include <cstddef>
#include "common-cxx/CollectionConfig.hpp"
#include "common-cxx/ConfigBase.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * C++ class wrapper for the #fiftyoneDegreesConfigIpi
		 * configuration structure. See ipi.h.
		 *
		 * This extends the ConfigBase class to add IP Intelligence
		 * specific configuration options.
		 *
		 * Configuration options are set using setter methods and fetched
		 * using corresponding getter methods. The names are self
		 * explanatory.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * using namespace FiftyoneDegrees::IpIntelligence;
		 * string dataFilePath;
		 * RequiredPropertiesConfig *properties;
		 *
		 * // Construct a new configuration
		 * ConfigIpi *config = new ConfigIpi();
		 *
		 * // Use the configuration when constructing an engine
		 * EngineIpi *engine = new EngineIpi(
		 *     dataFilePath,
		 *     config,
		 *     properties);
		 * ```
		 */
		class ConfigIpi : public ConfigBase {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Construct a new instance using the default configuration
			 * #fiftyoneDegreesIpiDefaultConfig.
			 */
			ConfigIpi();

			/**
			 * Construct a new instance using the configuration provided.
			 * The values are copied and no reference to the provided
			 * parameter is retained.
			 * @param config pointer to the configuration to copy
			 */
			ConfigIpi(fiftyoneDegreesConfigIpi *config);

			/** 
			 * @}
			 * @name Setters
			 * @{
			 */

			/**
			 * Set the collections to use the high performance
			 * configuration.
			 * See #fiftyoneDegreesIpiHighPerformanceConfig
			 */
			void setHighPerformance();

			/**
			 * Set the collections to use the balanced configuration.
			 * See #fiftyoneDegreesIpiBalancedConfig
			 */
			void setBalanced();

			/**
			 * Set the collections to use the balanced temp configuration.
			 * See #fiftyoneDegreesIpiBalancedTempConfig
			 */
			void setBalancedTemp();

			/**
			 * Set the collections to use the low memory configuration.
			 * See #fiftyoneDegreesIpiLowMemoryConfig
			 */
			void setLowMemory();

			/**
			 * Set the collections to use the entirely in memory
			 * configuration.
			 * See #fiftyoneDegreesIpiInMemoryConfig
			 */
			void setMaxPerformance();

			/**
			 * Set the expected concurrent requests for all the data set's
			 * collections. All collections in the data set which use
			 * cached elements will have their caches constructued to allow
			 * for the concurrency value set here.
			 * See CollectionConfig::setConcurrency
			 * @param concurrency expected concurrent requests
			 */
			void setConcurrency(uint16_t concurrency);


			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the configuration for the strings collection.
			 * @return strings collection configuration
			 */
			CollectionConfig getStrings();

			/**
			 * Get the configuration for the properties collection.
			 * @return properties collection configuration
			 */
			CollectionConfig getProperties();

			/**
			 * Get the configuration for the values collection.
			 * @return values collection configuration
			 */
			CollectionConfig getValues();

			/**
			 * Get the configuration for the profiles collection.
			 * @return profiles collection configuration
			 */
			CollectionConfig getProfiles();

			/**
			 * Get the configuration for the ipv4Graph collection.
			 * @return ipv4 ranges collection configuration
			 */
			CollectionConfig getIpv4Ranges();

			/**
			 * Get the configuration for the ipv6Graph collection.
			 * @return ipv6 ranges collection configuration
			 */
			CollectionConfig getIpv6Ranges();

			/**
			 * Get the configuration for the profilesCombinations collection.
			 * @return profiles combinations collection configuration
			 */
			CollectionConfig getProfileCombinations();

			/**
			 * Get the configuration for the profile offsets collection.
			 * @return profile offsets collection configuration
			 */
			CollectionConfig getProfileOffsets();

			/**
			 * Get the lowest concurrency value in the list of possible
			 * concurrencies.
			 * @return a 16 bit integer with the minimum concurrency value.
			 */
			uint16_t getConcurrency() const override;

			 /**
			  * Gets the configuration data structure for use in C code.
			  * Used internally.
			  * @return pointer to the underlying configuration data
			  * structure.
			  */
			fiftyoneDegreesConfigIpi* getConfig();

			/** 
			 * @}
			 */
		private:
			/** The underlying configuration structure */
			fiftyoneDegreesConfigIpi config;

			/** The underlying strings configuration structure */
			CollectionConfig strings;

			/** The underlying properties configuration structure */
			CollectionConfig properties;

			/** The underlying values configuration structure */
			CollectionConfig values;

			/** The underlying profiles configuration structure */
			CollectionConfig profiles;

			/** The underlying ipv4 ranges configuration structure */
			CollectionConfig ipRoots;

			/** The underlyig ipv6 ranges configuration structure */
			CollectionConfig ipNodes;

			/** The underlying profile combinations configuration structure */
			CollectionConfig profileCombinations;

			/** The underlying profile offsets configuration structure */
			CollectionConfig profileOffsets;

			/** The underlying data set maps configuration structure */
			CollectionConfig maps;

			/** The underlying components configuration structure */
			CollectionConfig components;

			/**
			 * Initialise the collection configurations by creating
			 * instances from the IP Intelligence configuration structure.
			 */
			void initCollectionConfig();

			/**
			 * Set the performance profile from an existing configuration.
			 * @param existing pointer to a configuration to copy the
			 * performance profile from
			 */
			void setPerformanceFromExistingConfig(
				fiftyoneDegreesConfigIpi *existing);
		};
	}
}
#endif	