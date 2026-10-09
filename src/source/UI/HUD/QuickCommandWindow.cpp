
#include "stdafx.h"
#include "UI/HUD/QuickCommandWindow.h"
#include "UI/Core/WindowSystem.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The five actions, top to bottom: trade, buy, party, follow, duel.
constexpr int kQuickCommandCount = 5;
constexpr int kQuickCommandTextIds[kQuickCommandCount] = {943, 1124, 944, 948, 949};
} // namespace

// cppcheck-suppress uninitMemberVar
mu::ui::window::CQuickCommandWindow::CQuickCommandWindow()
{
    m_pNewUIMng = NULL;
    m_iSelectedCharacterIndex = -1;
}

mu::ui::window::CQuickCommandWindow::~CQuickCommandWindow()
{
    Release();
}

bool mu::ui::window::CQuickCommandWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_QUICK_COMMAND, this);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CQuickCommandWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

bool mu::ui::window::CQuickCommandWindow::UpdateMouseEvent()
{
    if (m_iSelectedCharacterIndex < 0)
    {
        return true;
    }

    // The rows are the document's (quick_command_run); a click anywhere else closes the menu.
    const bool overMenu = UI::RmlBridge::IsPointerOver(m_RmlView.Document());
    if (!overMenu && mu::ui::window::IsRelease(VK_LBUTTON))
    {
        CloseQuickCommand();
        return false;
    }

    return !overMenu;
}

void mu::ui::window::CQuickCommandWindow::RunCommand(int index)
{
    if (m_iSelectedCharacterIndex < 0)
        return;

    CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
    switch (index)
    {
    case 0:
        g_pCommandWindow->CommandTrade(pCha);
        break;
    case 1:
        g_pCommandWindow->CommandPurchase(pCha);
        break;
    case 2:
        g_pCommandWindow->CommandParty(pCha->Key);
        break;
    case 3:
        g_pCommandWindow->CommandFollow(m_iSelectedCharacterIndex);
        break;
    case 4:
        g_pCommandWindow->CommandDual(pCha);
        break;
    default:
        return;
    }
    CloseQuickCommand();
}

bool mu::ui::window::CQuickCommandWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUICK_COMMAND) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            CloseQuickCommand();
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CQuickCommandWindow::Update()
{
    if (m_iSelectedCharacterIndex >= 0)
    {
        CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];

        if (wcscmp(pCha->ID, m_strID) != 0 || pCha->Object.Live == false || pCha->Object.Kind != KIND_PLAYER)
        {
            CloseQuickCommand();
        }

        float fPos_x = pCha->Object.Position[0] - Hero->Object.Position[0];
        float fPos_y = pCha->Object.Position[1] - Hero->Object.Position[1];
        float fDistance = sqrtf((fPos_x * fPos_x) + (fPos_y * fPos_y));

        if (fDistance > 300.f)
        {
            CloseQuickCommand();
        }
    }
    else
    {
        CloseQuickCommand();
    }

    SyncRmlModel();
    return true;
}

bool mu::ui::window::CQuickCommandWindow::Render()
{
    // Nothing native left: frame, name, rows, hover and clicks are RmlUi.
    return true;
}

void mu::ui::window::CQuickCommandWindow::BindRmlModel(Rml::DataModelConstructor& c, QuickCommandRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("target_name", &model.targetName);

    auto row = c.RegisterStruct<QuickCommandRowEntry>();
    row.RegisterMember("label", &QuickCommandRowEntry::label);
    c.RegisterArray<std::vector<QuickCommandRowEntry>>();
    c.Bind("rows", &model.rows);
    c.BindEventCallback("quick_command_run",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
        {
            if (!args.empty())
                RunCommand(args[0].Get<int>(-1));
        });

    model.rows.clear();
    for (int i = 0; i < kQuickCommandCount; ++i)
        model.rows.push_back({StringUtils::WideToNarrow(I18N::Game::Lookup(kQuickCommandTextIds[i]))});
}

void mu::ui::window::CQuickCommandWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CQuickCommandWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // The original drew nothing while no player was attached to the menu.
    const bool visible = IsVisible() && m_iSelectedCharacterIndex >= 0;
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), visible);
    if (!visible)
        return;

    SyncSlotPlacement();
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    QuickCommandRmlModel& model = m_RmlView.GetModel();
    const float boldTextPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    if (model.boldTextPx != boldTextPx)
    {
        model.boldTextPx = boldTextPx;
        m_RmlView.MarkDirty("bold_text_px");
    }

    const Rml::String targetName = StringUtils::WideToNarrow(m_strID);
    if (model.targetName != targetName)
    {
        model.targetName = targetName;
        m_RmlView.MarkDirty("target_name");
    }
}

float mu::ui::window::CQuickCommandWindow::GetLayerDepth()
{
    return 2.0f;
}

float mu::ui::window::CQuickCommandWindow::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CQuickCommandWindow::OpenningProcess()
{
    m_iSelectedCharacterIndex = -1;
}

void mu::ui::window::CQuickCommandWindow::ClosingProcess()
{
    m_iSelectedCharacterIndex = -1;
}

void mu::ui::window::CQuickCommandWindow::OpenQuickCommand(const wchar_t* strID, int iIndex)
{
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_QUICK_COMMAND);

    SetID(strID);
    SetSelectedCharacterIndex(iIndex);

    // Beside the pointer, as the original: 10 units right of it and 50 above, never off the top.
    const float scale = UI::Scaling::TypographyScale(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    PlaceDocument(g_fWindowMouseX + 10.f * scale, (std::max)(g_fWindowMouseY - 50.f * scale, 0.f), scale);
}

void mu::ui::window::CQuickCommandWindow::CloseQuickCommand()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUICK_COMMAND) == true)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_QUICK_COMMAND);
    }
}

void mu::ui::window::CQuickCommandWindow::SetID(const wchar_t* strID)
{
    wcscpy(m_strID, strID);
}

void mu::ui::window::CQuickCommandWindow::SetSelectedCharacterIndex(int iIndex)
{
    m_iSelectedCharacterIndex = iIndex;
}