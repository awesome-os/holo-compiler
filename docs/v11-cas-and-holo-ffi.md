# `v11` Universal Object Store & `holo-ffi`

## 1. `v11` in Git Terms

The unified `v11` store (`HOLO_CAS_DIR/v11`) is a **SHA-512 Universal Git Object Store** whose blobs are stored raw (zero zlib wrapper) so the OS can hardlink (`os.Link`) and `mmap` them directly:

| Git Concept | `v11` Equivalent |
| :--- | :--- |
| `.git/objects/xx/<sha1>` (`blob`) | `v11/files/<sha512[:2]>/<sha512>` (`-exec` = mode `100755`) |
| `.git/objects/xx/<sha1>` (`tree`) | Sorted `GitForgeTreeManifest` JSON stored in `v11/files/<xx>/<treeSha512>` |
| `.git/objects/xx/<sha1>` (`commit`) | `holo-compiler-lockfile.json` / `RollingReleaseManifest.json` binding `sourceTreeSha512` + `artifactTreeSha512` |
| `.git/index` | `v11/index.db` (SQLite WAL) + `v11/go-actions/` (`b8g-gocacheprog`) |
| `git worktree` | 0-byte hardlink projection via `project://` (`0.37 ms`) |

---

## 2. `holo-ffi` (Zero-Hash Native & WASM FFI)

`holo-ffi` upgrades `ghostFFI` to load `.dll` / `.so` / `.dylib` / `.wasm` directly out of `v11`:

1. Resolves `repository://github.com/awesome-os/<repo>@<tag>` from `holo-compiler-lockfile.json` (no manual hash copy-pasting).
2. Projects a **0-byte hardlink** into `v11/links/<sha512[:16]>-<name>.{dll,so,dylib,wasm}` so Windows `LoadLibraryW` and macOS `dlopen` see the expected file extension.
3. Optionally verifies `B8G_ABI_MAGIC_V3` (`0x33564742`) via `abiMagicSymbol` before any call crosses the FFI boundary.
4. Executes `.wasm` modules directly via `WebAssembly.instantiate` with zero native addon required.
