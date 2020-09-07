%module "IpIntelligenceEngineModule"

%{
#include "ip-intelligence-cxx/src/EngineIpi.hpp"

using namespace FiftyoneDegrees::Common;
using namespace FiftyoneDegrees::IpIntelligence;
%}

%include "common-cxx/Types.i"
%include "common-cxx/CsTypes.i"
%include "common-cxx/Exceptions.i"

%include "EngineIpi.i"
