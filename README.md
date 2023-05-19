# MU Online Client Sources

This is my special fork of the Season 5.2 client sorces which were [uploaded by Luois](https://github.com/LouisEmulator/Main5.2).

The ultimate goal is to clean it up and making it compatible and feature complete
to Season 6 Episode 3.

What has been done by me yet:
  * The network stack has been replaced by MUnique.OpenMU.Network, making it easier
    to apply changes. I embedded a .NET 7 runtime for that and had to adapt some
    code to allow async networking.
  * Network protocol has been adapted for season 6 episode 3 - there are probably
    still some things to do, but it connects to [OpenMU](https://github.com/MUnique/OpenMU) and is playable.
  * The frame rate has been increased to 30 fps. However, some further adaptions
    are required so that some animations (which rely on frame counting) run at
    normal speed. If that's solved, we can go even higher.
  * Meaningful changes of Qubit have been integrated, such as:
    * Rage Fighter class
    * Visual Bug when Dark Lord walks with Raven
    * Item equip with right mouse click
    * Glow for red, blue and black fenrir
    * Additional screen resolutions
    * MU Helper dialog (Not fully working yet)
  * Removed defines for ragefighter class, since we target season 6.    
  * Some minor bug fixes
    * Storm crow item labels
    * Ancient set labels

What has to be done for season 6:
  * Extended Inventory and Vault
  * The master skill tree system has to be overhauled:
    * The old one has up to 5 master levels per skill, and each was defined as
      seperate skill. The new system has one per normal skill, one per master skill
      and additionally levels which are sent to the client.
  * Lucky Items
  * Karutan Map
  * MU Helper


## Credits

  * Webzen
  * Louis
  * Qubit (tuservermu.com.ve)
  * Community members of RaGEZONE for posting fixes