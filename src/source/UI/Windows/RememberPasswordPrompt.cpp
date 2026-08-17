#include "stdafx.h"
#include "UI/Windows/RememberPasswordPrompt.h"

#include "Audio/DSPlaySound.h"
#include "Core/Input/Input.h"
#include "I18N/All.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Property.h>

// RmlUi migration, Batch 2: previously built on the shared SEASON3B::CNewUICommonMessageBox /
// g_MessageBox engine (~80 other unrelated dialogs also ride that singleton stack -- confirmed by
// exhaustive grep before this rewrite). Bypassing it here for this one dialog is safe (none of
// the other consumers reference this dialog or its state) and removes two couplings: the login
// scene no longer needs to hand-pump g_MessageBox (see LoginScene.cpp's render path, which
// already drives RmlUiRuntime unconditionally every frame regardless of scene), and CLoginWin no
// longer gates its own input on the *entire* shared message-box stack being empty, just on this
// dialog's own Pending state.
namespace
{
    struct PromptModel
    {
        Rml::String titleText;
        Rml::String bodyText;
        Rml::String okLabel;
        Rml::String cancelLabel;
    };

    RmlModelBinder<PromptModel> g_Binder;
    Rml::ElementDocument* g_pDoc = nullptr;
    UI::Login::RememberPasswordChoice g_Choice = UI::Login::RememberPasswordChoice::None;

    void Resolve(UI::Login::RememberPasswordChoice choice)
    {
        g_Choice = choice;
        PlayBuffer(SOUND_CLICK01);
        if (g_pDoc)
            g_pDoc->Hide();
    }

    // Document/model created once, guarded the same way CLoginWin::Create() is -- see that
    // class's header comment. This module has no owning class/Create() entry point of its own
    // (it's plain free functions), so the guard lives here instead, called lazily from
    // OpenRememberPasswordPrompt().
    void EnsureCreated()
    {
        if (g_pDoc || !RmlUiRuntime::Instance().IsCreated())
            return;

        const bool modelCreated = g_Binder.Create(RmlUiRuntime::Instance().GetContext(), "remember_password_prompt",
            [](Rml::DataModelConstructor& c, PromptModel& model)
            {
                c.Bind("title_text", &model.titleText);
                c.Bind("body_text", &model.bodyText);
                c.Bind("ok_label", &model.okLabel);
                c.Bind("cancel_label", &model.cancelLabel);

                c.BindEventCallback("prompt_ok_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Resolve(UI::Login::RememberPasswordChoice::Ok); });
                c.BindEventCallback("prompt_cancel_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Resolve(UI::Login::RememberPasswordChoice::Cancel); });
            });

        if (modelCreated)
            g_pDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/remember_password_prompt.rml");

        if (g_pDoc)
        {
            // Centered once at creation -- this document has no CWin/resize hook (see the
            // migration plan's known-gap note), so a live resolution change while it's open
            // would leave it off-center until the next open. Accepted, not solved here.
            if (Rml::Element* panel = g_pDoc->GetElementById("panel"))
            {
                const Rml::Property* widthProp = panel->GetProperty("width");
                const Rml::Property* heightProp = panel->GetProperty("height");
                const float panelWidth = widthProp ? widthProp->Get<float>() : 0.0f;
                const float panelHeight = heightProp ? heightProp->Get<float>() : 0.0f;
                panel->SetProperty("left", std::to_string((CInput::Instance().GetScreenWidth() - panelWidth) / 2) + "px");
                panel->SetProperty("top", std::to_string((CInput::Instance().GetScreenHeight() - panelHeight) / 2) + "px");
            }
        }
    }

    void SyncLabels()
    {
        auto syncLabel = [](Rml::String PromptModel::* field, const char* boundName, const wchar_t* text)
        {
            const std::string utf8 = StringUtils::WideToNarrow(text);
            if (g_Binder.GetModel().*field != utf8)
            {
                g_Binder.GetModel().*field = utf8;
                g_Binder.MarkDirty(boundName);
            }
        };
        syncLabel(&PromptModel::titleText, "title_text", I18N::Game::LoginSavePasswordWarningTitle);
        syncLabel(&PromptModel::bodyText, "body_text", I18N::Game::LoginSavePasswordWarningBody);
        syncLabel(&PromptModel::okLabel, "ok_label", I18N::Game::OK);
        syncLabel(&PromptModel::cancelLabel, "cancel_label", I18N::Game::Cancel);
    }
} // namespace

namespace UI::Login
{
void OpenRememberPasswordPrompt()
{
    g_Choice = RememberPasswordChoice::Pending;
    EnsureCreated();
    if (g_pDoc)
    {
        SyncLabels();
        g_pDoc->Show();
    }
}

RememberPasswordChoice RememberPasswordChoiceState()
{
    return g_Choice;
}

void ClearRememberPasswordChoice()
{
    g_Choice = RememberPasswordChoice::None;
}

void Tick()
{
    if (g_Choice != RememberPasswordChoice::Pending)
        return;

    if (CInput::Instance().IsKeyDown(VK_RETURN))
        Resolve(RememberPasswordChoice::Ok);
    else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
        Resolve(RememberPasswordChoice::Cancel);
}
} // namespace UI::Login
