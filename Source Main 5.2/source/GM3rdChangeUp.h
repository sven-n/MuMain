// GM3rdChangeUp.h: interface for the CGM3rdChangeUp class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GM3RDCHANGEUP_H__09F84DB0_6262_425E_98B4_7EB06EA595CC__INCLUDED_)
#define AFX_GM3RDCHANGEUP_H__09F84DB0_6262_425E_98B4_7EB06EA595CC__INCLUDED_

#pragma once

class BMD;

namespace SEASON3A
{
    class CGM3rdChangeUp
    {
    protected:
        bool m_nDarkElfAppearance;

    protected:
        CGM3rdChangeUp();

    public:
        virtual ~CGM3rdChangeUp();

        static CGM3rdChangeUp& Instance();

        bool IsBalgasBarrackMap();
        bool IsBalgasRefugeMap();

        bool CreateBalgasBarrackObject(OBJECT* pObject);
        bool CreateBalgasRefugeObject(OBJECT* pObject);
        bool MoveObject(OBJECT* pObject);
        bool RenderObjectVisual(OBJECT* pObject, BMD* pModel);
        bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
        void RenderAfterObjectMesh(OBJECT* o, BMD* b);

        bool CreateFireSnuff(PARTICLE* o);
        void PlayEffectSound(OBJECT* o);
        void PlayBGM();

        CHARACTER* CreateBalgasBarrackMonster(int iType, int PosX, int PosY, int Key);
        bool SetCurrentActionBalgasBarrackMonster(CHARACTER* c, OBJECT* o);
        bool AttackEffectBalgasBarrackMonster(CHARACTER* c, OBJECT* o, BMD* b);
        bool MoveBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
        void MoveBalgasBarrackBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
        bool RenderMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon);
        bool RenderBalgasBarrackMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    };
}

#endif // !defined(AFX_GM3RDCHANGEUP_H__09F84DB0_6262_425E_98B4_7EB06EA595CC__INCLUDED_)
