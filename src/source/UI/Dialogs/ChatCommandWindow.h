
#pragma once

#include "UI/Inventory/MyInventory.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Core/UILayoutPolicy.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Scaling/UITransform.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"
#include "GameLogic/Commands/ChatCommandTemplate.h"

#include <memory>
#include <vector>

namespace mu::ui::window
{
// Lets the player pick a server-advertised chat command instead of typing it; shows one
// page at a time (commands, parameters, or saved templates).
class CChatCommandWindow : public CObject
{
    // Frame art is drawn for width 190; a wider window would stretch and distort it.
    enum eIMAGE_LIST
    {
        IMAGE_CHATCOMMAND_BACK = CMessageBoxMng::IMAGE_MSGBOX_BACK,
        IMAGE_CHATCOMMAND_TOP = CMyInventory::IMAGE_INVENTORY_BACK_TOP,
        IMAGE_CHATCOMMAND_LEFT = CMyInventory::IMAGE_INVENTORY_BACK_LEFT,
        IMAGE_CHATCOMMAND_RIGHT = CMyInventory::IMAGE_INVENTORY_BACK_RIGHT,
        IMAGE_CHATCOMMAND_BOTTOM = CMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,
        IMAGE_CHATCOMMAND_BTN_EXIT = CMyInventory::IMAGE_INVENTORY_EXIT_BTN,
        IMAGE_CHATCOMMAND_BTN = CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL,
    };

    enum eWINDOW_SIZE
    {
        WINDOW_WIDTH = 190,
        FRAME_TOP_HEIGHT = 64,
        FRAME_SIDE_WIDTH = 21,
        FRAME_BOTTOM_HEIGHT = 45,
        // Side-piece texture is only this tall; drawing it taller samples past the
        // end, creating a seam near the bottom of the window.
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
        // Where rows must stop to avoid overlapping the buttons below.
        CONTENT_BOTTOM = BUTTON_ROW_Y - ROW_HEIGHT,
        VALUE_HEIGHT = 16,
        // A parameter takes its name, the box with its value, and a gap.
        PARAMETER_HEIGHT = ROW_HEIGHT + VALUE_HEIGHT + 4,
    };

    // What the window shows; each page owns the whole content area (no scrolling between pages).
    enum ePAGE
    {
        PAGE_COMMANDS,
        PAGE_PARAMETERS,
        PAGE_TEMPLATES,
    };

public:
    static constexpr float LayerDepth = UI::Layout::ForegroundPanelLayerDepth;
    static constexpr int WindowHeight = UI::Scaling::DockLogicalBottom;

    CChatCommandWindow();
    ~CChatCommandWindow() override;

    bool Create(CManager* pNewUIMng, int x, int y);
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
    // Favourites-first order over the catalog; stores indices since an incoming list
    // replaces the commands while the window is open.
    void RebuildCommandOrder();
    const GameLogic::Commands::ChatCommand* GetCommandAt(int row) const;
    const GameLogic::Commands::ChatCommand* GetSelectedCommand() const;

    void ShowPage(ePAGE page);
    void PickCommand(int row);
    void ExecuteSelectedCommand();
    void ExecuteTemplate(size_t index);
    void SaveSelectedAsTemplate();
    void ToggleFavouriteOfSelected();

    // Cycles a parameter through its fixed set of accepted values so nothing needs typing.
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
    // Wrapped once on page entry; measuring against the font every frame would be wasteful.
    void WrapDescriptionOfSelected();
    // How many wrapped description lines fit; a long description yields space to the parameters.
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
    CManager* m_pNewUIMng;
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

    CButton m_BtnExit;
    CButton m_BtnLeft;
    CButton m_BtnRight;
};
} // namespace mu::ui::window
