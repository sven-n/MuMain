# macOS - VS Code

Build the native macOS client with the CMake Tools extension. For the
prerequisites (Homebrew GLEW/jpeg-turbo, the .NET SDK) and shared concepts, see
[macos/console.md](console.md).

## Setup

1. Install the **CMake Tools** and **C/C++** extensions.
2. Open the repository folder.
3. Add the configure options in `.vscode/settings.json`. Use the Homebrew
   prefix for your machine (`/opt/homebrew` on Apple Silicon, `/usr/local` on
   Intel):
   ```json
   {
     "cmake.generator": "Ninja",
     "cmake.configureSettings": {
       "CMAKE_BUILD_TYPE": "Release",
       "ENABLE_EDITOR": "ON",
       "CMAKE_PREFIX_PATH": "/opt/homebrew;/opt/homebrew/opt/jpeg-turbo;/opt/homebrew/opt/glew"
     }
   }
   ```
4. Run **CMake: Configure**, then **CMake: Build** (or the build button on the
   status bar). Pick a Release/Debug Clang kit when prompted.

## Run

The simplest way to get the right working directory (so the client finds its
assets and `MUnique.Client.Library.dylib`) is to launch from the terminal:

```bash
cd build-macos/src && ./Main
```

To run/debug from the editor, add a `launch.json` entry whose `cwd` is the
build output's `src` directory and `program` is `.../src/Main`.

## Tests

Set `"BUILD_TESTING": "ON"` in `cmake.configureSettings`; the CMake Tools test
explorer runs the CTest cases.
