#ifndef FIFTYONE_DEGREES_BINARY_VALUE_HPP
#define FIFTYONE_DEGREES_BINARY_VALUE_HPP

#include "common-cxx/data.h"

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		class BinaryValue {
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			/**
			 * Default constructor
			 */
			BinaryValue();


			/**
			 * Construct a new instance of BinaryValue from
			 * a value and its size
			 * @param value the value
			 * @param size the size of the value
			 */
			BinaryValue(const byte *value, uint32_t size);

			/**
			 * A copy constructor
			 * Construct a new instance from another instance
			 * @param target the target value
			 */
			BinaryValue(const BinaryValue& target);

			/**
			 * Release the byte array allocated when create
			 * a the new instance
			 */
			~BinaryValue();

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the value
			 * @return the value
			 */
			const byte *getValue() const;

			/**
			 * Get the size of the value
			 * @return the size of the value
			 */
			const uint32_t getSize() const;

			/**
			 * @}
			 * @name Opeartors
			 * @{
			 */

			/**
			 * Override the assignment operator so the internal byte array can be
			 * allocated and copied properly
			 * @param other the other BinaryValue instance
			 * @return a copy of the other instance
			 */
			BinaryValue& operator=(const BinaryValue& other);

			/**
			 * Override the less than operator 
			 * @param other the other value to compare
			 * @return true if this value is smaller than the other
			 */
			const bool operator< (BinaryValue other) const;

			/**
			 * Override the equality operator
			 * @param other the other value to compare
			 * @return true if both values are equal
			 */
			const bool operator== (BinaryValue other) const;

			/**
			 * @}
			 */

		private:
			/** The value in byte array */
			byte *value;
			/** The size of the value */
			uint32_t size;
		};
	}
}

#endif