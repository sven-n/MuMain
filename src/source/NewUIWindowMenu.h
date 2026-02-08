// NewUIWindowMenu.h: interface for the CNewUIWindowMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_)
#define AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_

#pragma once

#include "NewUIManager.h"
#include "NewUIMessageBox.h"

namespace SEASON3B
{
    class CNewUIWindowMenu : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_WINDOW_MENU_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
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
            // 추가시에 MENU_MAX_INDEX늘리면 사이즈 자동 변경
            MENU_MAX_INDEX = 6,
            STANDARD_POS_X = 640 - 112,
            STANDARD_POS_Y = 480 - 156,
        };
#endif //PBG_ADD_GENSRANKING
    public:
        CNewUIWindowMenu();
        virtual ~CNewUIWindowMenu();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
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
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        int m_iSelectedIndex;
    };
}

#endif // !defined(AFX_NEWUIWINDOWMENU_H__26535D16_A947_4BC3_B129_59F0EFFBA04E__INCLUDED_)
