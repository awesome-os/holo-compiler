# holo-compiler

Fast, GN/Ninja-free incremental C/C++/WASI build orchestrator with **4-level pre-compile shard tree-shaking**, **late-link planning**, **raw binary `.b8p` CAS object packs**, and **crash-resilient micro-cycle builds**.

## Key Capabilities
- **4-Level Nested Shard Tree-Shaker (`src/nested-shard-treeshaker.mts`):** Prunes dead targets before compilation, tracks macro-sensitivity Merkle hashes, and emits dual-mode late-link plans.
- **Micro-Cycle Resumable Builder (`src/cyclic-builder.mts`):** Breaks multi-hour native builds (e.g., Chromium/V8) into small crash-resilient buckets packed into raw binary `B8PB` v2 SHA-256 `.b8p` archives.
- **Zero-Cost GitHub Releases CAS Bridge (`src/gh-cas-bridge.mts`):** Uses public GitHub Releases as a global CDN-backed `.b8p` cache with local hit skipping and fail-closed SHA-256 verification.

## Quick Start
```bash
node --test src/nested-shard-treeshaker.test.mts src/cyclic-builder.test.mts
```

## License
MIT
