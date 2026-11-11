// GMNewTown.h: interface for the GMNewTown class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMNEWTOWN_H__3FBE665B_5FFE_4BF4_B723_A6A9A5DFE479__INCLUDED_)
#define AFX_GMNEWTOWN_H__3FBE665B_5FFE_4BF4_B723_A6A9A5DFE479__INCLUDED_

#pragma once

#include "Render/Models/DrawLook.h"

namespace SEASON3B
{
    class GMNewTown
    {
    public:
        GMNewTown();
        virtual ~GMNewTown();

        static bool IsCurrentMap();
        static bool IsNewMap73_74();
        //. 오브젝트
        static void CreateObject(OBJECT* pObject);
        static bool MoveObject(OBJECT* pObject);
        static bool UpdateObjectEffects(OBJECT* pObject, BMD* pModel);
        static bool RenderObject(const OBJECT* pObject, DrawLook& look, BMD* pModel, bool ExtraMon = 0);
        static void RenderObjectAfterCharacter(const OBJECT* pObject, DrawLook& look, BMD* pModel, bool ExtraMon = 0);

        //. 몬스터
        static bool MoveMonsterVisual(OBJECT* pObject, BMD* pModel);
        static void MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
        static bool UpdateMonsterEffects(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

        static bool AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

        static void PlayObjectSound(OBJECT* pObject);

    };
}
#endif // !defined(AFX_GMNEWTOWN_H__3FBE665B_5FFE_4BF4_B723_A6A9A5DFE479__INCLUDED_)
