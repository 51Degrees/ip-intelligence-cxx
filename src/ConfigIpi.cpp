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

#include <algorithm>
#include "ConfigIpi.hpp"

using namespace std;
using namespace FiftyoneDegrees::IpIntelligence;

ConfigIpi::ConfigIpi() : ConfigBase(&this->config.b) {
	config = fiftyoneDegreesIpiDefaultConfig;
	initCollectionConfig();
}

ConfigIpi::ConfigIpi(fiftyoneDegreesConfigIpi *config) :
	ConfigBase(&config->b) {
	this->config = config != nullptr ?
		*config : fiftyoneDegreesIpiBalancedConfig;
	initCollectionConfig();
}

void ConfigIpi::setPerformanceFromExistingConfig(
	fiftyoneDegreesConfigIpi *existing) {
	config.strings = existing->strings;
	config.properties = existing->properties;
	config.values = existing->values;
	config.profiles = existing->profiles;
	config.graphs = existing->graphs;
	config.graph = existing->graph;
	config.profileOffsets = existing->profileOffsets;
	config.maps = existing->maps;
	config.components = existing->components;
	config.b.allInMemory = existing->b.allInMemory;
}

void ConfigIpi::setHighPerformance() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesIpiHighPerformanceConfig);
}

void ConfigIpi::setBalanced() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesIpiBalancedConfig);
}

void ConfigIpi::setBalancedTemp() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesIpiBalancedTempConfig);
}

void ConfigIpi::setLowMemory() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesIpiLowMemoryConfig);
}

void ConfigIpi::setMaxPerformance() {
	setPerformanceFromExistingConfig(&fiftyoneDegreesIpiInMemoryConfig);
}

CollectionConfig ConfigIpi::getStrings() {
	return strings;
}

CollectionConfig ConfigIpi::getProperties() {
	return properties;
}

CollectionConfig ConfigIpi::getValues() {
	return values;
}

CollectionConfig ConfigIpi::getProfiles() {
	return profiles;
}

CollectionConfig ConfigIpi::getIpv4Ranges() {
	return graph;
}

CollectionConfig ConfigIpi::getIpv6Ranges() { 
	return graphs;
}

CollectionConfig ConfigIpi::getProfileCombinations() {
	return profileCombinations;
}

CollectionConfig ConfigIpi::getProfileOffsets() {
	return profileOffsets;
}

void ConfigIpi::initCollectionConfig() {
	strings = CollectionConfig(&config.strings);
	properties = CollectionConfig(&config.properties);
	values = CollectionConfig(&config.values);
	profiles = CollectionConfig(&config.profiles);
	graphs = CollectionConfig(&config.graphs);
	graph = CollectionConfig(&config.graph);
	profileOffsets = CollectionConfig(&config.profileOffsets);
	maps = CollectionConfig(&config.maps);
	components = CollectionConfig(&config.components);
}

/**
 * Gets the configuration data structure for use in C code. Used internally.
 * @return the underlying configuration data structure.
 */
fiftyoneDegreesConfigIpi* ConfigIpi::getConfig() {
	return &config;
}

/**
 * Provides the lowest concurrency value in the list of possible concurrencies.
 * @return a 16 bit integer with the minimum concurrency value.
 */
uint16_t ConfigIpi::getConcurrency() const {
	uint16_t concurrencies[] = {
		strings.getConcurrency(),
		properties.getConcurrency(),
		values.getConcurrency(),
		profiles.getConcurrency(),
		graph.getConcurrency(),
		graphs.getConcurrency(),
        profileCombinations.getConcurrency(),
		profileOffsets.getConcurrency(),
		maps.getConcurrency(),
		components.getConcurrency()};
	return *min_element(concurrencies, 
		concurrencies + (sizeof(concurrencies) / sizeof(uint16_t)));
}

void ConfigIpi::setConcurrency(uint16_t concurrency) {
	strings.setConcurrency(concurrency);
	properties.setConcurrency(concurrency);
	values.setConcurrency(concurrency);
	profiles.setConcurrency(concurrency);
	graph.setConcurrency(concurrency);
	// TODO: Restore
    // ipNodes.setConcurrency(concurrency);
    // profileCombinations.setConcurrency(concurrency);
	profileOffsets.setConcurrency(concurrency);
	maps.setConcurrency(concurrency);
	components.setConcurrency(concurrency);
}