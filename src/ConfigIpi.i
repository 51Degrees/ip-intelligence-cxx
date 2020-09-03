%include stdint.i

%include "common-cxx/ConfigBase.i"
%include "common-cxx/CollectionConfig.i"

%nodefaultctor ConfigIpi;

%rename (ConfigIpiSwig) ConfigIpi;

class ConfigIpi : public ConfigBase {
public:
    ConfigIpi();
    void setHighPerformance();
    void setBalanced();
    void setBalancedTemp();
    void setLowMemory();
    void setMaxPerformance();
    void setConcurrency(uint16_t concurrency);
    CollectionConfig getStrings();
    CollectionConfig getProperties();
    CollectionConfig getValues();
    CollectionConfig getProfiles();
    CollectionConfig getIpv4Ranges();
    CollectionConfig getIpv6Ranges();
    CollectionConfig getProfileCombinations();
    CollectionConfig getProfileOffsets();
    uint16_t getConcurrency();
};
