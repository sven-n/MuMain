# macOS - CLion

CLion builds the native macOS client with its bundled CMake and Apple Clang.
For the prerequisites (Homebrew GLEW/jpeg-turbo, the .NET SDK) and the shared
concepts, see [macos/console.md](console.md).

## Setup

1. **Open** the repository folder in CLion (`File > Open`).
2. **CMake profile** (`Settings > Build, Execution, Deployment > CMake`):
   - Build type: `Release` (or `Debug`).
   - CMake options:
     ```
     -DENABLE_EDITOR=ON
     -DCMAKE_PREFIX_PATH=<brew-prefix>;<brew-prefix>/opt/jpeg-turbo;<brew-prefix>/opt/glew
     ```
     Replace `<brew-prefix>` with `$(brew --prefix)` (typically
     `/opt/homebrew` on Apple Silicon, `/usr/local` on Intel).
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
`MUnique.Client.Library.dylib`. CLion's default build directory is
`cmake-build-release/` (or `-debug`) inside the project.

## Tests

Set `-DBUILD_TESTING=ON` in the CMake options; CLion discovers the CTest cases
and lets you run them from the IDE.
