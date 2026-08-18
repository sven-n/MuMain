# macOS - Terminal

Native macOS build from the command line. Produces a full, playable client: the
engine, the `osx-arm64` or `osx-x64` network library (matching the host),
SDL/OpenGL rendering.

See [the build guide](../README.md) for shared concepts (editor flag, the
network library, output layout). No toolchain file and no CMake presets: this
is a native host build, same as Linux.

## Prerequisites (one-time)

Install Xcode Command Line Tools, then Homebrew packages:

```bash
xcode-select --install

brew install cmake ninja pkg-config glew jpeg-turbo
```

`jpeg-turbo` is keg-only. Pass its prefix to CMake (see below) so
`find_library(turbojpeg)` succeeds.

The .NET 10 SDK builds the `osx-arm64` / `osx-x64` network library. Install it
natively:

```bash
curl -sSL https://dot.net/v1/dotnet-install.sh | bash -s -- --channel 10.0 --install-dir "$HOME/.dotnet"
export PATH="$HOME/.dotnet:$PATH" DOTNET_ROOT="$HOME/.dotnet"
```

(Or use the installer from https://dotnet.microsoft.com. Add the `export` line
to your shell profile to make it permanent.)

Initialize git submodules from the repository root if CMake has not already:

```bash
git submodule update --init
```

## Configure and build

```bash
cmake -S . -B build-macos -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_EDITOR=ON \
  -DCMAKE_PREFIX_PATH="$(brew --prefix);$(brew --prefix jpeg-turbo);$(brew --prefix glew)"

cmake --build build-macos -j"$(sysctl -n hw.ncpu)"
```

This builds the engine, the host-arch `MUnique.Client.Library.dylib`, and
copies the assets and the library next to the executable. Drop
`-DENABLE_EDITOR=ON` (or set it `OFF`) for a player build.

Apple Silicon gets `osx-arm64`; Intel Macs get `osx-x64`. Universal binaries
are not produced.

## Run

```bash
cd build-macos/src
./Main
```

Run from `build-macos/src` so the client finds its assets, `config.ini`, and
`MUnique.Client.Library.dylib`. Set the server in `config.ini` (`ServerIP` /
`ServerPort`), or pass `connect /u<IP> /p<PORT>`.

OpenGL on macOS is deprecated and tops out at 4.1 Core. The client requests
4.5, then 4.3, then 4.1, then 3.3.

## Tests

```bash
cmake -S . -B build-macos -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=ON \
  -DCMAKE_PREFIX_PATH="$(brew --prefix);$(brew --prefix jpeg-turbo);$(brew --prefix glew)"
cmake --build build-macos -j"$(sysctl -n hw.ncpu)"
ctest --test-dir build-macos --output-on-failure
```

They run natively (no emulation). With `ENABLE_EDITOR=OFF` this includes the
`editor_leak` guard, which fails the build if any `src/MuEditor/` source was
compiled into the non-editor client.
