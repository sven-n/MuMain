// NewUISiegeWarSoldier.h: interface for the NewUISiegeWarSoldier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_)
#define AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_

#pragma once

#include "NewUISiegeWarBase.h"

namespace SEASON3B
{
    class CNewUISiegeWarSoldier : public CNewUISiegeWarBase
    {
    public:
        CNewUISiegeWarSoldier();
        virtual ~CNewUISiegeWarSoldier();

    private:
        virtual bool OnCreate(int x, int y);
        virtual bool OnUpdate();
        virtual bool OnRender();
        virtual void OnRelease();

        virtual bool OnUpdateMouseEvent();
        virtual bool OnUpdateKeyEvent();
        virtual bool OnBtnProcess();
        virtual void OnSetPos(int x, int y);

        virtual void OnLoadImages();
        virtual void OnUnloadImages();

        void RenderCharPosInMiniMap();
    };
}

#endif // !defined(AFX_NEWUISIEGEWARSOLDIER_H__6316C0AE_6E09_4BBE_9308_9DC81353DD59__INCLUDED_)
