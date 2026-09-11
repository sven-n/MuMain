
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Dialogs/ChatCommandWindow.h"

#include "Audio/DSPlaySound.h"
#include "Core/Text/TextLineWrap.h"
#include "GameLogic/Commands/ChatCommandFavourites.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;
using GameLogic::Commands::Catalog;
using GameLogic::Commands::ChatCommand;
using GameLogic::Commands::ChatCommandCatalog;
using GameLogic::Commands::ChatCommandParameter;
using GameLogic::Commands::ChatCommandParameterType;
using GameLogic::Commands::ChatCommandTemplate;

namespace
{
// A leading character instead of an icon, so it works with any font.
constexpr const wchar_t* FavouriteMarker = L"* ";
// Signals a command takes parameters; their names don't fit at this window width.
constexpr const wchar_t* ParameterMarker = L" ...";

struct TextColor
{
    BYTE Red;
    BYTE Green;
    BYTE Blue;
};

constexpr TextColor TitleColor = {255, 220, 120};
constexpr TextColor NormalColor = {220, 220, 220};
constexpr TextColor FavouriteColor = {255, 220, 120};
constexpr TextColor DescriptionColor = {200, 220, 255};
constexpr TextColor MissingValueColor = {255, 150, 150};
constexpr TextColor ActionColor = {150, 210, 255};

void UseTextColor(const TextColor& color)
{
    g_pRenderText->SetTextColor(color.Red, color.Green, color.Blue, 255);
    g_pRenderText->SetBgColor(0);
}

// Renders one line; empty text must be skipped -- the renderer measures a placeholder
// for it and leaves a stray glyph when a box width is given.
void RenderLine(int x, int y, const wchar_t* text, int boxWidth, int boxHeight = 0, int sort = RT3_SORT_LEFT)
{
    if (text == nullptr || text[0] == L'\0')
    {
        return;
    }

    g_pRenderText->RenderText(x, y, text, boxWidth, boxHeight, sort);
}

int MeasureInReferenceUnits(const wchar_t* text, size_t length)
{
    return g_pRenderText->MeasureText(text, static_cast<int>(length)).cx;
}

void RenderValueBackground(int x, int y, int width, int height)
{
    glColor4ub(0, 0, 0, 160);
    RenderColor(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));
    EndRenderColor();
    glColor4f(1.f, 1.f, 1.f, 1.f);
}
} // namespace

mu::ui::window::CChatCommandWindow::CChatCommandWindow()
{
    m_pNewUIMng = nullptr;
    m_Pos.x = 0;
    m_Pos.y = 0;
    m_page = PAGE_COMMANDS;
    m_selectedRow = -1;
    m_scrollOffset = 0;
    m_editedParameter = -1;
}

mu::ui::window::CChatCommandWindow::~CChatCommandWindow()
{
    Release();
}

bool mu::ui::window::CChatCommandWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == nullptr)
    {
        return false;
    }

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_COMMAND_LIST, this);

    LoadImages();

    m_pValueInput = std::make_unique<CUITextInputBox>();
    m_pValueInput->Init(g_hWnd, CONTENT_WIDTH - 4, VALUE_HEIGHT - 2);
    m_pValueInput->SetTextColor(255, 255, 230, 210);
    m_pValueInput->SetBackColor(0, 0, 0, 0);
    m_pValueInput->SetFont(g_hFont);
    m_pValueInput->SetState(UISTATE_HIDE);

    SetPos(x, y);
    InitButtons();
    Show(false);

    return true;
}

void mu::ui::window::CChatCommandWindow::Release()
{
    UnloadImages();
    m_pValueInput.reset();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void mu::ui::window::CChatCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + EXIT_BUTTON_X, m_Pos.y + EXIT_BUTTON_Y, EXIT_BUTTON_WIDTH, EXIT_BUTTON_HEIGHT);
    m_BtnLeft.ChangeButtonInfo(m_Pos.x + CONTENT_LEFT, m_Pos.y + BUTTON_ROW_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
    m_BtnRight.ChangeButtonInfo(m_Pos.x + WINDOW_WIDTH - CONTENT_LEFT - BUTTON_WIDTH, m_Pos.y + BUTTON_ROW_Y,
                                BUTTON_WIDTH, BUTTON_HEIGHT);
}

