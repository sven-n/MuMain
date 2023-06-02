// NewUIItemExplanationWindow.h: interface for the CNewUIItemExplanationWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIITEMEXPLANATIONWINDOW_H__4029DCB0_6E92_4032_A68B_CE62B878F615__INCLUDED_)
#define AFX_NEWUIITEMEXPLANATIONWINDOW_H__4029DCB0_6E92_4032_A68B_CE62B878F615__INCLUDED_

#pragma once

#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIItemExplanationWindow : public CNewUIObj
    {
    public:
        CNewUIItemExplanationWindow();
        virtual ~CNewUIItemExplanationWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 6.5f
        float GetKeyEventOrder();	// 10.f;

        void OpenningProcess();
        void ClosingProcess();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT			m_Pos;
    };
}

#endif // !defined(AFX_NEWUIITEMEXPLANATIONWINDOW_H__4029DCB0_6E92_4032_A68B_CE62B878F615__INCLUDED_)
