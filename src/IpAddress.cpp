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
    init(ipAddress, type);
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
    // Initialize the IP address object
    init(eIpAddress->address, eIpAddress->type);

    // Free the previously allocated IP address
    free(eIpAddress);
}

void IpAddress::init(unsigned char *ipAddress,
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

void IpAddress::getCopyOfIpAddress(unsigned char copy[FIFTYONE_DEGREES_IPV6_LENGTH]) {
    memcpy(copy, this->ipAddress, FIFTYONE_DEGREES_IPV6_LENGTH);
}