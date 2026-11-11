#include "stdafx.h"
#include "UI/Windows/RememberPasswordPrompt.h"

#include "Audio/DSPlaySound.h"
#include "Core/Input/Input.h"
#include "I18N/All.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlThemedView.h"
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

    UI::Login::RememberPasswordChoice g_Choice = UI::Login::RememberPasswordChoice::None;

    void Resolve(UI::Login::RememberPasswordChoice choice);

    void BindModel(Rml::DataModelConstructor& c, PromptModel& model)
    {
        c.Bind("title_text", &model.titleText);
        c.Bind("body_text", &model.bodyText);
        c.Bind("ok_label", &model.okLabel);
        c.Bind("cancel_label", &model.cancelLabel);

        c.BindEventCallback("prompt_ok_click",
            [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Resolve(UI::Login::RememberPasswordChoice::Ok); });
        c.BindEventCallback("prompt_cancel_click",
            [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { Resolve(UI::Login::RememberPasswordChoice::Cancel); });
    }

    // Built lazily, on first open. Modal: without it the login document underneath stays clickable
    // and can steal focus back. Centering is #panel's own `.center-both` RCSS class.
    UI::RmlBridge::ThemedView<PromptModel> g_View{"remember_password_prompt", BindModel,
        {{"Data/Interface/RmlUi/remember_password_prompt.rml"}},
        {.modal = Rml::ModalFlag::Modal, .focus = Rml::FocusFlag::Document}};

    void Resolve(UI::Login::RememberPasswordChoice choice)
    {
        g_Choice = choice;
        PlayBuffer(SOUND_CLICK01);
        g_View.Hide();
    }

    void SyncLabels()
    {
        auto syncLabel = [](Rml::String PromptModel::* field, const char* boundName, const wchar_t* text)
        {
            const std::string utf8 = StringUtils::WideToNarrow(text);
            if (g_View.GetModel().*field != utf8)
            {
                g_View.GetModel().*field = utf8;
                g_View.MarkDirty(boundName);
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
    if (g_View.Ensure())
    {
        SyncLabels();
        g_View.Document()->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
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
