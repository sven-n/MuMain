#pragma once

void DeleteNpcs();
void OpenNpc(int Type);
void DeleteMonsters();
void OpenMonsterModel(EMonsterModelType Type);

void OpenModel(int Type, wchar_t* Dir, wchar_t* ModelFileName, ...);
void OpenBasicData(HDC hDC);
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
// Sends the key configuration while connected to the game server with a character in the game world;
// does nothing elsewhere, where there is no character to save it for.
void SaveOptionsIfConnected();
void SaveMacro(const wchar_t* FileName);
void OpenMacro(const wchar_t* FileName);

extern wchar_t AbuseFilter[][20];
extern wchar_t AbuseNameFilter[][20];
extern int AbuseFilterNumber;
extern int AbuseNameFilterNumber;