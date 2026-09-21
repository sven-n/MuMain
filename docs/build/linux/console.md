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
(or set it `OFF`) for a player build.

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

### Wayland

Wayland compositors decide where windows go, so changing the resolution in
windowed mode resizes the client but does not re-center it: drag it where you
want it, or restart the client, which saves the chosen resolution and lets the
compositor place the new window. On COSMIC the window can also snap back to the
previous size after you switch to another window; that is an SDL bug tracked in
[#607](https://github.com/sven-n/MuMain/issues/607), and the same restart works
around it.

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
