#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(dead_code)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::ffi::CString;

// Helper function to copy a config (mimics C's "ConfigIpi config = IpiLowMemoryConfig;")
unsafe fn copy_config(config_ptr: *const fiftyoneDegreesConfigIpi) -> fiftyoneDegreesConfigIpi {
    std::ptr::read(config_ptr)
}

/// Safe wrapper for initializing the IP Intelligence engine
pub struct IpiEngine {
    manager: Box<fiftyoneDegreesResourceManager>,
}

impl IpiEngine {
    pub fn new(data_file_path: &str) -> Result<Self, String> {
        unsafe {
            let mut manager_data = Box::new(std::mem::zeroed::<fiftyoneDegreesResourceManager>());
            let path = CString::new(data_file_path).map_err(|e| e.to_string())?;

            let status;

            #[cfg(not(target_arch = "wasm32"))]
            {
                // Use C wrapper to initialize manager (native)
                status = init_manager_from_file_c_pattern(
                    manager_data.as_mut(),
                    path.as_ptr(),
                );
            }

            #[cfg(target_arch = "wasm32")]
            {
                // Call C function directly for WASM
                let mut exception: fiftyoneDegreesException = std::mem::zeroed();
                exception.status = 26; // FIFTYONE_DEGREES_STATUS_NOT_SET

                // Use the global config directly (pass its address to the C function)
                extern "C" {
                    static mut fiftyoneDegreesIpiLowMemoryConfig: fiftyoneDegreesConfigIpi;
                }

                status = fiftyoneDegreesIpiInitManagerFromFile(
                    manager_data.as_mut(),
                    &mut fiftyoneDegreesIpiLowMemoryConfig,
                    std::ptr::null_mut(), // NULL for all properties
                    path.as_ptr(),
                    &mut exception,
                ) as i32;
            }

            if status != 0 {
                return Err(format!("Failed to initialize: status={}", status));
            }

            Ok(IpiEngine { manager: manager_data })
        }
    }

    pub fn get_manager(&self) -> &fiftyoneDegreesResourceManager {
        &self.manager
    }

    pub fn get_manager_mut(&mut self) -> &mut fiftyoneDegreesResourceManager {
        &mut self.manager
    }

    pub fn get_manager_ptr(&self) -> *const fiftyoneDegreesResourceManager {
        self.manager.as_ref() as *const _
    }

    pub fn get_manager_mut_ptr(&mut self) -> *mut fiftyoneDegreesResourceManager {
        self.manager.as_mut() as *mut _
    }
}

impl Drop for IpiEngine {
    fn drop(&mut self) {
        unsafe {
            fiftyoneDegreesResourceManagerFree(self.manager.as_mut());
        }
    }
}

/// Safe wrapper for IP Intelligence results
pub struct IpiResults {
    results: *mut fiftyoneDegreesResultsIpi,
}

impl IpiResults {
    pub fn new(engine: &IpiEngine) -> Self {
        unsafe {
            let results = fiftyoneDegreesResultsIpiCreate(
                engine.manager.as_ref() as *const _ as *mut _,
            );

            if results.is_null() {
                panic!("Failed to create results - ResultsIpiCreate returned NULL!");
            }

            IpiResults { results }
        }
    }

    pub fn process_ip_address(&mut self, ip_address: &str) -> Result<(), String> {
        unsafe {
            let ip_cstring = CString::new(ip_address).unwrap();

            #[cfg(not(target_arch = "wasm32"))]
            {
                // Use C wrapper function for native (this works!)
                test_process_ip_from_rust(self.results, ip_cstring.as_ptr());
            }

            #[cfg(target_arch = "wasm32")]
            {
                // Call C function directly for WASM
                let mut exception: fiftyoneDegreesException = std::mem::zeroed();
                exception.status = 26; // FIFTYONE_DEGREES_STATUS_NOT_SET

                fiftyoneDegreesResultsIpiFromIpAddressString(
                    self.results,
                    ip_cstring.as_ptr(),
                    ip_address.len(),
                    &mut exception,
                );
            }

            Ok(())
        }
    }

    pub fn get_value(&self, property_name: &str) -> Result<String, String> {
        unsafe {
            let prop_name = CString::new(property_name).unwrap();
            let mut buffer = vec![0u8; 4096];

            let len;

            #[cfg(not(target_arch = "wasm32"))]
            {
                // Use C wrapper function for native (which works!)
                len = test_get_value_from_rust(
                    self.results,
                    prop_name.as_ptr(),
                    buffer.as_mut_ptr() as *mut i8,
                    buffer.len(),
                );
            }

            #[cfg(target_arch = "wasm32")]
            {
                // Call C function directly for WASM
                let mut exception: fiftyoneDegreesException = std::mem::zeroed();
                exception.status = 26; // FIFTYONE_DEGREES_STATUS_NOT_SET

                let separator = CString::new("|").unwrap();
                len = fiftyoneDegreesResultsIpiGetValuesString(
                    self.results,
                    prop_name.as_ptr(),
                    buffer.as_mut_ptr() as *mut i8,
                    buffer.len(),
                    separator.as_ptr(),
                    &mut exception,
                ) as i32;
            }

            if len <= 1 {
                return Err(format!("Property '{}' not found or has no value", property_name));
            }

            let result = String::from_utf8_lossy(&buffer[..len as usize])
                .trim_end_matches('\0')
                .to_string();

            Ok(result)
        }
    }
}

impl Drop for IpiResults {
    fn drop(&mut self) {
        unsafe {
            if !self.results.is_null() {
                fiftyoneDegreesResultsIpiFree(self.results);
            }
        }
    }
}