void mu::ui::window::CChatCommandWindow::InitButtons()
{
    wchar_t closeText[256] = {};
    mu_swprintf_s(closeText, I18N::Game::CloseS, L"J");

    m_BtnExit.ChangeButtonImgState(true, IMAGE_CHATCOMMAND_BTN_EXIT);
    m_BtnExit.ChangeToolTipText(closeText, true);

    m_BtnLeft.ChangeButtonImgState(true, IMAGE_CHATCOMMAND_BTN, true);
    m_BtnRight.ChangeButtonImgState(true, IMAGE_CHATCOMMAND_BTN, true);
}

float mu::ui::window::CChatCommandWindow::GetLayerDepth()
{
    return LayerDepth;
}

float mu::ui::window::CChatCommandWindow::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CChatCommandWindow::OpenningProcess()
{
    // Commands may have changed since last time, so always rebuild and start at the top.
    RebuildCommandOrder();
    m_selectedRow = -1;
    ShowPage(PAGE_COMMANDS);
}

void mu::ui::window::CChatCommandWindow::ClosingProcess()
{
    StopEditing();
}

void mu::ui::window::CChatCommandWindow::RebuildCommandOrder()
{
    const auto& commands = Catalog().GetCommands();
    m_commandOrder.clear();
    m_commandOrder.reserve(commands.size());

    for (size_t i = 0; i < commands.size(); ++i)
    {
        m_commandOrder.push_back(static_cast<int>(i));
    }

    // The favourites move to the top, the rest keeps the order of the server.
    std::stable_partition(m_commandOrder.begin(), m_commandOrder.end(), [&commands](int index)
                          { return GameLogic::Commands::Favourites::Contains(commands[index].Command); });
}

const ChatCommand* mu::ui::window::CChatCommandWindow::GetCommandAt(int row) const
{
    if (row < 0 || static_cast<size_t>(row) >= m_commandOrder.size())
    {
        return nullptr;
    }

    const auto& commands = Catalog().GetCommands();
    const auto index = m_commandOrder[row];
    if (index < 0 || static_cast<size_t>(index) >= commands.size())
    {
        return nullptr;
    }

    return &commands[index];
}

const ChatCommand* mu::ui::window::CChatCommandWindow::GetSelectedCommand() const
{
    return GetCommandAt(m_selectedRow);
}

void mu::ui::window::CChatCommandWindow::ShowPage(ePAGE page)
{
    StopEditing();
    m_page = page;
    m_scrollOffset = 0;

    switch (page)
    {
    case PAGE_COMMANDS:
        m_BtnRight.ChangeText(&I18N::Game::ChatCommandsTemplates);
        break;

    case PAGE_PARAMETERS:
        m_BtnLeft.ChangeText(&I18N::Game::ChatCommandsBack);
        m_BtnRight.ChangeText(&I18N::Game::ChatCommandsExecute);
        WrapDescriptionOfSelected();
        break;

    case PAGE_TEMPLATES:
        m_BtnLeft.ChangeText(&I18N::Game::ChatCommandsBack);
        m_templates = GameLogic::Commands::Templates::GetAll();
        break;
    }
}

void mu::ui::window::CChatCommandWindow::PickCommand(int row)
{
    m_selectedRow = row;
    m_parameterValues.clear();

    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return;
    }

    // No parameters means nothing to show; execute immediately.
    if (command->Parameters.empty())
    {
        ChatCommandCatalog::Execute(command->Command);
        PlayBuffer(SOUND_CLICK01);
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND_LIST);
        return;
    }

    m_parameterValues.resize(command->Parameters.size());
    ShowPage(PAGE_PARAMETERS);
}

bool mu::ui::window::CChatCommandWindow::AreRequiredValuesSet() const
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return false;
    }

    for (size_t i = 0; i < command->Parameters.size(); ++i)
    {
        if (command->Parameters[i].IsRequired && m_parameterValues[i].empty())
        {
            return false;
        }
    }

    return true;
}

void mu::ui::window::CChatCommandWindow::ExecuteSelectedCommand()
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return;
    }

    // Avoid a round trip for a server error; block locally when required values are missing.
    if (!AreRequiredValuesSet())
    {
        g_pSystemLogBox->AddText(I18N::Game::ChatCommandsFillRequired, mu::ui::window::TYPE_ERROR_MESSAGE);
        return;
    }

    ChatCommandCatalog::Execute(ChatCommandCatalog::BuildCommandLine(*command, m_parameterValues));
    PlayBuffer(SOUND_CLICK01);
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND_LIST);
}

