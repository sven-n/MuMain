# External Integrations

**Analysis Date:** 2025-02-08

## APIs & External Services

**Game Server (OpenMU / MU Online compatible):**
- OpenMU - Primary target server ([github.com/MUnique/OpenMU](https://github.com/MUnique/OpenMU))
- Protocol: Custom TCP, Season 6 Episode 3 adapted
- Version: Client identifies as 2.04d, serial `k1Pk2jcET48mxL3b`
- Connect: `/u<host> /p<port>` or default localhost:44406
- Encryption: SimpleModulus / XOR for game server; unencrypted for connect server

## Network Layer

**ClientLibrary (.NET Native AOT):**
- Location: `ClientLibrary/` → publishes `MUnique.Client.Library.dll`
- C++ interop: `LoadLibrary` / `GetProcAddress` loads DLL; `UnmanagedCallersOnly` exports `ConnectionManager_Connect`, `ConnectionManager_Send`, etc.
- Bridge: `src/source/Dotnet/Connection.h`, `Connection.cpp` - wraps .NET ConnectionManager
- Packet definitions: `MUnique.OpenMU.Network.Packets` NuGet package; XSLT generates `PacketFunctions_*.cpp/h`, `PacketBindings_*.h` from XML

**Connect Flow:**
- ConnectServer (login) → ChatServer → GameServer (ClientToServer)
- Handles: integer IDs for each connection; callbacks `OnPacketReceived`, `OnDisconnected`

## Data Storage

**File-based only:**
- Game assets in `src/bin/` (textures, sounds, data files)
- Translations: `src/bin/Translations/{locale}/game.json`, `editor.json`, `metadata.json`
- No database; all data loaded from files at runtime

## Authentication & Identity

**In-game only:**
- No external auth provider
- Login credentials sent to ConnectServer (OpenMU)
- Session managed by server; client stores connection handles

## CI/CD & Deployment

**GitHub Actions:**
- Workflows: `.github/workflows/mingw-build.yml`, `mingw-build-pr.yml`, `mingw-build-dev.yml`
- MinGW cross-compilation from Ubuntu
- Builds libjpeg-turbo static, CMake + Ninja, produces `Main.exe` artifact
- No .NET ClientLibrary in MinGW CI (Linux dotnet can't produce win-x86 AOT)

## Environment Configuration

**Development:**
- Working directory: `src/bin` or build output (assets copied post-build)
- Editor: `ENABLE_EDITOR=ON` + `_EDITOR` define for ImGui editor
- ConstantsReplacer: Windows Forms tool for SQL constant replacement (separate from main build)

**Production:**
- Release build: no editor, minimal binary
- Assets shipped alongside executable
- No env vars or secrets

## Webhooks & Callbacks

**None** - Client-only; no HTTP, webhooks, or callbacks.

---

*Integration audit: 2025-02-08*
*Update when adding/removing external services*
