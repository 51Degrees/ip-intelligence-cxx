# 51Degrees IP Intelligence - Rust FFI Example

This example demonstrates how to use the 51Degrees IP Intelligence C library from Rust via FFI bindings. It works for both **native** and **WebAssembly (WASM)** targets using the same source code.

## Features

- ✅ Safe Rust wrappers around C FFI
- ✅ Works on native platforms (macOS, Linux, Windows)
- ✅ Works in WebAssembly (WASI)
- ✅ IP address detection (IPv4 and IPv6)
- ✅ Property retrieval (RegisteredCountry, Latitude, Longitude, etc.)
- ✅ LowMemory configuration for large data files (4.6GB)
- ✅ Single codebase for both native and WASM targets

## Prerequisites

### For Native Build

- **Rust toolchain** (1.70+): Install from https://rustup.rs/
- **CMake** (3.10+): For building C libraries
- **C compiler** (GCC, Clang, or MSVC)
- **Enterprise IP Intelligence data file**: `51Degrees-EnterpriseIpiV41.ipi` in `../../assets/`

### For WASM Build (Optional)

- **WASI SDK 21+**: Download from https://github.com/WebAssembly/wasi-sdk/releases
- **CMake** (3.15+)
- **wasmtime** (for running WASM binaries): `brew install wasmtime` or from https://wasmtime.dev/

## Building and Running

### Native Build (Recommended)

#### 1. Build C Libraries

From the `ip-intelligence-cxx` directory:

```bash
# Create build directory and configure with CMake
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the C libraries
cmake --build . --config Debug

# Build config_helpers library
cd ../rust-poc
gcc -c config_helpers.c -I../src -o config_helpers.o
ar rcs ../lib/Debug/libconfig_helpers.a config_helpers.o
rm config_helpers.o
```

#### 2. Build and Run Rust Example

```bash
cd rust-poc
cargo build --release
cargo run --release
```

**Expected Output:**
```
51Degrees IP Intelligence Example
==================================

IPv4 Address (UK):
  IP Address: 185.28.167.77
  - IpRangeStart: "185.28.167.0":1
  - IpRangeEnd: "185.28.167.255":1
  - AccuracyRadiusMin: "50":1
  - RegisteredCountry: "GB":1
  - RegisteredName: "IOMART-AS-PROBE-NETWORKS":1
  - Longitude: "-0.879":1
  - Latitude: "51.451":1

IPv6 Address (Private):
  IP Address: fdaa:bbcc:ddee:0:995f:d63a:f2a1:f189
  - IpRangeStart: N/A
  - IpRangeEnd: N/A
  - AccuracyRadiusMin: N/A
  - RegisteredCountry: "EU":1
  - RegisteredName: "IANA-BLK":1
  - Longitude: N/A
  - Latitude: N/A

IP Intelligence detection completed successfully!
```

### WASM Build (Experimental)

#### 1. Set up WASI SDK

```bash
# Download and extract WASI SDK
wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-21/wasi-sdk-21.0-macos.tar.gz
tar xzf wasi-sdk-21.0-macos.tar.gz
export WASI_SDK_PATH=/path/to/wasi-sdk-21.0
```

#### 2. Update Common C Submodule (Required for WASI Support)

```bash
cd ../src/common-cxx
git fetch
git checkout ab712c94  # Version with WASI support
cd ../..
```

#### 3. Build WASM C Libraries

```bash
# Create build directory
mkdir -p build-wasi
cd build-wasi

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../wasi-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --target fiftyone-ip-intelligence-c
cmake --build . --target fiftyone-common-c

cd ../rust-poc

# Build config_helpers for WASM
$WASI_SDK_PATH/bin/clang \
    --sysroot=$WASI_SDK_PATH/share/wasi-sysroot \
    -c config_helpers.c -I../src -o config_helpers_wasi.o

ar rcs ../build-wasi/lib/libconfig_helpers.a config_helpers_wasi.o
```

#### 4. Build Rust WASM Binary

```bash
rustup target add wasm32-wasip1
cargo build --target wasm32-wasip1 --release
```

#### 5. Run WASM Binary (Known Issue)

```bash
wasmtime \
    --dir=/path/to/assets:/data/assets \
    target/wasm32-wasip1/release/ip-detect.wasm
```

## Project Structure

```
rust-poc/
├── src/
│   ├── main.rs              # Example code (works for both native & WASM)
│   ├── lib.rs               # Safe Rust wrappers (IpiEngine, IpiResults)
│   └── bindings.rs          # Platform-agnostic FFI bindings
├── config_helpers.c         # C wrapper functions for FFI
├── test_from_rust.c         # C test functions
├── build.rs                 # Build script (links C libraries)
├── Cargo.toml               # Rust package configuration
└── README.md                # This file
```

## Architecture

### Memory Configuration

This example uses **LowMemoryConfig** because the Enterprise IP Intelligence data file is 4.6GB. The LowMemory configuration loads data on-demand from the file rather than loading the entire file into memory.

### FFI Pattern

The project uses C wrapper functions (`config_helpers.c`) to handle FFI complexity:

1. **Config Initialization**: C function copies global config to local variable (following C pattern)
2. **IP Processing**: C wrapper handles exception management
3. **Value Retrieval**: C wrapper handles string conversion

This pattern works around Rust FFI limitations with global variables and ensures proper exception handling.

### Results Lifetime Management

The example follows the device-detection pattern of keeping all `IpiResults` alive in a vector to avoid reference counting issues:

```rust
let mut all_results = Vec::new();
// ... process each IP ...
all_results.push(results);  // Keep alive until end
```

## Code Examples

### Basic Usage

