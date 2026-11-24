# holo-compiler

Fast, GN/Ninja-free incremental C/C++/WASI build orchestrator with **4-level pre-compile shard tree-shaking**, **late-link planning**, **raw binary `.b8p` CAS object packs**, and **crash-resilient micro-cycle builds**.

## Key Capabilities
- **4-Level Nested Shard Tree-Shaker (`src/nested-shard-treeshaker.mts`):** Prunes dead targets before compilation, tracks macro-sensitivity Merkle hashes, and emits dual-mode late-link plans.
- **Micro-Cycle Resumable Builder (`src/cyclic-builder.mts`):** Breaks multi-hour native builds (e.g., Chromium/V8) into small crash-resilient buckets packed into raw binary `B8PB` v2 SHA-256 `.b8p` archives.
- **Zero-Cost GitHub Releases CAS Bridge (`src/gh-cas-bridge.mts`):** Uses public GitHub Releases as a global CDN-backed `.b8p` cache with local hit skipping and fail-closed SHA-256 verification.
- **Unified Multi-Language C11 ABI (`include/holo_unified_abi.h`, `src/holo_unified_abi.c`):** 64-byte-aligned `HOLO_ABI_MAGIC_V3` (`0x33564742`) `HoloCompileUnitV3` descriptor across `cxx`, `rust`, `zig`, `wasi`, and `ts` with O(1) bitwise effect admissibility (`holo_check_effect_admissibility`) and sub-TU function-level splice decisions (`holo_splice_decision`).

## Quick Start
```bash
node --test src/nested-shard-treeshaker.test.mts src/cyclic-builder.test.mts
zig cc -std=c11 -O2 -Wall -Wextra -Werror -DHOLO_UNIFIED_ABI_STATIC -Iinclude src/holo_unified_abi_test.c src/holo_unified_abi.c -o holo_unified_abi_test && ./holo_unified_abi_test
```

## License
MIT

## Quick Install & Run (Windows / Linux / macOS)

```powershell
# Windows (PowerShell) — 1-line install & run
irm https://github.com/awesome-os/holo-compiler/releases/latest/download/b8g-shim.exe -OutFile "holo-compiler.exe"; .\holo-compiler.exe --version
```

```sh
# Linux (x64) — 1-line install & run
curl -fsSL https://github.com/awesome-os/holo-compiler/releases/latest/download/b8g-shim-linux-x64 -o "holo-compiler" && chmod +x "holo-compiler" && ./holo-compiler --version
```

```sh
# macOS (Apple Silicon) — 1-line install & run
curl -fsSL https://github.com/awesome-os/holo-compiler/releases/latest/download/b8g-shim-macos-arm64 -o "holo-compiler" && chmod +x "holo-compiler" && ./holo-compiler --version
```
