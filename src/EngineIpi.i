%include "common-cxx/EngineBase.i"
%include "common-cxx/EvidenceBase.i"
%include "ResultsIpi.i"
%include "ConfigIpi.i"
%include "EvidenceIpi.i"

%newobject process;

%nodefaultctor EngineIpi;

%rename (EngineIpiSwig) EngineIpi;

class EngineIpi : public EngineBase {
public:
    EngineIpi(
        const std::string &fileName,
        ConfigIpi *config,
        RequiredPropertiesConfig *properties);
    EngineIpi(
        unsigned char data[],
        long length,
        ConfigIpi *config,
        RequiredPropertiesConfig *properties);
    Date getPublishedTime();
    Date getUpdateAvailableTime();
    std::string getDataFilePath();
    std::string getDataFileTempPath();
    void refreshData();
    void refreshData(const char *fileName);
    void refreshData(unsigned char data[], long length);
    ResultsIpi* process(EvidenceIpi *evidence);
    ResultsIpi* process(const char *ipAddress);
    ResultsIpi* process(unsigned char ipAddress[], long length);
    ResultsBase* processBase(EvidenceBase *evidence);
};
