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

// Bypasses the shared g_MessageBox stack: this dialog owns its own RmlUi document/state
// independently, so callers gate on its own Pending state rather than the whole message-box stack.
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

    // Creates the document/model once, lazily, on first open.
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

        // Centering is handled by #panel's own `.center-both` RCSS class, not pushed from here.
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
        // Modal: without it the login document underneath stays clickable and can steal focus back.
        g_pDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
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

void ReloadRmlTheme()
{
    if (!g_pDoc) return; // never opened; EnsureCreated() will pick up the new theme later

    const bool wasPending = (g_Choice == RememberPasswordChoice::Pending);
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    g_Binder.Destroy(context);
    context->UnloadDocument(g_pDoc);
    g_pDoc = nullptr;

    EnsureCreated();
    if (g_pDoc && wasPending)
    {
        SyncLabels();
        g_pDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}
} // namespace UI::Login
