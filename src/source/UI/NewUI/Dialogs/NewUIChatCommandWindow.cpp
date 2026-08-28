// NewUIChatCommandWindow.cpp: implementation of the CNewUIChatCommandWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I18N/All.h"

#include "UI/NewUI/Dialogs/NewUIChatCommandWindow.h"

#include "Audio/DSPlaySound.h"
#include "Core/Text/TextLineWrap.h"
#include "GameLogic/Commands/ChatCommandFavourites.h"
#include "UI/NewUI/NewUISystem.h"

#include <algorithm>

using namespace SEASON3B;
using GameLogic::Commands::Catalog;
using GameLogic::Commands::ChatCommand;
using GameLogic::Commands::ChatCommandCatalog;
using GameLogic::Commands::ChatCommandParameter;
using GameLogic::Commands::ChatCommandParameterType;
using GameLogic::Commands::ChatCommandTemplate;

namespace
{
    // A favourite is marked with a leading character instead of an icon, so
    // that it works with every font the client is started with.
    constexpr const wchar_t* FavouriteMarker = L"* ";
    // What tells the player that a command wants something filled in. The names
    // of the parameters don't fit next to the command at this width.
    constexpr const wchar_t* ParameterMarker = L" ...";

    struct TextColor
    {
        BYTE Red;
        BYTE Green;
        BYTE Blue;
    };

    constexpr TextColor TitleColor = { 255, 220, 120 };
    constexpr TextColor NormalColor = { 220, 220, 220 };
    constexpr TextColor FavouriteColor = { 255, 220, 120 };
    constexpr TextColor DescriptionColor = { 200, 220, 255 };
    constexpr TextColor MissingValueColor = { 255, 150, 150 };
    constexpr TextColor ActionColor = { 150, 210, 255 };

    void UseTextColor(const TextColor& color)
    {
        g_pRenderText->SetTextColor(color.Red, color.Green, color.Blue, 255);
        g_pRenderText->SetBgColor(0);
    }

    // Renders one line of the window. An empty text has to be skipped: the text
    // renderer measures a placeholder for it and would leave a stray glyph
    // behind whenever a box width is given.
    void RenderLine(int x, int y, const wchar_t* text, int boxWidth, int boxHeight = 0, int sort = RT3_SORT_LEFT)
    {
        if (text == nullptr || text[0] == L'\0')
        {
            return;
        }

        g_pRenderText->RenderText(x, y, text, boxWidth, boxHeight, sort);
    }

    // The text renderer works in real pixels while the window is laid out in
    // reference units, which is what the screen rate converts between.
    int MeasureInReferenceUnits(const wchar_t* text, size_t length)
    {
        SIZE size = { 0, 0 };
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), text, static_cast<int>(length), &size);
        return (g_fScreenRate_x > 0.f) ? static_cast<int>(size.cx / g_fScreenRate_x) : size.cx;
    }

    void RenderValueBackground(int x, int y, int width, int height)
    {
        glColor4ub(0, 0, 0, 160);
        RenderColor(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));
        EndRenderColor();
        glColor4f(1.f, 1.f, 1.f, 1.f);
    }
}

SEASON3B::CNewUIChatCommandWindow::CNewUIChatCommandWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
    m_page = PAGE_COMMANDS;
    m_selectedRow = -1;
    m_scrollOffset = 0;
    m_editedParameter = -1;
}

SEASON3B::CNewUIChatCommandWindow::~CNewUIChatCommandWindow()
{
    Release();
}

bool SEASON3B::CNewUIChatCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_COMMAND_LIST, this);

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

void SEASON3B::CNewUIChatCommandWindow::Release()
{
    UnloadImages();
    m_pValueInput.reset();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIChatCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + EXIT_BUTTON_X, m_Pos.y + EXIT_BUTTON_Y, EXIT_BUTTON_WIDTH, EXIT_BUTTON_HEIGHT);
    m_BtnLeft.ChangeButtonInfo(m_Pos.x + CONTENT_LEFT, m_Pos.y + BUTTON_ROW_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
    m_BtnRight.ChangeButtonInfo(m_Pos.x + WINDOW_WIDTH - CONTENT_LEFT - BUTTON_WIDTH, m_Pos.y + BUTTON_ROW_Y, BUTTON_WIDTH, BUTTON_HEIGHT);
}

