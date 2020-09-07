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

#ifndef FIFTYONE_DEGREES_RESULTS_IPI_HPP
#define FIFTYONE_DEGREES_RESULTS_IPI_HPP

#include <sstream>
#include <vector>
#include "common-cxx/ResultsBase.hpp"
#include "WeightedValue.hpp"
#include "IpAddress.hpp"
#include "ipi.h"

using namespace FiftyoneDegrees::Common;

class EngineIpIntelligenceTests;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Encapsulates the results of an IP Intelligence engine's
		 * processing. The class is constructed using an instance of a C
		 * #fiftyoneDegreesResultsIpi structure which are then
		 * referenced to return associated values and metrics.
		 *
		 * The key used to get the value for a property can be either the
		 * name of the property, or the index of the property in the
		 * required properties structure.
		 *
		 * Results instances should only be created by an Engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::IpIntelligence;
		 * ResultsIpi *results;
		 *
		 * // Get property value as a coordinate for property "RangeStart"
		 * Value<pair<float, float>> coordinate =
		 *	getValueAsCoordinate("RangeStart");
		 *
		 * // Get the network id for the IP range
		 * string networkId = results->getNetworkId();
		 *
		 * // Delete the results
		 * delete results;
		 * ```
		 */
		class ResultsIpi : public ResultsBase {
			friend class ::EngineIpIntelligenceTests;
		public:
			/**
			 * @name Constructors and Destructors
			 * @{
			 */

			 /**
			  * @copydoc Common::ResultsBase::ResultsBase
			  */
			ResultsIpi(
				fiftyoneDegreesResultsIpi *results,
				shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * Release the reference to the underlying results and
			 * and associated data set.
			 */
			virtual ~ResultsIpi();

			/**
			 * @}
			 * @name Value Getters
			 * @{
			 */

			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(const char *propertyName);

			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(const string &propertyName);

			/**
			 * Get a vector with all weighted boolean representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted boolean values for the property
			 */
			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(int requiredPropertyIndex);
			
			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
                        getValuesAsWeightedStringList(const char *propertyName);

			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
						getValuesAsWeightedStringList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
                        getValuesAsWeightedStringList(const string &propertyName);

			/**
			 * Get a vector with all weighted string representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted string values for the property
			 */
			Common::Value<vector<WeightedValue<string>>>
                        getValuesAsWeightedStringList(int requiredPropertyIndex);
			
			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
						getValuesAsWeightedIntegerList(const char *propertyName);

			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
                        getValuesAsWeightedIntegerList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
						getValuesAsWeightedIntegerList(const string &propertyName);

			/**
			 * Get a vector with all weighted integer representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted integer values for the property
			 */
			Common::Value<vector<WeightedValue<int>>>
                        getValuesAsWeightedIntegerList(int requiredPropertyIndex);

			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
						getValuesAsWeightedDoubleList(const char *propertyName);

			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
                        getValuesAsWeightedDoubleList(const string *propertyName);
			
			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property name. If the name
			 * is not valid an empty vector is returned.
			 * @param propertyName pointer to a string containing the property
			 * name
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
						getValuesAsWeightedDoubleList(const string &propertyName);

			/**
			 * Get a vector with all weighted double representations of the 
			 * values associated with the required property index. If the index
			 * is not valid an empty vector is returned.
			 * @param requiredPropertyIndex in the required properties
			 * @return a vector of weighted double values for the property
			 */
			Common::Value<vector<WeightedValue<double>>>
                        getValuesAsWeightedDoubleList(int requiredPropertyIndex);
			
			/**
			 * Get a float pair representation of the value associated with the
			 * required property name. If the property name is not valid then
			 * hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return a float pair representation of the value for the property
			 */
			Common::Value<pair<float, float>>
						getValueAsCoordinate(const char *propertyName);

			/**
			 * Get a float pair representation of the value associated with the
			 * required property name. If the property name is not valid then
			 * hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return a float pair representation of the value for the property
			 */
			Common::Value<pair<float, float>>
						getValueAsCoordinate(const string *propertyName);

			/**
			 * Get a float pair representation of the value associated with the
			 * required property name. If the property name is not valid then
			 * hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return a float pair representation of the value for the property
			 */
			Common::Value<pair<float, float>>
						getValueAsCoordinate(const string &propertyName);
			
			/**
			 * Get a float pair representation of the value associated with the
			 * required property index. If the index is not valid then
			 * hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return a float pair representation of the value for the property
			 */
			Common::Value<pair<float, float>>
						getValueAsCoordinate(int requiredPropertyIndex);

			/**
			 * Get an IpAddress instace representation of the value associated 
			 * with the required property name. If the property name is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpAddress> getValueAsIpAddress(const char *propertyName);

			/**
			 * Get an IpAddress instace representation of the value associated 
			 * with the required property name. If the property name is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpAddress> getValueAsIpAddress(const string &propertyName);

			/**
			 * Get an IpAddress instace representation of the value associated 
			 * with the required property name. If the property name is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpAddress> getValueAsIpAddress(const string *propertyName);
			
			/**
			 * Get an IpAddress instace representation of the value associated 
			 * with the required property index. If the index is not valid
			 * then hasValue returns false with NoValueReason and its message.
			 * @param propertyName string containing the property name
			 * @return an IpAddress representation of the value for the property
			 */
			Common::Value<IpAddress> getValueAsIpAddress(int requiredPropertyIndex);

			/**
			 * Override the get boolean representation of the value associated
			 * with the required property name.
			 * This now always returns a no value. The reason is always too many results.
			 */
			Common::Value<bool> getValueAsBool(int requiredPropertyIndex) override;

			/**
			 * Override the get integer representation of the value associated
			 * with the required property name.
			 * This now always returns a no value. The reason is always too many results.
			 */
			Common::Value<int> getValueAsInteger(int requiredPropertyIndex) override;

			/**
			 * Override the get double representation of the value associated
			 * with the required property name.
			 * This now always returns a no value. The reason is always too many results.
			 */
			Common::Value<double> getValueAsDouble(int requiredPropertyIndex) override;

			/**
			 * @}
			 * @name Metric Getters
			 * @{
			 */

			/**
			 * Returns the unique network id if the Id property was included
			 * in the required list of properties when the Provider was
			 * constructed.
			 * For IP Intelligence there should be maximum of one result
			 * returned so the output of this function should be the same
			 * as getNetworkId().
			 *
			 * @param resultIndex index of the individual entry in the
			 * results
			 * @return profile ids and their percentage with ':' and '|'
			 * e.g. 'ProfileId1:Percentage1|ProfileId2:Percentage2'
			 */
			string getNetworkId(uint32_t resultIndex);

			/**
			 * Returns the unique network id if the Id property was included
			 * in the required list of properties when the Provider was
			 * constructed.
			 * @return profile ids and their percentage with ':' and '|'
			 * e.g. 'ProfileId1:Percentage1|ProfileId2:Percentage2'
			 */
			string getNetworkId();

			/**
			 * @}
			 */

		protected:
			void getValuesInternal(
				int requiredPropertyIndex,
				vector<string> &values);

			bool hasValuesInternal(int requiredPropertyIndex);

			const char* getNoValueMessageInternal(
				fiftyoneDegreesResultsNoValueReason reason);

			fiftyoneDegreesResultsNoValueReason getNoValueReasonInternal(
				int requiredPropertyIndex);

		private:
			/**
			 * Utility function to check the property value type
			 * Consumers of this function should always checked for
			 * exception status before using the returned value.
			 * @param requiredPropertyIndex index in the required
			 * properties list
			 * @param exception object which is used when an exception
			 * occurs
			 * @return the value type of the property
			 */
			fiftyoneDegreesPropertyValueType getPropertyValueType(
				int requiredPropertyIndex,
				fiftyoneDegreesException *exception);

			fiftyoneDegreesResultsIpi *results;
		};
	}
}

#endif
