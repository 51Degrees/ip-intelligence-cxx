%include std_string.i

%nodefaultctor WeightedValue;

%rename (WeightedValueSwig) WeightedValue;

template <class T> class WeightedValue {
public:
    T getValue();
    void setValue(T v);
    float getWeight();
    void setWeight();
};

%template(WeightedValueStringSwig) WeightedValue<std::string>;
%template(WeightedValueBoolSwig) WeightedValue<bool>;
%template(WeightedValueIntSwig) WeightedValue<int>;
%template(WeightedValueDoubleSwig) WeightedValue<double>;
