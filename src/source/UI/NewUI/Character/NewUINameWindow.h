// NewUINameWindow.h: interface for the CNewUINameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
#define AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_

#pragma once

#include "UI/NewUI/NewUIManager.h"

namespace SEASON3B
{
    // item name
    class CNewUINameWindow : public CNewUIObj
    {
    public:
        CNewUINameWindow();
        virtual ~CNewUINameWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();		// 1.0f

    private:
        void RenderName();

        CNewUIManager* m_pNewUIMng;		// UI manager
        POINT m_Pos;					// window position

        bool m_bShowItemName;
        bool m_bShowMonsterHealthBar;

        void RenderMonsterHealthBars();
    };
}

#endif // !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
