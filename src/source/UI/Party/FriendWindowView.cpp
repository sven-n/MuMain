#include "stdafx.h"
#include "UI/Party/FriendWindowView.h"

#include "UI/Party/UIWindows.h"

#include <algorithm>

void FriendWindowViews::Sync(const std::list<CUIBaseWindow*>& windows, bool familyShown)
{
    const auto isShown = [familyShown](CUIBaseWindow* window)
    { return familyShown && window->GetState() != UISTATE_HIDE && window->GetState() != UISTATE_READY; };

    bool restack = false;
    std::list<DWORD> order;
    for (CUIBaseWindow* window : windows)
    {
        if (window == nullptr || !window->HasSemanticView())
            continue;
        const bool shown = isShown(window);
        restack |= window->SyncSemanticView(shown);
        if (shown)
            order.push_back(window->GetUIID());
    }

    // Stack the shown documents in the manager's draw order, in front of the other windows.
    if (restack || order != m_Order)
    {
        for (auto* window : windows)
        {
            if (window != nullptr && window->HasSemanticView() && isShown(window))
                window->PullSemanticViewToFront();
        }
        m_Order = std::move(order);
    }
}

CUIBaseWindow* CUIWindowMgr::GetFieldFocusWindow() const
{
    for (const auto& [uiid, window] : m_WindowMap)
    {
        if (window != nullptr && window->SemanticFieldHasFocus())
            return window;
    }
    return nullptr;
}

bool CUIWindowMgr::RmlFieldHasFocus(DWORD dwUIID) const
{
    auto it = m_WindowMap.find(dwUIID);
    return it != m_WindowMap.end() && it->second->SemanticFieldHasFocus();
}

void CUIWindowMgr::SyncRmlViews(bool familyShown)
{
    if (!m_pRmlViews)
        m_pRmlViews = std::make_unique<FriendWindowViews>();
    // Draw order first, then the windows the arrange list does not hold (hidden ones keep their
    // documents).
    std::list<CUIBaseWindow*> windows;
    for (DWORD uiid : m_WindowArrangeList)
    {
        auto it = m_WindowMap.find(uiid);
        if (it != m_WindowMap.end())
            windows.push_back(it->second);
    }
    for (const auto& [uiid, window] : m_WindowMap)
    {
        if (std::find(m_WindowArrangeList.begin(), m_WindowArrangeList.end(), uiid) == m_WindowArrangeList.end())
            windows.push_back(window);
    }
    m_pRmlViews->Sync(windows, familyShown);
}
