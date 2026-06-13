# Windows - WSL + MinGW (cross-compile)

Cross-compile the Windows client from WSL/Linux with the MinGW-w64 toolchain.
This is a fast development loop (native ext4 I/O) and what the CI uses.

See [the build guide](../README.md) for shared concepts. For a native *Linux*
client instead, see [../linux/console.md](../linux/console.md).

> **Keep the repo on the WSL filesystem** (`/home/<user>/MuMain`), not under
> `/mnt/c/...`. The Windows-filesystem path goes through a slow translation
> layer.

## Prerequisites (one-time)

```bash
sudo apt-get update
# 64-bit target:
sudo apt-get install -y mingw-w64 g++-mingw-w64-x86-64 cmake ninja-build
# 32-bit target instead:
sudo apt-get install -y mingw-w64 g++-mingw-w64-i686 cmake ninja-build
```

You also need a **static** MinGW `libturbojpeg.a` (so the client doesn't depend
on `libturbojpeg.dll`). Build it once per arch, e.g. for x86_64:

```bash
git clone --depth 1 --branch 3.1.3 https://github.com/libjpeg-turbo/libjpeg-turbo.git _deps/libjpeg-turbo
cmake -S _deps/libjpeg-turbo -B _deps/build-turbo \
  -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_BUILD_TYPE=Release -DENABLE_SHARED=OFF -DENABLE_STATIC=ON -DWITH_SIMD=OFF \
  -DCMAKE_INSTALL_PREFIX="$PWD/_deps/mingw-x86_64"
cmake --build _deps/build-turbo -j"$(nproc)" && cmake --install _deps/build-turbo
```

(Replace `x86_64` with `i686` / `i686-w64-mingw32-gcc` for the 32-bit toolchain.)

## Configure and build

```bash
cmake -S . -B build-mingw -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_EDITOR=ON \
  -DMU_TURBOJPEG_STATIC_LIB="$PWD/_deps/mingw-x86_64/lib/libturbojpeg.a"

cmake --build build-mingw -j"$(nproc)"
```

Use `cmake/toolchains/mingw-w64-i686.cmake` and the i686 turbojpeg path for a
32-bit build.

## The network library (`.dll`)

Native AOT cannot cross-OS compile: a **Linux** `dotnet` cannot produce the
Windows `MUnique.Client.Library.dll`. Two options:

- **With Windows `dotnet.exe` via WSL interop** (`/mnt/c/Program
  Files/dotnet/dotnet.exe` on `PATH`): CMake uses it to build the real `.dll`,
  giving a connectable client.
- **With only native Linux `dotnet`:** the client compiles and links but the
  `.dll` is skipped (CMake warns); it runs but cannot connect. Build the Windows
  client on Windows, or use the [native Linux client](../linux/console.md), for
  online play.

## Run

```bash
cd build-mingw/src && ./Main.exe
```

(Runs under WSLg, or copy the `build-mingw/src` directory to Windows.)

## Tests

Install `wine` (and `wine32` for 32-bit), then:

```bash
cmake -S . -B build-mingw -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-w64-x86_64.cmake \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DMU_TURBOJPEG_STATIC_LIB="$PWD/_deps/mingw-x86_64/lib/libturbojpeg.a"
cmake --build build-mingw -j"$(nproc)"
ctest --test-dir build-mingw --output-on-failure
```
