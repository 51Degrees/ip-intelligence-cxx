#include "ResultsIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::IpIntelligence;

#define RESULT(r,i) ((ResultIpi*)r->b.items + i)

IpIntelligence::ResultsIpi::ResultsIpi(
	fiftyoneDegreesResultsIpi *results,
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: ResultsBase(&results->b, manager) {
	this->results = results;
  _jsHardwareProfileRequiredIndex = PropertiesGetRequiredPropertyIndexFromName(
      this->available, "javascripthardwareprofile");
}


IpIntelligence::ResultsIpi::~ResultsIpi() {
	ResultsIpiFree(results);
}

void 
IpIntelligence::ResultsIpi::getValuesInternal(int requiredPropertyIndex, vector<string> &values) {
    // Do nothing
}

fiftyoneDegreesPropertyValueType
IpIntelligence::ResultsIpi::getPropertyValueType(
    int requiredPropertyIndex,
    fiftyoneDegreesException *exception) {
    fiftyoneDegreesPropertyValueType valueType;
    DataSetIpi *dataSet = (DataSetIpi*)results->b.dataSet;

	// Work out the property index from the required property index.
	uint32_t propertyIndex = PropertiesGetPropertyIndexFromRequiredIndex(
		dataSet->b.b.available,
		requiredPropertyIndex);

	// Set the property that will be available in the results structure. 
	// This may also be needed to work out which of a selection of results 
	// are used to obtain the values.
	Property *property = PropertyGet(
		dataSet->properties,
		propertyIndex,
		&results->propertyItem,
		exception);
    if (property != NULL && EXCEPTION_OKAY) {
        valueType = (fiftyoneDegreesPropertyValueType)property->valueType;
    }
    return valueType;
}

Common::Value<pair<float, float>>
IpIntelligence::ResultsIpi::getValuesAsCoordinate(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    ProfilePercentage *valuesItems;
    Common::Value<pair<float, float>> result;
    fiftyoneDegreesPropertyValueType valueType;

    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    if (EXCEPTION_OKAY
        && valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE) {
        // Get a pointer to the first value item for the property.
        valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
        EXCEPTION_THROW;
        
        if (valuesItems == NULL) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }
        
        // Add the values in their original form to the result.
        if (results->values.count > 1) {
            result.setNoValueReason(
                FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
                nullptr);
        }
        else {
            fiftyoneDegreesCoordinate coordinate = IpiGetCoordinate(&valuesItems[0].item, exception);
            EXCEPTION_THROW;

            pair<float, float> floatPair;
            floatPair.first = coordinate.lat;
            floatPair.second = coordinate.lon;
            result.setValue(floatPair);
        }
    }

    return result;
}

