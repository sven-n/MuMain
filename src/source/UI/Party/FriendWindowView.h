#pragma once

// Drives the RmlUi documents of the friends family (CUIWindowMgr's windows, UIWindows.h). Every
// window of the family now owns a semantic document of its own -- the shell, each chat room, and
// each letter being read or written -- so this is only the part that cannot live in any one of
// them: the manager's draw order, and which windows' native portraits sit in front of which.
//
// It replaced a transcription layer that rebuilt one document per window every frame from the
// native widgets' live pixel geometry. That layer is gone.

#include <list>
#include <vector>

class CUIBaseWindow;

// A rectangle in native reference px.
struct FriendWindowRect
{
    float left = 0.f;
    float top = 0.f;
    float right = 0.f;
    float bottom = 0.f;
};

class FriendWindowViews
{
public:
    // windows: the manager's windows in draw order (back to front); nullptr entries are skipped.
    void Sync(const std::list<CUIBaseWindow*>& windows, bool familyShown);

private:
    std::list<DWORD> m_Order; // the draw order the documents were last stacked in
};
