
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Dialogs/ChatCommandWindow.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"

#include "Audio/DSPlaySound.h"
#include "Core/Text/TextLineWrap.h"
#include "GameLogic/Commands/ChatCommandFavourites.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlNumericInputFilter.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

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

DWORD ToRgba(const TextColor& color)
{
    return RGBA(color.Red, color.Green, color.Blue, 255);
}

int MeasureInReferenceUnits(const wchar_t* text, size_t length)
{
    return g_pRenderText->MeasureText(text, static_cast<int>(length)).cx;
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

    SetPos(x, y);
    BuildRmlUi();
    Show(false);

    return true;
}

void mu::ui::window::CChatCommandWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    m_RmlView.Release();
}

void mu::ui::window::CChatCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
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
        break;

    case PAGE_PARAMETERS:
        WrapDescriptionOfSelected();
        break;

    case PAGE_TEMPLATES:
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
    if (command == nullptr || parameterIndex >= command->Parameters.size())
    {
        return;
    }

    m_editedParameter = static_cast<int>(parameterIndex);
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::editValue, "edit_value",
              Rml::String(StringUtils::WideToNarrow(m_parameterValues[parameterIndex].c_str())));

    // The parameter's own field takes the focus on the next SyncRmlModel();
    // Update() then claims RmlUi's text-input identity so Escape and Enter still reach this window.
    m_valueFieldFocusPending = true;
}

void mu::ui::window::CChatCommandWindow::CommitEditedValue()
{
    if (m_editedParameter < 0 || static_cast<size_t>(m_editedParameter) >= m_parameterValues.size())
    {
        return;
    }

    m_parameterValues[m_editedParameter] = ReadValueField();
}

void mu::ui::window::CChatCommandWindow::StopEditing()
{
    CommitEditedValue();
    if (Rml::Element* field = GetValueField())
    {
        field->SetAttribute("value", Rml::String());
        field->Blur();
    }
    m_editedParameter = -1;
    m_valueFieldFocusPending = false;

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

bool mu::ui::window::CChatCommandWindow::UpdateMouseEvent()
{
    // The rows, the value fields, the buttons, the exit button and the corner close are RmlUi's
    // (see Update()).
    if (!UI::RmlBridge::IsPointerOver(m_RmlView.Document()))
        return true;

    const auto hiddenRows = GetScrollableRowCount() - VISIBLE_ROWS;
    if (MouseWheel != 0 && hiddenRows > 0)
    {
        // MouseWheel counts notches, one row per notch.
        m_scrollOffset = std::max(0, std::min(m_scrollOffset - MouseWheel, hiddenRows));
        MouseWheel = 0;
    }

    return false;
}

void mu::ui::window::CChatCommandWindow::HandleHit(ChatCommandAction action, int index)
{
    // The document only holds the visible rows of the command and template lists.
    const int listIndex = m_scrollOffset + index;

    switch (action)
    {
    case ChatCommandAction::PickCommand:
        if (GetCommandAt(listIndex) != nullptr)
        {
            PlayBuffer(SOUND_CLICK01);
            PickCommand(listIndex);
        }
        break;

    case ChatCommandAction::EditValue:
    {
        const auto* command = GetSelectedCommand();
        if (command == nullptr || index < 0 || static_cast<size_t>(index) >= command->Parameters.size())
            break;
        if (IsPickedFromList(command->Parameters[index]))
        {
            StopEditing();
            CycleParameterValue(index);
        }
        else
        {
            BeginEditingParameter(index);
        }
        PlayBuffer(SOUND_CLICK01);
        break;
    }

    case ChatCommandAction::ToggleFavourite:
        ToggleFavouriteOfSelected();
        break;

    case ChatCommandAction::SaveTemplate:
        SaveSelectedAsTemplate();
        break;

    case ChatCommandAction::ExecuteTemplate:
        ExecuteTemplate(static_cast<size_t>(listIndex));
        break;

    case ChatCommandAction::RemoveTemplate:
        if (listIndex >= 0 && static_cast<size_t>(listIndex) < m_templates.size())
        {
            GameLogic::Commands::Templates::RemoveAt(static_cast<size_t>(listIndex));
            m_templates = GameLogic::Commands::Templates::GetAll();
            m_scrollOffset = std::max(0, std::min(m_scrollOffset, GetScrollableRowCount() - VISIBLE_ROWS));
            PlayBuffer(SOUND_CLICK01);
        }
        break;
    }
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
    // Clicks RmlUi reported (the original's UpdateMouseEvent()), in its order: exit, the left
    // and right buttons, then the page's own areas.
    const bool exit = m_pendingExit;
    const bool left = m_pendingLeft;
    const bool right = m_pendingRight;
    std::vector<PendingHit> hits;
    hits.swap(m_pendingHits);
    m_pendingExit = m_pendingLeft = m_pendingRight = false;
    if (IsVisible())
    {
        if (exit)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_COMMAND_LIST);
            PlayBuffer(SOUND_CLICK01);
        }
        else if (left && HasLeftButton())
        {
            StopEditing();
            ShowPage(PAGE_COMMANDS);
            PlayBuffer(SOUND_CLICK01);
        }
        else if (right && HasRightButton())
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
        }
        else if (!hits.empty())
        {
            HandleHit(hits.front().action, hits.front().index);
        }
    }

    SyncRmlModel();
    return true;
}

