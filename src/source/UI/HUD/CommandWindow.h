
#if !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
#define AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/HUD/CommandWindowRmlModel.h"
#include "UI/RmlBridge/RmlStackingOrder.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Scaling/UITransform.h"
#include "Engine/Object/ZzzCharacter.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
    class CCommandWindow : public CObject
    {
    public:
        static constexpr float LayerDepth = UI::RmlBridge::ForegroundPanelLayerDepth;

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

        int							m_iCurSelectCommand;
        int							m_iCurMouseCursor;
        bool						m_bSelectedChar;
        bool						m_bCanCommand;

        void BindRmlModel(Rml::DataModelConstructor& c, CommandWindowRmlModel& model);
        UI::RmlBridge::ThemedView<CommandWindowRmlModel> m_RmlView{"command_window",
            [this](Rml::DataModelConstructor& c, CommandWindowRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/command_window.rml"}}};
        // A button press, queued by RmlUi's click and run from Update(), outside RmlUi's own
        // event dispatch (Special opens another window).
        int m_PendingCommand = COMMAND_NONE;

    public:
        CCommandWindow();
        virtual ~CCommandWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();


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
        void BuildRmlUi();
        void SyncRmlModel();
        void SyncTitle();
        void SyncButtons();
        void SyncTarget();
        void PressCommandButton(int command);
        void RunCommand();
    };
};

#endif // !defined(AFX_NEWUICOMMANDWINDOW_H__6C0AA8A8_EF69_45F3_BCE4_F957F08310C5__INCLUDED_)
