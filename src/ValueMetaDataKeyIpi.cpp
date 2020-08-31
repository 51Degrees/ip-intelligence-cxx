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