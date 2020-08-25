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
		 * Encapsulates the results of a Hash device detection engine's
		 * processing. The class is constructed using an instance of a C
		 * #fiftyoneDegreesResultsHash structure which are then
		 * referenced to return associated values and metrics.
		 *
		 * Additional get methods are included on top of the device
		 * detection methods to return Hash specific metrics.
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
		 * using namespace FiftyoneDegrees::DeviceDetection::Hash;
		 * ResultsHash *results;
		 *
		 * // Get the maximum drift used to arrive at the result
		 * int drift = results->getDrift();
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
			  * @copydoc ResultsDeviceDetection::ResultsDeviceDetection
			  */
			ResultsIpi(
				fiftyoneDegreesResultsIpi *results,
				shared_ptr<fiftyoneDegreesResourceManager> manager);

			/**
			 * Release the reference to the underlying results and
			 * and associated data set.
			 */
			virtual ~ResultsIpi();

			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(const char *propertyName);

			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(const string *propertyName);
			
			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(const string &propertyName);

			Common::Value<vector<WeightedValue<bool>>>
                        getValuesAsWeightedBoolList(int requiredPropertyIndex);
			
			Common::Value<vector<WeightedValue<string>>>
                        getValuesAsWeightedStringList(const char *propertyName);

			Common::Value<vector<WeightedValue<string>>>
						getValuesAsWeightedStringList(const string *propertyName);
			
			Common::Value<vector<WeightedValue<string>>>
                        getValuesAsWeightedStringList(int requiredPropertyIndex);

			Common::Value<vector<WeightedValue<string>>>
                        getValuesAsWeightedStringList(const string &propertyName);
			
			Common::Value<vector<WeightedValue<double>>>
						getValuesAsWeightedDoubleList(const char *propertyName);

			Common::Value<vector<WeightedValue<double>>>
                        getValuesAsWeightedDoubleList(const string *propertyName);
			
			Common::Value<vector<WeightedValue<double>>>
						getValuesAsWeightedDoubleList(const string &propertyName);

			Common::Value<vector<WeightedValue<double>>>
                        getValuesAsWeightedDoubleList(int requiredPropertyIndex);
			
			Common::Value<pair<float, float>>
						getValuesAsCoordinate(const char *propertyName);

			Common::Value<pair<float, float>>
						getValuesAsCoordinate(const string *propertyName);

			Common::Value<pair<float, float>>
						getValuesAsCoordinate(const string &propertyName);
			
			Common::Value<pair<float, float>>
						getValuesAsCoordinate(int requiredPropertyIndex);

			Common::Value<IpAddress> getValuesAsIpAddress(const char *propertyName);

			Common::Value<IpAddress> getValuesAsIpAddress(const string &propertyName);

			Common::Value<IpAddress> getValuesAsIpAddress(const string *propertyName);
			
			Common::Value<IpAddress> getValuesAsIpAddress(int requiredPropertyIndex);

			Common::Value<string> getValueAsString(int requiredPropertyIndex);

			/**
			 * @}
			 * @name Metric Getters
			 * @{
			 */

			/**
			 * Returns the unique device id if the Id property was included
			 * in the required list of properties when the Provider was
			 * constructed.
			 * @param resultIndex index of the individual User-Agent in the
			 * results
			 * @return device id string
			 */
			string getNetworkId(uint32_t resultIndex);

			/**
			 * @}
			 * @name DeviceDetection::ResultsDeviceDetection Implementation
			 * @{
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
			 */
			fiftyoneDegreesPropertyValueType getPropertyValueType(
				int requiredPropertyIndex,
				fiftyoneDegreesException *exception);

			fiftyoneDegreesResultsIpi *results;

			/**
			 * The index in the available properties of the
			 * JavaScriptHardwareProfile property.
			 */
			int _jsHardwareProfileRequiredIndex;
		};
	}
}

#endif
