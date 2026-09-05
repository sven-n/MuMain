// NewUIGuildMakeWindow.h: interface for the CGuildMakeWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIGUILDMAKEWINDOW_H__68B0DE4B_7E07_4928_B8CF_2F7A6A15EEBD__INCLUDED_)
#define AFX_NEWUIGUILDMAKEWINDOW_H__68B0DE4B_7E07_4928_B8CF_2F7A6A15EEBD__INCLUDED_

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UIGuildMaster.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Widgets/NewUIButton.h"

namespace mu::ui::window
{
    class CManager;
    class CGuildMakeWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_GUILDMAKE_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_GUILDMAKE_MAKEBUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,
            IMAGE_GUILDMAKE_NEXTBUTTON = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
            IMAGE_GUILDMAKE_BACK_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
            IMAGE_GUILDMAKE_BACK_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
            IMAGE_GUILDMAKE_BACK_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
            IMAGE_GUILDMAKE_BACK_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
            IMAGE_GUILDMAKE_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
            IMAGE_GUILDMAKE_EDITBOX = BITMAP_GUILDMAKE_BEGIN,
        };

        enum
        {
            GUILDMAKE_WIDTH = 190,
            GUILDMAKE_HEIGHT = 429,
        };

        enum GUILDMAKE_STATE
        {
            GUILDMAKE_INFO = 0,
            GUILDMAKE_MARK,
            GUILDMAKE_RESULTINFO,
        };

        enum
        {
            MAXGUILDNAME = 8,
        };

        enum
        {
            GUILDMAKEBUTTON_INFO_MAKE = 0,

            GUILDMAKEBUTTON_MARK_LNEXT,
            GUILDMAKEBUTTON_MARK_RNEXT,

            GUILDMAKEBUTTON_RESULTINFO_LNEXT,
            GUILDMAKEBUTTON_RESULTINFO_RNEXT,

            GUILDMAKEBUTTON_COUNT,
        };

    public:
        CGuildMakeWindow();
        virtual ~CGuildMakeWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();

    public:
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();

    private:
        bool UpdateGMInfo();
        bool UpdateGMMark();
        bool UpdateGMResultInfo();

    public:
        bool Render();

    private:
        void RenderFrame();
        void RenderGMInfo();
        void RenderGMMark();
        void RenderGMResultInfo();

    public:
        void SetPos(int x, int y);
        const POINT& GetPos();
        float GetLayerDepth();	//. 4.4f

    private:
        void ChangeWindowState(const GUILDMAKE_STATE state);
        void ChangeEditBox(const UISTATES type);

    private:
        CManager* m_pNewUIMng;

        POINT					m_Pos;
        GUILDMAKE_STATE			m_GuildMakeState;

        CUITextInputBox* m_EditBox;
        CButton* m_Button;
        CButton			m_BtnExit;
    };

    inline
        void CGuildMakeWindow::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CGuildMakeWindow::GetPos()
    {
        return m_Pos;
    }
}

#endif // !defined(AFX_NEWUIGUILDMAKEWINDOW_H__68B0DE4B_7E07_4928_B8CF_2F7A6A15EEBD__INCLUDED_)
