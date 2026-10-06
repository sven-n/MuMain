//*****************************************************************************
// File: ServerMsgWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "ServerMsgWin.h"
#include "UI/Core/SceneUICoordinator.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/UsefulDef.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

namespace
{
// A side piece of the frame (server_ex02, 3 x 4) is drawn once per step, five steps per line.
constexpr int kSideStepHeight = 4;
} // namespace

CServerMsgWin g_ServerMsgWin;

// cppcheck-suppress uninitMemberVar
CServerMsgWin::CServerMsgWin() {}

CServerMsgWin::~CServerMsgWin()
{
    Release();
}

void CServerMsgWin::Create()
{
    Release();

    m_ptPos.x = m_ptPos.y = 0;
    m_nBgSideNow = 1;

    ::memset(m_aszMsg, 0, sizeof(wchar_t) * SMW_MSG_LINE_MAX * SMW_MSG_ROW_MAX);
    m_nMsgLine = 0;

    BuildRmlUi();

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_SERVER_MESSAGE, this);
    Show(false);
}

void CServerMsgWin::Release()
{
    // Hidden at once (not on the next Update(), which a released scene no longer runs): every
    // sibling window released at the character-select -> main-scene transition
    // (CSceneUICoordinator::CreateMainScene()) hides itself in its own Release(), so a server
    // notice showing at that moment must not linger either.
    mu::ui::window::CObject::Show(false);
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), false);
}

void CServerMsgWin::SetPosition(int nXCoord, int nYCoord)
{
    m_ptPos.x = nXCoord;
    m_ptPos.y = nYCoord;
}

int CServerMsgWin::SetLine(int nLine)
{
    nLine = LIMIT(nLine, 1, SMW_MSG_LINE_MAX * 5);

    const int nOldLine = m_nBgSideNow;
    m_nBgSideNow = nLine;
    return nOldLine;
}

void CServerMsgWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
}

void CServerMsgWin::AddMsg(wchar_t* pszMsg)
{
    if (++m_nMsgLine > SMW_MSG_LINE_MAX)
    {
        m_nMsgLine = SMW_MSG_LINE_MAX;
        for (int i = 0; i < SMW_MSG_LINE_MAX - 1; ++i)
            ::wcscpy(m_aszMsg[i], m_aszMsg[i + 1]);
    }
    else
        SetLine(m_nMsgLine * 5);

    wcscpy(m_aszMsg[m_nMsgLine - 1], pszMsg);

    Show(true);
}

bool CServerMsgWin::Render()
{
    // Nothing native left: the frame and the lines are RmlUi. Kept because CObject requires the
    // override.
    return true;
}

bool CServerMsgWin::Update()
{
    SyncRmlModel();
    return true;
}

void CServerMsgWin::BindRmlModel(Rml::DataModelConstructor& c, ServerMsgRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    c.Bind("text_px", &model.textPx);
    c.Bind("side_height", &model.sideHeight);
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("lines", &model.lines);
}

void CServerMsgWin::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CServerMsgWin::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 10: over the character-list scene's other windows.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    // LayoutMode::Legacy keeps the transform identity here: real pixels, as the original drew.
    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_ptPos);
    ServerMsgRmlModel& model = m_RmlView.GetModel();
    const float textPx =
        UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Fixed, UI::Scaling::GetActiveTransform());
    if (model.textPx != textPx)
    {
        model.textPx = textPx;
        m_RmlView.MarkDirty("text_px");
    }
    const float sideHeight = static_cast<float>(kSideStepHeight * m_nBgSideNow);
    if (model.sideHeight != sideHeight)
    {
        model.sideHeight = sideHeight;
        m_RmlView.MarkDirty("side_height");
    }
    std::vector<Rml::String> lines;
    for (int i = 0; i < m_nMsgLine; ++i)
        lines.push_back(StringUtils::WideToNarrow(m_aszMsg[i]));
    if (model.lines != lines)
    {
        model.lines = std::move(lines);
        m_RmlView.MarkDirty("lines");
    }
}
