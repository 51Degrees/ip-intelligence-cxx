#ifndef FIFTYONE_DEGREES_PROPERTY_META_DATA_BUILDER_IPI_HPP
#define FIFTYONE_DEGREES_PROPERTY_META_DATA_BUILDER_IPI_HPP

#include <vector>
#include "common-cxx/PropertyMetaData.hpp"
#include "common-cxx/EntityMetaDataBuilder.hpp"
#include "ipi.h"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Meta data builder class contains static helper methods used when
		 * building property meta data instances from an IP Intelligence data set.
		 */
		class PropertyMetaDataBuilderIpi : EntityMetaDataBuilder {
		public:
			/**
			 * Build a new instance of PropertyMetaData from the underlying
			 * property provided. The instance returned does not hold a
			 * reference to the data set or property, and contains a copy
			 * of all values.
			 * @param dataSet pointer to the data set containing the
			 * component
			 * @param property pointer to the underlying property to create
			 * the meta data from
			 */
			static PropertyMetaData* build(
				fiftyoneDegreesDataSetIpi *dataSet,
				fiftyoneDegreesProperty *property);
		private:
			/**
			 * Get a copy of the default value string from the underlying
			 * property.
			 * @param dataSet pointer to the data set containing the
			 * property
			 * @param valueIndex index of the value in the values
			 * collection
			 * @return copy of the value string
			 */
			static string getDefaultValue(
				fiftyoneDegreesDataSetIpi *dataSet,
				uint32_t valueIndex);

			/**
			 * Determine whether the a property with the name provided is
			 * available.
			 * @param dataSet pointer to the data set to find the property
			 * in
			 * @param name of the property to look for
			 */
			static bool propertyIsAvailable(
				fiftyoneDegreesDataSetIpi *dataSet,
				string *name);

			/**
			 * Get a copy of the type name for the underlying property
			 * provided.
			 * @param property pointer to the property to get the type of
			 * @return copy of the type string
			 */
			static string getPropertyType(fiftyoneDegreesProperty *property);

			/**
			 * Get the list of data set maps which contain the requested
			 * property.
			 * @param strings collection of strings containing the map
			 * names
			 * @param maps collection of maps containing maps for the
			 * property
			 * @param property pointer to the property to get the maps for
			 * @return a new vector containing copies of the maps name
			 * strings
			 */
			static vector<string> getPropertyMap(
				fiftyoneDegreesCollection *strings,
				fiftyoneDegreesCollection *maps,
				fiftyoneDegreesProperty *property);

			/**
			 * Get the unique id of the component which the property
			 * relates to.
			 * @param dataSet pointer to the data set containing the
			 * properties and components
			 * @param property pointer to the property to get the component
			 * id of
			 * @return unique component id
			 */
			static byte getComponentId(
				fiftyoneDegreesDataSetIpi *dataSet,
				fiftyoneDegreesProperty *property);

			static vector<uint32_t> getEvidenceProperties(
				fiftyoneDegreesDataSetIpi *dataSet,
				fiftyoneDegreesProperty *property);
		};
	}
}

#endif