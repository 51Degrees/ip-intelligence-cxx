%include std_string.i
%include std_map.i

/* Create a specific template for map<string, IpAddress> */
%template(MapStringIpAddressSwig) std::map<std::string, IpAddress>;

%include "IpAddress.i"

%nodefaultctor EvidenceIpi;

%rename (EvidenceIpiSwig) EvidenceIpi;

class EvidenceIpi : public std::map<std::string, IpAddress> {
public:
    EvidenceIpi();
    ~EvidenceIpi();
};
