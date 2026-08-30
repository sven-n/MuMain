// NewUIChatCommandWindow.h: interface for the CNewUIChatCommandWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/NewUI/Inventory/NewUIMyInventory.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/UILayoutPolicy.h"
#include "UI/NewUI/Widgets/NewUIButton.h"
#include "UI/Legacy/UIControls.h"
#include "UI/Scaling/UITransform.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"
#include "GameLogic/Commands/ChatCommandTemplate.h"

#include <memory>
#include <vector>

namespace SEASON3B
{
// Offers the chat commands which the server told us about, so that nobody
// has to know or type a command.
//
// It has the width of the command window and shows one thing at a time: the
// commands, the parameters of the command which was picked, or the saved
// templates. A command without parameters is sent right away, everything
// else leads to the parameter page first.
class CNewUIChatCommandWindow : public CNewUIObj
{
    // The frame is the one of the command window, which is drawn for a width
    // of 190 - a wider window would stretch and distort it.
    enum eIMAGE_LIST
    {
        IMAGE_CHATCOMMAND_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
        IMAGE_CHATCOMMAND_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,
        IMAGE_CHATCOMMAND_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,
        IMAGE_CHATCOMMAND_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
        IMAGE_CHATCOMMAND_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
        IMAGE_CHATCOMMAND_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        IMAGE_CHATCOMMAND_BTN = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
    };

    enum eWINDOW_SIZE
    {
        WINDOW_WIDTH = 190,
        FRAME_TOP_HEIGHT = 64,
        FRAME_SIDE_WIDTH = 21,
        FRAME_BOTTOM_HEIGHT = 45,
        // The side pieces are only this tall. Drawing them for more than
        // that samples past the texture, which repeats their upper end as
        // a seam near the bottom of the window.
        FRAME_SIDE_TEXTURE_HEIGHT = 320,
    };

    enum eLAYOUT
    {
        TITLE_Y = 12,
        CONTENT_LEFT = 22,
        CONTENT_WIDTH = WINDOW_WIDTH - 2 * CONTENT_LEFT,
        CONTENT_TOP = 36,
        ROW_HEIGHT = 15,
        // As many rows as fit above the buttons.
        VISIBLE_ROWS = 20,
        BUTTON_WIDTH = 64,
        BUTTON_HEIGHT = 29,
        BUTTON_ROW_Y = 358,
        EXIT_BUTTON_X = 13,
        EXIT_BUTTON_Y = 392,
        EXIT_BUTTON_WIDTH = 36,
        EXIT_BUTTON_HEIGHT = 29,
        // Where the rows have to end so that they don't run into the
        // buttons below them.
        CONTENT_BOTTOM = BUTTON_ROW_Y - ROW_HEIGHT,
        VALUE_HEIGHT = 16,
        // A parameter takes its name, the box with its value, and a gap.
        PARAMETER_HEIGHT = ROW_HEIGHT + VALUE_HEIGHT + 4,
    };

    // What the window shows. There is no scrolling between the pages: the
    // one which is shown owns the whole content area.
    enum ePAGE
    {
        PAGE_COMMANDS,
        PAGE_PARAMETERS,
        PAGE_TEMPLATES,
    };

public:
    static constexpr float LayerDepth = UI::Layout::ForegroundPanelLayerDepth;
    static constexpr int WindowHeight = UI::Scaling::DockLogicalBottom;

    CNewUIChatCommandWindow();
    ~CNewUIChatCommandWindow() override;

    bool Create(CNewUIManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override;
    bool Update() override;
    bool Render() override;

    float GetLayerDepth() override;
    float GetKeyEventOrder() override;

    void OpenningProcess();
    void ClosingProcess();

private:
    // The commands of the catalog, favourites first. Holds indices, because
    // a list which arrives while the window is open replaces the commands.
    void RebuildCommandOrder();
    const GameLogic::Commands::ChatCommand* GetCommandAt(int row) const;
    const GameLogic::Commands::ChatCommand* GetSelectedCommand() const;

    void ShowPage(ePAGE page);
    void PickCommand(int row);
    void ExecuteSelectedCommand();
    void ExecuteTemplate(size_t index);
    void SaveSelectedAsTemplate();
    void ToggleFavouriteOfSelected();

    // Steps to the next accepted value of a parameter which only takes a
    // known set of them, so that nothing has to be typed for it.
    void CycleParameterValue(size_t parameterIndex);
    void BeginEditingParameter(size_t parameterIndex);
    void CommitEditedValue();
    void StopEditing();
    static bool IsPickedFromList(const GameLogic::Commands::ChatCommandParameter& parameter);
    static std::vector<std::wstring> SplitValidValues(const std::wstring& validValues);
    bool AreRequiredValuesSet() const;

    void InitButtons();
    void LoadImages();
    void UnloadImages();

    int GetScrollableRowCount() const;
    // The description is wrapped once when the page is entered, because
    // measuring it against the font is too much work for every frame.
    void WrapDescriptionOfSelected();
    // How many of the wrapped lines are shown. A long description gives way
    // to the parameters, which are the part the player has to reach.
    int GetVisibleDescriptionLineCount() const;
    int GetParameterTop() const;
    int GetActionTop() const;
    bool HasLeftButton() const
    {
        return m_page != PAGE_COMMANDS;
    }
    bool HasRightButton() const
    {
        return m_page != PAGE_TEMPLATES;
    }

    bool UpdateCommandPageMouseEvent();
    bool UpdateParameterPageMouseEvent();
    bool UpdateTemplatePageMouseEvent();

    void RenderBaseWindow();
    void RenderTitle();
    void RenderCommandPage();
    void RenderParameterPage();
    void RenderParameter(size_t parameterIndex, int y);
    void RenderTemplatePage();

private:
    CNewUIManager* m_pNewUIMng;
    POINT m_Pos;

    ePAGE m_page;
    std::vector<int> m_commandOrder;
    int m_selectedRow;
    int m_scrollOffset;

    // The value of every parameter of the selected command, in its order.
    std::vector<std::wstring> m_parameterValues;
    std::vector<std::wstring> m_descriptionLines;
    // One box is enough: it is moved onto whichever parameter is edited.
    std::unique_ptr<CUITextInputBox> m_pValueInput;
    int m_editedParameter;

    std::vector<GameLogic::Commands::ChatCommandTemplate> m_templates;

    CNewUIButton m_BtnExit;
    CNewUIButton m_BtnLeft;
    CNewUIButton m_BtnRight;
};
} // namespace SEASON3B
