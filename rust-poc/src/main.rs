// 51Degrees IP Intelligence Example
// Works for both native and WASM targets
// Based on the GettingStarted.c example

use ip_intelligence_rust::*;

fn main() {
    println!("51Degrees IP Intelligence Example");
    println!("==================================\n");

    // Use different paths for native vs WASM
    #[cfg(not(target_arch = "wasm32"))]
    let data_file = "../../assets/51Degrees-EnterpriseIpiV41.ipi";

    #[cfg(target_arch = "wasm32")]
    let data_file = "/data/assets/51Degrees-EnterpriseIpiV41.ipi";

    // Initialize the engine
    let engine = match IpiEngine::new(data_file) {
        Ok(engine) => {
            println!("Initialized 51Degrees IP Intelligence engine successfully\n");
            engine
        }
        Err(e) => {
            eprintln!("Failed to initialize engine: {}", e);
            eprintln!("\nMake sure to run with:");
            eprintln!("  wasmtime --dir=<path-to-data>:/data/assets ip-detect.wasm");
            return;
        }
    };

    // Test with different IP addresses
    let test_cases = vec![
        ("IPv4 Address (UK)", "185.28.167.77"),
        ("IPv6 Address (Private)", "fdaa:bbcc:ddee:0:995f:d63a:f2a1:f189"),
    ];

    // Properties to display (matching GettingStarted.c)
    let properties = vec![
        "IpRangeStart",
        "IpRangeEnd",
        "AccuracyRadiusMin",
        "RegisteredCountry",
        "RegisteredName",
        "Longitude",
        "Latitude",
    ];

    // WASM workaround: Keep all results alive to avoid reference counting bug
    let mut all_results = Vec::new();

    for (description, ip_address) in test_cases {
        println!("{}:", description);
        println!("  IP Address: {}", ip_address);

        // Create results for each detection (keeping engine borrowed)
        let mut results = IpiResults::new(&engine);
        match results.process_ip_address(ip_address) {
            Ok(_) => {
                // Display properties
                for property in &properties {
                    match results.get_value(property) {
                        Ok(val) => {
                            // Remove confidence score (":1") from the value
                            let clean_val = val.split(':').next().unwrap_or(&val);
                            println!("  - {}: {}", property, clean_val);
                        },
                        Err(_) => println!("  - {}: N/A", property),
                    }
                }
            }
            Err(e) => {
                println!("  Detection failed: {}", e);
            }
        }
        println!();

        // Keep results alive to work around reference counting issue
        all_results.push(results);
    }

    println!("IP Intelligence detection completed successfully!");
}