void SEASON3B::CNewUIChatCommandWindow::InitButtons()
{
    wchar_t closeText[256] = {};
    mu_swprintf(closeText, I18N::Game::CloseS, L"J");

    m_BtnExit.ChangeButtonImgState(true, IMAGE_CHATCOMMAND_BTN_EXIT);
    m_BtnExit.ChangeToolTipText(closeText, true);

    m_BtnLeft.ChangeButtonImgState(true, IMAGE_CHATCOMMAND_BTN, true);
    m_BtnRight.ChangeButtonImgState(true, IMAGE_CHATCOMMAND_BTN, true);
}

float SEASON3B::CNewUIChatCommandWindow::GetLayerDepth()
{
    return 4.6f;
}

float SEASON3B::CNewUIChatCommandWindow::GetKeyEventOrder()
{
    return 10.f;
}

void SEASON3B::CNewUIChatCommandWindow::OpenningProcess()
{
    // The player may have gained or lost commands since the last time, so the
    // window always starts at the top of a freshly ordered list.
    RebuildCommandOrder();
    m_selectedRow = -1;
    ShowPage(PAGE_COMMANDS);
}

void SEASON3B::CNewUIChatCommandWindow::ClosingProcess()
{
    StopEditing();
}

void SEASON3B::CNewUIChatCommandWindow::RebuildCommandOrder()
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
        {
            return GameLogic::Commands::Favourites::Contains(commands[index].Command);
        });
}

const ChatCommand* SEASON3B::CNewUIChatCommandWindow::GetCommandAt(int row) const
{
    if (row < 0 || static_cast<size_t>(row) >= m_commandOrder.size())
    {
        return NULL;
    }

    const auto& commands = Catalog().GetCommands();
    const auto index = m_commandOrder[row];
    if (index < 0 || static_cast<size_t>(index) >= commands.size())
    {
        return NULL;
    }

    return &commands[index];
}

const ChatCommand* SEASON3B::CNewUIChatCommandWindow::GetSelectedCommand() const
{
    return GetCommandAt(m_selectedRow);
}

void SEASON3B::CNewUIChatCommandWindow::ShowPage(ePAGE page)
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

void SEASON3B::CNewUIChatCommandWindow::PickCommand(int row)
{
    m_selectedRow = row;
    m_parameterValues.clear();

    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return;
    }

    // Nothing to fill in means nothing to show - the command is what the player
    // wanted, so send it and get out of the way.
    if (command->Parameters.empty())
    {
        ChatCommandCatalog::Execute(command->Command);
        PlayBuffer(SOUND_CLICK01);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
        return;
    }

    m_parameterValues.resize(command->Parameters.size());
    ShowPage(PAGE_PARAMETERS);
}

bool SEASON3B::CNewUIChatCommandWindow::AreRequiredValuesSet() const
{
    const auto* command = GetSelectedCommand();
    if (command == NULL)
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

void SEASON3B::CNewUIChatCommandWindow::ExecuteSelectedCommand()
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return;
    }

    // A missing required value would only earn an error message from the
    // server, so point at the parameter instead of sending anything.
    if (!AreRequiredValuesSet())
    {
        g_pSystemLogBox->AddText(I18N::Game::ChatCommandsFillRequired, SEASON3B::TYPE_ERROR_MESSAGE);
        return;
    }

    ChatCommandCatalog::Execute(ChatCommandCatalog::BuildCommandLine(*command, m_parameterValues));
    PlayBuffer(SOUND_CLICK01);
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
}

void SEASON3B::CNewUIChatCommandWindow::ExecuteTemplate(size_t index)
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
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
        return;
    }

    // The command is gone, e.g. because the plugin was deactivated on the
    // server. Sending it anyway would only produce an error message.
    g_pSystemLogBox->AddText(I18N::Game::ChatCommandsUnknownCommand, SEASON3B::TYPE_ERROR_MESSAGE);
}

void SEASON3B::CNewUIChatCommandWindow::SaveSelectedAsTemplate()
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == NULL)
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

void SEASON3B::CNewUIChatCommandWindow::ToggleFavouriteOfSelected()
{
    const auto* command = GetSelectedCommand();
    if (command == NULL)
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
        if (candidate != NULL && candidate->Command == selected)
        {
            m_selectedRow = static_cast<int>(row);
            break;
        }
    }
}

