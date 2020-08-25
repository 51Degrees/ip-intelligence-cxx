#include <algorithm>
#include "ConfigIpi.hpp"

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
	config.ipv4Ranges = existing->ipv4Ranges;
    config.ipv6Ranges = existing->ipv6Ranges;
    config.profileCombinations = existing->profileCombinations;
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
	return ipv4Ranges;
}

CollectionConfig ConfigIpi::getIpv6Ranges() { 
	return ipv6Ranges;
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
	ipv4Ranges = CollectionConfig(&config.ipv4Ranges);
    ipv6Ranges = CollectionConfig(&config.ipv6Ranges);
    profileCombinations = CollectionConfig(&config.profileCombinations);
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
uint16_t ConfigIpi::getConcurrency() {
	uint16_t concurrencies[] = {
		strings.getConcurrency(),
		properties.getConcurrency(),
		values.getConcurrency(),
		profiles.getConcurrency(),
		ipv4Ranges.getConcurrency(),
		ipv6Ranges.getConcurrency(),
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
	ipv4Ranges.setConcurrency(concurrency);
    ipv6Ranges.setConcurrency(concurrency);
    profileCombinations.setConcurrency(concurrency);
	profileOffsets.setConcurrency(concurrency);
	maps.setConcurrency(concurrency);
	components.setConcurrency(concurrency);
}