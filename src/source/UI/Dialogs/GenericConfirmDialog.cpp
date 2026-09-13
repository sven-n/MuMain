//*****************************************************************************
// File: GenericConfirmDialog.cpp
//*****************************************************************************
#include "stdafx.h"
#include "UI/Dialogs/GenericConfirmDialog.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

namespace mu::ui::window
{

CGenericConfirmDialog* g_pGenericConfirmDialog = nullptr;

void CGenericConfirmDialog::Create(CManager* pMng)
{
    Release();

    if (RmlUiRuntime::Instance().IsCreated())
        BuildRmlUi();

    pMng->AddUIObj(mu::ui::window::INTERFACE_GENERIC_CONFIRM_DIALOG, this);
}

void CGenericConfirmDialog::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "generic_confirm_dialog",
        [this](Rml::DataModelConstructor& c, GenericDialogRmlModel& model)
        {
            auto line = c.RegisterStruct<LineEntry>();
            line.RegisterMember("text", &LineEntry::text);
            line.RegisterMember("bold", &LineEntry::bold);
            c.RegisterArray<std::vector<LineEntry>>();
            c.Bind("lines", &model.lines);

            c.Bind("show_cancel", &model.showCancel);
            c.Bind("primary_label", &model.primaryLabel);
            c.Bind("secondary_label", &model.secondaryLabel);

            c.BindEventCallback("gcd_primary_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bPrimaryClicked = true; });
            c.BindEventCallback("gcd_secondary_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bSecondaryClicked = true; });
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
            "Data/Interface/RmlUi/generic_confirm_dialog.rml");
}

void CGenericConfirmDialog::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    if (wasVisible)
    {
        SyncRmlModel();
        if (m_pRmlDoc)
            m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}

void CGenericConfirmDialog::Release()
{
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    m_bActive = false;
    m_Queue.clear();
}

void CGenericConfirmDialog::Show(GenericDialogConfig cfg)
{
    // Protects a pending dialog from being silently overwritten -- e.g. a network-pushed guild
    // invite arriving while a quest-giveup confirm is already open (see class comment).
    if (m_bActive)
    {
        m_Queue.push_back(std::move(cfg));
        return;
    }

    m_Active = std::move(cfg);
    m_bActive = true;
    m_bPrimaryClicked = false;
    m_bSecondaryClicked = false;

    if (m_pRmlDoc)
    {
        SyncRmlModel();
        // Modal: blocks the game world/other UI from stealing focus or clicks while this is open --
        // matches CMessageBoxMng's own input-blocking behavior for the system this replaces.
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}

void CGenericConfirmDialog::ShowNext()
{
    if (m_Queue.empty())
    {
        m_bActive = false;
        return;
    }

    m_Active = std::move(m_Queue.front());
    m_Queue.pop_front();
    m_bPrimaryClicked = false;
    m_bSecondaryClicked = false;

    if (m_pRmlDoc)
    {
        SyncRmlModel();
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}

void CGenericConfirmDialog::Resolve(bool primary)
{
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();

    // Move out before invoking -- the callback may itself call Show() (e.g. chaining a follow-up
    // confirm), which must not stomp m_Active while its own onPrimary/onSecondary still needs it.
    GenericDialogConfig cfg = std::move(m_Active);
    if (primary)
    {
        if (cfg.onPrimary) cfg.onPrimary();
    }
    else
    {
        if (cfg.onSecondary) cfg.onSecondary();
    }

    ShowNext();
}

bool CGenericConfirmDialog::Render()
{
    // RmlUi's #panel owns this dialog's entire visual -- nothing native to draw.
    SyncRmlModel();
    return true;
}

bool CGenericConfirmDialog::Update()
{
    if (!m_bActive)
        return true;

    if (m_bPrimaryClicked)
    {
        m_bPrimaryClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(true);
    }
    else if (m_bSecondaryClicked)
    {
        m_bSecondaryClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(false);
    }

    return true;
}

bool CGenericConfirmDialog::UpdateKeyEvent()
{
    if (!m_bActive)
        return true;

    if (mu::ui::window::IsPress(VK_RETURN))
    {
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(true);
    }
    else if (mu::ui::window::IsPress(VK_ESCAPE))
    {
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(m_Active.buttons == GenericDialogConfig::ButtonSet::Ok);
    }

    return !IsVisible();
}

void CGenericConfirmDialog::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    std::vector<LineEntry> newLines;
    newLines.reserve(m_Active.lines.size());
    for (const auto& line : m_Active.lines)
        newLines.push_back({ StringUtils::WideToNarrow(line.text.c_str()), line.bold });

    bool linesChanged = newLines.size() != model.lines.size();
    for (size_t i = 0; i < newLines.size() && !linesChanged; ++i)
        linesChanged = newLines[i].text != model.lines[i].text || newLines[i].bold != model.lines[i].bold;
    if (linesChanged)
    {
        model.lines = std::move(newLines);
        m_RmlBinder.MarkDirty("lines");
    }

    const bool showCancel = m_Active.buttons == GenericDialogConfig::ButtonSet::OkCancel;
    if (model.showCancel != showCancel)
    {
        model.showCancel = showCancel;
        m_RmlBinder.MarkDirty("show_cancel");
    }

    const std::string primaryLabel = StringUtils::WideToNarrow(m_Active.primaryLabel.c_str());
    if (model.primaryLabel != primaryLabel)
    {
        model.primaryLabel = primaryLabel;
        m_RmlBinder.MarkDirty("primary_label");
    }

    const std::string secondaryLabel = StringUtils::WideToNarrow(m_Active.secondaryLabel.c_str());
    if (model.secondaryLabel != secondaryLabel)
    {
        model.secondaryLabel = secondaryLabel;
        m_RmlBinder.MarkDirty("secondary_label");
    }
}

} // namespace mu::ui::window
