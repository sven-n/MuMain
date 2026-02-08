// NewUISiegeWarObserver.h: interface for the CNewUISiegeWarObserver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_)
#define AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_

#pragma once

#include "NewUISiegeWarBase.h"

namespace SEASON3B
{
    class CNewUISiegeWarObserver : public CNewUISiegeWarBase
    {
    public:
        CNewUISiegeWarObserver();
        virtual ~CNewUISiegeWarObserver();

    private:
        virtual bool OnCreate(int x, int y);
        virtual bool OnUpdate();
        virtual bool OnRender();
        virtual void OnRelease();

        virtual bool OnUpdateMouseEvent();
        virtual bool OnUpdateKeyEvent();
        virtual bool OnBtnProcess();
        virtual void OnSetPos(int x, int y);

        virtual	void OnLoadImages();
        virtual void OnUnloadImages();

        void RenderCharPosInMiniMap();
    };
}

#endif // !defined(AFX_NEWUISIEGEWAROBSERVER_H__023A30CE_13AE_4C7F_B690_9243328FB0BC__INCLUDED_)
