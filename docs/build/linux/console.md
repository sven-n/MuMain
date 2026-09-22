# Linux - Terminal

Native 64-bit Linux build from the command line. Produces a full, playable
client: the engine, the `linux-x64` network library, SDL/OpenGL rendering.

See [the build guide](../README.md) for shared concepts (editor flag, the
network library, output layout).

## Prerequisites (one-time)

```bash
# Build tools and Native AOT prerequisites
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build pkg-config \
  clang zlib1g-dev

# OpenGL / GLEW / GLU / turbojpeg
sudo apt-get install -y \
  libgl1-mesa-dev libglu1-mesa-dev libglew-dev libegl1-mesa-dev \
  libturbojpeg0-dev

# SDL3 build dependencies (X11 + Wayland + audio)
sudo apt-get install -y \
  libx11-dev libxext-dev libxrandr-dev libxi-dev libxcursor-dev \
  libxfixes-dev libxrender-dev libxss-dev libxtst-dev libxkbcommon-dev \
  libdrm-dev libgbm-dev \
  libwayland-dev libdecor-0-dev wayland-protocols \
  libasound2-dev libpulse-dev libdbus-1-dev libudev-dev
```

The .NET 10 SDK builds the `linux-x64` network library. Install it natively
(not a Windows `dotnet.exe`):

```bash
curl -sSL https://dot.net/v1/dotnet-install.sh | bash -s -- --channel 10.0 --install-dir "$HOME/.dotnet"
export PATH="$HOME/.dotnet:$PATH" DOTNET_ROOT="$HOME/.dotnet"
```

(Or use your distro's `dotnet-sdk-10.0` package. Add the `export` line to your
shell profile to make it permanent.)

## Configure and build

```bash
cmake -S . -B build-linux -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_EDITOR=ON

cmake --build build-linux -j"$(nproc)"
```

This builds the engine, the `linux-x64` `MUnique.Client.Library.so`, and copies
the assets and the library next to the executable. Drop `-DENABLE_EDITOR=ON`
(or set it `OFF`) for a player build. Add `-DENABLE_CONTROL_SOCKET=ON` when
test scripts need to drive the client (see `docs/control-socket.md`); it is
`OFF` by default and must stay off in anything handed to players.

### Using the presets

`CMakePresets.json` carries the same two configurations, with a
`Ninja Multi-Config` generator and `CMAKE_EXPORT_COMPILE_COMMANDS=ON`:

```bash
# Configure (pick one)
cmake --preset linux-x64                  # player build: ENABLE_EDITOR=OFF, ENABLE_CONTROL_SOCKET=OFF
cmake --preset linux-x64-mueditor         # developer build: ENABLE_EDITOR=ON, ENABLE_CONTROL_SOCKET=ON

# Build (pick the matching Debug/Release build preset)
cmake --build --preset linux-x64-mueditor-debug
cmake --build --preset linux-x64-mueditor-release
```

Each preset builds into `out/build/<preset-name>/`, so the two configurations
never share a build tree. The plain preset is the player configuration: it
contains neither the editor nor the control socket, and the `control_socket_leak`
test in its test suite proves the latter. To get the socket without the editor,
configure by hand: `cmake --preset linux-x64 -DENABLE_CONTROL_SOCKET=ON`.

Because `src/MuEditor/` is only added to the target when `ENABLE_EDITOR=ON`, the
`linux-x64` compile database contains no editor translation units and clangd
cannot index them. Configuring `linux-x64-mueditor` once produces an
`out/build/linux-x64-mueditor/compile_commands.json` that covers both the client
and the editor (and the control socket, which is also only compiled when
enabled); point your editor's clangd at it with
`--compile-commands-dir=out/build/linux-x64-mueditor`. Configuring is enough for
indexing — you do not have to build that tree.

If the packaged runtime has no global library path for SDL's video backends or the Vulkan loader (the client stops with `SDL video init failed` and an SDL error such as `wayland not available`), configure with `-DMU_LINK_SDL_PLATFORM_BACKENDS=ON`: SDL then links those libraries at build time and `Main` records their `RUNPATH`.

## Run

```bash
cd build-linux/src
./Main
```

Run from `build-linux/src` so the client finds its assets, `config.ini`, and
`MUnique.Client.Library.so` (loaded from the working directory). Set the server
in `config.ini` (`ServerIP` / `ServerPort`).

### Hardware OpenGL under WSLg

If you build inside WSL, the default GL driver is Mesa software rendering
(`llvmpipe`), which is slow. Force the WSLg GPU passthrough driver for hardware
acceleration:

```bash
MESA_LOADER_DRIVER_OVERRIDE=d3d12 ./Main
```

## Reporting a Wayland problem

From the repository root, run `scripts/wayland-diag.sh --timeout 60 --`.
It writes a sanitized `summary.txt` under `diag/`; paste that into the report
and attach only relevant raw-log excerpts. A pre-existing `MuError.log` is
copied into the bundle as `MuError.log.previous` before the fresh run. `--tar`
creates a local archive and does not upload it.

## Tests

```bash
cmake -S . -B build-linux -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-linux -j"$(nproc)"
ctest --test-dir build-linux --output-on-failure
```

They run natively (no emulation). With `ENABLE_EDITOR=OFF` this includes the
`editor_leak` guard, which fails the build if any `src/MuEditor/` source was
compiled into the non-editor client.
