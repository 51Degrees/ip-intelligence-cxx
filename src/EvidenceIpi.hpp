#ifndef FIFTYONE_DEGREES_EVIDENCE_IPI_HPP
#define FIFTYONE_DEGREES_EVIDENCE_IPI_HPP

#include "common-cxx/EvidenceBase.hpp"
#include "IpAddress.hpp"

using namespace FiftyoneDegrees::Common;

namespace FiftyoneDegrees {
	namespace IpIntelligence {
		/**
		 * IP Intelligence specific evidence class containing evidence to be
		 * processed by an IP Intelligence engine.
		 * This wraps a dynamically generated C evidence structure.
		 *
		 * The class does not extend the EvidenceBase class as EvidenceBase
		 * class only support string data while IP address is required to
		 * in its simplest form which is a byte array. In this implementation
		 * the byte array is wrapped in the IpAddress wrapper class.
		 *
		 * ## Usage Example
		 *
		 * ```
		 * using namespace FiftyoneDegrees::IpIntelligence;
		 * EngineIpi *engine;
		 *
		 * // Construct a new evidence instance
		 * EvidenceIpi *evidence = new EvidenceIpi();
		 *
		 * // Construct a new IpAddress instance
		 * IpAddress ipAddress = IpAddress("Some IP address");
		 *
		 * // Add an item of evidence
		 * evidence->operator[]("evidence key") = ipAddress;
		 *
		 * // Give the evidence to an engine for processing
		 * ResultsIpi *results = engine->process(evidence);
		 *
		 * // Do something with the results (and delete them once finished)
		 * // ...
		 *
		 * // Delete the evidence
		 * delete evidence;
		 * ```
		 */
		class EvidenceIpi : public map<string, IpAddress> {
		public:
			/**
			 * @name Constructor and Destructors
			 * @{
			 */

			/**
			 * Construct a new instance containing no evidence
			 */
            EvidenceIpi();

			/**
			 * Free al the underlying memory containing the evidence
			 */
			~EvidenceIpi();

			/**
			 * @}
			 * @name Getters
			 * @{
			 */

			/**
			 * Get the underlying C structure containing the evidence. This
			 * only includes evidence which is relevant to the engine. Any
			 * evidence which is irrelevant will not be included in the result.
			 * @return pointer to a populated C evidence structure
			 */
			fiftyoneDegreesEvidenceKeyValuePairArray* get();

			/**
			 * @}
			 * @name Overrides
			 * @{
			 */

			/**
			 * Clear all evidence items from the instance.
			 */
			void clear();

			/**
			 * Remove the evidence item at the  position indicated.
			 * @param position of the item to remove
			 */
			void erase(iterator position);

			/**
			 * Remove the evidence items between the two position indicated.
			 * @param first item to remove
			 * @param last item to remove
			 */
			void erase(iterator first, iterator last);

			/**
			 * @}
			 */
		protected:
			/**
			 * Get whether or not the evidence key prefix is relevant or not.
			 * If the prefix is not relevant or not known then it is of no use
			 * to the engine processing it.
			 * @param prefix extracted from the evidence key
			 * @return true if the key prefix relevant and should be used
			 */
			bool isRelevant(fiftyoneDegreesEvidencePrefix prefix);

		private:
			/** The underlying evidence structure. */
            fiftyoneDegreesEvidenceKeyValuePairArray* evidence;
		};
	}
}

#endif
