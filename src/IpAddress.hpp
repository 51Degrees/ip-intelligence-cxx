#ifndef FIFTYONE_DEGREES_IP_ADDRESS_HPP
#define FIFTYONE_DEGREES_IP_ADDRESS_HPP

#include "ipi.h"

namespace FiftyoneDegrees {
    namespace IpIntelligence {
    	class IpAddress {
        public:
            /**
             * @name Constructors
             * @{
             */

            /**
             * Construct a default instance with an
             * invalid IP address
             */
            IpAddress();
    
            /**
             * Construct an instance with a given
             * combination of IP address byte array
             * and its type
             * @param ipAddress the IP address byte array
             * @param type the type of the IP address
             */
            IpAddress(unsigned char ipAddress[],
                      fiftyoneDegreesEvidenceIpType type);

            /**
             * Construct an instance with a given
             * IP address string. The type of the IP
             * address is determined by parsing the string
             * @param ipAddressString the IP address string
             */
            IpAddress(const char *ipAddressString);

            /**
             * @}
             * @name Getters
             * @{
             */

            /**
             * Get the IP address byte array
             * @return a constant pointer to the internal
             * byte array
             */
            const unsigned char *getIpAddress() const{
                return (const unsigned char *)ipAddress;
            };

            /**
             * Returns a copy of the IP address byte array
             * This is used mainly for SWIG so that other
             * language can get a value of the byte array.
             * By using carrays.i in SWIG, any access to
             * this copy can be done via SWIG array functions.
             *
             * To get the actual pointer, the getIpAddress
             * should be used.
             * @param copy which will hold an copy of the byte array
             */
            void getCopyOfIpAddress(unsigned char copy[FIFTYONE_DEGREES_IPV6_LENGTH]);

            /**
             * Get the type of the IP address
             * @return the type of IP address
             */
            fiftyoneDegreesEvidenceIpType getType(){ return type; };

            /**
             *@}
             */
    
        private:
            /**
             * Initialization function
             * @param ipAddress the byte array IP address
             * @param type the type of the IP
             */
            void init(unsigned char *ipAddress,
                      fiftyoneDegreesEvidenceIpType type);

            /** The type of the IP address */
            fiftyoneDegreesEvidenceIpType type;
            /** The IP address byte array */
            unsigned char ipAddress[FIFTYONE_DEGREES_IPV6_LENGTH];
        };
    }
}

#endif