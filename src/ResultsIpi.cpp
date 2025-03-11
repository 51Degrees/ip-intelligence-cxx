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

#include "ResultsIpi.hpp"
#include "fiftyone.h"
#include "common-cxx/wkbtot.hpp"
#include "constantsIpi.h"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;

#define RESULT(r,i) ((ResultIpi*)r->b.items + i)

IpIntelligence::ResultsIpi::ResultsIpi(
	fiftyoneDegreesResultsIpi *results,
	shared_ptr<fiftyoneDegreesResourceManager> manager)
	: ResultsBase(&results->b, manager) {
	this->results = results;
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
	const ProfilePercentage *valuesItems;
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
	// Add the values in their original form to the result.
	for (i = 0; i < results->values.count; i++) {
        // Clear the string stream
        stream.str("");
        // Check value type to appropriately retrieve the string value of
        // the item
        switch(valueType) {
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS:
            IpiGetIpAddressAsString(
                &valuesItems[i].item,
                results->items[0].type,
                buffer,
                MAX_PROFILE_PERCENTAGE_STRING_LENGTH,
                exception);
            if (EXCEPTION_OKAY) {
                stream << buffer;
            }
            break;
        case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WKB:
            writeWkbStringToStringStream(
                (const VarLengthByteArray *)valuesItems[i].item.data.ptr,
                stream, DefaultWktDecimalPlaces, exception);
            break;
        default:
            stream << STRING((String*)valuesItems[i].item.data.ptr);
            break;
        }
        if (EXCEPTION_OKAY) {
            stream << ":";
            stream << (float)valuesItems[i].rawWeighting / 65535.f;
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
    // Default to string type. Consumers of
    // this function should always check for exception status
    fiftyoneDegreesPropertyValueType valueType
        = FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_STRING; // overwritten later
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

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    const ProfilePercentage *valuesItems;
    Common::Value<IpAddress> result;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        fiftyoneDegreesPropertyValueType valueType = 
            getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
            if (valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS) {
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
                    const unsigned char * const ipAddressBytes =
                        &((const VarLengthByteArray *)(valuesItems->item.data.ptr))->firstByte;
                    if (ipAddressBytes != NULL) {
                        ipAddress = IpAddress(
                            ipAddressBytes, results->items[0].type);
                    }
                    result.setValue(ipAddress);
                }
            }
            else {
                // Default to the smallest IP address
                if (results->items[0].type == IP_TYPE_IPV4) {
                    result.setValue(IpAddress("0.0.0.0"));
                }
                else {
                    result.setValue(IpAddress("0000:0000:0000:0000:0000:0000:0000:0000"));
                }
            }
        }
    }
    return result;
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(const char *propertyName) {
    return getValueAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(const string &propertyName) {
    return getValueAsIpAddress(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<IpIntelligence::IpAddress>
IpIntelligence::ResultsIpi::getValueAsIpAddress(const string *propertyName) {
    return getValueAsIpAddress(
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
#ifdef _MSC_VER
    (void)requiredPropertyIndex; // suppress C4100 "unused formal parameter"
#endif
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
#ifdef _MSC_VER
    (void)requiredPropertyIndex; // suppress C4100 "unused formal parameter"
#endif
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
#ifdef _MSC_VER
    (void)requiredPropertyIndex; // suppress C4100 "unused formal parameter"
#endif
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
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<bool>>> result;
    vector<WeightedValue<bool>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        fiftyoneDegreesPropertyValueType valueType = 
            getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
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
                if (valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE
                    && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS) {
                    const char * const theString = STRING((String *)valuesItems[i].item.data.ptr);
                    weightedBool.setValue(strcmp(theString, "True") == 0);
                }
                else {
                    // Coordinate and IP range cannot be converted to boolean so default to false
                    weightedBool.setValue(false);
                }
                weightedBool.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedBool);
            }
            result.setValue(values);
        }
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
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<string>>> result;
    vector<WeightedValue<string>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        fiftyoneDegreesPropertyValueType valueType = 
            getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
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

            stringstream stream;
            // Add the values in their original form to the result.
            for (i = 0; i < results->values.count; i++) {
                WeightedValue<string> weightedString;
                // Clear stream before the construction
                stream.str("");
                switch(valueType) {
                case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS: {
                    char buffer[MAX_PROFILE_PERCENTAGE_STRING_LENGTH];
                    IpiGetIpAddressAsString(
                        &valuesItems[i].item,
                        results->items[0].type,
                        buffer,
                        MAX_PROFILE_PERCENTAGE_STRING_LENGTH,
                        exception);
                    if (EXCEPTION_OKAY) {
                        stream << buffer;
                    }
                    break;
                }
                case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WKB:
                    writeWkbStringToStringStream(
                        (const VarLengthByteArray *)valuesItems[i].item.data.ptr,
                        stream, DefaultWktDecimalPlaces, exception);
                    break;
                default:
                    stream << STRING((String*)valuesItems[i].item.data.ptr);
                    break;
                }
                weightedString.setValue(stream.str());
                weightedString.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedString);
            }
            result.setValue(values);
        }
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

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
    const int requiredPropertyIndex,
    const byte decimalPlaces) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<string>>> result;
    vector<WeightedValue<string>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        fiftyoneDegreesPropertyValueType valueType =
            getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
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

            stringstream stream;
            // Add the values in their original form to the result.
            for (i = 0; i < results->values.count; i++) {
                WeightedValue<string> weightedString;
                // Clear stream before the construction
                stream.str("");
                if (valueType == FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WKB) {
                    writeWkbStringToStringStream(
                        (const VarLengthByteArray *)valuesItems[i].item.data.ptr,
                        stream, decimalPlaces, exception);
                } else {
                    stream << STRING((String*)valuesItems[i].item.data.ptr);
                }
                weightedString.setValue(stream.str());
                weightedString.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedString);
            }
            result.setValue(values);
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
const char *propertyName,
byte decimalPlaces) {
    return getValuesAsWeightedWKTStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName),
        decimalPlaces);
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
const string &propertyName,
byte decimalPlaces) {
    return getValuesAsWeightedWKTStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()),
        decimalPlaces);
}

