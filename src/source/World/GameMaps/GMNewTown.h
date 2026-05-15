// GMNewTown.h: interface for the GMNewTown class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMNEWTOWN_H__3FBE665B_5FFE_4BF4_B723_A6A9A5DFE479__INCLUDED_)
#define AFX_GMNEWTOWN_H__3FBE665B_5FFE_4BF4_B723_A6A9A5DFE479__INCLUDED_

#pragma once

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
        static bool RenderObjectVisual(OBJECT* pObject, BMD* pModel);
        static bool RenderObject(OBJECT* pObject, BMD* pModel, bool ExtraMon = 0);
        static void RenderObjectAfterCharacter(OBJECT* pObject, BMD* pModel, bool ExtraMon = 0);

        //. 몬스터
        static CHARACTER* CreateNewTownMonster(int iType, int PosX, int PosY, int Key);

        static bool MoveMonsterVisual(OBJECT* pObject, BMD* pModel);
        static void MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
        static bool RenderMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

        static bool AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
        static bool SetCurrentActionMonster(CHARACTER* pCharacter, OBJECT* pObject);

        static bool PlayMonsterSound(OBJECT* pObject);
        static void PlayObjectSound(OBJECT* pObject);

        static bool IsCheckMouseIn();
        static bool CharacterSceneCheckMouse(OBJECT* pObj);
        static bool m_bCharacterSceneCheckMouse;
    };
}
#endif // !defined(AFX_GMNEWTOWN_H__3FBE665B_5FFE_4BF4_B723_A6A9A5DFE479__INCLUDED_)