void mu::ui::window::CChatCommandWindow::ExecuteTemplate(size_t index)
{
    if (index >= m_templates.size())
    {
        return;
    }

    const auto& entry = m_templates[index];
    for (const auto& command : Catalog().GetCommands())
    {
        if (command.Command != entry.Command)
        {
            continue;
        }

        ChatCommandCatalog::Execute(ChatCommandCatalog::BuildCommandLine(command, entry.Values));
        PlayBuffer(SOUND_CLICK01);
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND_LIST);
        return;
    }

    // Command no longer exists (e.g. plugin deactivated); sending would just error.
    g_pSystemLogBox->AddText(I18N::Game::ChatCommandsUnknownCommand, mu::ui::window::TYPE_ERROR_MESSAGE);
}

void mu::ui::window::CChatCommandWindow::SaveSelectedAsTemplate()
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return;
    }

    ChatCommandTemplate entry;
    entry.Label = ChatCommandCatalog::BuildCommandLine(*command, m_parameterValues);
    entry.Command = command->Command;
    entry.Values = m_parameterValues;
    GameLogic::Commands::Templates::Add(entry);
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CChatCommandWindow::ToggleFavouriteOfSelected()
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return;
    }

    GameLogic::Commands::Favourites::Toggle(command->Command);
    PlayBuffer(SOUND_CLICK01);

    // The order changed, so keep pointing at the same command.
    const auto selected = command->Command;
    RebuildCommandOrder();
    for (size_t row = 0; row < m_commandOrder.size(); ++row)
    {
        const auto* candidate = GetCommandAt(static_cast<int>(row));
        if (candidate != nullptr && candidate->Command == selected)
        {
            m_selectedRow = static_cast<int>(row);
            break;
        }
    }
}

std::vector<std::wstring> mu::ui::window::CChatCommandWindow::SplitValidValues(const std::wstring& validValues)
{
    std::vector<std::wstring> values;
    size_t start = 0;
    while (true)
    {
        const auto separator = validValues.find(L'|', start);
        if (separator == std::wstring::npos)
        {
            values.push_back(validValues.substr(start));
            return values;
        }

        values.push_back(validValues.substr(start, separator - start));
        start = separator + 1;
    }
}

bool mu::ui::window::CChatCommandWindow::IsPickedFromList(const ChatCommandParameter& parameter)
{
    // Server sends accepted values for anything with a fixed set, booleans included.
    return !parameter.ValidValues.empty();
}

void mu::ui::window::CChatCommandWindow::CycleParameterValue(size_t parameterIndex)
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr || parameterIndex >= command->Parameters.size())
    {
        return;
    }

    const auto& parameter = command->Parameters[parameterIndex];
    if (parameter.ValidValues.empty())
    {
        return;
    }

    const auto values = SplitValidValues(parameter.ValidValues);
    const auto& current = m_parameterValues[parameterIndex];
    size_t next = 0;
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i] == current)
        {
            next = i + 1;
            break;
        }
    }

    // Cycling past the last value clears it -- how an optional parameter is left blank.
    m_parameterValues[parameterIndex] = (next >= values.size()) ? std::wstring() : values[next];
}

void mu::ui::window::CChatCommandWindow::BeginEditingParameter(size_t parameterIndex)
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == nullptr || parameterIndex >= command->Parameters.size() || m_pValueInput == nullptr)
    {
        return;
    }

    m_editedParameter = static_cast<int>(parameterIndex);
    const auto valueY = GetParameterTop() + static_cast<int>(parameterIndex) * PARAMETER_HEIGHT + ROW_HEIGHT;
    m_pValueInput->SetPosition(m_Pos.x + CONTENT_LEFT + 2, valueY + 1);

    // Restrict input to digits when the parameter is numeric, matching what the server accepts.
    m_pValueInput->SetOption(command->Parameters[parameterIndex].Type == ChatCommandParameterType::Number
                                 ? UIOPTION_NUMBERONLY
                                 : UIOPTION_NULL);
    m_pValueInput->SetText(m_parameterValues[parameterIndex].c_str());
    m_pValueInput->SetState(UISTATE_NORMAL);
    m_pValueInput->GiveFocus();

    // Needed so Escape still reaches this window while the input field has focus.
    SetRelatedWnd(m_pValueInput->GetHandle());
}

