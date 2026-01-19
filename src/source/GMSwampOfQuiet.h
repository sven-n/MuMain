// GMSwampOfQuiet.h: interface for the GMSwampOfQuiet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMSWAMPOFQUIET_H__26257272_673D_4866_8C8D_F37CA48B6125__INCLUDED_)
#define AFX_GMSWAMPOFQUIET_H__26257272_673D_4866_8C8D_F37CA48B6125__INCLUDED_

#pragma once

namespace SEASON3C
{
    class GMSwampOfQuiet
    {
    public:
        GMSwampOfQuiet();
        virtual ~GMSwampOfQuiet();

        static bool IsCurrentMap();
        static void RenderBaseSmoke();

        //. 오브젝트
        static void CreateObject(OBJECT* pObject);
        static bool MoveObject(OBJECT* pObject);
        static bool RenderObjectVisual(OBJECT* pObject, BMD* pModel);
        static bool RenderObject(OBJECT* pObject, BMD* pModel, bool ExtraMon = 0);
        static void RenderObjectAfterCharacter(OBJECT* pObject, BMD* pModel, bool ExtraMon = 0);

        //. 몬스터
        static CHARACTER* CreateSwampOfQuietMonster(int iType, int PosX, int PosY, int Key);

        static bool MoveMonsterVisual(OBJECT* pObject, BMD* pModel);
        static void MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
        static bool RenderMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);

        static bool AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel);
        static bool SetCurrentActionMonster(CHARACTER* pCharacter, OBJECT* pObject);

        static bool PlayMonsterSound(OBJECT* pObject);
        static void PlayObjectSound(OBJECT* pObject);
    };
}

#endif // !defined(AFX_GMSWAMPOFQUIET_H__26257272_673D_4866_8C8D_F37CA48B6125__INCLUDED_)
