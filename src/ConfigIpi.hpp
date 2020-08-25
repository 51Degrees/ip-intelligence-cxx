#ifndef FIFTYONE_DEGREES_CONFIG_HASH_HPP
#define FIFTYONE_DEGREES_CONFIG_HASH_HPP

#include <cstddef>
#include "common-cxx/CollectionConfig.hpp"
#include "common-cxx/ConfigBase.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * C++ class wrapper for the #fiftyoneDegreesConfigHash
		 * configuration structure. See hash.h.
		 *
		 * This extends the ConfigDeviceDetection class to add Hash
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
		 * using namespace FiftyoneDegrees::DeviceDetection::Hash;
		 * string dataFilePath;
		 * RequiredPropertiesConfig *properties;
		 *
		 * // Construct a new configuration
		 * ConfigHash *config = new ConfigHash();
		 *
		 * // Configure the engine to load the entire data set into memory
		 * // for maximum performance, and set the maximum drift and
		 * // difference to allow when finding substring hashes
		 * config->setMaxPerformance();
		 * config->setDrift(2);
		 * config->setDifference(10);
		 *
		 * // Use the configuration when constructing an engine
		 * EngineHash *engine = new EngineHash(
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
			 * #fiftyoneDegreesHashDefaultConfig.
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
			 * See #fiftyoneDegreesHashHighPerformanceConfig
			 */
			void setHighPerformance();

			/**
			 * Set the collections to use the balanced configuration.
			 * See #fiftyoneDegreesHashBalancedConfig
			 */
			void setBalanced();

			/**
			 * Set the collections to use the balanced temp configuration.
			 * See #fiftyoneDegreesHashBalancedTempConfig
			 */
			void setBalancedTemp();

			/**
			 * Set the collections to use the low memory configuration.
			 * See #fiftyoneDegreesHashLowMemoryConfig
			 */
			void setLowMemory();

			/**
			 * Set the collections to use the entirely in memory
			 * configuration.
			 * See #fiftyoneDegreesHashInMemoryConfig
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
			 * Get the configuration for the nodes collection.
			 * @return nodes collection configuration
			 */
			CollectionConfig getIpv4Ranges();

			CollectionConfig getIpv6Ranges();

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
			uint16_t getConcurrency();

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

			/** The underlying nodes configuration structure */
			CollectionConfig ipv4Ranges;

			CollectionConfig ipv6Ranges;

			CollectionConfig profileCombinations;

			/** The underlying profile offsets configuration structure */
			CollectionConfig profileOffsets;

			/** The underlying data set maps configuration structure */
			CollectionConfig maps;

			/** The underlying components configuration structure */
			CollectionConfig components;

			/**
			 * Initialise the collection configurations by creating
			 * instances from the Hash configuration structure.
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