void mu::ui::window::CChatCommandWindow::CommitEditedValue()
{
    if (m_editedParameter < 0 || m_pValueInput == nullptr ||
        static_cast<size_t>(m_editedParameter) >= m_parameterValues.size())
    {
        return;
    }

    wchar_t text[MAX_TEXT_LENGTH] = {0};
    m_pValueInput->GetText(text, MAX_TEXT_LENGTH);
    m_parameterValues[m_editedParameter] = text;
}

void mu::ui::window::CChatCommandWindow::StopEditing()
{
    CommitEditedValue();
    m_editedParameter = -1;
    if (m_pValueInput != nullptr)
    {
        m_pValueInput->SetText(nullptr);
        m_pValueInput->SetState(UISTATE_HIDE);
    }

    SetRelatedWnd(g_hWnd);
}

int mu::ui::window::CChatCommandWindow::GetScrollableRowCount() const
{
    if (m_page == PAGE_TEMPLATES)
    {
        return static_cast<int>(m_templates.size());
    }

    return static_cast<int>(m_commandOrder.size());
}

void mu::ui::window::CChatCommandWindow::WrapDescriptionOfSelected()
{
    m_descriptionLines.clear();

    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return;
    }

    g_pRenderText->SetFont(g_hFont);
    m_descriptionLines = WrapTextToWidth(command->Description, CONTENT_WIDTH, MeasureInReferenceUnits);
}

int mu::ui::window::CChatCommandWindow::GetVisibleDescriptionLineCount() const
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return 0;
    }

    // Space needed for the parameters, the two actions below them, and the gap before them.
    const auto reserved = static_cast<int>(command->Parameters.size()) * PARAMETER_HEIGHT + 3 * ROW_HEIGHT;
    const auto available = CONTENT_BOTTOM - CONTENT_TOP - reserved;
    const auto fitting = std::max(0, available / ROW_HEIGHT);
    return std::min(fitting, static_cast<int>(m_descriptionLines.size()));
}

int mu::ui::window::CChatCommandWindow::GetParameterTop() const
{
    return m_Pos.y + CONTENT_TOP + (GetVisibleDescriptionLineCount() + 1) * ROW_HEIGHT;
}

int mu::ui::window::CChatCommandWindow::GetActionTop() const
{
    const auto* command = GetSelectedCommand();
    const auto parameterCount = (command == nullptr) ? 0 : static_cast<int>(command->Parameters.size());
    return GetParameterTop() + parameterCount * PARAMETER_HEIGHT + ROW_HEIGHT;
}

bool mu::ui::window::CChatCommandWindow::UpdateMouseEvent()
{
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_COMMAND_LIST))
    {
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND_LIST);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (HasLeftButton() && m_BtnLeft.UpdateMouseEvent())
    {
        StopEditing();
        ShowPage(PAGE_COMMANDS);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (HasRightButton() && m_BtnRight.UpdateMouseEvent())
    {
        if (m_page == PAGE_COMMANDS)
        {
            ShowPage(PAGE_TEMPLATES);
            PlayBuffer(SOUND_CLICK01);
        }
        else
        {
            ExecuteSelectedCommand();
        }

        return false;
    }

    const bool handled = (m_page == PAGE_COMMANDS)     ? UpdateCommandPageMouseEvent()
                         : (m_page == PAGE_PARAMETERS) ? UpdateParameterPageMouseEvent()
                                                       : UpdateTemplatePageMouseEvent();
    if (handled)
    {
        return false;
    }

    if (!mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WindowHeight).Contains(MouseX, MouseY))
    {
        return true;
    }

    const auto hiddenRows = GetScrollableRowCount() - VISIBLE_ROWS;
    if (MouseWheel != 0 && hiddenRows > 0)
    {
        // MouseWheel counts notches, one row per notch.
        m_scrollOffset = std::max(0, std::min(m_scrollOffset - MouseWheel, hiddenRows));
        MouseWheel = 0;
    }

    return false;
}

bool mu::ui::window::CChatCommandWindow::UpdateCommandPageMouseEvent()
{
    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto index = m_scrollOffset + row;
        if (GetCommandAt(index) == nullptr)
        {
            break;
        }

        if (CheckMouseIn(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP + row * ROW_HEIGHT, CONTENT_WIDTH, ROW_HEIGHT) &&
            IsRelease(VK_LBUTTON))
        {
            PlayBuffer(SOUND_CLICK01);
            PickCommand(index);
            return true;
        }
    }

    return false;
}

