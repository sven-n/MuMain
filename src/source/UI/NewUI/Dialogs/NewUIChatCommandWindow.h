// NewUIChatCommandWindow.h: the window which offers the chat commands of the player.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/Inventory/NewUIInventoryCtrl.h"
#include "UI/Legacy/UIControls.h"
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
            // A border which is built from corners and repeating edges, so it
            // fits any size. The frame of the inventory is drawn for its own
            // width of 190 and gets distorted when it's stretched.
            IMAGE_COMMAND_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,
            IMAGE_COMMAND_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,
            IMAGE_COMMAND_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,
            IMAGE_COMMAND_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,
            IMAGE_COMMAND_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,
            IMAGE_COMMAND_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,
            IMAGE_COMMAND_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,
            IMAGE_COMMAND_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,
        };

        static constexpr float WINDOW_WIDTH = 380.f;
        static constexpr float WINDOW_HEIGHT = 460.f;
        // The corner pieces are 14x14, the edges repeat between them.
        static constexpr float BORDER = 14.f;
        static constexpr float FRAME_TOP = 34.f;
        static constexpr float FRAME_SIDE = 16.f;
        static constexpr float FRAME_BOTTOM = 34.f;
        static constexpr int VISIBLE_ROWS = 14;
        static constexpr float ROW_HEIGHT = 16.f;
        static constexpr float SCROLLBAR_WIDTH = 8.f;

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
        // Moves the text box onto a parameter which takes a free value.
        void BeginEditingParameter(size_t parameterIndex);
        // Takes what was typed over into the value of the edited parameter.
        void CommitEditedValue();
        void StopEditing();
        static bool HasFixedValues(const GameLogic::Commands::ChatCommandParameter& parameter);
        static std::vector<std::wstring> SplitValidValues(const std::wstring& validValues);

        void LoadImages();
        void UnloadImages();

        void RenderFrame();
        // The list starts here, and the detail area below it.
        int GetListTop() const;
        int GetDetailTop() const;
        void RenderCommandList();
        void RenderSelectedCommand();

    private:
        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;

        int m_selectedIndex;
        int m_scrollOffset;
        // The value of each parameter of the selected command, in its order.
        std::vector<std::wstring> m_parameterValues;
        // One box is enough: it's moved to whichever parameter is being edited.
        CUITextInputBox* m_pValueInput;
        int m_editedParameter;
    };
}
