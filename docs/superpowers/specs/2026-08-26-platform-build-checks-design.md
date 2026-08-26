# Platform Build Checks Design

## Goal

Restore Linux and macOS CI coverage without restoring build matrices or
duplicated release artifacts.

## Workflow

`.github/workflows/ci.yml` keeps five jobs in this order:

1. `quality`
2. `build-windows`
3. `build-linux`
4. `build-macos`
5. `release`

Linux builds x64 Release with the editor disabled. macOS builds arm64 Release
with the editor disabled. Both jobs configure, build, run tests, and validate
the runtime output. Neither job uploads artifacts.

Windows remains the only uploaded runtime. Semantic Release waits for all
three platform checks, then publishes the Windows no-data archive.

The standalone Linux and MinGW matrix workflows remain deleted. Developers
build other architectures, configurations, and editor variants locally.

## Performance

Each hosted platform builds once. Dependency caches remain enabled. Data-only
pushes continue to bypass code CI and use the independent data-assets workflow.

## Verification

The workflow contract checks exact job order, fixed platform configurations,
absence of matrices and non-Windows uploads, release dependencies, and updated
documentation. YAML, JSON, CMake asset tests, and whitespace checks remain
required.
