/*
 * Swig interface for fiftyoneDegreesEvidenceIpType to used with IpAddress class 
 */
%{
#include "common-cxx/ip.h"
%}

%rename (fiftyoneDegreesEvidenceIpTypeSwig) fiftyoneDegreesEvidenceIpType;

typedef enum e_fiftyone_degrees_evidence_ip_type {
        FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV4 = 0, /**< An IPv4 address */
        FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_IPV6 = 1, /**< An IPv6 address */
        FIFTYONE_DEGREES_EVIDENCE_IP_TYPE_INVALID = 2, /**< Invalid IP address */
} fiftyoneDegreesEvidenceIpType;


/*
 * Swig interface for the IpAddress class
 */
%nodefaultctor IpAddress;

%rename (IpAddressSwig) IpAddress;

class IpAddress {
public:
    IpAddress(unsigned char ipAddress[], fiftyoneDegreesEvidenceIpType type);
    IpAddress(const char *ipAddressString);
    void getCopyOfIpAddress(unsigned char copy[], uint32_t size);
    fiftyoneDegreesEvidenceIpType getType();
};
