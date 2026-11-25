# Quickstart — Holo Compiler & Projections

## 1. Install (Windows / Linux / macOS)

```powershell
# Windows (PowerShell)
irm https://awesome-os.com/install.ps1 | iex
```

```bash
# Linux / macOS
curl -fsSL https://awesome-os.com/install.sh | sh
```

## 2. Configure the Unified `v11` CAS Root (Optional)

By default, every `holo-*` tool shares one deduplicated `v11` store (`~/.b8g/cas/v11`). Override it anywhere with `HOLO_CAS_DIR` (legacy `B8G_GHOST_CACHE_DIR` is automatically respected as fallback):

```powershell
$env:HOLO_CAS_DIR = "D:\holo-cas"
```

## 3. Use `repository://` with Stock `git`

With `bin/` on `PATH`, stock `git` automatically invokes `git-remote-repository`:

```powershell
git ls-remote repository://local/awesome-os/cargo
git clone repository://local/awesome-os/models
```

## 4. Run Any Projection with `holo-project`

```powershell
# Ephemeral 0-byte hardlink projection (auto-purges after run)
holo-project "project://go/gitaly?from=repository://github.com/awesome-os/gitaly@main"

# Persistent workspace projection
holo-project "project://node/app.mts?from=repository://github.com/awesome-os/models@main&mode=persistent"
```
