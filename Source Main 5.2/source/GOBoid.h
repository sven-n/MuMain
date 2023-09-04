//////////////////////////////////////////////////////////////////////////
//  GOBoid.h
//////////////////////////////////////////////////////////////////////////
#ifndef __GOBOID_H__
#define __GOBOID_H__

#include "zzzObject.h"

bool CreateMountSub(int Type, vec3_t Position, OBJECT* Owner, OBJECT* o, int SubType = 0, int LinkBone = 0);
void CreateMount(int Type, vec3_t Position, OBJECT* Owner, int SubType = 0, int LinkBone = 0);
bool MoveMount(OBJECT* o, bool bForceRender = false);
void MoveMounts(void);
bool RenderMount(OBJECT* o, bool bForceRender = false);
void RenderMount(void);
void RenderDarkHorseSkill(OBJECT* o, BMD* b);
void RenderSkillEarthQuake(CHARACTER* c, OBJECT* o, BMD* b, int iMaxSkill = 30);
void DeleteMount(OBJECT* Owner);
// 아이템이 펫 아이템인가?
bool IsMount(ITEM* pItem);

void MoveBoids(void);
void RenderBoids(bool bAfterCharacter = false);
void DeleteBoids(void);

void MoveFishs(void);
void RenderFishs(void);

#endif// __GOBOID_H__