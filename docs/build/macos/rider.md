# macOS - Rider

Rider can open the project two ways, because this repository is both a C++
client and a C# library:

- **C++ client** (`Main`): Rider opens the CMake project using the same engine
  as CLion. Configuration is identical to [macos/clion.md](clion.md) - set
  `-DENABLE_EDITOR=ON` and `CMAKE_PREFIX_PATH` for Homebrew, native toolchain,
  and point the run configuration's working directory at `<build-dir>/src`.
- **C# network library** (`ClientLibrary/MUnique.Client.Library.csproj`): open
  the `.csproj` to edit, refactor, or debug the managed networking code. Rider
  is the most comfortable editor for that part.

For the system prerequisites (Homebrew GLEW/jpeg-turbo, the .NET SDK) and how
to run, see [macos/console.md](console.md).

> Note: the C# library is normally built by the CMake build via Native AOT, not
> by Rider. Build/run the *client* through the CMake `Main` target; use Rider on
> the `.csproj` for editing and analysis of the managed code.
