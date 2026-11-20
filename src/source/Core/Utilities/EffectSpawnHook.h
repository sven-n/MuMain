#pragma once

#ifdef _EDITOR

// Lets the editor create things in the game's effect lists inside the game's update: after the sprites of the last
// frame are cleared and the world's effects moved and spawned theirs, before the world is culled and drawn (design
// section 7a). The Style Editor's views create their items' things on bones here. The scene code does not know the
// editor, so the editor sets the listener.
namespace Core::EffectSpawnHook
{
using Listener = void (*)();

void SetListener(Listener listener);

// Called by the main scene's update once a frame.
void Notify();
} // namespace Core::EffectSpawnHook

#endif // _EDITOR
