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

#include <sstream>
#include "ValueMetaDataBuilderIpi.hpp"
#include "common-cxx/Exceptions.hpp"
#include "fiftyone.h"
#include "common-cxx/wkbtot.h"
#include "common-cxx/wkbtot.hpp"
#include "constantsIpi.h"

using namespace FiftyoneDegrees::IpIntelligence;

/* Maximum buffer length to hold an IP address string */
#define IP_ADDRESS_STRING_MAX_LENGTH 50
/* Coordinate floating point precision */
#define COORDINATE_PRECISION 7

string ValueMetaDataBuilderIpi::getDynamicString(
	fiftyoneDegreesCollection *stringsCollection,
	uint32_t offset) {
	EXCEPTION_CREATE;
	string result;
	fiftyoneDegreesCollectionItem item;
	fiftyoneDegreesString *str;
	fiftyoneDegreesDataReset(&item.data);
	str = fiftyoneDegreesStringGet(
		stringsCollection,
		offset,
		&item,
		exception);
	EXCEPTION_THROW;

	stringstream stream;
	if (str != nullptr) {
		switch(str->value) {
		case FIFTYONE_DEGREES_STRING_COORDINATE:
			{
				Coordinate coordinate = IpiGetCoordinate(&item, exception);
				EXCEPTION_THROW;
				stream.precision(COORDINATE_PRECISION);
				stream << fixed << coordinate.lat;
				stream << ",";
				stream << fixed << coordinate.lon;
			}
			break;
		case FIFTYONE_DEGREES_STRING_IP_ADDRESS:
			{
				char buffer[IP_ADDRESS_STRING_MAX_LENGTH];
				memset(buffer, 0, IP_ADDRESS_STRING_MAX_LENGTH);

				// Get the actual address size
				uint16_t addressSize = str->size - 1;
				// Get the type of the IP address
				fiftyoneDegreesIpType type =
					addressSize == FIFTYONE_DEGREES_IPV4_LENGTH ?
					IP_TYPE_IPV4 :
					IP_TYPE_IPV6;
				// Get the string representation of the IP address
				IpiGetIpAddressAsString(
					&item, 
					type, 
					buffer, 
					IP_ADDRESS_STRING_MAX_LENGTH, 
					exception);
				EXCEPTION_THROW;

				stream << buffer;
			}
			break;
			case FIFTYONE_DEGREES_PROPERTY_VALUE_TYPE_WKB: {
				writeWkbStringToStringStream(
					str, stream, DefaultWktDecimalPlaces, exception);
				break;
			}
		default:
			stream << &str->value;
			break;
		}
		result.append(stream.str());
	}
	FIFTYONE_DEGREES_COLLECTION_RELEASE(
		stringsCollection,
		&item);
	return result;
}



ValueMetaData* ValueMetaDataBuilderIpi::build(
	fiftyoneDegreesDataSetIpi *dataSet,
	fiftyoneDegreesValue *value) {
	EXCEPTION_CREATE;
	ValueMetaData *result = nullptr;
	Item item;
	Property *property;
	DataReset(&item.data);
	property = PropertyGet(
		dataSet->properties,
		value->propertyIndex, 
		&item,
		exception);
	EXCEPTION_THROW;
	if (property != nullptr) {
		result = new ValueMetaData(
			ValueMetaDataKey(
				getString(dataSet->strings, property->nameOffset),
				getDynamicString(dataSet->strings, value->nameOffset)),
			value->descriptionOffset == -1 ?
			"" :
			getString(dataSet->strings, value->descriptionOffset),
			value->urlOffset == -1 ?
			"" :
			getString(dataSet->strings, value->urlOffset));
		COLLECTION_RELEASE(dataSet->properties, &item);
	}
	return result;
}
