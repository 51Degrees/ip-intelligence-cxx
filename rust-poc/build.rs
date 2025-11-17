use std::env;
use std::path::PathBuf;

fn main() {
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    // Get the project root (parent directory of rust-poc)
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let project_root = PathBuf::from(&manifest_dir).parent().unwrap().to_path_buf();

    // Detect target architecture and environment
    let target_arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();

    // Check if this is a WASM target
    let is_wasm = target_arch == "wasm32";

    // Use different build directories for native vs WASM
    let lib_dir = if is_wasm {
        let build_dir = if target_os == "wasi" {
            project_root.join("build-wasi")
        } else {
            project_root.join("build-wasm")
        };
        build_dir.join("lib")
    } else {
        // Native build - using Debug directory from Xcode
        project_root.join("lib/Debug")
    };

    println!("cargo:rerun-if-changed=../src/");

    // Link against pre-built CMake libraries
    println!("cargo:rustc-link-search=native={}", lib_dir.display());
    println!("cargo:rustc-link-lib=static=fiftyone-ip-intelligence-c");
    println!("cargo:rustc-link-lib=static=fiftyone-common-c");

    // Link against helper C library (only for native builds, causes WASM issues)
    if !is_wasm {
        println!("cargo:rustc-link-lib=static=config_helpers");
    }

    // Link against system libraries
    if !is_wasm && target_os != "windows" {
        println!("cargo:rustc-link-lib=dylib=m");
        println!("cargo:rustc-link-lib=dylib=pthread");
    } else if is_wasm && target_os == "wasi" {
        // Link against WASI emulated libraries for signal and getpid support
        let wasi_sdk_path = env::var("WASI_SDK_PATH")
            .unwrap_or_else(|_| {
                let home = env::var("HOME").unwrap_or_else(|_| "/Users/eugene".to_string());
                format!("{}/wasi-sdk", home)
            });
        let wasi_lib_path = format!("{}/share/wasi-sysroot/lib/wasm32-wasip1", wasi_sdk_path);
        println!("cargo:rustc-link-search=native={}", wasi_lib_path);
        println!("cargo:rustc-link-lib=static=wasi-emulated-signal");
        println!("cargo:rustc-link-lib=static=wasi-emulated-getpid");
        // Export function table to allow indirect function calls
        println!("cargo:rustc-link-arg=--export-table");
    }

    // Use manually crafted bindings for both WASM and native
    // This keeps the code consistent across platforms
    let bindings_src = PathBuf::from(&manifest_dir).join("src/bindings.rs");
    std::fs::copy(&bindings_src, out_path.join("bindings.rs"))
        .expect("Failed to copy bindings");
}
