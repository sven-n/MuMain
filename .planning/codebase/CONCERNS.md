# Codebase Concerns

**Analysis Date:** 2025-02-08

## Tech Debt

**Generated C++/C# from XSLT:**
- Issue: Packet bindings and functions generated from NuGet XML; manual edits can be overwritten
- Files: `src/source/Dotnet/PacketFunctions_*.cpp`, `PacketBindings_*.h`, `ClientLibrary/ConnectionManager.*Functions.cs`
- Why: MUnique.OpenMU.Network.Packets provides single source of truth
- Impact: Custom changes must go in `*Custom.*` or extension files
- Fix approach: Keep custom logic in `PacketFunctions_Custom.cpp`, `ConnectionManager.*.Custom.cs`; document extension points

**PreBuild XSLT on non-CI:**
- Issue: ConstantsReplacer and ClientLibrary XSLT runs only when `ci` is not set
- File: `ClientLibrary/MUnique.Client.Library.csproj` PreBuild target
- Why: CI uses pre-generated files; local dev regenerates
- Impact: First build or packet XML changes require local XSLT run
- Fix approach: Ensure CI commits generated files or document regeneration steps

**Legacy MU Online structure:**
- Issue: Many `GM*`, `CS*` files from original Season 5.2 fork; mixed patterns
- Files: `src/source/` (GM*.cpp, CS*.cpp, etc.)
- Why: Gradual migration from Louis/Qubit sources
- Impact: Inconsistent style, some magic values replaced by enums, some remain
- Fix approach: Continue refactoring toward constants/enums; document patterns in CONVENTIONS.md

## Known Gaps (from README)

**Lucky Items:**
- Problem: Season 6 Lucky Items not implemented
- Current workaround: N/A
- Blocks: Full Season 6 Episode 3 compatibility
- Implementation complexity: Medium (item logic, visuals)

**Linux build:**
- Problem: README notes "Linux build !Not Working Yet!"
- File: `README.md` (Option 5)
- Current workaround: Use WSL + MinGW for cross-compilation
- Blocks: Native Linux client
- Implementation complexity: Unknown (OpenGL/Win32 dependencies)

## Security Considerations

**Network protocol:**
- Risk: Custom protocol; no TLS to game server (encryption via SimpleModulus/XOR)
- Current mitigation: Server-side validation; client trusts server
- Recommendations: Document protocol assumptions; consider TLS if exposed over public networks

**No auth provider:**
- Risk: Credentials sent in plaintext over custom protocol
- Current mitigation: Same as original MU Online design
- Recommendations: If extending, consider hashed/auth flows

## Performance Bottlenecks

**Vertex arrays:**
- README notes optimization via vertex arrays for many visible objects
- No specific measurements in codebase

**Frame rate:**
- V-Sync default; 60 fps fallback; `$fps`, `$vsync` chat commands for tuning
- "Reduce effects" option for higher FPS

## Fragile Areas

**Dotnet bridge (Connection.h):**
- File: `src/source/Dotnet/Connection.h`
- Why fragile: LoadLibrary at global scope; symbol lookup at runtime
- Common failures: Missing DLL, wrong architecture, AOT mismatch
- Safe modification: Ensure ClientLibrary targets same arch (x86/x64) as Main
- Test coverage: None

**XSLT dependency:**
- Files: ClientLibrary PreBuild, `Generate*.xslt`
- Why fragile: NuGet package version drives XML paths; CI skips PreBuild
- Common failures: Package upgrade changes paths; generated files out of sync
- Safe modification: Pin MUnique.OpenMU.Network.Packets version; test regeneration locally

## Dependencies at Risk

**MUnique.OpenMU.Network.Packets 0.9.8:**
- Risk: External package; version upgrade may change XML structure
- Impact: XSLT may break; generated code invalid
- Migration plan: Test upgrades in branch; extend XSLT if schema changes

**ImGui (submodule):**
- Risk: Submodule can drift; API changes
- Impact: MuEditor build breaks
- Migration plan: Pin submodule ref; test ImGui upgrades

## Test Coverage Gaps

**All areas:**
- What's not tested: Entire codebase
- Risk: Regressions undetected; refactoring risky
- Priority: High for network, data loading; Medium for UI
- Difficulty: C++ game client needs harness; .NET easier to unit test

---

*Concerns audit: 2025-02-08*
*Update as issues are fixed or new ones discovered*
