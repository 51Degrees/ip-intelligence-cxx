// Platform-agnostic bindings for 51Degrees IP Intelligence
// Works for both native (64-bit) and WASM (32-bit) targets

use std::os::raw::{c_char, c_int, c_uint, c_void};

// Opaque structs - actual sizes determined at compile time
#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesResourceManager {
    pub active: *mut c_void,
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesException {
    pub status: c_uint,
    pub message: *const c_char,
    pub file: *const c_char,
    pub line: c_int,
}

// Config struct - 160 bytes based on actual C struct size
#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesConfigIpi {
    _data: [u8; 160],  // Actual size of ConfigIpi struct
}

#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesPropertiesRequired {
    pub array: *mut c_void,
    pub count: c_uint,
    pub string: *const c_char,
    pub existing: *mut c_void,
}

// Opaque structs - sizes vary by platform
#[repr(C)]
#[derive(Copy, Clone)]
pub struct fiftyoneDegreesResultsIpi {
    _data: [u8; 0],  // Zero-sized, used as opaque type
}

// Status codes enum (same on all platforms)
pub type e_fiftyone_degrees_status_code = c_uint;
pub const e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_SUCCESS: e_fiftyone_degrees_status_code = 0;
pub const e_fiftyone_degrees_status_code_FIFTYONE_DEGREES_STATUS_NOT_SET: e_fiftyone_degrees_status_code = 26;

// Type alias for compatibility
pub type StatusCode = e_fiftyone_degrees_status_code;

// Function declarations from IP Intelligence C library
extern "C" {
    pub fn fiftyoneDegreesIpiInitManagerFromFile(
        manager: *mut fiftyoneDegreesResourceManager,
        config: *mut fiftyoneDegreesConfigIpi,
        properties: *mut fiftyoneDegreesPropertiesRequired,
        fileName: *const c_char,
        exception: *mut fiftyoneDegreesException,
    ) -> e_fiftyone_degrees_status_code;

    pub fn fiftyoneDegreesResourceManagerFree(
        manager: *mut fiftyoneDegreesResourceManager,
    );

    pub fn fiftyoneDegreesResultsIpiCreate(
        manager: *mut fiftyoneDegreesResourceManager,
    ) -> *mut fiftyoneDegreesResultsIpi;

    pub fn fiftyoneDegreesResultsIpiFromIpAddressString(
        results: *mut fiftyoneDegreesResultsIpi,
        ipAddress: *const c_char,
        ipAddressLength: usize,
        exception: *mut fiftyoneDegreesException,
    );

    pub fn fiftyoneDegreesResultsIpiGetValuesString(
        results: *mut fiftyoneDegreesResultsIpi,
        propertyName: *const c_char,
        buffer: *mut c_char,
        bufferLength: usize,
        separator: *const c_char,
        exception: *mut fiftyoneDegreesException,
    ) -> usize;

    pub fn fiftyoneDegreesResultsIpiFree(results: *mut fiftyoneDegreesResultsIpi);
}

// Global config variables (available for all platforms)
extern "C" {
    pub static fiftyoneDegreesIpiInMemoryConfig: fiftyoneDegreesConfigIpi;
    pub static fiftyoneDegreesIpiLowMemoryConfig: fiftyoneDegreesConfigIpi;
    pub static fiftyoneDegreesIpiBalancedConfig: fiftyoneDegreesConfigIpi;
}

// Helper functions to work with global configs (native only)
#[cfg(not(target_arch = "wasm32"))]
extern "C" {
    pub fn get_ipi_in_memory_config() -> *mut fiftyoneDegreesConfigIpi;
    pub fn get_ipi_low_memory_config() -> *mut fiftyoneDegreesConfigIpi;
    pub fn get_ipi_balanced_config() -> *mut fiftyoneDegreesConfigIpi;
    pub fn get_properties_default() -> *mut fiftyoneDegreesPropertiesRequired;

    // C wrapper to initialize manager using C pattern (copies config to local)
    pub fn init_manager_from_file_c_pattern(
        manager: *mut fiftyoneDegreesResourceManager,
        dataFilePath: *const c_char,
    ) -> c_int;

    // Test functions to process IP and get values from C
    pub fn test_process_ip_from_rust(
        results: *mut fiftyoneDegreesResultsIpi,
        ipAddress: *const c_char,
    ) -> c_int;

    pub fn test_get_value_from_rust(
        results: *mut fiftyoneDegreesResultsIpi,
        propertyName: *const c_char,
        buffer: *mut c_char,
        bufferSize: usize,
    ) -> c_int;
}
