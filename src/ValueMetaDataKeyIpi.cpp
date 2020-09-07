/* *********************************************************************
 * This Original Work is copyright of 51 Degrees Mobile Experts Limited.
 * Copyright 2020 51 Degrees Mobile Experts Limited, 5 Charlotte Close,
 * Caversham, Reading, Berkshire, United Kingdom RG4 7BY.
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
#include "ValueMetaDataKeyIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

ValueMetaDataKeyIpi::ValueMetaDataKeyIpi() {
	this->propertyName = "invalid";
}

ValueMetaDataKeyIpi::ValueMetaDataKeyIpi(
	string propertyName,
	BinaryValue binValue) {
	this->propertyName = propertyName;
	this->value = binValue;
}

const BinaryValue ValueMetaDataKeyIpi::getValue() const {
	return this->value;
}

const string ValueMetaDataKeyIpi::getValueName() const {
	string result;
	const String *stringValue = (String *)this->value.getValue();
	if (stringValue->value == 1) {
		// The data in the string item is a float pair
		// Thus return the value in string
		stringstream stream;
		stream << FLOAT_TO_NATIVE(*(Float *)(&stringValue->value + 1))
			<< "," << FLOAT_TO_NATIVE(*((Float *)(&stringValue->value + 1) + 1));
		result = stream.str();
	}
	else {
		result.append(&stringValue->value);
	}
	return result;
}

const bool 
ValueMetaDataKeyIpi::operator< (ValueMetaDataKeyIpi other) const {
	if (other.getPropertyName() == getPropertyName()) {
		// Keys are the same property, so compare the secondary key (value)
		return other.getValue() < getValue();
	}
	return other.getPropertyName() < getPropertyName();
}

const bool 
ValueMetaDataKeyIpi::operator== (ValueMetaDataKeyIpi other) const {
	return (other.getPropertyName() == getPropertyName()) &&
		(other.getValue() == getValue());
}