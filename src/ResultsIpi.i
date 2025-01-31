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

%include stdint.i
%include std_pair.i
%include std_vector.i

%include "common-cxx/Coordinate.i"
%include "common-cxx/ResultsBase.i"
%include "WeightedValue.i"
%include "common-cxx/IpAddress.i"

%template(WeightedStringListSwig) std::vector<WeightedValue<std::string>>;
%template(WeightedBoolListSwig) std::vector<WeightedValue<bool>>;
%template(WeightedIntListSwig) std::vector<WeightedValue<int>>;
%template(WeightedDoubleListSwig) std::vector<WeightedValue<double>>;
%template(WeightedCoordinateListSwig) std::vector<WeightedValue<fiftyoneDegreesCoordinate>>;

%template(WeightedStringListValueSwig) Value<std::vector<WeightedValue<std::string>>>;
%template(WeightedBoolListValueSwig) Value<std::vector<WeightedValue<bool>>>;
%template(WeightedIntListValueSwig) Value<std::vector<WeightedValue<int>>>;
%template(WeightedDoubleListValueSwig) Value<std::vector<WeightedValue<double>>>;
%template(WeightedCoordinateListValueSwig) Value<std::vector<WeightedValue<fiftyoneDegreesCoordinate>>>;
%template(CoordinateValueSwig) Value<fiftyoneDegreesCoordinate>;
%template(IpAddressValueSwig) Value<IpAddress>;

%nodefaultctor ResultsIpi;

%rename (ResultsIpiSwig) ResultsIpi;

class ResultsIpi : public ResultsBase {
public:
    virtual ~ResultsIpi();
    std::string getNetworkId();
    std::string getNetworkId(uint32_t resultsIndex);

    Value<std::vector<WeightedValue<std::string>>> getValuesAsWeightedStringList(
        const std::string &propertyName);
    Value<std::vector<WeightedValue<std::string>>> getValuesAsWeightedStringList(
        int requiredPropertyIndex);

    Value<std::vector<WeightedValue<std::string>>> getValuesAsWeightedWKTStringList(
        const std::string &propertyName, uint8_t decimalPlaces);
    Value<std::vector<WeightedValue<std::string>>> getValuesAsWeightedWKTStringList(
        int requiredPropertyIndex, uint8_t decimalPlaces);

    Value<std::vector<WeightedValue<bool>>> getValuesAsWeightedBoolList(
        const std::string &propertyName);
    Value<std::vector<WeightedValue<bool>>> getValuesAsWeightedBoolList(
        int requiredPropertyIndex);
    
    Value<std::vector<WeightedValue<int>>> getValuesAsWeightedIntegerList(
        const std::string &propertyName);
    Value<std::vector<WeightedValue<int>>> getValuesAsWeightedIntegerList(
        int requiredPropertyIndex);

    Value<std::vector<WeightedValue<double>>> getValuesAsWeightedDoubleList(
        const std::string &propertyName);
    Value<std::vector<WeightedValue<double>>> getValuesAsWeightedDoubleList(
        int requiredPropertyIndex);

    Value<std::vector<WeightedValue<fiftyoneDegreesCoordinate>>> getValuesAsWeightedCoordinateList(
        const std::string &propertyName);
    Value<std::vector<WeightedValue<fiftyoneDegreesCoordinate>>> getValuesAsWeightedCoordinateList(
        int requiredPropertyIndex);

    Value<fiftyoneDegreesCoordinate> getValueAsCoordinate(
        const std::string &propertyName);
    Value<fiftyoneDegreesCoordinate> getValueAsCoordinate(
        int requiredPropertyIndex);

    Value<IpAddress> getValueAsIpAddress(
        const std::string &propertyName);
    Value<IpAddress> getValueAsIpAddress(
        int requiredPropertyIndex);
};
