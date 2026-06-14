# Linux - CLion

CLion builds the native Linux client with its bundled CMake and your system
GCC/Clang. For the prerequisites (GL, SDL, turbojpeg, the .NET SDK) and the
shared concepts, see [linux/console.md](console.md).

## Setup

1. **Open** the repository folder in CLion (`File > Open`).
2. **CMake profile** (`Settings > Build, Execution, Deployment > CMake`):
   - Build type: `Release` (or `Debug`).
   - CMake options:
     ```
     -DENABLE_EDITOR=ON
     ```
   - Generator: Ninja (recommended).
   - No toolchain file is needed - this is a native build.
3. **Reload** the CMake project. The `Main` target appears in the run configs.

## Run

Edit the `Main` run configuration and set the **Working directory** to the
build output's `src` folder, e.g.:

```
<build-dir>/src
```

so the client finds its assets, `config.ini`, and
`MUnique.Client.Library.so`. CLion's default build directory is
`cmake-build-release/` (or `-debug`) inside the project.

If GL is slow under WSLg, add `MESA_LOADER_DRIVER_OVERRIDE=d3d12` to the run
configuration's environment variables (see [console.md](console.md)).

## Tests

Set `-DBUILD_TESTING=ON` in the CMake options; CLion discovers the CTest cases
and lets you run them from the IDE.
