# Roadmap: MuMain

## Overview

This roadmap guides MuMain from its current state toward full Season 6 Episode 3 compatibility. Phases focus on build verification, known gaps (e.g., Lucky Items), and goals you define.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

- [ ] **Phase 1: Setup & verification** — Confirm build and runtime on Windows 11 with CMakePresets + Ninja
- [ ] **Phase 2: Lucky Items** — Implement Season 6 Lucky Items (README gap)
- [ ] **Phase 3: [TBD]** — User-defined goals

## Phase Details

### Phase 1: Setup & verification
**Goal**: Verify the project builds and runs correctly on Windows 11 using CMakePresets + Ninja
**Depends on**: Nothing
**Requirements**: Build pipeline
**Success Criteria**:
  1. `cmake --preset windows-x86` configures successfully
  2. `cmake --build --preset windows-x86-debug` produces Main.exe
  3. Client connects to OpenMU and launches without crash
**Plans**: 1 plan

Plans:
- [ ] 01-01: Configure and build x86/x64 Release + MuEditor; manual verify game launches and connects

### Phase 2: Lucky Items
**Goal**: Implement Season 6 Lucky Items feature
**Depends on**: Phase 1
**Requirements**: Full Season 6 Episode 3 compatibility
**Success Criteria**:
  1. Lucky item logic implemented
  2. Visuals/UI for lucky items
  3. Compatible with OpenMU
**Plans**: TBD

### Phase 3: [TBD]
**Goal**: User-defined
**Depends on**: Phase 2
**Plans**: TBD

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Setup & verification | 0/1 | Not started | - |
| 2. Lucky Items | 0/? | Not started | - |
| 3. TBD | 0/? | Not started | - |
