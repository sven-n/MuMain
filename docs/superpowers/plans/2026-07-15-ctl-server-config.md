# ctl Server Configuration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `./ctl run` translate ignored `.env` server settings into Main's existing `/u` and `/p` CLI arguments.

**Architecture:** Keep configuration in workspace-root `.env`. Parse only `MUMAIN_SERVER_IP` and `MUMAIN_SERVER_PORT` without sourcing shell code. Explicit CLI arguments override configured values.

**Tech Stack:** Bash, CMake-managed native executable, temporary fake executable integration check.

---

### Task 1: Launcher Configuration

**Files:**
- Modify: `ctl`
- Test: temporary shell fixture under `/tmp`

- [x] Add safe `.env` parsing for the two supported keys.
- [x] Validate effective IP and port before launch.
- [x] Append `/u<ip>` and `/p<port>` only when not explicitly supplied.
- [x] Run `bash -n ctl`.
- [x] Run fake-`Main` checks for defaults, overrides, missing file, and invalid port.
- [x] Run `./ctl build` to preserve build and staging behavior.
