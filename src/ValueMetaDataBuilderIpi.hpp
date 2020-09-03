#ifndef FIFTYONE_DEGREES_VALUE_META_DATA_BUILDER_IPI_HPP
#define FIFTYONE_DEGREES_VALUE_META_DATA_BUILDER_IPI_HPP

#include <vector>
#include "common-cxx/ValueMetaData.hpp"
#include "common-cxx/EntityMetaDataBuilder.hpp"
#include "BinaryValue.hpp"
#include "ipi.h"

using namespace std;

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Meta data builder class contains static helper methods used when
		 * building value meta data instances from an IP Intelligence data set.
		 */
		class ValueMetaDataBuilderIpi: EntityMetaDataBuilder {
		public:
			/**
			 * Build a new instance of ValueMetaData from the underlying
			 * value provided. The instance returned does not hold a
			 * reference to the data set or value, and contains a copy of
			 * all meta data.
			 * @param dataSet pointer to the data set containing the
			 * component
			 * @param value pointer to the underlying value to create the
			 * meta data from
			 */
			static ValueMetaData* build(
				fiftyoneDegreesDataSetIpi *dataSet,
				fiftyoneDegreesValue *value);
		private:
			/**
			 * Get the binary value from the strings collection
			 * @param stringsCollection the string collection
			 * @param offset the offset in the string collection
			 */
			static BinaryValue getBinaryValue(
				fiftyoneDegreesCollection *stringsCollection,
				uint32_t offset);
		};
	}
}

#endif