std::vector<std::wstring> SEASON3B::CNewUIChatCommandWindow::SplitValidValues(const std::wstring& validValues)
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

bool SEASON3B::CNewUIChatCommandWindow::IsPickedFromList(const ChatCommandParameter& parameter)
{
    // The server sends the accepted values for everything which only takes a
    // known set of them, booleans included.
    return !parameter.ValidValues.empty();
}

void SEASON3B::CNewUIChatCommandWindow::CycleParameterValue(size_t parameterIndex)
{
    const auto* command = GetSelectedCommand();
    if (command == NULL || parameterIndex >= command->Parameters.size())
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

    // One step past the last value clears it again, which is how an optional
    // parameter is left out.
    m_parameterValues[parameterIndex] = (next >= values.size()) ? std::wstring() : values[next];
}

void SEASON3B::CNewUIChatCommandWindow::BeginEditingParameter(size_t parameterIndex)
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == NULL || parameterIndex >= command->Parameters.size() || m_pValueInput == nullptr)
    {
        return;
    }

    m_editedParameter = static_cast<int>(parameterIndex);
    const auto valueY = GetParameterTop() + static_cast<int>(parameterIndex) * PARAMETER_HEIGHT + ROW_HEIGHT;
    m_pValueInput->SetPosition(m_Pos.x + CONTENT_LEFT + 2, valueY + 1);

    // A number field which accepts letters only leads to a command the server
    // rejects, so let the field enforce what the parameter takes.
    m_pValueInput->SetOption(command->Parameters[parameterIndex].Type == ChatCommandParameterType::Number
        ? UIOPTION_NUMBERONLY
        : UIOPTION_NULL);
    m_pValueInput->SetText(m_parameterValues[parameterIndex].c_str());
    m_pValueInput->SetState(UISTATE_NORMAL);
    m_pValueInput->GiveFocus();

    // While a field owns the keyboard, only the window it belongs to gets the
    // key events - without this, escape wouldn't reach us anymore.
    SetRelatedWnd(m_pValueInput->GetHandle());
}

void SEASON3B::CNewUIChatCommandWindow::CommitEditedValue()
{
    if (m_editedParameter < 0 || m_pValueInput == nullptr
        || static_cast<size_t>(m_editedParameter) >= m_parameterValues.size())
    {
        return;
    }

    wchar_t text[MAX_TEXT_LENGTH] = { 0 };
    m_pValueInput->GetText(text, MAX_TEXT_LENGTH);
    m_parameterValues[m_editedParameter] = text;
}

void SEASON3B::CNewUIChatCommandWindow::StopEditing()
{
    CommitEditedValue();
    m_editedParameter = -1;
    if (m_pValueInput != nullptr)
    {
        m_pValueInput->SetText(NULL);
        m_pValueInput->SetState(UISTATE_HIDE);
    }

    SetRelatedWnd(NULL);
}

int SEASON3B::CNewUIChatCommandWindow::GetScrollableRowCount() const
{
    if (m_page == PAGE_TEMPLATES)
    {
        return static_cast<int>(m_templates.size());
    }

    return static_cast<int>(m_commandOrder.size());
}

void SEASON3B::CNewUIChatCommandWindow::WrapDescriptionOfSelected()
{
    m_descriptionLines.clear();

    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return;
    }

    g_pRenderText->SetFont(g_hFont);
    m_descriptionLines = WrapTextToWidth(command->Description, CONTENT_WIDTH, MeasureInReferenceUnits);
}

int SEASON3B::CNewUIChatCommandWindow::GetVisibleDescriptionLineCount() const
{
    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return 0;
    }

    // What the parameters and the two actions below them need, plus the gap
    // which separates them from the description.
    const auto reserved = static_cast<int>(command->Parameters.size()) * PARAMETER_HEIGHT + 3 * ROW_HEIGHT;
    const auto available = CONTENT_BOTTOM - CONTENT_TOP - reserved;
    const auto fitting = std::max(0, available / ROW_HEIGHT);
    return std::min(fitting, static_cast<int>(m_descriptionLines.size()));
}

int SEASON3B::CNewUIChatCommandWindow::GetParameterTop() const
{
    return m_Pos.y + CONTENT_TOP + (GetVisibleDescriptionLineCount() + 1) * ROW_HEIGHT;
}

