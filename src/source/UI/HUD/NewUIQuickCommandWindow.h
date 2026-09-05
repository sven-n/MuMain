// NewUIQuickCommandWindow.h: interface for the CQuickCommandWindow class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_)
#define AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_

#pragma once

#include "UI/Core/NewUIManager.h"
#include "UI/Dialogs/NewUIWindowMenu.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"

namespace mu::ui::window
{
    class CQuickCommandWindow : public CObject
    {
        enum IMAGE_LIST
        {
            IMAGE_QUICKCOMMAND_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_QUICKCOMMAND_FRAME_MIDDLE = CWindowMenu::IMAGE_WINDOW_MENU_FRAME_MIDDLE,
            IMAGE_QUICKCOMMAND_FRAME_DOWN = CWindowMenu::IMAGE_WINDOW_MENU_FRAME_DOWN,
            IMAGE_QUICKCOMMAND_LINE = CWindowMenu::IMAGE_WINDOW_MENU_LINE,
            IMAGE_QUICKCOMMAND_ARROWL = CWindowMenu::IMAGE_WINDOW_MENU_ARROWL,
            IMAGE_QUICKCOMMAND_ARROWR = CWindowMenu::IMAGE_WINDOW_MENU_ARROWR,
            IMAGE_QUICKCOMMAND_FRAME_UP = BITMAP_QUICKCOMMAND_BEGIN,
        };

    public:
        CQuickCommandWindow();
        virtual ~CQuickCommandWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
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
        CManager* m_pNewUIMng;
        POINT			m_Pos;

        int m_iSelectedIndex;
        wchar_t m_strID[32];
        int m_iSelectedCharacterIndex;
    };
}

#endif // !defined(AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_)
