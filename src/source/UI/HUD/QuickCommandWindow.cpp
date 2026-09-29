
#include "stdafx.h"
#include "UI/HUD/QuickCommandWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
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
    m_Pos.x = 0;
    m_Pos.y = 0;

    m_iSelectedIndex = -1;
    m_iSelectedCharacterIndex = -1;
}

mu::ui::window::CQuickCommandWindow::~CQuickCommandWindow()
{
    Release();
}

bool mu::ui::window::CQuickCommandWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_QUICK_COMMAND, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void mu::ui::window::CQuickCommandWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CQuickCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CQuickCommandWindow::UpdateMouseEvent()
{
    if (m_iSelectedCharacterIndex < 0)
    {
        return true;
    }

    POINT pt = {m_Pos.x, m_Pos.y + 38};

    for (int i = 0; i < kQuickCommandCount; ++i)
    {
        if (CheckMouseIn(pt.x, pt.y, 112, 19) == true)
        {
            m_iSelectedIndex = i;
            break;
        }

        pt.y += 20.f;
    }

    if (m_iSelectedIndex > -1 && mu::ui::window::IsRelease(VK_LBUTTON))
    {
        switch (m_iSelectedIndex)
        {
        case 0:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandTrade(pCha);
            CloseQuickCommand();

            return false;
        }
        break;
        case 1:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandPurchase(pCha);
            CloseQuickCommand();

            return false;
        }
        break;
        case 2:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandParty(pCha->Key);
            CloseQuickCommand();

            return false;
        }
        break;
        case 3:
        {
            g_pCommandWindow->CommandFollow(m_iSelectedCharacterIndex);
            CloseQuickCommand();

            return false;
        }
        break;
        case 4:
        {
            CHARACTER* pCha = &CharactersClient[m_iSelectedCharacterIndex];
            g_pCommandWindow->CommandDual(pCha);
            CloseQuickCommand();

            return false;
        }
        break;
        }
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y + 30, 112, 110) == false)
    {
        m_iSelectedIndex = -1;

        if (mu::ui::window::IsRelease(VK_LBUTTON))
        {
            CloseQuickCommand();
            return false;
        }
    }

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, 112, 140).Contains(MouseX, MouseY))
    {
        return false;
    }

    return true;
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
    // Nothing native left: frame, name, rows and arrows are RmlUi. The hover index and the
    // clicks stay native (UpdateMouseEvent()), so the document only mirrors them.
    return true;
}

void mu::ui::window::CQuickCommandWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "quick_command",
                                                 [](Rml::DataModelConstructor& c, QuickCommandRmlModel& model)
                                                 {
                                                     c.Bind("root_x", &model.rootX);
                                                     c.Bind("root_y", &model.rootY);
                                                     c.Bind("root_scale", &model.rootScale);
                                                     c.Bind("text_px", &model.textPx);
                                                     c.Bind("bold_text_px", &model.boldTextPx);
                                                     c.Bind("target_name", &model.targetName);

                                                     auto row = c.RegisterStruct<QuickCommandRowEntry>();
                                                     row.RegisterMember("label", &QuickCommandRowEntry::label);
                                                     row.RegisterMember("selected", &QuickCommandRowEntry::selected);
                                                     c.RegisterArray<std::vector<QuickCommandRowEntry>>();
                                                     c.Bind("rows", &model.rows);
                                                 });

    if (!modelCreated)
        return;

    QuickCommandRmlModel& model = m_RmlBinder.GetModel();
    model.rows.clear();
    for (int i = 0; i < kQuickCommandCount; ++i)
        model.rows.push_back({StringUtils::WideToNarrow(I18N::Game::Lookup(kQuickCommandTextIds[i])), false});

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/quick_command.rml");
}

void mu::ui::window::CQuickCommandWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CQuickCommandWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // The original drew nothing while no player was attached to the menu.
    const bool visible = IsVisible() && m_iSelectedCharacterIndex >= 0;
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, visible);
    if (!visible)
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    QuickCommandRmlModel& model = m_RmlBinder.GetModel();
    const float boldTextPx =
        UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, UI::Scaling::GetActiveTransform());
    if (model.boldTextPx != boldTextPx)
    {
        model.boldTextPx = boldTextPx;
        m_RmlBinder.MarkDirty("bold_text_px");
    }

    const Rml::String targetName = StringUtils::WideToNarrow(m_strID);
    if (model.targetName != targetName)
    {
        model.targetName = targetName;
        m_RmlBinder.MarkDirty("target_name");
    }

    SyncRows();
}

void mu::ui::window::CQuickCommandWindow::SyncRows()
{
    QuickCommandRmlModel& model = m_RmlBinder.GetModel();
    bool changed = false;
    for (int i = 0; i < static_cast<int>(model.rows.size()); ++i)
    {
        const bool selected = i == m_iSelectedIndex;
        changed = changed || model.rows[i].selected != selected;
        model.rows[i].selected = selected;
    }
    if (changed)
        m_RmlBinder.MarkDirty("rows");
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
    m_iSelectedIndex = -1;
    m_iSelectedCharacterIndex = -1;
}

void mu::ui::window::CQuickCommandWindow::ClosingProcess()
{
    m_iSelectedIndex = -1;
    m_iSelectedCharacterIndex = -1;
}

void mu::ui::window::CQuickCommandWindow::OpenQuickCommand(const wchar_t* strID, int iIndex, int x, int y)
{
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_QUICK_COMMAND);

    SetID(strID);
    SetSelectedCharacterIndex(iIndex);
    SetPos(x, y);
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