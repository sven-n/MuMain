// MuHelperBar.cpp: implementation of the CMuHelperBar class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/HUD/MuHelperBar.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/NewUISystem.h"
#include "World/MapInfra/MapManager.h"
#include "MUHelper/MuHelper.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

using namespace SEASON3B;

CMuHelperBar::CMuHelperBar()
{
    m_pNewUIMng = NULL;
    m_CurHeroPosition.x = m_CurHeroPosition.y = 0;
}

CMuHelperBar::~CMuHelperBar()
{
    Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CMuHelperBar::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MU_HELPER_BAR, this);

    // RmlUi migration -- see this class's header comment. Guarded like every other hybrid
    // window's Create() (re-run on resolution change), so the document/model are created once,
    // ever.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "mu_helper_bar",
            [this](Rml::DataModelConstructor& c, MuHelperBarRmlModel& model)
            {
                c.Bind("position_text", &model.positionText);
                c.Bind("mu_helper_active", &model.muHelperActive);
                c.Bind("config_tooltip", &model.configTooltip);
                c.Bind("start_tooltip", &model.startTooltip);
                c.Bind("stop_tooltip", &model.stopTooltip);

                c.BindEventCallback("mu_helper_config_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickConfig(); });
                c.BindEventCallback("mu_helper_toggle_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickToggle(); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/mu_helper_bar.rml");

        // Deliberately NOT Show()n here -- see NewUIMainFrameWindow.cpp's identical comment.
        // Create() runs during WebzenScene()'s boot-time loading screen, well before SceneFlag
        // ever reaches MAIN_SCENE; SyncDocVisibility() (called every frame regardless of scene)
        // shows it the first time CNewUISystem::SyncMainSceneHudVisibility()'s gate allows it.
    }

    Show(true);

    return true;
}

void CMuHelperBar::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    // See CLoginWin::PreRelease()'s identical rationale -- this object's release has no knowledge
    // of m_pRmlDoc otherwise, and RmlUi renders last in the frame regardless of scene.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

bool CMuHelperBar::UpdateMouseEvent()
{
    // RmlUi's own context does hit-testing now (see this class's header comment) -- never
    // consumes the legacy mouse event.
    return true;
}

bool CMuHelperBar::UpdateKeyEvent()
{
    return true;
}

bool CMuHelperBar::Update()
{
    if (m_bRmlConfigClicked)
    {
        m_bRmlConfigClicked = false;
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MUHELPER);
        PlayBuffer(SOUND_CLICK01);
    }
    else if (m_bRmlToggleClicked)
    {
        m_bRmlToggleClicked = false;
        MUHelper::g_MuHelper.Toggle();
        PlayBuffer(SOUND_CLICK01);
    }

    if ((IsVisible() == true) && (Hero != NULL))
    {
        m_CurHeroPosition.x = (Hero->PositionX);
        m_CurHeroPosition.y = (Hero->PositionY);
    }

    SyncRmlModel();

    return true;
}

bool CMuHelperBar::Render()
{
    // RmlUi's #panel now owns 100% of this widget's visuals -- see this class's header comment.
    // Nothing left to draw here; SyncRmlModel() (called from Update()) is what keeps the RmlUi
    // model current.
    return true;
}

void CMuHelperBar::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    wchar_t szText[255] = {};
    mu_swprintf(szText, L"%ls (%d , %d)", gMapManager.GetMapName(gMapManager.WorldActive), m_CurHeroPosition.x, m_CurHeroPosition.y);
    const std::string positionUtf8 = StringUtils::WideToNarrow(szText);
    if (model.positionText != positionUtf8)
    {
        model.positionText = positionUtf8;
        m_RmlBinder.MarkDirty("position_text");
    }

    const bool active = MUHelper::g_MuHelper.IsActive();
    if (model.muHelperActive != active)
    {
        model.muHelperActive = active;
        m_RmlBinder.MarkDirty("mu_helper_active");
    }

    auto syncLabel = [this](Rml::String MuHelperBarRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncLabel(&MuHelperBarRmlModel::configTooltip, "config_tooltip", I18N::Game::OfficialMUHelperSetting);
    syncLabel(&MuHelperBarRmlModel::startTooltip, "start_tooltip", I18N::Game::StartOfficialMUHelper);
    syncLabel(&MuHelperBarRmlModel::stopTooltip, "stop_tooltip", I18N::Game::StopOfficialMUHelper);
}

float CMuHelperBar::GetLayerDepth()
{
    return 4.3f;
}

void CMuHelperBar::SyncDocVisibility(bool sceneAllowsShow)
{
    if (!m_pRmlDoc) return;

    if (IsVisible() && sceneAllowsShow)
        m_pRmlDoc->Show();
    else
        m_pRmlDoc->Hide();
}

void CMuHelperBar::OpenningProcess()
{
}

void CMuHelperBar::ClosingProcess()
{
}
