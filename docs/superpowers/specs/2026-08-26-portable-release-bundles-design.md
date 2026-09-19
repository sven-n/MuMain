# Portable Release Bundles Design

## Goal

Make packaged clients resolve the network library beside the running
executable. Publish Release bundles on every platform. Keep the no-data bundle
as the normal distribution for users who provide their own assets.

## Confirmed behavior

- The Linux executable currently embeds the GitHub Actions build directory
  through `MU_DOTNET_LIB_DIR`, so moving the bundle breaks network-library
  loading.
- The standalone Linux workflow already builds Release. The main CI workflow
  uploads Debug Linux and macOS bundles.
- Linux treats backslashes as ordinary characters. Existing POSIX file shims
  normalize legacy Windows asset paths before opening files.
- No-data bundles intentionally exclude only `Data/` and `fonts/`. They require
  users to add compatible copies before running the client.

## Design

Remove the build-directory compile definition. Construct the managed-library
path from the running executable path at process startup, then append the
platform library name. Keep the existing bare-name behavior on Windows. A
failed executable-path lookup falls back to the bare library name and the
existing loader error handling.

Change the main CI Linux and macOS jobs to build, test, validate, and upload
Release outputs. Keep their existing editor matrix and both data/no-data
artifact variants. The standalone Linux and Windows workflows remain Release.

Document that the data bundle contains the full runtime directory, while the
no-data bundle is the redistributable runtime requiring user-supplied
`Data/` and `fonts/`.

## Verification

- A focused loader-path test proves the path is derived from the test
  executable directory rather than the CMake build directory.
- Workflow contracts require Release paths and artifact names for uploaded
  Linux and macOS bundles.
- Existing CTest suites, workflow contract, JSON validation, and diff checks
  remain green.
- Hosted verification checks that uploaded binaries contain no runner build
  path after the commit is pushed.
