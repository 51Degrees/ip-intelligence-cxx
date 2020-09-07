%include stdint.i
%include std_pair.i
%include std_vector.i

%include "common-cxx/ResultsBase.i"
%include "WeightedValue.i"
%include "IpAddress.i"

%template(WeightedStringListSwig) std::vector<WeightedValue<std::string>>;
%template(WeightedBoolListSwig) std::vector<WeightedValue<bool>>;
%template(WeightedIntListSwig) std::vector<WeightedValue<int>>;
%template(WeightedDoubleListSwig) std::vector<WeightedValue<double>>;

%template(FloatPairSwig) std::pair<float, float>;

%template(WeightedStringListValueSwig) Value<std::vector<WeightedValue<std::string>>>;
%template(WeightedBoolListValueSwig) Value<std::vector<WeightedValue<bool>>>;
%template(WeightedIntListValueSwig) Value<std::vector<WeightedValue<int>>>;
%template(WeightedDoubleListValueSwig) Value<std::vector<WeightedValue<double>>>;
%template(FloatPairValueSwig) Value<std::pair<float, float>>;
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

    Value<std::pair<float, float>> getValueAsCoordinate(
        const std::string &propertyName);
    Value<std::pair<float, float>> getValueAsCoordinate(
        int requiredPropertyIndex);

    Value<IpAddress> getValueAsIpAddress(
        const std::string &propertyName);
    Value<IpAddress> getValueAsIpAddress(
        int requiredPropertyIndex);
};
