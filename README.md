# MU Online Client Sources

This is my special fork of the Season 5.2 client sources [uploaded by Luois](https://github.com/LouisEmulator/Main5.2).

The ultimate goal is to clean it up and make it compatible and feature complete
to Season 6 Episode 3.

What I have done so far:
  * Replaced the network stack with MUnique.OpenMU.Network to make it easier to
    apply changes. I included a .NET 7 runtime for this and had to change some
    code to allow async networking.
  * The network protocol has been adapted for Season 6 Episode 3 - there is probably
    still some work to do, but it connects to [OpenMU](https://github.com/MUnique/OpenMU)
    and is playable.
  * The framerate has been increased to 30 fps. However, some further adjustments
    so that some animations (which rely on frame counting) can run at normal speed.
    Once that's fixed, we can go even higher.
  * Significant changes from Qubit have been incorporated, such as
    * Rage Fighter class
    * Visual bug when Dark Lord walks with Raven
    * Item equipping with right mouse click
    * Glow for red, blue and black fenrir
    * Additional screen resolutions
    * MU helper dialog (not fully functional yet)
  * Removed definitions for Rage Fighter class as we are targeting Season 6.    
  * Some minor bug fixes
    * Storm Crow item labels
    * Ancient set labels

What needs to be done for Season 6:
  * Inventory and Vault extensions
  * The master skill tree system needs to be overhauled:
    * The old one has up to 5 master levels per skill, and each was a separate skill.
    * The new system has one per normal skill, one per master skill, and additional
      levels that are sent to the client.
  * Lucky Items
  * Karutan Map
  * MU Helper

## How to build & run

Because of the integrated .NET code, you need to publish the ManagedLibrary first
to the debug output folder of the main.exe, so that all required managed DLLs are
copied. A simple build is not enough in this case, however the publish just needs
to be done once.

## Credits

  * Webzen
  * Louis
  * Qubit (tuservermu.com.ve)
  * Community members of RaGEZONE for posting fixes