bool mu::ui::window::CChatCommandWindow::UpdateParameterPageMouseEvent()
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return false;
    }

    for (size_t i = 0; i < command->Parameters.size(); ++i)
    {
        const auto valueY = GetParameterTop() + static_cast<int>(i) * PARAMETER_HEIGHT + ROW_HEIGHT;
        if (!CheckMouseIn(m_Pos.x + CONTENT_LEFT, valueY, CONTENT_WIDTH, VALUE_HEIGHT) || !IsRelease(VK_LBUTTON))
        {
            continue;
        }

        if (IsPickedFromList(command->Parameters[i]))
        {
            StopEditing();
            CycleParameterValue(i);
        }
        else
        {
            BeginEditingParameter(i);
        }

        PlayBuffer(SOUND_CLICK01);
        return true;
    }

    const auto actionTop = GetActionTop();
    if (CheckMouseIn(m_Pos.x + CONTENT_LEFT, actionTop, CONTENT_WIDTH, ROW_HEIGHT) && IsRelease(VK_LBUTTON))
    {
        ToggleFavouriteOfSelected();
        return true;
    }

    if (CheckMouseIn(m_Pos.x + CONTENT_LEFT, actionTop + ROW_HEIGHT, CONTENT_WIDTH, ROW_HEIGHT) &&
        IsRelease(VK_LBUTTON))
    {
        SaveSelectedAsTemplate();
        return true;
    }

    return false;
}

bool mu::ui::window::CChatCommandWindow::UpdateTemplatePageMouseEvent()
{
    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto index = static_cast<size_t>(m_scrollOffset + row);
        if (index >= m_templates.size())
        {
            break;
        }

        const auto rowY = m_Pos.y + CONTENT_TOP + row * ROW_HEIGHT;
        if (!IsRelease(VK_LBUTTON))
        {
            continue;
        }

        if (CheckMouseIn(m_Pos.x + CONTENT_LEFT + CONTENT_WIDTH - ROW_HEIGHT, rowY, ROW_HEIGHT, ROW_HEIGHT))
        {
            GameLogic::Commands::Templates::RemoveAt(index);
            m_templates = GameLogic::Commands::Templates::GetAll();
            PlayBuffer(SOUND_CLICK01);
            return true;
        }

        if (CheckMouseIn(m_Pos.x + CONTENT_LEFT, rowY, CONTENT_WIDTH - ROW_HEIGHT, ROW_HEIGHT))
        {
            ExecuteTemplate(index);
            return true;
        }
    }

    return false;
}

bool mu::ui::window::CChatCommandWindow::UpdateKeyEvent()
{
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_COMMAND_LIST))
    {
        return true;
    }

    if (IsPress(VK_ESCAPE))
    {
        // Escape backs out one step at a time: field, then page, then close.
        if (m_editedParameter >= 0)
        {
            StopEditing();
        }
        else if (m_page != PAGE_COMMANDS)
        {
            ShowPage(PAGE_COMMANDS);
        }
        else
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND_LIST);
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (m_editedParameter >= 0)
    {
        if (IsPress(VK_RETURN))
        {
            StopEditing();
            return false;
        }

        // Everything else is typed into the field.
        return true;
    }

    const auto hiddenRows = GetScrollableRowCount() - VISIBLE_ROWS;
    if (IsPress(VK_DOWN) && m_scrollOffset < hiddenRows)
    {
        ++m_scrollOffset;
        return false;
    }

    if (IsPress(VK_UP) && m_scrollOffset > 0)
    {
        --m_scrollOffset;
        return false;
    }

    return true;
}

bool mu::ui::window::CChatCommandWindow::Update()
{
    return true;
}