bool mu::ui::window::CChatCommandWindow::Render()
{
    // Nothing native left: the frame, the pages, the value field and the buttons are RmlUi.
    // Kept because CObject requires the override.
    return true;
}

Rml::Element* mu::ui::window::CChatCommandWindow::GetValueField() const
{
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document == nullptr || m_editedParameter < 0)
        return nullptr;

    // Every parameter row has a field (see chat_command.rml); the edited row's is the one shown.
    Rml::ElementList rows;
    document->QuerySelectorAll(rows, ".cc-parameter");
    if (static_cast<size_t>(m_editedParameter) >= rows.size())
        return nullptr;
    return rows[m_editedParameter]->QuerySelector("input");
}

std::wstring mu::ui::window::CChatCommandWindow::ReadValueField() const
{
    // No element lookup: the value lives in the model. Its one caller already guards on
    // m_editedParameter.
    return StringUtils::NarrowToWide(m_RmlView.GetModel().editValue);
}

void mu::ui::window::CChatCommandWindow::BindRmlModel(Rml::DataModelConstructor& c, ChatCommandRmlModel& model)
{
    c.Bind("text_px", &model.textPx);

    auto lineType = c.RegisterStruct<ChatCommandLine>();
    lineType.RegisterMember("text", &ChatCommandLine::text);
    lineType.RegisterMember("text_px", &ChatCommandLine::textPx);
    c.RegisterArray<std::vector<ChatCommandLine>>();
    c.Bind("title", &model.title);
    c.Bind("empty_message", &model.emptyMessage);
    c.Bind("description_lines", &model.descriptionLines);
    c.Bind("template_rows", &model.templateRows);
    c.Bind("favourite_action", &model.favouriteAction);
    c.Bind("save_action", &model.saveAction);
    auto commandRow = c.RegisterStruct<ChatCommandRow>();
    commandRow.RegisterMember("text", &ChatCommandRow::text);
    commandRow.RegisterMember("text_px", &ChatCommandRow::textPx);
    commandRow.RegisterMember("favourite", &ChatCommandRow::favourite);
    c.RegisterArray<std::vector<ChatCommandRow>>();
    c.Bind("command_rows", &model.commandRows);
    auto parameter = c.RegisterStruct<ChatCommandParameterRow>();
    parameter.RegisterMember("label", &ChatCommandParameterRow::label);
    parameter.RegisterMember("label_text_px", &ChatCommandParameterRow::labelTextPx);
    parameter.RegisterMember("missing", &ChatCommandParameterRow::missing);
    parameter.RegisterMember("value", &ChatCommandParameterRow::value);
    parameter.RegisterMember("value_text_px", &ChatCommandParameterRow::valueTextPx);
    parameter.RegisterMember("placeholder", &ChatCommandParameterRow::placeholder);
    parameter.RegisterMember("edited", &ChatCommandParameterRow::edited);
    c.RegisterArray<std::vector<ChatCommandParameterRow>>();
    c.Bind("parameters", &model.parameters);
    c.Bind("page", &model.page);
    c.Bind("edit_value", &model.editValue);
    c.Bind("has_left_button", &model.hasLeftButton);
    c.Bind("has_right_button", &model.hasRightButton);
    c.Bind("left_text", &model.leftText);
    c.Bind("right_text", &model.rightText);
    c.Bind("label_line_px", &model.labelLinePx);
    c.Bind("exit_tooltip", &model.exitTooltip);

    c.BindEventCallback(
        "chat_command_hit",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
        {
            if (arguments.size() == 2)
                m_pendingHits.push_back(
                    {static_cast<ChatCommandAction>(arguments[0].Get<int>(0)), arguments[1].Get<int>(-1)});
        });
    c.BindEventCallback("chat_command_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_pendingExit = true; });
    c.BindEventCallback("chat_command_left", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_pendingLeft = true; });
    c.BindEventCallback("chat_command_right", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_pendingRight = true; });

    wchar_t closeText[256] = {};
    mu_swprintf_s(closeText, I18N::Game::CloseS, L"J");
    model.exitTooltip = StringUtils::WideToNarrow(closeText);
}

