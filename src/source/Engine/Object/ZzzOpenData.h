#pragma once

#include <functional>
#include <vector>

void DeleteNpcs();
void OpenNpc(int Type);
void DeleteMonsters();
void OpenMonsterModel(EMonsterModelType Type);

// The startup data load, as steps for the title screen to run across frames.
std::vector<std::function<void()>> OpenBasicDataSteps();
void ReleaseMainData();
bool OpenFont();
void OpenLogoSceneData();
void ReleaseLogoSceneData();
void OpenCharacterSceneData();
void ReleaseCharacterSceneData();

void OpenPlayerTextures();
void OpenItemTextures();
// The textures of the effects and the looks in code, at their BITMAP_ indices (OpenItemTextures loads them).
void OpenEffectTextures();
// Makes mesh 2 of the Grand Soul pants' model the grid their cloth moves
// (OpenItems once the item models are open; the item editor again after their
// slot took other data).
void PrepareGrandSoulPantsClothMesh();

void SaveOptions();
// SaveOptions, once the character's key configuration has arrived, or 2 seconds after the character joined the map
// server without one (a new character has none stored, and the server sends none). Asked earlier, the save waits
// for SaveOptionsIfDue (see GameLogic/KeyConfiguration/KeyConfigurationGate.h).
void SaveOptionsIfInGame();
// Each frame of the main scene: the save SaveOptionsIfInGame held back, once the 2 seconds are over.
void SaveOptionsIfDue();
// The game, the session or the game server is left: SaveOptions when SaveOptionsIfInGame would save now; else
// nothing, as the hotkeys in memory may not be the character's yet.
void SaveOptionsBeforeLeaving();
// Whether the selected character's key configuration has arrived; false also forgets the join. Until it has, the
// hotkeys in memory are the reset (empty) ones, and saving them would overwrite the character's stored layout.
// `everythingRestored` is false when a part of it stored nothing to restore (see KeyConfigurationGate::Received).
void SetKeyConfigurationReceived(bool received, bool everythingRestored = true);
// Another character is selected: everything SaveOptions writes goes back to what a character without a stored key
// configuration starts with (empty skill keys and Q/W/E/R, see UI::Options::ResetForNewCharacter for the options and
// the chat box), and the configuration counts as not arrived until the new character's comes.
void ResetKeyConfiguration();
// The selected character joined the map server and its map has loaded; its key configuration, when it has one,
// follows at once (it waited in the packet queue during the load).
void SetJoinedMapServer();
void SaveMacro();
void OpenMacro();

extern wchar_t AbuseFilter[][20];
extern wchar_t AbuseNameFilter[][20];
extern int AbuseFilterNumber;
extern int AbuseNameFilterNumber;