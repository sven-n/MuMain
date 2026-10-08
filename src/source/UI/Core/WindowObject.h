#ifndef _NEWUIBASE_H_
#define _NEWUIBASE_H_

#pragma once

#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Scaling/UITransform.h"

extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

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
        UI::Scaling::Transform m_slotTransform{1.f, 1.f, 0.f, 0.f, 1.f};
        UI::RmlBridge::FillPlacementSize m_fillSize;
        UI::RmlBridge::SlotPlacement m_slotPlacement;
    public:
        CObject()
            : m_hRelatedWnd(nullptr), m_bRender(true), m_bUpdate(true), m_bActive(true),
              m_layoutMode(UI::Scaling::LayoutMode::Stage)
        {
        }
        virtual ~CObject() {}

        void SetRelatedWnd(HWND hWnd = g_hWnd)
        {
            m_hRelatedWnd = hWnd;
        }
        HWND GetRelatedWnd() const { return m_hRelatedWnd; }
        // While the player types in a RmlUi field, keys go only to the window that claims the
        // field's document; every other window's keys and hotkeys wait (CManager::UpdateKeyEvent()).
        virtual bool TakesTypingFrom(const Rml::ElementDocument* document) const { return false; }
        void SetLayoutMode(UI::Scaling::LayoutMode mode) { m_layoutMode = mode; }
        UI::Scaling::LayoutMode GetLayoutMode() const { return m_layoutMode; }
        // A window drawn wholly by RmlUi whose hit box reads its #panel returns that document: a
        // theme's data-fit="fill" slot then sizes the #panel. How the content fills it is the theme's.
        virtual Rml::ElementDocument* GetFillDocument() const { return nullptr; }
        virtual bool SupportsFillPlacement() const { return GetFillDocument() != nullptr; }
        virtual void SetFillPlacementSize(float width, float height)
        {
            if (m_fillSize.Set(width, height))
                m_fillSize.Apply(GetFillDocument(), "panel");
        }
        // Once a frame: gives a document rebuilt since (a theme switch) its fill size again.
        void SyncFillPlacement() const { m_fillSize.Sync(GetFillDocument(), "panel"); }
        // The smallest size a fill slot may give this window, in its layout units; without one the
        // window's content size.
        virtual bool GetFillMinimumSize(float&, float&) const { return false; }
        // A window drawn by a document whose #panel the workspace places: a slot gives the panel its
        // position and scale, so the window binds neither.
        virtual Rml::ElementDocument* GetPlacedDocument() const { return nullptr; }
        // The id of the element the slot places in that document.
        virtual const char* PlacedRootId() const { return "panel"; }
        // The workspace places this window: its logical space is `transform`, with (0, 0) at the
        // slot's top-left.
        void PlaceInSlot(const UI::Scaling::Transform& transform)
        {
            m_layoutMode = UI::Scaling::LayoutMode::Slot;
            m_slotTransform = transform;
            PlaceDocument(transform.offsetX, transform.offsetY, transform.scaleX);
        }
        // Places the document's root at `left`/`top` (screen pixels) and `scale`, leaving the
        // window's own layout mode alone (a HUD part keeps its HUD space).
        void PlaceDocument(float left, float top, float scale)
        {
            if (m_slotPlacement.Set(left, top, scale))
                m_slotPlacement.Apply(GetPlacedDocument(), PlacedRootId());
        }
        // The theme gives this window no slot any more: it returns to `mode`.
        void LeaveSlot(UI::Scaling::LayoutMode mode)
        {
            m_layoutMode = mode;
            if (m_slotPlacement.Set(0.f, 0.f, 0.f))
                m_slotPlacement.Apply(GetPlacedDocument(), PlacedRootId());
        }
        // Once a frame: gives a document rebuilt since (a theme switch) its slot placement again.
        void SyncSlotPlacement() const { m_slotPlacement.Sync(GetPlacedDocument(), PlacedRootId()); }
        // The transform this window's logical coordinates map through to screen pixels.
        UI::Scaling::Transform GetLayoutTransform() const
        {
            if (m_layoutMode == UI::Scaling::LayoutMode::Slot)
                return m_slotTransform;
            return UI::Scaling::TransformForLayout(m_layoutMode, static_cast<int>(WindowWidth),
                                                   static_cast<int>(WindowHeight));
        }

        // Virtual so a window needing more than a flag flip on show/hide (e.g. toggling its own
        // sprites) still runs correctly through CManager's generic CObject*/IObject* dispatch.
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

        // Shown-vs-active split mirrors CWin's UpdateWhileShow()/UpdateWhileActive() shape.
        // Default IsActive()==true; the base Update() below is inert for any subclass that
        // already overrides Update() itself (all current ones do).
        virtual bool IsActive() const { return m_bActive; }
        // Virtual so a window can also bump its own GetLayerDepth() baseline on activation
        // (e.g. floating-dialog raise-to-front).
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

        // Called every frame for each visible object, under its layout transform, before any
        // window renders: a document filled here shows this frame's state (CNameWindow's world
        // labels, recorded from the scene's overlay passes).
        virtual void PrepareFrame() {}
    };
}

#endif // _NEWUIBASE_H_
