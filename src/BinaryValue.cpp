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

#include <algorithm>
#include <cstring>
#include "BinaryValue.hpp"

using namespace std;
using namespace FiftyoneDegrees::IpIntelligence;

BinaryValue::BinaryValue()
	: BinaryValue::BinaryValue(nullptr, 0) {}

BinaryValue::BinaryValue(const byte *value, uint32_t size) {
	this->value = new byte[size];
	memcpy(this->value, value, size);
	this->size = size;
}

BinaryValue::BinaryValue(const BinaryValue& target) 
	: BinaryValue::BinaryValue(target.getValue(), target.getSize()) { }

BinaryValue::~BinaryValue() {
	if (this->value != nullptr) {
		delete[] this->value;
	}
}

const byte* BinaryValue::getValue() const {
	return this->value;
}

const uint32_t BinaryValue::getSize() const {
	return this->size;
}

BinaryValue& BinaryValue::operator= (const BinaryValue& other) {
	// Check for self-assignment
	if (this != &other) {
		// Create a new byte array
		byte *newValue = new byte[other.getSize()];
		copy(other.getValue(), other.getValue() + other.getSize(), newValue);

		// Delete the existing one
		if (this->value != nullptr) {
			delete[] this->value;
		}

		// Assigne the new byte array
		this->value = newValue;
		this->size = other.getSize();
	}
	return *this;
}

const bool
BinaryValue::operator< (BinaryValue other) const {
	// Get the comparison size
	uint32_t compareSize = this->size <= other.getSize() ? this->size : other.getSize();
	// Get the tmp result from memcmp
	int tmpResult = memcmp(this->value, other.getValue(), compareSize);

	return (tmpResult < 0 || (tmpResult == 0 && this->size < other.getSize()));
}

const bool
BinaryValue::operator== (BinaryValue other) const {
	// Get the comparison size
	uint32_t compareSize = this->size <= other.getSize() ? this->size : other.getSize();
	// Get the tmp result from memcmp
	int tmpResult = memcmp(this->value, other.getValue(), compareSize);

	return (tmpResult == 0 && this->size == other.getSize());
}
