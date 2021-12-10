//////////////////////////////////////////////////////////////////////////
//  GOBoid.h
//////////////////////////////////////////////////////////////////////////
#ifndef __GOBOID_H__
#define __GOBOID_H__

#include "zzzObject.h"

bool CreateBugSub ( int Type, vec3_t Position, OBJECT *Owner, OBJECT * o, int SubType=0, int LinkBone=0 );
void CreateBug ( int Type, vec3_t Position, OBJECT *Owner, int SubType=0, int LinkBone=0 );
bool MoveBug ( OBJECT* o, bool bForceRender=false );
void MoveBugs ( void );
bool RenderBug ( OBJECT* o, bool bForceRender=false );
void RenderBugs ( void );
void RenderDarkHorseSkill ( OBJECT* o, BMD* b );
void RenderSkillEarthQuake ( CHARACTER* c, OBJECT* o, BMD* b, int iMaxSkill = 30);
void DeleteBug(OBJECT *Owner);
// 아이템이 펫 아이템인가?
bool IsBug(ITEM* pItem);	

void MoveBoids ( void );            
void RenderBoids ( bool bAfterCharacter = false );
void DeleteBoids ( void );

void MoveFishs ( void );            
void RenderFishs ( void );




#endif// __GOBOID_H__