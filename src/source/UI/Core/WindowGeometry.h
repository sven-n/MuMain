//////////////////////////////////////////////////////////////////////
// WindowGeometry.h: opt-in 2D rect + hit-test, composed into widgets that
// have a real screen-space position (not used by 3D-projected widgets).
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WINDOWGEOMETRY_H__3B7B6C0A_3C0E_4B4E_9C7B_9A2E7B2D6F4A__INCLUDED_)
#define AFX_WINDOWGEOMETRY_H__3B7B6C0A_3C0E_4B4E_9C7B_9A2E7B2D6F4A__INCLUDED_

#pragma once

namespace mu::ui::window
{
    // Value object, not a mixin base (composition over inheritance). Contains()
    // does no scaling of its own -- it assumes CManager has already re-projected
    // MouseX/MouseY into the widget's own space via ScopedActiveTransform.
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

        // True if (windowX, windowY) falls inside this rect (right/bottom edge exclusive).
        bool Contains(int windowX, int windowY) const;

    private:
        POINT m_pos{ 0, 0 };
        SIZE m_size{ 0, 0 };
    };
}

#endif // !defined(AFX_WINDOWGEOMETRY_H__3B7B6C0A_3C0E_4B4E_9C7B_9A2E7B2D6F4A__INCLUDED_)
