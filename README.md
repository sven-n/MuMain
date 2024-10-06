# MU Online Client Sources

This is my special fork of the Season 5.2 client sources [uploaded by Luois](https://github.com/LouisEmulator/Main5.2).

The ultimate goal is to clean it up and make it compatible and feature complete
to Season 6 Episode 3.

What I have done so far:
  * 🔥 The framerate has been increased to 60 fps and can be adjusted with the chat
    command `$fps <value>`. The options menu includes a checkbox to reduce effects.
  * 🔥 Optimized some OpenGL calls by using vertex arrays. This should result in
    a better frame rate when many players and objects are visible.
  * 🔥 Added inventory and vault extensions.
  * 🔥 Unicode support: The client works with UTF-16LE instead of ANSI in memory.
    All strings and char arrays have been changed to use wide characters.
    Strings coming from files and the network are handled as UTF-8.
  * 🔥 Replaced the network stack with MUnique.OpenMU.Network to make it easier to
    apply changes. The main.exe hosts a .NET 8 runtime for this and allow for async
    networking.
  * 🔥 The network protocol has been adapted for Season 6 Episode 3 - there is probably
    still some work to do, but it connects to [OpenMU](https://github.com/MUnique/OpenMU)
    and is playable. Additionally, the protocol has been extended so it's not standard
    anymore.
    * Damage, Exp etc. can exceed 16 bit now.
    * Improved item serialization
    * Improved appearance serialization
    * Added monster health status bar after attack
  * 🔥 Significant changes from Qubit have been incorporated, such as
    * Rage Fighter class
    * Visual bug when Dark Lord walks with Raven
    * Item equipping with right mouse click
    * Glow for red, blue and black fenrir
    * Additional screen resolutions
    * MU helper dialog (not fully functional yet)
  * Removed if-defs for Rage Fighter class as we are targeting Season 6, so Rage
    Fighter should always be included.
  * Some minor bug fixes, e.g.:
    * Storm Crow item labels
    * Ancient set labels
  * The code has been refactored. A lot of magic values have been replaced by
    enums and constants.

What needs to be done for Season 6:
  * The master skill tree system needs to be overhauled:
    * The old one has up to 5 master levels per skill, and each was a separate skill.
    * The new system has one per normal skill, one per master skill, and additional
      levels that are sent to the client.
  * Lucky Items
  * MU Helper logic

## How to build & run

It requires:
  * [.NET 8 x86 runtime](https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-8.0.0-windows-x86-installer) to be installed.
  * Visual Studio 2022 with the newest update, workloads for C++ and C#
  * A compatible server, e.g. [OpenMU](https://github.com/MUnique/OpenMU)

Because of the integrated .NET code, you need to publish the ManagedLibrary first
to the debug output folder of the main.exe, so that all required managed DLLs are
copied. A simple build is not enough in this case, however the publish just needs
to be done once.

It supports the common starting parameters `/u` and `/p`, example: `main.exe connect /u192.168.0.20 /p55902`.
The [OpenMU launcher](https://github.com/MUnique/OpenMU/releases/download/v0.8.17/MUnique.OpenMU.ClientLauncher_0.8.17.zip)
will work as well. By default, it connects to localhost and port `44406`.
The client identifies itself with Version `2.04d` and serial `k1Pk2jcET48mxL3b`.

## Credits

  * Webzen
  * Louis
  * Qubit (tuservermu.com.ve)
  * Community members of RaGEZONE and tuservermu.com.ve for posting fixes
