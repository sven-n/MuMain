
#include "stdafx.h"
#include "UI/Combat/DuelWatchUserListWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "GameLogic/Combat/DuelMgr.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

// cppcheck-suppress uninitMemberVar
CDuelWatchUserListWindow::CDuelWatchUserListWindow() {}

CDuelWatchUserListWindow::~CDuelWatchUserListWindow()
{
    Release();
}

bool CDuelWatchUserListWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DUELWATCH_USERLIST, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CDuelWatchUserListWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CDuelWatchUserListWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CDuelWatchUserListWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    POINT ptSize = {57, 17};
    POINT ptOrigin = {m_Pos.x, m_Pos.y - (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount()};

    if (mu::ui::window::WindowGeometry(ptOrigin.x, ptOrigin.y, ptSize.x, (ptSize.y + 1) * g_DuelMgr.GetDuelWatchUserCount() + 10).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CDuelWatchUserListWindow::UpdateKeyEvent()
{
    return true;
}

bool CDuelWatchUserListWindow::Update()
{
    SyncView();
    return true;
}

bool CDuelWatchUserListWindow::Render()
{
    // Nothing native left: the boxes and the names are RmlUi (SyncView()). Kept because CObject
    // requires the override.
    return true;
}

void CDuelWatchUserListWindow::OpeningProcess() {}

void CDuelWatchUserListWindow::ClosingProcess() {}

float CDuelWatchUserListWindow::GetLayerDepth()
{
    return 5.0f;
}

void CDuelWatchUserListWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    const bool modelCreated = m_RmlBinder.Create(context, "duel_watch_spectators",
                                                 [](Rml::DataModelConstructor& c, DuelWatchSpectatorsRmlModel& model)
                                                 {
                                                     c.Bind("scale_x", &model.scaleX);
                                                     c.Bind("scale_y", &model.scaleY);
                                                     c.Bind("inverse_scale_x", &model.inverseScaleX);
                                                     c.Bind("inverse_scale_y", &model.inverseScaleY);
                                                     c.Bind("panel_x", &model.panelX);
                                                     c.Bind("text_px", &model.textPx);
                                                     c.Bind("text_top", &model.textTop);
                                                     auto spectator = c.RegisterStruct<DuelWatchSpectatorEntry>();
                                                     spectator.RegisterMember("name", &DuelWatchSpectatorEntry::name);
                                                     spectator.RegisterMember("top", &DuelWatchSpectatorEntry::top);
                                                     c.RegisterArray<std::vector<DuelWatchSpectatorEntry>>();
                                                     c.Bind("spectators", &model.spectators);
                                                 });
    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(context, "Data/Interface/RmlUi/duel_watch_spectators.rml");
}

void CDuelWatchUserListWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CDuelWatchUserListWindow::SyncView()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    // CManager scopes LayoutMode::HudFrame around the window: the bottom HUD's uniform scale, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::scaleX, "scale_x", transform.scaleX);
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::scaleY, "scale_y", transform.scaleY);
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::inverseScaleX, "inverse_scale_x", 1.0f / transform.scaleX);
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::inverseScaleY, "inverse_scale_y", 1.0f / transform.scaleY);
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::panelX, "panel_x", static_cast<float>(m_Pos.x));
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::textPx, "text_px",
              UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform));

    // The original's Render(): a 57 x 17 box per spectator, 18 apart upwards from m_Pos.y, the
    // first name in the top box; each name centred on the box, (17 - font height) / 2 + 1 whole
    // units down.
    g_pRenderText->SetFont(g_hFont);
    const long fontHeight = static_cast<long>(g_pRenderText->MeasureText(L"Q", 1).cy);
    SyncField(m_RmlBinder, &DuelWatchSpectatorsRmlModel::textTop, "text_top",
              static_cast<float>((17 - fontHeight) / 2 + 1));

    const int count = g_DuelMgr.GetDuelWatchUserCount();
    std::vector<DuelWatchSpectatorEntry> spectators;
    spectators.reserve(static_cast<size_t>(std::max(0, count)));
    for (int i = 0; i < count; ++i)
    {
        spectators.push_back({StringUtils::WideToNarrow(g_DuelMgr.GetDuelWatchUser(i)),
                              static_cast<float>(m_Pos.y - 18 * count + 18 * i)});
    }
    DuelWatchSpectatorsRmlModel& model = m_RmlBinder.GetModel();
    const bool same = model.spectators.size() == spectators.size() &&
                      std::equal(model.spectators.begin(), model.spectators.end(), spectators.begin(),
                                 [](const DuelWatchSpectatorEntry& a, const DuelWatchSpectatorEntry& b)
                                 { return a.name == b.name && a.top == b.top; });
    if (!same)
    {
        model.spectators = std::move(spectators);
        m_RmlBinder.MarkDirty("spectators");
    }
}

bool CDuelWatchUserListWindow::BtnProcess()
{
    return false;
}
