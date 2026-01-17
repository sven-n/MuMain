// NewUIQuickCommandWindow.h: interface for the CNewUIQuickCommandWindow class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_)
#define AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_

#pragma once

#include "NewUIManager.h"
#include "NewUIWindowMenu.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"

namespace SEASON3B
{
    class CNewUIQuickCommandWindow : public CNewUIObj
    {
        enum IMAGE_LIST
        {
            IMAGE_QUICKCOMMAND_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_QUICKCOMMAND_FRAME_MIDDLE = CNewUIWindowMenu::IMAGE_WINDOW_MENU_FRAME_MIDDLE,
            IMAGE_QUICKCOMMAND_FRAME_DOWN = CNewUIWindowMenu::IMAGE_WINDOW_MENU_FRAME_DOWN,
            IMAGE_QUICKCOMMAND_LINE = CNewUIWindowMenu::IMAGE_WINDOW_MENU_LINE,
            IMAGE_QUICKCOMMAND_ARROWL = CNewUIWindowMenu::IMAGE_WINDOW_MENU_ARROWL,
            IMAGE_QUICKCOMMAND_ARROWR = CNewUIWindowMenu::IMAGE_WINDOW_MENU_ARROWR,
            IMAGE_QUICKCOMMAND_FRAME_UP = BITMAP_QUICKCOMMAND_BEGIN,
        };

    public:
        CNewUIQuickCommandWindow();
        virtual ~CNewUIQuickCommandWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.0f
        float GetKeyEventOrder();	// 10.f;

        void OpenningProcess();
        void ClosingProcess();
        void OpenQuickCommand(const wchar_t* strID, int iIndex, int x, int y);
        void CloseQuickCommand();
        void SetID(const wchar_t* strID);
        void SetSelectedCharacterIndex(int iIndex);

    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        void RenderContents();
        void RenderArrow();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT			m_Pos;

        int m_iSelectedIndex;
        wchar_t m_strID[32];
        int m_iSelectedCharacterIndex;
    };
}

#endif // !defined(AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_)
