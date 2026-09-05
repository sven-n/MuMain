// NewUICommandWindow.h: interface for the CCommandWindow class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
#define AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_

#pragma once

#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/Core/UILayoutPolicy.h"
#include "UI/Widgets/NewUIButton.h"
#include "UI/Scaling/UITransform.h"

#define MAX_COMMAND_BTN (12)

#define COMMAND_BTN_INTERVAL_SIZE	(1)

namespace mu::ui::window
{
    class CCommandWindow : public CObject
    {
    public:
        static constexpr float LayerDepth = UI::Layout::ForegroundPanelLayerDepth;

        enum eIMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_COMMAND_BASE_WINDOW_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_COMMAND_BASE_WINDOW_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
            IMAGE_COMMAND_BASE_WINDOW_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,			//. newui_item_back02-l.tga	(21,320)
            IMAGE_COMMAND_BASE_WINDOW_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_COMMAND_BASE_WINDOW_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,		//. newui_item_back03.tga	(190,45)
            IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga
            IMAGE_COMMAND_BTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,						//. newui_btn_empty.tga
            IMAGE_COMMAND_SELECTID_BG = BITMAP_COMMAND_WINDOW_BEGIN,								//. newUI_cursorid_wnd.jpg	(128,32)
        };

        enum eCOMMAND_WINDOW_SIZE
        {
            COMMAND_WINDOW_WIDTH = 190,
            COMMAND_WINDOW_HEIGHT = UI::Scaling::DockLogicalBottom,
            // How tall newui_item_back02-L/R actually are.
            COMMAND_WINDOW_SIDE_TEXTURE_HEIGHT = 320,
        };

    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

        // RadioGroupButton
        CButton				m_BtnCommand[MAX_COMMAND_BTN];
        // Exit Button
        CButton				m_BtnExit;
        int							m_iCurSelectCommand;
        int							m_iCurMouseCursor;
        bool						m_bSelectedChar;
        bool						m_bCanCommand;

    public:
        CCommandWindow();
        virtual ~CCommandWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 4.6f

        void OpenningProcess();
        void ClosingProcess();

        int	GetCurCommandType();

        void SetMouseCursor(int iCursorType);
        int	 GetMouseCursor();

        bool CommandTrade(CHARACTER* pSelectedCha);
        bool CommandPurchase(CHARACTER* pSelectedCha);
        bool CommandParty(SHORT iChaKey);
        bool CommandWhisper(CHARACTER* pSelectedCha);
        bool CommandGuild(CHARACTER* pSelectedCha);
        bool CommandGuildUnion(CHARACTER* pSelectedCha);
        bool CommandGuildRival(CHARACTER* pSelectedCha);
        bool CommandCancelGuildRival(CHARACTER* pSelectedCha);
        bool CommandAddFriend(CHARACTER* pSelectedCha);
        bool CommandFollow(int iSelectedChaIndex);
        int CommandDual(CHARACTER* pSelectedCha);
    private:
        void LoadImages();
        void UnloadImages();
        void InitButtons();
        void RenderBaseWindow();
        void RunCommand();
        void SelectCommand();
        void SetBtnState(int iBtnType, bool bStateDown);
    };
};

#endif // !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