int SEASON3B::CNewUIChatCommandWindow::GetActionTop() const
{
    const auto* command = GetSelectedCommand();
    const auto parameterCount = (command == NULL) ? 0 : static_cast<int>(command->Parameters.size());
    return GetParameterTop() + parameterCount * PARAMETER_HEIGHT + ROW_HEIGHT;
}

bool SEASON3B::CNewUIChatCommandWindow::UpdateMouseEvent()
{
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, SEASON3B::INTERFACE_COMMAND_LIST))
    {
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
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

    const bool handled = (m_page == PAGE_COMMANDS) ? UpdateCommandPageMouseEvent()
        : (m_page == PAGE_PARAMETERS) ? UpdateParameterPageMouseEvent()
        : UpdateTemplatePageMouseEvent();
    if (handled)
    {
        return false;
    }

    if (!CheckMouseIn(m_Pos.x, m_Pos.y, WINDOW_WIDTH, WINDOW_HEIGHT))
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

bool SEASON3B::CNewUIChatCommandWindow::UpdateCommandPageMouseEvent()
{
    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto index = m_scrollOffset + row;
        if (GetCommandAt(index) == NULL)
        {
            break;
        }

        if (CheckMouseIn(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP + row * ROW_HEIGHT, CONTENT_WIDTH, ROW_HEIGHT)
            && IsRelease(VK_LBUTTON))
        {
            PlayBuffer(SOUND_CLICK01);
            PickCommand(index);
            return true;
        }
    }

    return false;
}

bool SEASON3B::CNewUIChatCommandWindow::UpdateParameterPageMouseEvent()
{
    const auto* command = GetSelectedCommand();
    if (command == NULL)
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

    if (CheckMouseIn(m_Pos.x + CONTENT_LEFT, actionTop + ROW_HEIGHT, CONTENT_WIDTH, ROW_HEIGHT) && IsRelease(VK_LBUTTON))
    {
        SaveSelectedAsTemplate();
        return true;
    }

    return false;
}

bool SEASON3B::CNewUIChatCommandWindow::UpdateTemplatePageMouseEvent()
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

bool SEASON3B::CNewUIChatCommandWindow::UpdateKeyEvent()
{
    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND_LIST))
    {
        return true;
    }

    if (IsPress(VK_ESCAPE))
    {
        // The first escape leaves the field, the next one goes back a page and
        // then closes the window.
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
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
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

bool SEASON3B::CNewUIChatCommandWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIChatCommandWindow::Render()
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

void SEASON3B::CNewUIChatCommandWindow::RenderBaseWindow()
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);
    const auto middleHeight = static_cast<float>(WINDOW_HEIGHT - FRAME_TOP_HEIGHT - FRAME_BOTTOM_HEIGHT);

    RenderImage(IMAGE_CHATCOMMAND_BACK, x, y, float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
    RenderImage(IMAGE_CHATCOMMAND_TOP, x, y, float(WINDOW_WIDTH), float(FRAME_TOP_HEIGHT));

    // The side pieces are stretched instead of drawn one to one: the window is
    // taller than they are, and asking for more of them than they have samples
    // past their end. They are a plain vertical border, so stretching them
    // doesn't show.
    RenderImageStretch(IMAGE_CHATCOMMAND_LEFT, x, y + float(FRAME_TOP_HEIGHT), float(FRAME_SIDE_WIDTH), middleHeight,
        0.f, 0.f, float(FRAME_SIDE_WIDTH), float(FRAME_SIDE_TEXTURE_HEIGHT));
    RenderImageStretch(IMAGE_CHATCOMMAND_RIGHT, x + float(WINDOW_WIDTH - FRAME_SIDE_WIDTH), y + float(FRAME_TOP_HEIGHT), float(FRAME_SIDE_WIDTH), middleHeight,
        0.f, 0.f, float(FRAME_SIDE_WIDTH), float(FRAME_SIDE_TEXTURE_HEIGHT));

    RenderImage(IMAGE_CHATCOMMAND_BOTTOM, x, y + float(WINDOW_HEIGHT - FRAME_BOTTOM_HEIGHT), float(WINDOW_WIDTH), float(FRAME_BOTTOM_HEIGHT));
}

