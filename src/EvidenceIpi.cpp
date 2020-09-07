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

#include "EvidenceIpi.hpp"
#include "fiftyone.h"

using namespace FiftyoneDegrees::IpIntelligence;

EvidenceIpi::EvidenceIpi() {
	evidence = NULL;
}

EvidenceIpi::~EvidenceIpi() {
	if (evidence != NULL) {
		EvidenceFree(evidence);
        evidence = NULL;
	}
}

fiftyoneDegreesEvidenceKeyValuePairArray* EvidenceIpi::get() {
    if (evidence != NULL) {
        EvidenceFree(evidence);
        evidence = NULL;
    }
    evidence = EvidenceCreate((uint32_t)size());
    if (evidence != NULL) {
        for (map<string, IpAddress>::const_iterator iterator = begin();
             iterator != end(); iterator++) {
            EvidencePrefixMap* map = EvidenceMapPrefix(iterator->first.c_str());
            if (map != NULL && isRelevant(map->prefixEnum)) {
                EvidenceAddString(evidence, map->prefixEnum,
                                  iterator->first.c_str() + map->prefixLength,
                                  (const char *)iterator->second.getIpAddress());
            }
        }
    }
    return evidence;
}

bool EvidenceIpi::isRelevant(
	fiftyoneDegreesEvidencePrefix prefix) {
	return 
		(prefix & FIFTYONE_DEGREES_EVIDENCE_IPV4) ==
			FIFTYONE_DEGREES_EVIDENCE_IPV4 ||
        (prefix & FIFTYONE_DEGREES_EVIDENCE_IPV6) ==
            FIFTYONE_DEGREES_EVIDENCE_IPV6;
}

void EvidenceIpi::clear() {
    map<string, IpAddress>::clear();
    EvidenceFree(evidence);
    evidence = NULL;
}

void EvidenceIpi::erase(iterator position) {
    map<string, IpAddress>::erase(position);
    EvidenceFree(evidence);
    evidence = NULL;
}

void EvidenceIpi::erase(iterator first, iterator last) {
    map<string, IpAddress>::erase(first, last);
    EvidenceFree(evidence);
    evidence = NULL;
}