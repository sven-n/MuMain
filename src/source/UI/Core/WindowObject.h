#ifndef _NEWUIBASE_H_
#define _NEWUIBASE_H_

#pragma once

#include "UI/Scaling/UITransform.h"

 
namespace mu::ui::window
{
    class IObject
    {
    public:
        virtual bool Render() = 0;
        virtual bool Update() = 0;
        virtual bool UpdateMouseEvent() = 0;
        virtual bool UpdateKeyEvent() = 0;

        virtual float GetLayerDepth() = 0;
        virtual float GetKeyEventOrder() = 0;

        virtual bool IsVisible() const = 0;
        virtual bool IsEnabled() const = 0;
    };

    class CObject : public IObject
    {
        HWND m_hRelatedWnd;
        bool m_bRender, m_bUpdate;
        bool m_bActive;
        UI::Scaling::LayoutMode m_layoutMode;
    public:
        CObject()
            : m_hRelatedWnd(nullptr), m_bRender(true), m_bUpdate(true), m_bActive(true),
              m_layoutMode(UI::Scaling::LayoutMode::Dialog)
        {
        }
        virtual ~CObject() {}

        void SetRelatedWnd(HWND hWnd = g_hWnd)
        {
            m_hRelatedWnd = hWnd;
        }
        HWND GetRelatedWnd() const { return m_hRelatedWnd; }
        void SetLayoutMode(UI::Scaling::LayoutMode mode) { m_layoutMode = mode; }
        UI::Scaling::LayoutMode GetLayoutMode() const { return m_layoutMode; }

        // Virtual (CUIMng/CNewUIManager merger) -- a window that must do more than flip a flag on
        // show/hide (e.g. CCreditWin toggling its own sprites' visibility, matching CWin::Show()'s
        // equivalent override contract) needs this to actually run when called through a base
        // CObject*/IObject* pointer, e.g. CManager::ShowInterface()'s generic dispatch.
        virtual void Show(bool bShow)
        {
            m_bRender = bShow;
        }
        void Enable(bool bEnable)
        {
            m_bUpdate = bEnable;
        }

        bool IsVisible() const override { return m_bRender; }
        bool IsEnabled() const override { return m_bUpdate; }

        // Shown-vs-active split, mirroring CWin's UpdateWhileShow()/UpdateWhileActive() shape
        // (Win.h) so a migrated window's existing shown/active logic ports over directly instead
        // of needing a redesign -- not the exact signature (CWin's hooks take a dDeltaTick
        // nothing in this tier's Update() loop threads through; a migrated window reads whatever
        // timing source it already uses internally). Default IsActive()==true, and the base
        // Update() below only ever runs for a subclass that does NOT override Update() itself --
        // every existing CObject subclass already does, so this is inert for all of them; it only
        // activates for a future subclass that overrides UpdateWhileShown()/UpdateWhileActive()
        // instead.
        virtual bool IsActive() const { return m_bActive; }
        // Virtual, not just a setter -- a window implementing floating-dialog raise-to-front
        // behavior on activation can override this to also bump its own GetLayerDepth() baseline.
        // Needs no other base-class support: GetLayerDepth() is already each subclass's own
        // responsibility, and CManager already re-sorts by it every frame.
        virtual void SetActive(bool bActive) { m_bActive = bActive; }

        bool Update() override
        {
            bool bResult = UpdateWhileShown();
            if (bResult && m_bActive)
                bResult = UpdateWhileActive();
            return bResult;
        }
        virtual bool UpdateWhileShown() { return true; }
        virtual bool UpdateWhileActive() { return true; }

        virtual float GetKeyEventOrder() { return 3.0f; }		//. Default
    };
}

#endif // _NEWUIBASE_H_
