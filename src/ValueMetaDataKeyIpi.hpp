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

#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_KEY_IPI_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_KEY_IPI_HPP

#include "common-cxx/ValueMetaData.hpp"
#include "BinaryValue.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		class ValueMetaDataKeyIpi : public ValueMetaDataKey {
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			/**
			 * Default constructor. This should not be used externally as it 
			 * returns an invalid instance.
			 */
			ValueMetaDataKeyIpi();

			/**
			 * Construct a new instance of ValueMetaDataKeyIpi from the unique
			 * combination of property, value and type.
			 * @param propertyName the name of the property the value relates
			 * to
			 * @param value the value
			 */
			ValueMetaDataKeyIpi(
				string propertyName, 
				BinaryValue value);

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the value which is being keyed.
			 * @return the value
			 */
			const BinaryValue getValue() const;

			/**
			 * Get the name of the value which is being keyed.
			 * @return name of the value
			 */
			const string getValueName() const override;

			/**
			 * @}
			 * @name Operators
			 * @{
			 */

			/**
			 * Override the less than operator so the unique key can be used to
			 * order lists.
			 * @param other the other key to compare
			 * @return true if this key comes before the other key
			 */
			const bool operator< (ValueMetaDataKeyIpi other) const;

			/**
			 * Override the equality operator so the unique key can be found in
			 * a generic collection.
			 * @param other the other key to compare
			 * @return true if both keys are equal
			 */
			const bool operator== (ValueMetaDataKeyIpi other) const;

			/**
			 * @}
			 */

		private:
			/** The value as a BinaryValue instance*/
			BinaryValue value;
		};
	}
}

#endif