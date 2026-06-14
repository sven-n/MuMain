# Linux - Rider

Rider can open the project two ways, because this repository is both a C++
client and a C# library:

- **C++ client** (`Main`): Rider opens the CMake project using the same engine
  as CLion. Configuration is identical to [linux/clion.md](clion.md) - set
  `-DENABLE_EDITOR=ON`, native toolchain, and point the run configuration's
  working directory at `<build-dir>/src`.
- **C# network library** (`ClientLibrary/MUnique.Client.Library.csproj`): open
  the `.csproj` to edit, refactor, or debug the managed networking code. Rider
  is the most comfortable editor for that part.

For the system prerequisites (GL, SDL, turbojpeg, the .NET SDK) and how to run,
see [linux/console.md](console.md).

> Note: the C# library is normally built by the CMake build via Native AOT, not
> by Rider. Build/run the *client* through the CMake `Main` target; use Rider on
> the `.csproj` for editing and analysis of the managed code.
