//////////////////////////////////////////////////////////////////////
// WindowGeometry.h: opt-in, composed 2D rect + hit-test for mu::ui::window
// widgets that have a real screen-space position (docs/ui-target-architecture.md
// Section C). Not a CObject base-class field -- a window that has a
// meaningful rect owns one of these and forwards to it; a window whose
// position is a per-frame WorldToScreen() projection (CMyInventory's
// 3D-anchored slots, CCharInfoBalloonMng) never creates one.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINDOWGEOMETRY_H__3B7B6C0A_3C0E_4B4E_9C7B_9A2E7B2D6F4A__INCLUDED_)
#define AFX_WINDOWGEOMETRY_H__3B7B6C0A_3C0E_4B4E_9C7B_9A2E7B2D6F4A__INCLUDED_

#pragma once

namespace mu::ui::window
{
    // Value object, not a mixin base -- composition over inheritance (Section C).
    // Position/size are stored in whatever space the owning widget already draws
    // in (the reference-resolution space every LayoutMode but Legacy rescales
    // against). Contains() compares directly against the caller's coordinates
    // with no further scaling of its own: by the time a widget's UpdateMouseEvent()
    // runs, CManager has already re-projected MouseX/MouseY into that same space
    // via ScopedActiveTransform's transformMouse=true (UI/Core/WindowManager.cpp),
    // exactly the assumption the existing mu::ui::window::CheckMouseIn() free
    // function (WindowCommon.h) already relies on -- this is a drop-in replacement
    // for that per-widget hand-rolled rect check, not a new hit-testing rule.
    class WindowGeometry
    {
    public:
        WindowGeometry() = default;
        WindowGeometry(int x, int y, int width, int height);

        void SetPosition(int x, int y);
        void SetSize(int width, int height);
        void SetBounds(int x, int y, int width, int height);

        const POINT& Position() const { return m_pos; }
        const SIZE& Size() const { return m_size; }
        RECT Bounds() const;

        // True if (windowX, windowY) -- e.g. the global MouseX/MouseY -- falls
        // inside this rect. Half-open, matching CheckMouseIn(): the right/bottom
        // edge is exclusive.
        bool Contains(int windowX, int windowY) const;

    private:
        POINT m_pos{ 0, 0 };
        SIZE m_size{ 0, 0 };
    };
}

#endif // !defined(AFX_WINDOWGEOMETRY_H__3B7B6C0A_3C0E_4B4E_9C7B_9A2E7B2D6F4A__INCLUDED_)
