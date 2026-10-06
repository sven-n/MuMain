
#include "stdafx.h"
#include "UI/Combat/DuelWindow.h"
#include "GameLogic/Combat/DuelMgr.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <string>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
template <typename Model, typename T>
void Sync(RmlModelBinder<Model>& binder, T Model::* field, const char* name, T value)
{
    Model& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}
} // namespace

mu::ui::window::CDuelWindow::CDuelWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

mu::ui::window::CDuelWindow::~CDuelWindow()
{
    Release();
}

bool mu::ui::window::CDuelWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DUEL_WINDOW, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CDuelWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void mu::ui::window::CDuelWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CDuelWindow::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CDuelWindow::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CDuelWindow::Update()
{
    SyncRmlModel();
    return true;
}

bool mu::ui::window::CDuelWindow::Render()
{
    // Nothing native left: the back, the names and the scores are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

float mu::ui::window::CDuelWindow::GetLayerDepth()
{
    return 1.1f;
}

void mu::ui::window::CDuelWindow::BindRmlModel(Rml::DataModelConstructor& c, DuelWindowRmlModel& model)
{
    c.Bind("scale_x", &model.scaleX);
    c.Bind("scale_y", &model.scaleY);
    c.Bind("inverse_scale_x", &model.inverseScaleX);
    c.Bind("inverse_scale_y", &model.inverseScaleY);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("panel_x", &model.panelX);
    c.Bind("panel_y", &model.panelY);
    c.Bind("hero_name", &model.heroName);
    c.Bind("hero_score", &model.heroScore);
    c.Bind("enemy_name", &model.enemyName);
    c.Bind("enemy_score", &model.enemyScore);
}

void mu::ui::window::CDuelWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CDuelWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 1.1: behind every other document of the background context (RmlStackingOrder.cpp).
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    // CManager scopes LayoutMode::HudFrame around this window: the bottom HUD's uniform scale, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::scaleX, "scale_x", transform.scaleX);
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::scaleY, "scale_y", transform.scaleY);
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::inverseScaleX, "inverse_scale_x", 1.0f / transform.scaleX);
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::inverseScaleY, "inverse_scale_y", 1.0f / transform.scaleY);
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::boldTextPx, "bold_text_px",
         UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::panelX, "panel_x", static_cast<float>(m_Pos.x));
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::panelY, "panel_y", static_cast<float>(m_Pos.y));

    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::heroName, "hero_name",
         Rml::String(StringUtils::WideToNarrow(g_DuelMgr.GetDuelPlayerID(DUEL_HERO))));
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::enemyName, "enemy_name",
         Rml::String(StringUtils::WideToNarrow(g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY))));
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::heroScore, "hero_score", std::to_string(g_DuelMgr.GetScore(DUEL_HERO)));
    Sync(m_RmlView.Binder(), &DuelWindowRmlModel::enemyScore, "enemy_score", std::to_string(g_DuelMgr.GetScore(DUEL_ENEMY)));
}