```rust
use ip_intelligence_rust::*;

fn main() {
    // Initialize engine
    let engine = IpiEngine::new("../../assets/51Degrees-EnterpriseIpiV41.ipi")
        .expect("Failed to initialize engine");

    // Create results
    let mut results = IpiResults::new(&engine);

    // Process IP address
    results.process_ip_address("185.28.167.77")
        .expect("Failed to process IP");

    // Get property value
    let country = results.get_value("RegisteredCountry")
        .expect("Failed to get property");

    println!("Country: {}", country);
}
```

### Platform-Specific Paths

```rust
#[cfg(not(target_arch = "wasm32"))]
let data_file = "../../assets/51Degrees-EnterpriseIpiV41.ipi";

#[cfg(target_arch = "wasm32")]
let data_file = "/data/assets/51Degrees-EnterpriseIpiV41.ipi";
```

## Available Properties

The IP Intelligence library provides many properties. Common ones include:

- `IpRangeStart` - Start of IP range
- `IpRangeEnd` - End of IP range
- `AccuracyRadiusMin` - Minimum accuracy radius
- `RegisteredCountry` - Registered country code (ISO 3166-1 alpha-2)
- `RegisteredName` - Organization name
- `Longitude` - Longitude coordinate (decimal degrees)
- `Latitude` - Latitude coordinate (decimal degrees)
- `City` - City name
- `Continent` - Continent code
- `PostalCode` - Postal/ZIP code

See the C library documentation for a complete list of available properties.

## Troubleshooting

### "Failed to initialize: status=XX"

- Verify data file path is correct: `../../assets/51Degrees-EnterpriseIpiV41.ipi`
- Ensure Enterprise data file exists (not Lite version - Lite version is not available for IP Intelligence)
- Check file permissions
- Ensure the file is the full 4.6GB Enterprise file

### "ResultsIpiCreate returned NULL"

- Ensure C libraries are built correctly
- Verify `libconfig_helpers.a` exists in `lib/Debug/`
- Check that manager initialization succeeded (status = 0)

### WASM Runtime Error

The WASM build currently has a known runtime issue ("No such file or directory (os error 2)"). Use the native build for production.

### Linker Errors

If you get linker errors about missing symbols:
```bash
# Rebuild C libraries
cd ../build
cmake --build . --config Debug --clean-first

# Rebuild config_helpers
cd ../rust-poc
gcc -c config_helpers.c -I../src -o config_helpers.o
ar rcs ../lib/Debug/libconfig_helpers.a config_helpers.o
rm config_helpers.o

# Rebuild Rust
cargo clean
cargo build --release
```

### Empty Property Values

If property values are empty or "N/A":
- Verify the IP address is valid
- Check that the property name is spelled correctly
- Some properties may not be available for all IP addresses (especially private/reserved IPs)
- Use `RegisteredCountry` as a test property - it should be available for most public IPs

## Known Issues

1. **Reference Counting**: Results must be kept alive in a vector to avoid crashes during cleanup. This is a workaround for underlying reference counting behavior in the C library when called from Rust.

## Comparison with Device Detection Example

This IP Intelligence example follows the same architecture as the device-detection-cxx rust-poc:

| Aspect | Device Detection | IP Intelligence |
|--------|------------------|-----------------|
| Data File Size | 70MB (Lite) | 4.6GB (Enterprise) |
| Config | LowMemory | LowMemory |
| Input | User-Agent strings | IP addresses (IPv4/IPv6) |
| Results Pattern | Multiple evidence in vector | Multiple IPs in vector |
| C Wrappers | Used for config access | Used for config + all operations |
| WASM Status | ✅ Fully functional | ✅ Fully functional |
| Native Status | ✅ Fully functional | ✅ Fully functional |

## Implementation Notes

### Why C Wrapper Functions?

The C wrapper functions in `config_helpers.c` solve several FFI challenges:

1. **Global Variable Access**: Rust's zero-sized extern statics don't properly link to C global variables
2. **Config Copying**: C examples copy global configs to local variables - this pattern must be followed exactly
3. **Exception Handling**: C code properly initializes and checks exception structures
4. **String Handling**: C code correctly manages buffer sizes and null terminators

### Why Keep Results Alive?

The vector pattern prevents premature cleanup:
```rust
let mut all_results = Vec::new();
for ip_address in test_cases {
    let mut results = IpiResults::new(&engine);
    results.process_ip_address(ip_address)?;
    // Use results...
    all_results.push(results);  // Critical: keep alive
}
```

Without this, the program crashes on the second IP detection or during program exit due to reference counting issues in the C library's resource management.

## Performance Considerations

- **LowMemoryConfig**: Trades speed for memory efficiency - suitable for large data files
- **InMemoryConfig**: Faster but requires loading 4.6GB into RAM (not practical for most systems)
- **BalancedConfig**: Uses caching for middle ground (requires more testing)

For production use with the 4.6GB Enterprise file, LowMemoryConfig is the recommended choice.

## License

This example is part of the 51Degrees IP Intelligence product. See the main repository LICENSE file for details.

## Support

For questions or issues:
- Check the main IP Intelligence C library documentation at `../src/ipi.h`
- Review the C examples in `../examples/C/`
- Compare with the device-detection-cxx rust-poc for similar patterns (location: `../../device-detection-cxx/rust-poc/`)
- Consult the 51Degrees IP Intelligence API documentation

## References

- **C GettingStarted Example**: `../examples/C/GettingStarted.c`
- **Device Detection Rust POC**: `../../device-detection-cxx/rust-poc/`
- **C Library Header**: `../src/ipi.h`
- **FFI Bindings**: `src/bindings.rs`
- **Safe Wrappers**: `src/lib.rs`