Common::Value<pair<float, float>>
IpIntelligence::ResultsIpi::getValuesAsCoordinate(
    const char *propertyName) {
    return getValuesAsCoordinate(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<pair<float, float>>
IpIntelligence::ResultsIpi::getValuesAsCoordinate(
	const string &propertyName) {
    return getValuesAsCoordinate(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<pair<float, float>>
IpIntelligence::ResultsIpi::getValuesAsCoordinate(
    const string *propertyName) {
    return getValuesAsCoordinate(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<IpAddress> 
IpIntelligence::ResultsIpi::getValuesAsIpAddress(int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    ProfilePercentage *valuesItems;
    Common::Value<IpAddress> result;
    fiftyoneDegreesPropertyValueType valueType;

    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    if (EXCEPTION_OKAY
        && valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_RANGE) {
        // Get a pointer to the first value item for the property.
        valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
        EXCEPTION_THROW;
        
        if (valuesItems == NULL) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }
        
        // Add the values in their original form to the result.
        if (results->values.count > 1) {
            result.setNoValueReason(
                FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
                nullptr);
        }
        else {
            IpAddress ipAddress;
            if (results->items[0].type == FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4) {
                ipAddress = IpAddress(
                    ((Ipv4Range *)valuesItems[0].item.data.ptr)->start,
                    FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4);
            }
            else {
                ipAddress = IpAddress(
                    ((Ipv6Range *)valuesItems[0].item.data.ptr)->start,
                    FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6);
            }
            result.setValue(ipAddress);
        }
    }

    return result;
}

Common::Value<IpAddress>
IpIntelligence::ResultsIpi::getValuesAsIpAddress(const char *propertyName) {
    return getValuesAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<IpAddress>
IpIntelligence::ResultsIpi::getValuesAsIpAddress(const string &propertyName) {
    return getValuesAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<IpAddress>
IpIntelligence::ResultsIpi::getValuesAsIpAddress(const string *propertyName) {
    return getValuesAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<string>
IpIntelligence::ResultsIpi::getValueAsString(int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    ProfilePercentage *valuesItems;
    Common::Value<string> result;
    fiftyoneDegreesPropertyValueType valueType;

    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    if (EXCEPTION_OKAY
        && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE
        && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_RANGE) {
        // Get a pointer to the first value item for the property.
        valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
        EXCEPTION_THROW;

        if (valuesItems == NULL) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }

        // Add the values in their original form to the result.
        if (results->values.count > 0) {
            stringstream stream;
            for (i = 0; i < results->values.count; i++) {
                if (i != 0) {
                    stream << "|";
                }
                stream << STRING((String *)valuesItems[i].item.data.ptr);
                stream << ":";
                stream << valuesItems[i].percentage;
            }
            result.setValue(stream.str());
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<bool>>> result;
    vector<WeightedValue<bool>> values;
    fiftyoneDegreesPropertyValueType valueType;

    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    if (EXCEPTION_OKAY
        && valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_BOOLEAN) {
        // Get a pointer to the first value item for the property.
        valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
        EXCEPTION_THROW;

        if (valuesItems == NULL) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }

        // Set enough space in the vector for all the strings that will be
        // inserted.
        values.reserve(results->values.count);

        // Add the values in their original form to the result.
        for (i = 0; i < results->values.count; i++) {
            WeightedValue<bool> weightedBool;
            weightedBool.setValue(strcmp(STRING((String *)valuesItems[i].item.data.ptr), "True") == 0 ? true : false);
            weightedBool.setWeight(valuesItems[i].percentage);
            values.push_back(weightedBool);
        }
        result.setValue(values);
    }
    return result;
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    const char *propertyName) {
  return getValuesAsWeightedBoolList(
      ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    const string &propertyName) {
  return getValuesAsWeightedBoolList(
      ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<bool>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedBoolList(
    const string *propertyName) {
  return getValuesAsWeightedBoolList(
      ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<string>>> result;
    vector<WeightedValue<string>> values;
    fiftyoneDegreesPropertyValueType valueType;

    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    if (EXCEPTION_OKAY
        && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE
        && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_RANGE) {
        // Get a pointer to the first value item for the property.
        valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
        EXCEPTION_THROW;

        if (valuesItems == NULL) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }

        // Set enough space in the vector for all the strings that will be
        // inserted.
        values.reserve(results->values.count);

        // Add the values in their original form to the result.
        for (i = 0; i < results->values.count; i++) {
            WeightedValue<string> weightedString;
            weightedString.setValue(
                string(STRING((String *)valuesItems[i].item.data.ptr)));
            weightedString.setWeight(valuesItems[i].percentage);
            values.push_back(weightedString);
        }
        result.setValue(values);
    }
    return result;
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const char *propertyName) {
    return getValuesAsWeightedStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const string &propertyName) {
    return getValuesAsWeightedStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedStringList(
    const string *propertyName) {
    return getValuesAsWeightedStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<double>>> result;
    vector<WeightedValue<double>> values;
    fiftyoneDegreesPropertyValueType valueType;

    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    if (EXCEPTION_OKAY
        && valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_DOUBLE) {
        // Get a pointer to the first value item for the property.
        valuesItems = ResultsIpiGetValues(results, requiredPropertyIndex, exception);
        EXCEPTION_THROW;

        if (valuesItems == NULL) {
            // No pointer to values was returned.
            throw NoValuesAvailableException();
        }

        // Set enough space in the vector for all the strings that will be
        // inserted.
        values.reserve(results->values.count);

        // Add the values in their original form to the result.
        for (i = 0; i < results->values.count; i++) {
            WeightedValue<double> weightedDouble;
            weightedDouble.setValue(
                strtod((STRING((String *)valuesItems[i].item.data.ptr)), nullptr));
            weightedDouble.setWeight(valuesItems[i].percentage);
            values.push_back(weightedDouble);
        }
        result.setValue(values);
    }
    return result;
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    const char *propertyName) {
    return getValuesAsWeightedDoubleList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    const string &propertyName) {
    return getValuesAsWeightedDoubleList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    const string *propertyName) {
    return getValuesAsWeightedDoubleList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

bool IpIntelligence::ResultsIpi::hasValuesInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	bool hasValues = fiftyoneDegreesResultsIpiGetHasValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;
	return hasValues;
}

const char* IpIntelligence::ResultsIpi::getNoValueMessageInternal(
	fiftyoneDegreesResultsNoValueReason reason) {
	return fiftyoneDegreesResultsIpiGetNoValueReasonMessage(reason);
}

fiftyoneDegreesResultsNoValueReason
IpIntelligence::ResultsIpi::getNoValueReasonInternal(
	int requiredPropertyIndex) {
	EXCEPTION_CREATE;
	fiftyoneDegreesResultsNoValueReason reason =
		fiftyoneDegreesResultsIpiGetNoValueReason(
			results,
			requiredPropertyIndex,
			exception);
	EXCEPTION_THROW;
	return reason;
}

string IpIntelligence::ResultsIpi::getNetworkId(
	uint32_t resultIndex) {
	EXCEPTION_CREATE;
	char networkId[1024] = "";
	if (resultIndex < results->count) {
		IpiGetNetworkIdFromResult(
			results,
			&results->items[resultIndex],
			networkId,
			sizeof(networkId),
			exception);
		EXCEPTION_THROW;
	}
	return string(networkId);
}

string IpIntelligence::ResultsIpi::getNetworkId() {
	EXCEPTION_CREATE;
	char networkId[1024] = "";
	IpiGetNetworkIdFromResults(
		results,
		networkId,
		sizeof(networkId),
		exception);
	EXCEPTION_THROW;
	return string(networkId);
}