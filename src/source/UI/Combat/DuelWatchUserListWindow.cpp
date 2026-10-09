
#include "stdafx.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/Combat/DuelWatchUserListWindow.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Combat/DuelMgr.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"

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

    Show(false);

    return true;
}

void CDuelWatchUserListWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
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

    // The list takes no pointer events, but its names still hold the pointer, as the original's did.
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document != nullptr && UI::RmlBridge::IsPointerWithin(document->QuerySelector("#panel .stack")))
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

void CDuelWatchUserListWindow::BindRmlModel(Rml::DataModelConstructor& c, DuelWatchSpectatorsRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("spectators", &model.spectators);
}

void CDuelWatchUserListWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CDuelWatchUserListWindow::SyncView()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    SyncField(m_RmlView.Binder(), &DuelWatchSpectatorsRmlModel::textPx, "text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal));

    const int count = g_DuelMgr.GetDuelWatchUserCount();
    std::vector<Rml::String> spectators;
    spectators.reserve(static_cast<size_t>(std::max(0, count)));
    for (int i = 0; i < count; ++i)
        spectators.push_back(StringUtils::WideToNarrow(g_DuelMgr.GetDuelWatchUser(i)));
    DuelWatchSpectatorsRmlModel& model = m_RmlView.GetModel();
    if (model.spectators != spectators)
    {
        model.spectators = std::move(spectators);
        m_RmlView.MarkDirty("spectators");
    }
}

bool CDuelWatchUserListWindow::BtnProcess()
{
    return false;
}
