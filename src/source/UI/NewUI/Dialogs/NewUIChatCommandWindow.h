// NewUIChatCommandWindow.h: the window which offers the chat commands of the player.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"

namespace SEASON3B
{
    // Lists the chat commands the server offers to this player and executes
    // them, so that nobody has to know or type a command.
    //
    // Commands without required parameters are executed right away. For the
    // others, one row per parameter is shown: values which only accept a set of
    // values and booleans are cycled by clicking, everything else is typed.
    class CNewUIChatCommandWindow : public CNewUIObj
    {
        enum IMAGE_LIST
        {
            IMAGE_COMMAND_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
            IMAGE_COMMAND_BUTTON = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
        };

        static constexpr float WINDOW_WIDTH = 320.f;
        static constexpr float WINDOW_HEIGHT = 400.f;
        static constexpr int VISIBLE_ROWS = 10;
        static constexpr float ROW_HEIGHT = 16.f;

    public:
        CNewUIChatCommandWindow();
        virtual ~CNewUIChatCommandWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();
        float GetKeyEventOrder();

        void OpenningProcess();
        void ClosingProcess();

    private:
        const GameLogic::Commands::ChatCommand* GetSelectedCommand() const;
        void SelectCommand(int index);
        void ExecuteSelectedCommand();
        // Steps to the next accepted value of a parameter which has a limited
        // set of them, so that no text has to be entered for it.
        void CycleParameterValue(size_t parameterIndex);
        static std::vector<std::wstring> SplitValidValues(const std::wstring& validValues);

        void RenderFrame();
        void RenderCommandList();
        void RenderSelectedCommand();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        int m_selectedIndex;
        int m_scrollOffset;
        // The value of each parameter of the selected command, in its order.
        std::vector<std::wstring> m_parameterValues;
    };
}
