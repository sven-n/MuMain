// NewUICommandWindow.h: interface for the CNewUICommandWindow class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
#define AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_

#pragma once

#include "NewUIMyInventory.h"
#include "NewUIButton.h"

#define MAX_COMMAND_BTN		( 11 )

#define COMMAND_BTN_INTERVAL_SIZE	(1)

namespace SEASON3B
{
    class CNewUICommandWindow : public CNewUIObj
    {
    public:
        enum eIMAGE_LIST
        {
            // Base Window (Reference)
            IMAGE_COMMAND_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
            IMAGE_COMMAND_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
            IMAGE_COMMAND_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,			//. newui_item_back02-l.tga	(21,320)
            IMAGE_COMMAND_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
            IMAGE_COMMAND_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,		//. newui_item_back03.tga	(190,45)
            IMAGE_COMMAND_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga
            IMAGE_COMMAND_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY,						//. newui_btn_empty.tga
            IMAGE_COMMAND_SELECTID_BG = BITMAP_COMMAND_WINDOW_BEGIN,								//. newUI_cursorid_wnd.jpg	(128,32)
        };

        enum eCOMMAND_WINDOW_SIZE
        {
            COMMAND_WINDOW_WIDTH = 190,
            COMMAND_WINDOW_HEIGHT = 429,
        };

    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

        // RadioGroupButton
        CNewUIButton				m_BtnCommand[MAX_COMMAND_BTN];
        // Exit Button
        CNewUIButton				m_BtnExit;
        int							m_iCurSelectCommand;
        int							m_iCurMouseCursor;
        bool						m_bSelectedChar;
        bool						m_bCanCommand;

    public:
        CNewUICommandWindow();
        virtual ~CNewUICommandWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
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
