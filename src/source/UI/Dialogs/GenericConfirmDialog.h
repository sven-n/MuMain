//*****************************************************************************
// File: GenericConfirmDialog.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <deque>
#include <functional>
#include <string>
#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CManager;

    // Config for one confirm dialog. Passed by value to CGenericConfirmDialog::Show() -- the whole
    // point of this primitive is that a new confirm dialog is a value of this struct, not a new
    // C++ class (contrast with CCommonMessageBox's ~140 TMsgBoxLayout<> subclasses, one per dialog).
    struct GenericDialogConfig
    {
        enum class ButtonSet { Ok, OkCancel };
        ButtonSet buttons = ButtonSet::Ok;
        std::wstring primaryLabel = L"OK";
        std::wstring secondaryLabel = L"Cancel"; // ignored unless buttons == OkCancel

        struct Line { std::wstring text; bool bold = false; };
        std::vector<Line> lines;

        std::function<void()> onPrimary;   // OK / Enter
        std::function<void()> onSecondary; // Cancel / Esc -- optional, safe to leave empty
    };

    // Reusable RmlUi confirm dialog -- one document/model, one instance, shown with different
    // GenericDialogConfig content per call. Meant as the replacement primitive for
    // CCommonMessageBox/CustomMessageBox's whole native TMsgBoxLayout<> family (see STATUS.md);
    // this class + generic_confirm_dialog.rml/.rcss is proven on 3 real dialogs first
    // (Guild/GuildInfoWindow.cpp's CGuildOutPerson, UI/Quests/MyQuestInfoWindow.cpp's
    // RmlClickGiveUp(), Network/Server/WSclient.cpp's ReceiveGuild()) before any of the other ~140
    // native dialogs are ported onto it.
    //
    // Single active instance, not a stack: if Show() is called while one is already open, the new
    // config is queued and shown once the active one resolves -- matches CMsgWin's own single-
    // instance shape. CMessageBoxMng's vector suggests real stacking but in practice only ever
    // drives the single highest-priority box per frame anyway (confirmed by reading its Render()/
    // Update()), so this isn't a functional regression, just a more explicit version of the same
    // "one at a time" behavior.
    class CGenericConfirmDialog : public CObject
    {
    public:
        void Create(CManager* pMng);
        void Release();

        // Shows now if idle, otherwise queues (see class comment).
        void Show(GenericDialogConfig cfg);

        bool Render() override;
        bool Update() override;
        // Unconditionally claims input while shown -- a true modal, matching CMessageBoxMng's own
        // "return false while any box is open" contract (CMsgWin's RmlUi-tier equivalent doesn't
        // block key dispatch the same way; this class deliberately does, since it stands in for a
        // system that did).
        bool UpdateMouseEvent() override { return !IsVisible(); }
        bool UpdateKeyEvent() override;
        // CObject::IsVisible() reflects Show(bool)/m_bRender, which this class never touches --
        // visibility here is purely "is a dialog currently active," so this is overridden instead.
        bool IsVisible() const override { return m_bActive; }
        // Above CMsgWin's 50.0f -- a confirm dialog should sit on top of an ordinary message window.
        float GetLayerDepth() override { return 60.0f; }
        // Matches CMessageBoxMng's own GetKeyEventOrder() (10.f) -- the system this replaces.
        float GetKeyEventOrder() override { return 10.0f; }
        void ReloadRmlTheme() override;

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void ShowNext();            // pops m_Queue (if non-empty) and opens the document
        void Resolve(bool primary); // hides the document, invokes the chosen callback, then ShowNext()

        struct LineEntry { Rml::String text; bool bold = false; };
        struct GenericDialogRmlModel
        {
            std::vector<LineEntry> lines;
            bool showCancel = false;
            Rml::String primaryLabel;
            Rml::String secondaryLabel;
        };
        RmlModelBinder<GenericDialogRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // Set by RmlUi click bindings; polled and cleared in Update() -- never act synchronously
        // inside the RmlUi callback itself, same convention as CMsgWin/RememberPasswordPrompt.
        bool m_bPrimaryClicked = false;
        bool m_bSecondaryClicked = false;

        std::deque<GenericDialogConfig> m_Queue;
        GenericDialogConfig m_Active;
        bool m_bActive = false;
    };

    // Convenience global for the scattered native call sites this primitive is meant to replace
    // (guild/quest UI code, network packet handlers) -- same convention as g_pUIPopup. Set once,
    // in CSystem::Create() (WindowSystem.cpp), alongside every other app-lifetime CObject-tier
    // window; never null after that point during normal play.
    extern CGenericConfirmDialog* g_pGenericConfirmDialog;
}