Common::Value<vector<WeightedValue<string>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedWKTStringList(
const string *propertyName,
byte decimalPlaces) {
    return getValuesAsWeightedWKTStringList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()),
        decimalPlaces);
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<int>>> result;
    vector<WeightedValue<int>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        fiftyoneDegreesPropertyValueType valueType = 
            getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
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
                WeightedValue<int> weightedInteger;
                if (valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE
                    && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS) {
                    const char * const theString = STRING((String *)valuesItems[i].item.data.ptr);
                    weightedInteger.setValue(atoi(theString));
                }
                else {
                    // Coordinate and IP address cannot be converted to int
                    // so default to 0
                    weightedInteger.setValue(0);
                }
                weightedInteger.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedInteger);
            }
            result.setValue(values);
        }
    }
    return result;
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    const char *propertyName) {
    return getValuesAsWeightedIntegerList(
        ResultsBase::getRequiredPropertyIndex(propertyName));
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    const string &propertyName) {
    return getValuesAsWeightedIntegerList(
        ResultsBase::getRequiredPropertyIndex(propertyName.c_str()));
}

Common::Value<vector<WeightedValue<int>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedIntegerList(
    const string *propertyName) {
    return getValuesAsWeightedIntegerList(
        ResultsBase::getRequiredPropertyIndex(propertyName->c_str()));
}

Common::Value<vector<WeightedValue<double>>>
IpIntelligence::ResultsIpi::getValuesAsWeightedDoubleList(
    int requiredPropertyIndex) {
    EXCEPTION_CREATE;
    uint32_t i;
    const ProfilePercentage *valuesItems;
    Common::Value<vector<WeightedValue<double>>> result;
    vector<WeightedValue<double>> values;
    if (!(hasValuesInternal(requiredPropertyIndex)))
    {
        fiftyoneDegreesResultsNoValueReason reason =
			getNoValueReasonInternal(requiredPropertyIndex);
		result.setNoValueReason(
			reason,
			getNoValueMessageInternal(reason));
    }
    else {
        fiftyoneDegreesPropertyValueType valueType = 
            getPropertyValueType(requiredPropertyIndex, exception);
        if (EXCEPTION_OKAY) {
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
                if (valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_COORDINATE
                    && valueType != FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_IP_ADDRESS) {
                    const char * const theString = STRING((String *)valuesItems[i].item.data.ptr);
                    weightedDouble.setValue(strtod(theString, nullptr));
                }
                else {
                    // Coordinate and IP address cannot be converted to double
                    // so default to 0
                    weightedDouble.setValue(0);
                }
                weightedDouble.setRawWeight(valuesItems[i].rawWeighting);
                values.push_back(weightedDouble);
            }
            result.setValue(values);
        }
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
