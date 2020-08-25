#include <string>
#include <stdexcept>
#include "IpAddress.hpp"

using namespace std;
using namespace FiftyoneDegrees::IpIntelligence;

IpAddress::IpAddress() {
    memset(this->ipAddress, 0, FIFTYONE_DEGREES_IPV6_LENGTH);
    this->type = FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_INVALID;
}

IpAddress::IpAddress(unsigned char ipAddress[],
    fiftyoneDegreesEvidenceIpType type) {
    switch (type) {
    case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4:
        memcpy(this->ipAddress, ipAddress, FIFTYONE_DEGREES_IPV4_LENGTH);
        break;
    case FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6:
        memcpy(this->ipAddress, ipAddress, FIFTYONE_DEGREES_IPV6_LENGTH);
        break;
    default:
        memset(this->ipAddress, 0, FIFTYONE_DEGREES_IPV6_LENGTH);
        break;
    }
    this->type = type;
}

IpAddress::IpAddress(const char *ipAddressString) {
    fiftyoneDegreesEvidenceIpAddress *eIpAddress = 
		fiftyoneDegreesIpParseAddress(
			malloc,
			ipAddressString,
			ipAddressString + strlen(ipAddressString));
    // Make sure the ip address has been parsed successfully
	if (eIpAddress == NULL) {
		throw bad_alloc();
	}
    // Construct the ip address object
    IpAddress::IpAddress(eIpAddress->address, eIpAddress->type);
    // Free the previously allocated ip address
    free(eIpAddress);
}