#include "ExampleBase.hpp"

using namespace FiftyoneDegrees;
using namespace FiftyoneDegrees::Examples::IpIntelligence;

const char *ExampleBase::ipv4Address = "8.8.8.8";

const char *ExampleBase::ipv6Address = "2001:4860:4860::8888::2001:4860:4860::8844";

ExampleBase::ExampleBase(byte *data, long length, ConfigIpi *config) {
  this->config = config;

  // Set the properties to be returned for each Ip Address.
  string propertiesString = "RangeStart,RangeEnd,Country,City,AverageLocation";
  properties = new RequiredPropertiesConfig(propertiesString);

  // Initialise the engine for device detection.
  engine = new EngineIpi(data, length, config, properties);
}

ExampleBase::ExampleBase(string dataFilePath, ConfigIpi *config) {
  this->config = config;

  // Set the properties to be returned for each Ip Address.
  string propertiesString = "RangeStart,RangeEnd,Country,City,AverageLocation";
  properties = new RequiredPropertiesConfig(propertiesString);

  // Initialise the engine for ip intelligence.
  engine = new EngineIpi(dataFilePath, config, properties);
}

ExampleBase::ExampleBase(string dataFilePath)
    : ExampleBase(dataFilePath, new ConfigIpi()) {}

ExampleBase::~ExampleBase() {
  delete engine;
  delete config;
  delete properties;
}

void ExampleBase::reportStatus(fiftyoneDegreesStatusCode status,
                               const char *fileName) {
  const char *message = fiftyoneDegreesStatusGetMessage(status, fileName);
  cout << message;
  fiftyoneDegreesFree((void *)message);
}

unsigned long ExampleBase::generateHash(unsigned char *value) {
  unsigned long hashCode = 5381;
  int i = *value++;
  while (i != 0) {
    hashCode = ((hashCode << 5) + hashCode) + i;
    i = *value++;
  }
  return hashCode;
}

unsigned long ExampleBase::getHashCode(ResultsIpi *results) {
  unsigned long hashCode = 0;
  uint32_t requiredPropertyIndex;
  string valueName;

  for (requiredPropertyIndex = 0;
       requiredPropertyIndex < (uint32_t)results->getAvailableProperties();
       requiredPropertyIndex++) {
    valueName = *results->getValueAsString(requiredPropertyIndex);
    hashCode ^= generateHash((unsigned char *)(valueName.c_str()));
  }
  return hashCode;
}

void ExampleBase::processIpAddress(const char *ipAddress, void *state) {
  ThreadState *thread = (ThreadState *)state;

  ResultsIpi *results = thread->engine->process(ipAddress);

  thread->hashCode ^= getHashCode(results);

  delete results;
}

void ExampleBase::SharedState::processIpAddressesSingle() {
  const char ipAddress[40] = "";
  ThreadState thread(engine);
  fiftyoneDegreesTextFileIterate(ipAddressFilePath.c_str(), ipAddress,
                                 sizeof(ipAddress), &thread, processIpAddress);
  printf("Finished with hash code '%i'\r\n", thread.hashCode);
}

void ExampleBase::SharedState::processIpAddressesMulti(void *state) {
  SharedState *shared = (SharedState *)state;
  shared->processIpAddressesSingle();
  FIFTYONE_DEGREES_INTERLOCK_INC(&shared->threadsFinished);
}

void ExampleBase::SharedState::startThreads() {
  int i;
  for (i = 0; i < THREAD_COUNT; i++) {
    threads[i] = thread(processIpAddressesMulti, this);
  }
}

void ExampleBase::SharedState::joinThreads() {
  int i;
  for (i = 0; i < THREAD_COUNT; i++) {
    threads[i].join();
  }
}

ExampleBase::SharedState::SharedState(FiftyoneDegrees::IpIntelligence::EngineIpi *engine,
                                      string ipAddressFilePath) {
  this->engine = engine;
  this->threadsFinished = 0;
  this->ipAddressFilePath = ipAddressFilePath;
}

ExampleBase::ThreadState::ThreadState(
    FiftyoneDegrees::IpIntelligence::EngineIpi *engine) {
  this->engine = engine;
  this->hashCode = 0;
}