void SEASON3B::CNewUIChatCommandWindow::RenderTitle()
{
    const wchar_t* title = I18N::Game::ChatCommandsTitle;
    if (m_page == PAGE_TEMPLATES)
    {
        title = I18N::Game::ChatCommandsTemplates;
    }
    else if (m_page == PAGE_PARAMETERS && GetSelectedCommand() != NULL)
    {
        title = GetSelectedCommand()->Command.c_str();
    }

    g_pRenderText->SetFont(g_hFontBold);
    UseTextColor(TitleColor);
    RenderLine(m_Pos.x, m_Pos.y + TITLE_Y, title, WINDOW_WIDTH, 0, RT3_SORT_CENTER);
    g_pRenderText->SetFont(g_hFont);
}

void SEASON3B::CNewUIChatCommandWindow::RenderCommandPage()
{
    if (m_commandOrder.empty())
    {
        UseTextColor(NormalColor);
        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP, I18N::Game::ChatCommandsNotSupported, CONTENT_WIDTH, VISIBLE_ROWS * ROW_HEIGHT);
        return;
    }

    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto* command = GetCommandAt(m_scrollOffset + row);
        if (command == NULL)
        {
            break;
        }

        const bool isFavourite = GameLogic::Commands::Favourites::Contains(command->Command);
        UseTextColor(isFavourite ? FavouriteColor : NormalColor);

        // Only the command itself fits at this width, so the parameters are
        // announced by three dots instead of being named here.
        std::wstring text = isFavourite ? FavouriteMarker : L"";
        text += command->Command;
        if (!command->Parameters.empty())
        {
            text += ParameterMarker;
        }

        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP + row * ROW_HEIGHT, text.c_str(), CONTENT_WIDTH);
    }
}

void SEASON3B::CNewUIChatCommandWindow::RenderParameterPage()
{
    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return;
    }

    UseTextColor(DescriptionColor);
    const auto descriptionLines = GetVisibleDescriptionLineCount();
    for (int line = 0; line < descriptionLines; ++line)
    {
        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP + line * ROW_HEIGHT, m_descriptionLines[line].c_str(), CONTENT_WIDTH);
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
        isFavourite ? I18N::Game::ChatCommandsRemoveFavourite : I18N::Game::ChatCommandsAddFavourite, CONTENT_WIDTH);
    RenderLine(m_Pos.x + CONTENT_LEFT, actionTop + ROW_HEIGHT, I18N::Game::ChatCommandsSaveTemplate, CONTENT_WIDTH);
}

void SEASON3B::CNewUIChatCommandWindow::RenderParameter(size_t parameterIndex, int y)
{
    const auto* command = GetSelectedCommand();
    if (command == NULL || parameterIndex >= command->Parameters.size())
    {
        return;
    }

    const auto& parameter = command->Parameters[parameterIndex];
    const auto& value = m_parameterValues[parameterIndex];

    // A required parameter without a value is what keeps the command from being
    // sent, so it's the one to point at.
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

void SEASON3B::CNewUIChatCommandWindow::RenderTemplatePage()
{
    if (m_templates.empty())
    {
        UseTextColor(NormalColor);
        RenderLine(m_Pos.x + CONTENT_LEFT, m_Pos.y + CONTENT_TOP, I18N::Game::ChatCommandsNoTemplates, CONTENT_WIDTH, VISIBLE_ROWS * ROW_HEIGHT);
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

void SEASON3B::CNewUIChatCommandWindow::LoadImages()
{
    // The ids are shared with the other windows, but every window loads what it
    // draws - relying on another one having done it means an empty frame when
    // that window wasn't opened yet.
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_CHATCOMMAND_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_CHATCOMMAND_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_CHATCOMMAND_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_CHATCOMMAND_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_CHATCOMMAND_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_CHATCOMMAND_BTN_EXIT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_CHATCOMMAND_BTN, GL_LINEAR);
}

void SEASON3B::CNewUIChatCommandWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_CHATCOMMAND_BACK);
    DeleteBitmap(IMAGE_CHATCOMMAND_TOP);
    DeleteBitmap(IMAGE_CHATCOMMAND_LEFT);
    DeleteBitmap(IMAGE_CHATCOMMAND_RIGHT);
    DeleteBitmap(IMAGE_CHATCOMMAND_BOTTOM);
    DeleteBitmap(IMAGE_CHATCOMMAND_BTN_EXIT);
    DeleteBitmap(IMAGE_CHATCOMMAND_BTN);
}
