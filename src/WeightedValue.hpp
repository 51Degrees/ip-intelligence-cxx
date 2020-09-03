#ifndef FIFTYONE_DEGREES_WEIGHTED_VALUE_HPP
#define FIFTYONE_DEGREES_WEIGHTED_VALUE_HPP

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		template <class T> class WeightedValue {
			/**
			 * Class that represents a single value returned for a
			 * IP Intelligence property. Each value is associated
			 * with a weight in percentage to represent the likeness
			 * of its accuracy.
			 */
			public:
				/**
				 * @name Constructors
				 * @{
				 */

				/**
				 * Construct a default instance with default value
				 * and 0 weight
				 */
				WeightedValue<T>() {
					this->value = T();
					this->weight = 0;
				};

				/**
				 * Construct an instance with provided value and weight
				 * @param value the value of the instance
				 * @param weight of the value
				 */
				WeightedValue<T>(T value, float weight) {
					this->value = value;
					this->weight = weight;
				};

				/**
				 * @}
				 * @name Setters and Getters
				 * @{
				 */
		
				/**
				 * Get the value
				 * @return the value
				 */
				T getValue() { return value; };

				/**
				 * Set the value
				 * @param v the value to set
				 */
				void setValue(T v) { value = v; };

				/**
				 * Get the weight
				 * @return the weight
				 */
				float getWeight() { return weight; };

				/**
				 * Set the weight
				 * @param w the weight to set
				 */
				void setWeight(float w) { weight = w; };

				/**
				 * @}
				 */
			private:
				/** The value */
				T value;
				/** The weight of the value */
				float weight;
		};
	}
}

#endif
