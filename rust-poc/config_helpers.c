#include "../src/ipi.h"
#include "../src/fiftyone.h"
#include <stdio.h>
#include <string.h>

// Helper functions to get pointers to global configs
// This works around Rust's issue with zero-sized extern statics

fiftyoneDegreesConfigIpi* get_ipi_in_memory_config() {
    return &fiftyoneDegreesIpiInMemoryConfig;
}

fiftyoneDegreesConfigIpi* get_ipi_low_memory_config() {
    printf("[C helper] get_ipi_low_memory_config() returning %p\n", &fiftyoneDegreesIpiLowMemoryConfig);
    fflush(stdout);
    return &fiftyoneDegreesIpiLowMemoryConfig;
}

fiftyoneDegreesConfigIpi* get_ipi_balanced_config() {
    return &fiftyoneDegreesIpiBalancedConfig;
}

fiftyoneDegreesPropertiesRequired* get_properties_default() {
    printf("[C helper] get_properties_default() returning %p\n", &fiftyoneDegreesPropertiesDefault);
    fflush(stdout);
    return &fiftyoneDegreesPropertiesDefault;
}

// C wrapper to initialize manager (C pattern: copy config to local variable)
int init_manager_from_file_c_pattern(
    fiftyoneDegreesResourceManager* manager,
    const char* dataFilePath) {

    fiftyoneDegreesException exception;
    memset(&exception, 0, sizeof(exception));
    exception.status = FIFTYONE_DEGREES_STATUS_NOT_SET;

    // Copy config to local (like C examples do)
    fiftyoneDegreesConfigIpi config = fiftyoneDegreesIpiLowMemoryConfig;

    // Initialize manager with NULL properties (means all properties)
    fiftyoneDegreesStatusCode status = fiftyoneDegreesIpiInitManagerFromFile(
        manager,
        &config,
        NULL,  // NULL means all properties
        dataFilePath,
        &exception);

    return (int)status;
}

// Wrapper to process IP address from Rust (using C patterns)
int test_process_ip_from_rust(
    fiftyoneDegreesResultsIpi* results,
    const char* ipAddress) {

    if (results == NULL) {
        return -1;
    }

    fiftyoneDegreesException exception;
    memset(&exception, 0, sizeof(exception));
    exception.status = FIFTYONE_DEGREES_STATUS_NOT_SET;

    fiftyoneDegreesResultsIpiFromIpAddressString(
        results,
        ipAddress,
        strlen(ipAddress),
        &exception);

    return 0;
}

// Wrapper to get property value from results (using C patterns)
int test_get_value_from_rust(
    fiftyoneDegreesResultsIpi* results,
    const char* propertyName,
    char* buffer,
    size_t bufferSize) {

    fiftyoneDegreesException exception;
    memset(&exception, 0, sizeof(exception));
    exception.status = FIFTYONE_DEGREES_STATUS_NOT_SET;

    size_t len = fiftyoneDegreesResultsIpiGetValuesString(
        results,
        propertyName,
        buffer,
        bufferSize,
        "|",
        &exception);

    return (int)len;
}
