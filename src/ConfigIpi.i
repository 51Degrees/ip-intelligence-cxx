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
    const CollectionConfig &getStrings() const;
    const CollectionConfig &getComponents() const;
    const CollectionConfig &getMaps() const;
    const CollectionConfig &getProperties() const;
    const CollectionConfig &getValues() const;
    const CollectionConfig &getProfiles() const;
    const CollectionConfig &getGraphs() const;
    const CollectionConfig &getProfileGroups() const;
    const CollectionConfig &getProfileOffsets() const;
    const CollectionConfig &getPropertyTypes() const;
    const CollectionConfig &getGraph() const;
    uint16_t getConcurrency() const;
};
