#pragma once

#include <functional>
#include <vector>

void DeleteNpcs();
void OpenNpc(int Type);
void DeleteMonsters();
void OpenMonsterModel(EMonsterModelType Type);

void OpenModel(int Type, wchar_t* Dir, wchar_t* ModelFileName, ...);
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
// The textures of the item effects and the looks in code, at their BITMAP_ indices (OpenItemTextures loads them).
void OpenItemEffectTextures();
// Makes mesh 2 of the Grand Soul pants' model the grid their cloth moves
// (OpenItems once the item models are open; the item editor again after their
// slot took other data).
void PrepareGrandSoulPantsClothMesh();

void SaveOptions();
// SaveOptions, once the character's key configuration has arrived in the game world.
void SaveOptionsIfInGame();
// Whether the selected character's key configuration has arrived. Until it has, the hotkeys in memory are the reset
// ones (or another character's Q/W/E/R), and saving them would overwrite the character's stored layout.
void SetKeyConfigurationReceived(bool received);
void SaveMacro(const wchar_t* FileName);
void OpenMacro(const wchar_t* FileName);

extern wchar_t AbuseFilter[][20];
extern wchar_t AbuseNameFilter[][20];
extern int AbuseFilterNumber;
extern int AbuseNameFilterNumber;