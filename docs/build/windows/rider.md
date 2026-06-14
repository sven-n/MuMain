# Windows - Rider

Like on Linux, this repo is both a C++ client and a C# library, and Rider is
useful for both:

- **C++ client** (`Main`): Rider opens the CMake project (and reads
  `CMakePresets.json`). Configure exactly as in
  [windows/clion.md](clion.md) - select a preset or pass
  `-DCMAKE_TOOLCHAIN_FILE=.../toolchain-x64.cmake -DENABLE_EDITOR=ON`, build with
  MSVC, and point the run configuration's working directory at the build
  output's `src/<config>` folder.
- **C# network library** (`ClientLibrary/MUnique.Client.Library.csproj`): Rider
  is the most comfortable editor for the managed networking code - editing,
  refactoring, analysis, and debugging.

For shared concepts and the preset list, see [the build guide](../README.md)
and [windows/visual-studio.md](visual-studio.md).

> The C# library is built by the CMake build via Native AOT, not by Rider.
> Build/run the *client* through the CMake `Main` target; use Rider on the
> `.csproj` for the managed code itself.