bool mu::ui::window::CChatCommandWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    g_pRenderText->SetFont(g_hFont);
    UseTextColor(NormalColor);

    RenderBaseWindow();
    RenderTitle();

    switch (m_page)
    {
    case PAGE_COMMANDS:
        RenderCommandPage();
        break;

    case PAGE_PARAMETERS:
        RenderParameterPage();
        break;

    case PAGE_TEMPLATES:
        RenderTemplatePage();
        break;
    }

    if (HasLeftButton())
    {
        m_BtnLeft.SetFont(g_hFont);
        m_BtnLeft.Render();
    }

    if (HasRightButton())
    {
        m_BtnRight.SetFont(g_hFont);
        m_BtnRight.Render();
    }

    m_BtnExit.Render();
    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CChatCommandWindow::RenderBaseWindow()
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);
    const auto middleHeight = static_cast<float>(WindowHeight - FRAME_TOP_HEIGHT - FRAME_BOTTOM_HEIGHT);

    RenderImage(IMAGE_CHATCOMMAND_BACK, x, y, float(WINDOW_WIDTH), float(WindowHeight));
    RenderImage(IMAGE_CHATCOMMAND_TOP, x, y, float(WINDOW_WIDTH), float(FRAME_TOP_HEIGHT));

    // Side pieces are stretched rather than tiled -- the window is taller than the texture,
    // and they're a plain vertical border, so stretching doesn't show.
    RenderImageStretch(IMAGE_CHATCOMMAND_LEFT, x, y + float(FRAME_TOP_HEIGHT), float(FRAME_SIDE_WIDTH), middleHeight,
                       0.f, 0.f, float(FRAME_SIDE_WIDTH), float(FRAME_SIDE_TEXTURE_HEIGHT));
    RenderImageStretch(IMAGE_CHATCOMMAND_RIGHT, x + float(WINDOW_WIDTH - FRAME_SIDE_WIDTH), y + float(FRAME_TOP_HEIGHT),
                       float(FRAME_SIDE_WIDTH), middleHeight, 0.f, 0.f, float(FRAME_SIDE_WIDTH),
                       float(FRAME_SIDE_TEXTURE_HEIGHT));

    RenderImage(IMAGE_CHATCOMMAND_BOTTOM, x, y + float(WindowHeight - FRAME_BOTTOM_HEIGHT), float(WINDOW_WIDTH),
                float(FRAME_BOTTOM_HEIGHT));
}

void mu::ui::window::CChatCommandWindow::RenderTitle()
{
    const wchar_t* title = I18N::Game::ChatCommandsTitle;
    if (m_page == PAGE_TEMPLATES)
    {
        title = I18N::Game::ChatCommandsTemplates;
    }
    else if (m_page == PAGE_PARAMETERS && GetSelectedCommand() != nullptr)
    {
        title = GetSelectedCommand()->Command.c_str();
    }

    g_pRenderText->SetFont(g_hFontBold);
    UseTextColor(TitleColor);
    RenderLine(m_Pos.x, m_Pos.y + TITLE_Y, title, WINDOW_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetFont(g_hFont);
}

void mu::ui::window::CChatCommandWindow::RenderCommandPage()
{
    if (m_commandOrder.empty())
    {
        UseTextColor(NormalColor);
        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP, I18N::Game::ChatCommandsNotSupported, CONTENT_WIDTH,
                   VISIBLE_ROWS * ROW_HEIGHT);
        return;
    }

    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto* command = GetCommandAt(m_scrollOffset + row);
        if (command == nullptr)
        {
            break;
        }

        const bool isFavourite = GameLogic::Commands::Favourites::Contains(command->Command);
        UseTextColor(isFavourite ? FavouriteColor : NormalColor);

        // Only the command name fits at this width; parameters are just flagged with "...".
        std::wstring text = isFavourite ? FavouriteMarker : L"";
        text += command->Command;
        if (!command->Parameters.empty())
        {
            text += ParameterMarker;
        }

        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP + row * ROW_HEIGHT, text.c_str(), CONTENT_WIDTH);
    }
}

void mu::ui::window::CChatCommandWindow::RenderParameterPage()
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr)
    {
        return;
    }

    UseTextColor(DescriptionColor);
    const auto descriptionLines = GetVisibleDescriptionLineCount();
    for (int line = 0; line < descriptionLines; ++line)
    {
        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP + line * ROW_HEIGHT, m_descriptionLines[line].c_str(),
                   CONTENT_WIDTH);
    }

    for (size_t i = 0; i < command->Parameters.size(); ++i)
    {
        RenderParameter(i, GetParameterTop() + static_cast<int>(i) * PARAMETER_HEIGHT);
    }

    if (m_editedParameter >= 0 && m_pValueInput != nullptr)
    {
        m_pValueInput->Render();
    }

    const auto actionTop = GetActionTop();
    const bool isFavourite = GameLogic::Commands::Favourites::Contains(command->Command);
    UseTextColor(ActionColor);
    RenderLine(m_Pos.x + CONTENT_LEFT, actionTop,
               isFavourite ? I18N::Game::ChatCommandsRemoveFavourite : I18N::Game::ChatCommandsAddFavourite,
               CONTENT_WIDTH);
    RenderLine(m_Pos.x + CONTENT_LEFT, actionTop + ROW_HEIGHT, I18N::Game::ChatCommandsSaveTemplate, CONTENT_WIDTH);
}

