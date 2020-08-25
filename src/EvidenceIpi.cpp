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