// NewUIWindowMenu.h: interface for the CWindowMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_)
#define AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_

#pragma once

#include "UI/Core/NewUIManager.h"
#include "UI/Dialogs/NewUIMessageBox.h"

namespace mu::ui::window
{
    class CWindowMenu : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_WINDOW_MENU_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_WINDOW_MENU_FRAME_UP = BITMAP_WINDOW_MENU_BEGIN,
            IMAGE_WINDOW_MENU_FRAME_MIDDLE,
            IMAGE_WINDOW_MENU_FRAME_DOWN,
            IMAGE_WINDOW_MENU_LINE,
            IMAGE_WINDOW_MENU_ARROWL,
            IMAGE_WINDOW_MENU_ARROWR,
        };
#ifdef PBG_ADD_GENSRANKING
        enum
        {
            // �߰��ÿ� MENU_MAX_INDEX�ø��� ������ �ڵ� ����
            MENU_MAX_INDEX = 6,
            STANDARD_POS_X = REFERENCE_WIDTH - 112,
            STANDARD_POS_Y = REFERENCE_HEIGHT - 156,
        };
#endif //PBG_ADD_GENSRANKING
    public:
        CWindowMenu();
        virtual ~CWindowMenu();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 10.0f
        float GetKeyEventOrder();	// 10.f;

        void OpenningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        void RenderTexts();
        void RenderArrow();

    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

        int m_iSelectedIndex;
    };
}

#endif // !defined(AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_)
