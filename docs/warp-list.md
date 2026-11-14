# Warp list

The warp window (M) and the `/warp <map>` chat command list the warps of
`Data/Local/<lang>/movereq_<lang>.bmd`, which follows Webzen's Season 6.

OpenMU sends its own warp list when the character enters the game (the
*WarpListExtended* message, only to this client). The warp window and `/warp`
use that list then:

- a warp which is also in the data files keeps its translated name and its
  place in the list, with the level and zen of the server;
- a warp which only the server has (like Crywolf or Loren Market on OpenMU) is
  added after them, with the name the server sends in the language of the
  account;
- a warp which only the data files have isn't shown, because the server can't
  warp there.

The level requirement is reduced for Magic Gladiators, Dark Lords and Rage
Fighters in both cases, as before.

The list of the server applies until the client leaves it (back to the
character list, to the login screen, or to another game server); then the list
of the data files is shown again until the next server sends its own. A server
which doesn't send a warp list, like Webzen's, gets the list of the data files.
