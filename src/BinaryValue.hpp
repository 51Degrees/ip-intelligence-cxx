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

#ifndef FIFTYONE_DEGREES_BINARY_VALUE_HPP
#define FIFTYONE_DEGREES_BINARY_VALUE_HPP

#include "common-cxx/data.h"

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Binary value class, used to hold the raw data
		 * This is mainly used in ValueMetaDataKey but
		 * not limited to it.
		 */
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