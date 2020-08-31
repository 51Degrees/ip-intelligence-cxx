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

/* 
 * This is used to hold the string value of the item
 * Maximum size of an IP address string is 39
 * Maximum single precision floating point is ~ 3.4 * 10^38
 * 128 should be adequate to hold the string value
 * for a pair of coordinate:percentage
 * or ipaddress:percentage
 */
#define MAX_PROFILE_PERCENTAGE_STRING_LENGTH 128

/*
 * This will returns the profile percentages results
 * for a required property index in string form.
 * @param requiredPropertyIndex the required property index
 * @param values the array which will hold the returned value string
 */
void 
IpIntelligence::ResultsIpi::getValuesInternal(int requiredPropertyIndex, vector<string> &values) {
    EXCEPTION_CREATE;
	uint32_t i;
	ProfilePercentage *valuesItems;
	fiftyoneDegreesPropertyValueType valueType;

    // We should not have any undefined data type in the data file
    // If there is, the data file is not good to use so terminates.
    valueType = getPropertyValueType(requiredPropertyIndex, exception);
    EXCEPTION_THROW;

	// Get a pointer to the first value item for the property.
	valuesItems = ResultsIpiGetValues(
		results,
		requiredPropertyIndex,
		exception);
	EXCEPTION_THROW;

	if (valuesItems == NULL) {
		// No pointer to values was returned. 
		throw NoValuesAvailableException();
	}

	// Set enough space in the vector for all the strings that will be 
	// inserted.
	values.reserve(results->values.count);

    stringstream stream;
    char buffer[MAX_PROFILE_PERCENTAGE_STRING_LENGTH];
    fiftyoneDegreesCoordinate coordinate;
	// Add the values in their original form to the result.
	for (i = 0; i < results->values.count; i++) {
        // Clear the string stream
        stream.str("");
        // Check value type to appropriately retrieve the string value of
        // the item
        switch(valueType) {
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE:
            coordinate = IpiGetCoordinate(&valuesItems[i].item, exception);
            if (EXCEPTION_OKAY) {
                stream << FLOAT_TO_NATIVE(coordinate.lat);
                stream << ",";
                stream << FLOAT_TO_NATIVE(coordinate.lon);
            }
            break;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_RANGE:
            IpiGetIpRangeAsString(
                &valuesItems[i].item,
                results->items[0].type,
                buffer,
                MAX_PROFILE_PERCENTAGE_STRING_LENGTH,
                exception);
            if (EXCEPTION_OKAY) {
                stream << buffer;
            }
            break;
        default:
            stream << STRING((String*)valuesItems[i].item.data.ptr);
            break;
        }
        if (EXCEPTION_OKAY) {
            stream << ":";
            stream << FLOAT_TO_NATIVE(valuesItems[i].percentage);
            values.push_back(stream.str());
        }
	}
    // The value format in the data file should never be
    // in incorrect. If it happens the data file or
    // the memory is corrupted and we should terminate
    EXCEPTION_THROW
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

/*
 * Override the default getValueAsBool function.
 * Since for each property, we will always get a list of profile percentage pairs,
 * it is not appropriate to process the value as boolean here.
 * Thus always return #FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES
 */
Common::Value<bool> 
IpIntelligence::ResultsIpi::getValueAsBool(int requiredPropertyIndex) {
    Common::Value<bool> result;
    result.setNoValueReason(
        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
        nullptr);
    return result;
}

/*
 * Override the default getValueAsInteger function.
 * Since for each property, we will always get a list of profile percentage pairs,
 * it is not appropriate to process the value as integer here.
 * Thus always return #FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES
 */
Common::Value<int>
IpIntelligence::ResultsIpi::getValueAsInteger(int requiredPropertyIndex) {
    Common::Value<int> result;
    result.setNoValueReason(
        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
        nullptr);
    return result;
}

/*
 * Override the default getValueAsDouble function.
 * Since for each property, we will always get a list of profile percentage pairs,
 * it is not appropriate to process the value as double here.
 * Thus always return #FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES
 */
Common::Value<double> 
IpIntelligence::ResultsIpi::getValueAsDouble(int requiredPropertyIndex) {
    Common::Value<double> result;
    result.setNoValueReason(
        FIFTYONE_DEGREES_RESULTS_NO_VALUE_REASON_TOO_MANY_VALUES,
        nullptr);
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