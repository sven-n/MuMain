#pragma once

// Synchronizes visibility and stacking for the semantic documents owned by the friends family.
// Window order comes from CUIWindowMgr; native portraits are rendered by its overlay pass.

#include <list>

class CUIBaseWindow;

class FriendWindowViews
{
public:
    // windows: the manager's windows in draw order (back to front); nullptr entries are skipped.
    void Sync(const std::list<CUIBaseWindow*>& windows, bool familyShown);

private:
    std::list<DWORD> m_Order; // the draw order the documents were last stacked in
};
