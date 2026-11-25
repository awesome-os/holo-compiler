# Holo Compiler & SDK Documentation (`docs/`)

> **Central user documentation for the Awesome-OS Holo Stack: `repository://` (immutable `v11` source of truth) × `project://` (0-byte holographic worktrees & installations) × `B8G_ABI_MAGIC_V3` (`0x33564742`).**

---

## Guides

1. **[Quickstart](./quickstart.md)** — Install the Holo Stack, configure `HOLO_CAS_DIR`, and run your first `repository://` + `project://` workflow.
2. **[`repository://` & `project://` Protocol Guide](./repository-and-project-uris.md)** — Exact URI grammar, stock `git` compatibility (`git-remote-repository`), and `holo-project` ephemeral vs. persistent projections.
3. **[`v11` Universal Object Store & `holo-ffi`](./v11-cas-and-holo-ffi.md)** — How `v11/files/00..ff/<sha512>` unifies Git blobs/trees, packages, OCI layers, Releases, and native `.dll`/`.so`/`.dylib`/`.wasm` FFI loading.
4. **[`holo-*` Projections Reference](./projections-reference.md)** — All 12 canonical runtime projections (`holo-compiler`, `holo-pm`, `holo-ffi`, `holo-wasi-host`, `holo-node`, `holo-deno`, `holo-py`, `holo-go`, `holo-dart`, `holo-rust`, `holo-cpp`, `holo-jvm`) and legacy `ghost-*` aliases.

---

## Umbrella SDK

- **[`awesome-os/holo-sdk`](https://github.com/awesome-os/holo-sdk)** — Unified SDK manifest referencing all Holo core components and upstream hard-forks via `repository://github.com/awesome-os/<repo>@<ref>`.
