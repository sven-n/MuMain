# Linux - VS Code

Build the native Linux client with the CMake Tools extension. For the
prerequisites (GL, SDL, turbojpeg, the .NET SDK) and shared concepts, see
[linux/console.md](console.md).

## Setup

1. Install the **CMake Tools** and **C/C++** extensions.
2. Open the repository folder.
3. Add the configure options in `.vscode/settings.json`:
   ```json
   {
     "cmake.generator": "Ninja",
     "cmake.configureSettings": {
       "CMAKE_BUILD_TYPE": "Release",
       "ENABLE_EDITOR": "ON"
     }
   }
   ```
4. Run **CMake: Configure**, then **CMake: Build** (or the build button on the
   status bar). Pick a Release/Debug GCC or Clang kit when prompted.

## Run

The simplest way to get the right working directory (so the client finds its
assets and `MUnique.Client.Library.so`) is to launch from the terminal:

```bash
cd build/src && ./Main
```

To run/debug from the editor, add a `launch.json` entry whose `cwd` is the
build output's `src` directory and `program` is `.../src/Main`. Add
`MESA_LOADER_DRIVER_OVERRIDE=d3d12` to `environment` if GL is slow under WSLg.

## Tests

Set `"BUILD_TESTING": "ON"` in `cmake.configureSettings`; the CMake Tools test
explorer runs the CTest cases.
