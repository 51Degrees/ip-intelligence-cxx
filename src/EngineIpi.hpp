#ifndef FIFTYONE_DEGREES_ENGINE_HASH_HPP
#define FIFTYONE_DEGREES_ENGINE_HASH_HPP

#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <stdlib.h>
#include <sstream>
#include <algorithm>
#include "common-cxx/EngineBase.hpp"
#include "common-cxx/resource.h"
#include "common-cxx/RequiredPropertiesConfig.hpp"
#include "common-cxx/Date.hpp"
#include "EvidenceIpi.hpp"
#include "ConfigIpi.hpp"
#include "ResultsIpi.hpp"
#include "MetaDataIpi.hpp"

using namespace std;
using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * Encapsulates the Hash engine class which implements
		 * #EngineDeviceDetection. This carries out processing using a
		 * Hash data set.
		 *
		 * An engine is constructed with a configuration, and either a
		 * data file, or an in memory data set, then used to process
		 * evidence in order to return a set of results. It also exposes
		 * methods to refresh the data using a new data set, and get
		 * properties relating to the data set being used by the engine.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::Common;
		 * using namespace FiftyoneDegrees::DeviceDetection;
		 * using namespace FiftyoneDegrees::DeviceDetection::Hash;
		 * ConfigHash *config;
		 * string dataFilePath;
		 * void *inMemoryDataSet;
		 * long inMemoryDataSetLength;
		 * RequiredPropertiesConfig *properties;
		 * EvidenceDeviceDetection *evidence;
		 *
		 * // Construct the engine from a data file
		 * EngineHash *engine = new EngineHash(
		 *     dataFilePath,
		 *     config,
		 *     properties);
		 *
		 * // Or from a data file which has been loaded into continuous
		 * // memory
		 * EngineHash *engine = new EngineHash(
		 *     inMemoryDataSet,
		 *     inMemoryDataSetLength,
		 *     config,
		 *     properties);
		 *
		 * // Process some evidence
		 * ResultsHash *results = engine->process(evidence);
		 *
		 * // Or just process a single User-Agent string
		 * ResultsHash *results = engine->process("some User-Agent");
		 *
		 * // Do something with the results
		 * // ...
		 *
		 * // Delete the results and the engine
		 * delete results;
		 * delete engine;
		 * ```
		 */
		class EngineIpi : public EngineBase {
			friend class ::EngineIpIntelligenceTests;
		public:
			/**
			 * @name Constructors
			 * @{
			 */

			 /**
			  * @copydoc Common::EngineBase::EngineBase
			  * The data set is constructed from the file provided.
			  * @param fileName path to the file containing the data file
			  * to load
			  */
			EngineIpi(
				const char *fileName,
				ConfigIpi *config,
				RequiredPropertiesConfig *properties);

			/**
			 * @copydoc Common::EngineBase::EngineBase
			 * The data set is constructed from the file provided.
			 * @param fileName path to the file containing the data file to
			 * load
			 */
			EngineIpi(
				const string &fileName,
				ConfigIpi *config,
				RequiredPropertiesConfig *properties);

			/**
			 * @copydoc Common::EngineBase::EngineBase
			 * The data set is constructed from data stored in memory
			 * described by the data and length parameters.
			 * @param data pointer to the memory containing the data set
			 * @param length size of the data in memory
			 */
			EngineIpi(
				void *data,
				long length,
				ConfigIpi *config,
				RequiredPropertiesConfig *properties);

			/**
			 * @copydoc Common::EngineBase::EngineBase
			 * The data set is constructed from data stored in memory
			 * described by the data and length parameters.
			 * @param data pointer to the memory containing the data set
			 * @param length size of the data in memory
			 */
			EngineIpi(
				unsigned char data[],
				long length,
				ConfigIpi *config,
				RequiredPropertiesConfig *properties);

			/**
			 * @}
			 * @name Engine Methods
			 * @{
			 */

			/**
			 * @copydoc EngineDeviceDetection::processDeviceDetection(EvidenceDeviceDetection*)
			 */
			ResultsIpi* process(EvidenceIpi *evidence);

			/**
			 * @copydoc EngineDeviceDetection::processDeviceDetection(const char*)
			 */
			ResultsIpi* process(const char *ipAddress);

			ResultsIpi *process(unsigned char ipAddress[],
                                                   long length);

			/**
			 * @}
			 * @name Common::EngineBase Implementation
			 * @{
			 */

			void refreshData();

			void refreshData(const char *fileName);

			void refreshData(void *data, long length);

			void refreshData(unsigned char data[], long length);

			ResultsBase* processBase(EvidenceBase *evidence);

			Date getPublishedTime();

			Date getUpdateAvailableTime();

			string getDataFilePath();

			string getDataFileTempPath();

			string getProduct();

			string getType();


			/**
			 * @}
			 */

		protected:
			/**
			 * @copydoc EngineDeviceDetection::init
			 */
			void init(fiftyoneDegreesDataSetIpi *dataSet);

		private:
			void initMetaData();

			void init();

			void* copyData(void *data, size_t length);
		};
	}
}

#endif