void mu::ui::window::CChatCommandWindow::OnRmlUnloading()
{
    StopEditing();
}

void mu::ui::window::CChatCommandWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CChatCommandWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth UI::Layout::ForegroundPanelLayerDepth: over the HUD and its logs.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncContent();
    SyncValueField();
}

void mu::ui::window::CChatCommandWindow::SyncValueField()
{
    Rml::Element* field = GetValueField();
    if (field == nullptr || m_editedParameter < 0)
        return;

    // CUITextInputBox's UIOPTION_NUMBERONLY for a numeric parameter, matching what the server
    // accepts: anything else typed is dropped.
    const auto* command = GetSelectedCommand();
    if (command != nullptr && static_cast<size_t>(m_editedParameter) < command->Parameters.size() &&
        command->Parameters[m_editedParameter].Type == ChatCommandParameterType::Number)
    {
        SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::editValue, "edit_value",
                  UI::RmlBridge::KeepDigitsOnly(m_RmlView.GetModel().editValue));
    }

    // Focus the edited parameter's field once its row exists.
    if (m_valueFieldFocusPending && m_RmlView.Document()->IsVisible())
    {
        field->Focus();
        if (field->IsPseudoClassSet("focus"))
            m_valueFieldFocusPending = false;
    }
}

void mu::ui::window::CChatCommandWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();

    // One of the window's own lines: the document places it, so only what it says and the size the
    // native renderer would have shrunk it to for its box travel through the model.
    auto line = [&](const wchar_t* text, int width, bool bold = false) -> ChatCommandLine
    {
        if (text == nullptr || text[0] == L'\0')
            return {};
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        const int measured = MeasureInReferenceUnits(text, wcslen(text));
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        return {StringUtils::WideToNarrow(text),
                UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured),
                                                      static_cast<float>(width))};
    };

    // The original's RenderTitle().
    const wchar_t* titleText = I18N::Game::ChatCommandsTitle;
    if (m_page == PAGE_TEMPLATES)
        titleText = I18N::Game::ChatCommandsTemplates;
    else if (m_page == PAGE_PARAMETERS && GetSelectedCommand() != nullptr)
        titleText = GetSelectedCommand()->Command.c_str();
    ChatCommandLine title = line(titleText, WINDOW_WIDTH, true);

    ChatCommandLine emptyMessage;
    std::vector<ChatCommandRow> commandRows;
    std::vector<ChatCommandLine> descriptionLines;
    std::vector<ChatCommandLine> templateRows;
    std::vector<ChatCommandParameterRow> parameters;
    ChatCommandLine favouriteAction, saveAction;

    if (m_page == PAGE_COMMANDS)
    {
        // The original's RenderCommandPage() and UpdateCommandPageMouseEvent().
        if (m_commandOrder.empty())
            emptyMessage = line(I18N::Game::ChatCommandsNotSupported, CONTENT_WIDTH);
        for (int row = 0; row < VISIBLE_ROWS; ++row)
        {
            const auto* command = GetCommandAt(m_scrollOffset + row);
            if (command == nullptr)
                break;

            const bool isFavourite = GameLogic::Commands::Favourites::Contains(command->Command);
            // Only the command name fits at this width; parameters are just flagged with "...".
            std::wstring text = isFavourite ? FavouriteMarker : L"";
            text += command->Command;
            if (!command->Parameters.empty())
                text += ParameterMarker;

            const ChatCommandLine row_line = line(text.c_str(), CONTENT_WIDTH);
            commandRows.push_back({row_line.text, row_line.textPx, isFavourite});
        }
    }
    else if (m_page == PAGE_PARAMETERS)
    {
        // The original's RenderParameterPage()/RenderParameter() and
        // UpdateParameterPageMouseEvent().
        if (const auto* command = GetSelectedCommand())
        {
            const auto descriptionCount = GetVisibleDescriptionLineCount();
            for (int index = 0; index < descriptionCount; ++index)
                descriptionLines.push_back(line(m_descriptionLines[index].c_str(), CONTENT_WIDTH));

            for (size_t i = 0; i < command->Parameters.size(); ++i)
            {
                const auto& parameter = command->Parameters[i];
                const auto& value = m_parameterValues[i];

                // Required parameters that are still empty are what's blocking send.
                std::wstring label = parameter.Name;
                if (parameter.IsRequired)
                    label += L" *";
                ChatCommandParameterRow entry;
                const ChatCommandLine labelLine = line(label.c_str(), CONTENT_WIDTH);
                entry.label = labelLine.text;
                entry.labelTextPx = labelLine.textPx;
                entry.missing = parameter.IsRequired && value.empty();
                entry.edited = m_editedParameter == static_cast<int>(i);
                if (!entry.edited)
                {
                    entry.placeholder = value.empty();
                    const ChatCommandLine valueLine =
                        line(value.empty() ? parameter.ValidValues.c_str() : value.c_str(), CONTENT_WIDTH - 4);
                    entry.value = valueLine.text;
                    entry.valueTextPx = valueLine.textPx;
                }
                parameters.push_back(std::move(entry));
            }

            const bool isFavourite = GameLogic::Commands::Favourites::Contains(command->Command);
            favouriteAction = line(isFavourite ? I18N::Game::ChatCommandsRemoveFavourite
                                               : I18N::Game::ChatCommandsAddFavourite,
                                   CONTENT_WIDTH);
            saveAction = line(I18N::Game::ChatCommandsSaveTemplate, CONTENT_WIDTH);
        }
    }
    else
    {
        // The original's RenderTemplatePage() and UpdateTemplatePageMouseEvent().
        if (m_templates.empty())
            emptyMessage = line(I18N::Game::ChatCommandsNoTemplates, CONTENT_WIDTH);
        for (int row = 0; row < VISIBLE_ROWS; ++row)
        {
            const auto index = static_cast<size_t>(m_scrollOffset + row);
            if (index >= m_templates.size())
                break;
            templateRows.push_back(line(m_templates[index].Label.c_str(), CONTENT_WIDTH - ROW_HEIGHT));
        }
    }

    ChatCommandRmlModel& model = m_RmlView.GetModel();
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::page, "page", static_cast<int>(m_page));
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::title, "title", std::move(title));
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::emptyMessage, "empty_message", std::move(emptyMessage));
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::favouriteAction, "favourite_action", std::move(favouriteAction));
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::saveAction, "save_action", std::move(saveAction));
    if (model.commandRows != commandRows)
    {
        model.commandRows = std::move(commandRows);
        m_RmlView.MarkDirty("command_rows");
    }
    if (model.descriptionLines != descriptionLines)
    {
        model.descriptionLines = std::move(descriptionLines);
        m_RmlView.MarkDirty("description_lines");
    }
    if (model.templateRows != templateRows)
    {
        model.templateRows = std::move(templateRows);
        m_RmlView.MarkDirty("template_rows");
    }
    if (model.parameters != parameters)
    {
        model.parameters = std::move(parameters);
        m_RmlView.MarkDirty("parameters");
    }

    // The left and right buttons: CButton::Render()'s label in the normal font, white.
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::hasLeftButton, "has_left_button", HasLeftButton());
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::hasRightButton, "has_right_button", HasRightButton());
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::leftText, "left_text",
              StringUtils::WideToNarrow(I18N::Game::ChatCommandsBack));
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::rightText, "right_text",
              StringUtils::WideToNarrow(m_page == PAGE_COMMANDS ? I18N::Game::ChatCommandsTemplates
                                                                : I18N::Game::ChatCommandsExecute));
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlView.Binder(), &ChatCommandRmlModel::labelLinePx, "label_line_px",
              static_cast<float>(lineHeight) * transform.scaleY);
}
