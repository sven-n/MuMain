#pragma once

#ifdef _EDITOR

// The map that is active while the game loads a model or a texture, for the
// editor's record of what loaded an asset (CLoadData, CGlobalBitmap). The
// map code is not linked into every test, so the editor sets where the active
// map comes from; until it does, every asset counts as loaded on the loading
// screen.
namespace Core::AssetLoadWorld
{
// The world of the loading screen: WorldActive before any map.
constexpr int LoadingScreen = -1;

using Source = int (*)();

void SetSource(Source source);

// The active world, or LoadingScreen without a source.
int Get();
} // namespace Core::AssetLoadWorld

#endif // _EDITOR
