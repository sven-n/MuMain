# MU Online Client Sources

This is my special fork of the Season 5.2 client sources [uploaded by Luois](https://github.com/LouisEmulator/Main5.2).

The ultimate goal is to clean it up and make it compatible and feature complete
to Season 6 Episode 3.

What I have done so far:
  * 🔥 The framerate has been increased.
    * By default, it uses V-Sync without fps limit. If V-Sync is not
    available, it limits to 60 fps.
    * The options menu includes a checkbox to reduce effects to achieve higher frame rates.
    * Chat commands:
      * Change FPS-Limit: `$fps <value>`
      * Enable V-Sync: `$vsync on`
      * Disable V-Sync: `$vsync off`
  * 🔥 Optimized some OpenGL calls by using vertex arrays. This should result in
    a better frame rate when many players and objects are visible.
  * 🔥 Added inventory and vault extensions.
  * 🔥 The master skill tree system was upgraded to Season 6
  * 🔥 Unicode support: The client works with UTF-16LE instead of ANSI in memory.
    All strings and char arrays have been changed to use wide characters.
    Strings coming from files and the network are handled as UTF-8.
  * 🔥 Replaced the network stack with MUnique.OpenMU.Network to make it easier to
    apply changes. This repository includes a C# .NET 9 client library which is built
    with Native AOT.
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
  * 🔥 Incorporated MU Helper UI and logic - there's some work to do but core functionality is usable
  * Removed if-defs for Rage Fighter class as we are targeting Season 6, so Rage
    Fighter should always be included.
  * Some minor bug fixes, e.g.:
    * Storm Crow item labels
    * Ancient set labels
  * The code has been refactored. A lot of magic values have been replaced by
    enums and constants.

What needs to be done for Season 6:
  * Lucky Items

## How to build & run

It requires:
  * Visual Studio 2022 with the newest update, workloads for C++ and C#
  * A compatible server: [OpenMU](https://github.com/MUnique/OpenMU).

Because of the integrated C# code, you need to publish the ManagedLibrary first
to the debug output folder of the main.exe, so that the DLL is built with Native AOT.
A simple build is not enough in this case, however the publish just needs to be done once.

It supports the common starting parameters `/u` and `/p`, example: `main.exe connect /u192.168.0.20 /p55902`.
The [OpenMU launcher](https://github.com/MUnique/OpenMU/releases/download/v0.8.17/MUnique.OpenMU.ClientLauncher_0.8.17.zip)
will work as well. By default, it connects to localhost and port `44406`.
The client identifies itself with Version `2.04d` and serial `k1Pk2jcET48mxL3b`.

## Credits

  * Webzen
  * Louis
  * Qubit (tuservermu.com.ve)
  * Community members of RaGEZONE and tuservermu.com.ve for posting fixes
  * [Nitoy](https://github.com/nitoygo) for the MU Helper