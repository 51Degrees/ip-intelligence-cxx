#include "algorithm"
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