#ifndef FIFTYONE_DEGREES_IP_ADDRESS_HPP
#define FIFTYONE_DEGREES_IP_ADDRESS_HPP

#include "ipi.h"

namespace FiftyoneDegrees {
    namespace IpIntelligence {
    	class IpAddress {
        public:
            IpAddress();
    
            IpAddress(unsigned char ipAddress[],
                      fiftyoneDegreesEvidenceIpType type);

            IpAddress(const char *ipAddressString);

            const unsigned char *getIpAddress() const{
                return (const unsigned char *)ipAddress;
            };

            fiftyoneDegreesEvidenceIpType getType(){ return type; };
    
        private:
            fiftyoneDegreesEvidenceIpType type;
            // Use a fixed size byte array
            unsigned char ipAddress[FIFTYONE_DEGREES_IPV6_LENGTH];
        };
    }
}

#endif