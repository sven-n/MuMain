//////////////////////////////////////////////////////////////////////////
//  GMHellas.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "ZzzBMD.h"

bool    CreateWaterTerrain(int mapIndex);
bool    IsWaterTerrain(void);
void    AddWaterWave(int x, int y, int range, int height);
float   GetWaterTerrain(float x, float y);
void    MoveWaterTerrain(void);
bool    RenderWaterTerrain(void);
void    DeleteWaterTerrain(void);
void    RenderWaterTerrain(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation = 0.f, float Alpha = 1.f, float Height = 0.f);
void    SettingHellasColor(void);
BYTE    GetHellasLevel(CLASS_TYPE Class, int Level);
bool    GetUseLostMap(bool bDrawAlert = false);
int     RenderHellasItemInfo(ITEM* ip, int textNum);
bool	EnableKalima(CLASS_TYPE Class, int Level, int ItemLevel);

void    AddObjectDescription(wchar_t* Text, vec3_t position);
void    RenderObjectDescription(void);

bool    CreateHellasObject(OBJECT* o);
bool    MoveHellasObjectSetting(int& objCount, int object);
bool    MoveHellasObject(OBJECT* o, int& object, int& visibleObject);
bool    MoveHellasAllObject(OBJECT* o);
bool    MoveHellasVisual(OBJECT* o);
bool    RenderHellasVisual(OBJECT* o, BMD* b);
bool    RenderHellasObjectMesh(OBJECT* o, BMD* b);

int     CreateBigMon(OBJECT* o);
void    MoveBigMon(OBJECT* o);

void    CreateMonsterSkill_ReduceDef(OBJECT* o, int AttackTime, BYTE time, float Height);
void    CreateMonsterSkill_Poison(OBJECT* o, int AttackTime, BYTE time);
void    CreateMonsterSkill_Summon(OBJECT* o, int AttackTime, BYTE time);

void    SetActionDestroy_Def(OBJECT* o);
void    RenderDestroy_Def(OBJECT* o, BMD* b);
CHARACTER* CreateHellasMonster(EMonsterType Type, int PositionX, int PositionY, int Key);

bool    SettingHellasMonsterLinkBone(CHARACTER* c, int Type);

bool    SetCurrentAction_HellasMonster(CHARACTER* c, OBJECT* o);
bool    AttackEffect_HellasMonster(CHARACTER* c, CHARACTER* tc, OBJECT* o, OBJECT* to, BMD* b);
void    MonsterMoveWaterSmoke(OBJECT* o);
void    MonsterDieWaterSmoke(OBJECT* o);
bool    MoveHellasMonsterVisual(OBJECT* o, BMD* b);
bool    RenderHellasMonsterCloth(CHARACTER* c, OBJECT* o, bool Translate = true, int Select = 0);
bool    RenderHellasMonsterObjectMesh(OBJECT* o, BMD* b);
bool    RenderHellasMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
