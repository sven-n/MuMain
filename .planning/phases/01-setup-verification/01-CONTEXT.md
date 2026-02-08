# Phase 1: Setup & verification - Context

**Gathered:** 2025-02-08
**Status:** Ready for planning

## Phase Boundary

Verify the project builds and runs on Windows 11 using CMakePresets + Ninja. Scope: configure, compile, and manually verify the client launches and connects to OpenMU.

## Implementation Decisions

### Build Configurations
- **x86 Release** — `cmake --build --preset windows-x86-release`
- **x64 Release** — `cmake --build --preset windows-x64-release`
- **MuEditor** — MuEditor variant (x86 and/or x64) to verify in-game editor builds. Use `windows-x86-mueditor-release` and `windows-x64-mueditor-release`.

### Verification Depth
- **Manual verification** — User verifies game launches after build
- No automated smoke test or CI runtime check; human confirms client runs and connects

### OpenMU / Server
- **Server available** — Running locally on Arch Linux VM at `192.168.3.34`
- Connection: `Main.exe connect /u192.168.3.34 /p44406` (or applicable port)
- Assume server is reachable for manual verification

### Claude's Discretion
- Documentation updates — Plan can include updating `docs/build-guide.md` or README if findings warrant it

## Specific Ideas

- Server IP documented for connection testing: `192.168.3.34`

## Deferred Ideas

None — discussion stayed within phase scope.

---

*Phase: 01-setup-verification*
*Context gathered: 2025-02-08*
