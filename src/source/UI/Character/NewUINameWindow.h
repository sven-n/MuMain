// NewUINameWindow.h: interface for the CNameWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
#define AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_

#pragma once

#include "UI/Core/NewUIManager.h"

namespace mu::ui::window
{
    // item name
    class CNameWindow : public CObject
    {
    public:
        CNameWindow();
        virtual ~CNameWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();		// 1.0f

    private:
        void RenderName();

        CManager* m_pNewUIMng;		// UI manager
        POINT m_Pos;					// window position

        bool m_bShowItemName;
        bool m_bShowMonsterHealthBar;

        void RenderMonsterHealthBars();
    };
}

#endif // !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
