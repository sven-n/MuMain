
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Dialogs/ChatCommandWindow.h"

#include "Audio/DSPlaySound.h"
#include "Core/Text/TextLineWrap.h"
#include "GameLogic/Commands/ChatCommandFavourites.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

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

template <typename Model, typename T>
void SyncField(RmlModelBinder<Model>& binder, T Model::* field, const char* name, T value)
{
    Model& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

bool SameText(const ChatCommandTextEntry& a, const ChatCommandTextEntry& b)
{
    return a.text == b.text && a.left == b.left && a.top == b.top && a.width == b.width && a.textPx == b.textPx &&
           a.centred == b.centred && a.color == b.color;
}

bool SameHit(const ChatCommandHitEntry& a, const ChatCommandHitEntry& b)
{
    return a.left == b.left && a.top == b.top && a.width == b.width && a.height == b.height && a.action == b.action &&
           a.index == b.index && a.valueBox == b.valueBox;
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
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    Show(false);

    return true;
}

void mu::ui::window::CChatCommandWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

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
    if (Rml::Element* field = GetValueField())
        field->SetAttribute("value", StringUtils::WideToNarrow(m_parameterValues[parameterIndex].c_str()));

    // The field takes the focus once the document shows it at its new place (SyncRmlModel());
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
    m_editedParameter = -1;
    m_valueFieldFocusPending = false;
    if (Rml::Element* field = GetValueField())
    {
        field->SetAttribute("value", Rml::String());
        field->Blur();
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
    // The rows, the value fields, the buttons and the exit button are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_COMMAND_LIST))
    {
        PlayBuffer(SOUND_CLICK01);
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

void mu::ui::window::CChatCommandWindow::HandleHit(ChatCommandAction action, int index)
{
    switch (action)
    {
    case ChatCommandAction::PickCommand:
        if (GetCommandAt(index) != nullptr)
        {
            PlayBuffer(SOUND_CLICK01);
            PickCommand(index);
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
        ExecuteTemplate(static_cast<size_t>(index));
        break;

    case ChatCommandAction::RemoveTemplate:
        if (index >= 0 && static_cast<size_t>(index) < m_templates.size())
        {
            GameLogic::Commands::Templates::RemoveAt(static_cast<size_t>(index));
            m_templates = GameLogic::Commands::Templates::GetAll();
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

    // CManager::UpdateKeyEvent() only dispatches to a window whose GetRelatedWnd() matches the
    // focused handle, and it reports a focused RmlUi <input> as RmlUiRuntime's own address:
    // claiming it while the value field is focused keeps Escape and Enter reaching this window,
    // the role the CUITextInputBox's handle played (CChatInputBox::Update() does the same).
    const HWND rmlFocus = reinterpret_cast<HWND>(&RmlUiRuntime::Instance());
    Rml::Element* field = GetValueField();
    const bool fieldFocused = m_editedParameter >= 0 && field != nullptr && field->IsPseudoClassSet("focus");
    if (fieldFocused)
    {
        if (GetRelatedWnd() != rmlFocus)
            SetRelatedWnd(rmlFocus);
    }
    else if (GetRelatedWnd() != g_hWnd)
    {
        SetRelatedWnd(g_hWnd);
    }

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
    return m_pRmlDoc != nullptr ? m_pRmlDoc->GetElementById("value_field") : nullptr;
}

std::wstring mu::ui::window::CChatCommandWindow::ReadValueField() const
{
    Rml::Element* field = GetValueField();
    if (field == nullptr)
        return {};
    return StringUtils::NarrowToWide(field->GetAttribute<Rml::String>("value", Rml::String()));
}

void mu::ui::window::CChatCommandWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "chat_command",
        [this](Rml::DataModelConstructor& c, ChatCommandRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("window_height", &model.windowHeight);

            auto text = c.RegisterStruct<ChatCommandTextEntry>();
            text.RegisterMember("text", &ChatCommandTextEntry::text);
            text.RegisterMember("left", &ChatCommandTextEntry::left);
            text.RegisterMember("top", &ChatCommandTextEntry::top);
            text.RegisterMember("width", &ChatCommandTextEntry::width);
            text.RegisterMember("text_px", &ChatCommandTextEntry::textPx);
            text.RegisterMember("centred", &ChatCommandTextEntry::centred);
            text.RegisterMember("color", &ChatCommandTextEntry::color);
            c.RegisterArray<std::vector<ChatCommandTextEntry>>();
            c.Bind("title", &model.title);
            c.Bind("texts", &model.texts);

            auto hit = c.RegisterStruct<ChatCommandHitEntry>();
            hit.RegisterMember("left", &ChatCommandHitEntry::left);
            hit.RegisterMember("top", &ChatCommandHitEntry::top);
            hit.RegisterMember("width", &ChatCommandHitEntry::width);
            hit.RegisterMember("height", &ChatCommandHitEntry::height);
            hit.RegisterMember("action", &ChatCommandHitEntry::action);
            hit.RegisterMember("index", &ChatCommandHitEntry::index);
            hit.RegisterMember("value_box", &ChatCommandHitEntry::valueBox);
            c.RegisterArray<std::vector<ChatCommandHitEntry>>();
            c.Bind("hits", &model.hits);

            c.Bind("editing", &model.editing);
            c.Bind("edit_top", &model.editTop);
            c.Bind("has_left_button", &model.hasLeftButton);
            c.Bind("has_right_button", &model.hasRightButton);
            c.Bind("left_text", &model.leftText);
            c.Bind("right_text", &model.rightText);
            c.Bind("label_top", &model.labelTop);
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
        });
    if (!modelCreated)
        return;

    wchar_t closeText[256] = {};
    mu_swprintf_s(closeText, I18N::Game::CloseS, L"J");
    m_RmlBinder.GetModel().exitTooltip = StringUtils::WideToNarrow(closeText);
    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/chat_command.rml");
}

void mu::ui::window::CChatCommandWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    StopEditing();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CChatCommandWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth UI::Layout::ForegroundPanelLayerDepth: over the HUD and its logs.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
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
        const Rml::String value = field->GetAttribute<Rml::String>("value", Rml::String());
        Rml::String digits;
        std::copy_if(value.begin(), value.end(), std::back_inserter(digits),
                     [](char c) { return c >= '0' && c <= '9'; });
        if (digits != value)
            field->SetAttribute("value", digits);
    }

    // The field exists only while it is shown at its parameter (see chat_command.rml); focus it
    // once it does.
    if (m_valueFieldFocusPending && m_pRmlDoc->IsVisible() && m_RmlBinder.GetModel().editing)
    {
        field->Focus();
        if (field->IsPseudoClassSet("focus"))
            m_valueFieldFocusPending = false;
    }
}

void mu::ui::window::CChatCommandWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();

    // RenderText(): the text shrunk to its box when wider, its top at y.
    auto makeText = [&](const wchar_t* text, int left, int top, int width, const TextColor& color, bool bold = false,
                        bool centred = false)
    {
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        const int measured = MeasureInReferenceUnits(text, wcslen(text));
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        return ChatCommandTextEntry{StringUtils::WideToNarrow(text),
                                    static_cast<float>(left),
                                    static_cast<float>(top),
                                    static_cast<float>(width),
                                    UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured),
                                                                          static_cast<float>(width)),
                                    centred,
                                    UI::RmlBridge::RgbaToCss(ToRgba(color))};
    };

    std::vector<ChatCommandTextEntry> texts;
    std::vector<ChatCommandHitEntry> hits;
    // Empty text is not drawn, like the original's RenderLine().
    auto addText = [&](const wchar_t* text, int left, int top, int width, const TextColor& color, bool centred = false)
    {
        if (text != nullptr && text[0] != L'\0')
            texts.push_back(makeText(text, left, top, width, color, false, centred));
    };
    auto addHit =
        [&](int left, int top, int width, int height, ChatCommandAction action, int index, bool valueBox = false)
    {
        hits.push_back({static_cast<float>(left), static_cast<float>(top), static_cast<float>(width),
                        static_cast<float>(height), static_cast<int>(action), index, valueBox});
    };

    // The original's RenderTitle().
    const wchar_t* title = I18N::Game::ChatCommandsTitle;
    if (m_page == PAGE_TEMPLATES)
        title = I18N::Game::ChatCommandsTemplates;
    else if (m_page == PAGE_PARAMETERS && GetSelectedCommand() != nullptr)
        title = GetSelectedCommand()->Command.c_str();
    ChatCommandTextEntry titleEntry = makeText(title, 0, TITLE_Y, WINDOW_WIDTH, TitleColor, true, true);

    bool editing = false;
    float editTop = 0.f;
    if (m_page == PAGE_COMMANDS)
    {
        // The original's RenderCommandPage() and UpdateCommandPageMouseEvent().
        if (m_commandOrder.empty())
            addText(I18N::Game::ChatCommandsNotSupported, CONTENT_LEFT, CONTENT_TOP, CONTENT_WIDTH, NormalColor);
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

            const int top = CONTENT_TOP + row * ROW_HEIGHT;
            addText(text.c_str(), CONTENT_LEFT, top, CONTENT_WIDTH, isFavourite ? FavouriteColor : NormalColor);
            addHit(CONTENT_LEFT, top, CONTENT_WIDTH, ROW_HEIGHT, ChatCommandAction::PickCommand, m_scrollOffset + row);
        }
    }
    else if (m_page == PAGE_PARAMETERS)
    {
        // The original's RenderParameterPage()/RenderParameter() and
        // UpdateParameterPageMouseEvent().
        if (const auto* command = GetSelectedCommand())
        {
            const auto descriptionLines = GetVisibleDescriptionLineCount();
            for (int line = 0; line < descriptionLines; ++line)
                addText(m_descriptionLines[line].c_str(), CONTENT_LEFT, CONTENT_TOP + line * ROW_HEIGHT, CONTENT_WIDTH,
                        DescriptionColor);

            const int parameterTop = GetParameterTop() - m_Pos.y;
            for (size_t i = 0; i < command->Parameters.size(); ++i)
            {
                const int y = parameterTop + static_cast<int>(i) * PARAMETER_HEIGHT;
                const auto& parameter = command->Parameters[i];
                const auto& value = m_parameterValues[i];

                // Required parameters that are still empty are what's blocking send.
                const bool isMissing = parameter.IsRequired && value.empty();
                std::wstring label = parameter.Name;
                if (parameter.IsRequired)
                    label += L" *";
                addText(label.c_str(), CONTENT_LEFT, y, CONTENT_WIDTH, isMissing ? MissingValueColor : NormalColor);

                const bool edited = m_editedParameter == static_cast<int>(i);
                // The edited value box takes its clicks itself (the field), the others pick it.
                if (edited)
                {
                    editing = true;
                    editTop = static_cast<float>(y + ROW_HEIGHT + 1);
                    hits.push_back({static_cast<float>(CONTENT_LEFT), static_cast<float>(y + ROW_HEIGHT),
                                    static_cast<float>(CONTENT_WIDTH), static_cast<float>(VALUE_HEIGHT), 0,
                                    static_cast<int>(i), true});
                    continue;
                }
                addHit(CONTENT_LEFT, y + ROW_HEIGHT, CONTENT_WIDTH, VALUE_HEIGHT, ChatCommandAction::EditValue,
                       static_cast<int>(i), true);
                addText(value.empty() ? parameter.ValidValues.c_str() : value.c_str(), CONTENT_LEFT + 2,
                        y + ROW_HEIGHT + 1, CONTENT_WIDTH - 4, value.empty() ? DescriptionColor : NormalColor);
            }

            const int actionTop = GetActionTop() - m_Pos.y;
            const bool isFavourite = GameLogic::Commands::Favourites::Contains(command->Command);
            addText(isFavourite ? I18N::Game::ChatCommandsRemoveFavourite : I18N::Game::ChatCommandsAddFavourite,
                    CONTENT_LEFT, actionTop, CONTENT_WIDTH, ActionColor);
            addText(I18N::Game::ChatCommandsSaveTemplate, CONTENT_LEFT, actionTop + ROW_HEIGHT, CONTENT_WIDTH,
                    ActionColor);
            addHit(CONTENT_LEFT, actionTop, CONTENT_WIDTH, ROW_HEIGHT, ChatCommandAction::ToggleFavourite, 0);
            addHit(CONTENT_LEFT, actionTop + ROW_HEIGHT, CONTENT_WIDTH, ROW_HEIGHT, ChatCommandAction::SaveTemplate, 0);
        }
    }
    else
    {
        // The original's RenderTemplatePage() and UpdateTemplatePageMouseEvent().
        if (m_templates.empty())
            addText(I18N::Game::ChatCommandsNoTemplates, CONTENT_LEFT, CONTENT_TOP, CONTENT_WIDTH, NormalColor);
        for (int row = 0; row < VISIBLE_ROWS; ++row)
        {
            const auto index = static_cast<size_t>(m_scrollOffset + row);
            if (index >= m_templates.size())
                break;

            const int rowY = CONTENT_TOP + row * ROW_HEIGHT;
            addText(m_templates[index].Label.c_str(), CONTENT_LEFT, rowY, CONTENT_WIDTH - ROW_HEIGHT, NormalColor);
            addText(L"x", CONTENT_LEFT + CONTENT_WIDTH - ROW_HEIGHT, rowY, ROW_HEIGHT, MissingValueColor, true);
            addHit(CONTENT_LEFT + CONTENT_WIDTH - ROW_HEIGHT, rowY, ROW_HEIGHT, ROW_HEIGHT,
                   ChatCommandAction::RemoveTemplate, static_cast<int>(index));
            addHit(CONTENT_LEFT, rowY, CONTENT_WIDTH - ROW_HEIGHT, ROW_HEIGHT, ChatCommandAction::ExecuteTemplate,
                   static_cast<int>(index));
        }
    }

    ChatCommandRmlModel& model = m_RmlBinder.GetModel();
    SyncField(m_RmlBinder, &ChatCommandRmlModel::windowHeight, "window_height", static_cast<float>(WindowHeight));
    if (!SameText(model.title, titleEntry))
    {
        model.title = std::move(titleEntry);
        m_RmlBinder.MarkDirty("title");
    }
    if (model.texts.size() != texts.size() ||
        !std::equal(model.texts.begin(), model.texts.end(), texts.begin(), SameText))
    {
        model.texts = std::move(texts);
        m_RmlBinder.MarkDirty("texts");
    }
    if (model.hits.size() != hits.size() || !std::equal(model.hits.begin(), model.hits.end(), hits.begin(), SameHit))
    {
        model.hits = std::move(hits);
        m_RmlBinder.MarkDirty("hits");
    }
    SyncField(m_RmlBinder, &ChatCommandRmlModel::editing, "editing", editing);
    SyncField(m_RmlBinder, &ChatCommandRmlModel::editTop, "edit_top", editTop);

    // The left and right buttons: CButton::Render()'s label in the normal font, white.
    SyncField(m_RmlBinder, &ChatCommandRmlModel::hasLeftButton, "has_left_button", HasLeftButton());
    SyncField(m_RmlBinder, &ChatCommandRmlModel::hasRightButton, "has_right_button", HasRightButton());
    SyncField(m_RmlBinder, &ChatCommandRmlModel::leftText, "left_text",
              StringUtils::WideToNarrow(I18N::Game::ChatCommandsBack));
    SyncField(m_RmlBinder, &ChatCommandRmlModel::rightText, "right_text",
              StringUtils::WideToNarrow(m_page == PAGE_COMMANDS ? I18N::Game::ChatCommandsTemplates
                                                                : I18N::Game::ChatCommandsExecute));
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    const int labelTop = BUTTON_HEIGHT / 2 - lineHeight / 2;
    SyncField(m_RmlBinder, &ChatCommandRmlModel::labelTop, "label_top", static_cast<float>(labelTop));
    SyncField(m_RmlBinder, &ChatCommandRmlModel::labelLinePx, "label_line_px",
              static_cast<float>(lineHeight) * transform.scaleY);
}
