#pragma once

#ifdef _EDITOR

// Tells the editor that the game is about to clear the objects and effect
// pools of the world (CMapManager::DeleteObjects: a map change, a reload of
// the map, a scene change), while the pools still hold their objects. The
// map code does not know the editor, so the editor sets the listener.
namespace Core::WorldClearing
{
using Listener = void (*)();

void SetListener(Listener listener);

// Called by the map code before it clears the pools.
void Notify();
} // namespace Core::WorldClearing

#endif // _EDITOR
