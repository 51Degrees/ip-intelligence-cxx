#ifndef FIFTYONE_DEGREES_WEIGHTED_VALUE_HPP
#define FIFTYONE_DEGREES_WEIGHTED_VALUE_HPP

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		template <class T> class WeightedValue {
			public:
				WeightedValue<T>() {
					this->value = T();
					this->weight = 0;
				};

				WeightedValue<T>(T value, float weight) {
					this->value = value;
					this->weight = weight;
				};
		
				T getValue() { return value; };

				void setValue(T v) { value = v; };

				float getWeight() { return weight; };

				void setWeight(float w) { weight = w; };
			private:
				T value;
				float weight;
		};
	}
}

#endif