void mu::ui::window::CChatCommandWindow::RenderParameter(size_t parameterIndex, int y)
{
    const auto* command = GetSelectedCommand();
    if (command == nullptr || parameterIndex >= command->Parameters.size())
    {
        return;
    }

    const auto& parameter = command->Parameters[parameterIndex];
    const auto& value = m_parameterValues[parameterIndex];

    // Highlight required parameters that are still empty -- they're what's blocking send.
    const bool isMissing = parameter.IsRequired && value.empty();
    UseTextColor(isMissing ? MissingValueColor : NormalColor);

    std::wstring label = parameter.Name;
    if (parameter.IsRequired)
    {
        label += L" *";
    }

    RenderLine(m_Pos.x + CONTENT_LEFT, y, label.c_str(), CONTENT_WIDTH);

    RenderValueBackground(m_Pos.x + CONTENT_LEFT, y + ROW_HEIGHT, CONTENT_WIDTH, VALUE_HEIGHT);

    if (m_editedParameter == static_cast<int>(parameterIndex))
    {
        // The text box draws what's being typed.
        return;
    }

    UseTextColor(value.empty() ? DescriptionColor : NormalColor);
    const auto* shown = value.empty() ? parameter.ValidValues.c_str() : value.c_str();
    RenderLine(m_Pos.x + CONTENT_LEFT + 2, y + ROW_HEIGHT + 1, shown, CONTENT_WIDTH - 4);
}

void mu::ui::window::CChatCommandWindow::RenderTemplatePage()
{
    if (m_templates.empty())
    {
        UseTextColor(NormalColor);
        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP, I18N::Game::ChatCommandsNoTemplates, CONTENT_WIDTH,
                   VISIBLE_ROWS * ROW_HEIGHT);
        return;
    }

    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto index = static_cast<size_t>(m_scrollOffset + row);
        if (index >= m_templates.size())
        {
            break;
        }

        const auto rowY = m_Pos.y + CONTENT_TOP + row * ROW_HEIGHT;
        UseTextColor(NormalColor);
        RenderLine(m_Pos.x + CONTENT_LEFT, rowY, m_templates[index].Label.c_str(), CONTENT_WIDTH - ROW_HEIGHT);
        UseTextColor(MissingValueColor);
        RenderLine(m_Pos.x + CONTENT_LEFT + CONTENT_WIDTH - ROW_HEIGHT, rowY, L"x", ROW_HEIGHT, 0, RT3_SORT_CENTER);
    }
}

void mu::ui::window::CChatCommandWindow::LoadImages()
{
    // IDs are shared with other windows, but each window loads its own images --
    // relying on another window having loaded them first would show an empty frame.
    LoadBitmap(L"Interface/newui_msgbox_back.jpg", IMAGE_CHATCOMMAND_BACK, GL_LINEAR);
    LoadBitmap(L"Interface/newui_item_back01.tga", IMAGE_CHATCOMMAND_TOP, GL_LINEAR);
    LoadBitmap(L"Interface/newui_item_back02-L.tga", IMAGE_CHATCOMMAND_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface/newui_item_back02-R.tga", IMAGE_CHATCOMMAND_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface/newui_item_back03.tga", IMAGE_CHATCOMMAND_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface/newui_exit_00.tga", IMAGE_CHATCOMMAND_BTN_EXIT, GL_LINEAR);
    LoadBitmap(L"Interface/newui_btn_empty_small.tga", IMAGE_CHATCOMMAND_BTN, GL_LINEAR);
}

void mu::ui::window::CChatCommandWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_CHATCOMMAND_BACK);
    DeleteBitmap(IMAGE_CHATCOMMAND_TOP);
    DeleteBitmap(IMAGE_CHATCOMMAND_LEFT);
    DeleteBitmap(IMAGE_CHATCOMMAND_RIGHT);
    DeleteBitmap(IMAGE_CHATCOMMAND_BOTTOM);
    DeleteBitmap(IMAGE_CHATCOMMAND_BTN_EXIT);
    DeleteBitmap(IMAGE_CHATCOMMAND_BTN);
}
