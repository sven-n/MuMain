
#include "stdafx.h"
#include "UI/NPCs/GatemanWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NPCs/UIGateKeeper.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

extern CUIGateKeeper* g_pUIGateKeeper;

using namespace SEASON3B;
using namespace mu::ui::window;

CGatemanWindow::CGatemanWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CGatemanWindow::~CGatemanWindow()
{
    Release();
}

bool CGatemanWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GATEKEEPER, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CGatemanWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CGatemanWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CGatemanWindow::UpdateMouseEvent()
{
    // The guild master's public toggle stays a native hit test (the document leaves that area to
    // the pointer); the buttons are RmlUi's (see Update()).
    if (g_pUIGateKeeper->GetType() == TOUCH_TYPE_GUILD_MASTER)
    {
        const POINT ptOrigin = {m_Pos.x, m_Pos.y + 50};
        if (mu::ui::window::IsPress(VK_LBUTTON) &&
            mu::ui::window::CheckMouseIn(ptOrigin.x + 35, ptOrigin.y + 60, 100, 16))
            g_pUIGateKeeper->SendPublicSetting();
    }

    if (true == BtnProcess())
        return false;

    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                      static_cast<int>(panelHeight))
            .Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CGatemanWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GATEKEEPER) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CGatemanWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const GATEMAN_BUTTON button = m_PendingButton;
    m_PendingButton = GATEMAN_BUTTON_NONE;
    if (IsVisible() && button == GATEMAN_BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
    }
    else if (IsVisible() && button != GATEMAN_BUTTON_NONE)
    {
        switch (g_pUIGateKeeper->GetType())
        {
        case TOUCH_TYPE_PERSON:
            UpdateGuestMode(button);
            break;
        case TOUCH_TYPE_GUILD_STAFF:
            UpdateGuildMemeberMode(button);
            break;
        case TOUCH_TYPE_GUILD_MASTER:
            UpdateGuildMasterMode(button);
            break;
        }
    }

    SyncRmlModel();
    return true;
}

bool CGatemanWindow::Render()
{
    // Nothing native left: the frame, the page and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

void CGatemanWindow::OpeningProcess()
{
}

void CGatemanWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CGatemanWindow::GetLayerDepth()
{
    return 5.0f;
}

bool CGatemanWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GATEKEEPER);

    return false;
}

void CGatemanWindow::UpdateGuildMasterMode(GATEMAN_BUTTON button)
{
    if (button == GATEMAN_BUTTON_SET)
    {
        g_pUIGateKeeper->SendEnteranceFee();
    }
    else if (button == GATEMAN_BUTTON_FEE_UP)
    {
        g_pUIGateKeeper->EnteranceFeeUp();
    }
    else if (button == GATEMAN_BUTTON_FEE_DOWN)
    {
        g_pUIGateKeeper->EnteranceFeeDown();
    }
    else if (button == GATEMAN_BUTTON_ENTER)
    {
        g_pUIGateKeeper->SendEnter();
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
    }
}

void CGatemanWindow::UpdateGuildMemeberMode(GATEMAN_BUTTON button)
{
    if (button == GATEMAN_BUTTON_ENTER)
    {
        g_pUIGateKeeper->SendEnter();
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
    }
}

void CGatemanWindow::UpdateGuestMode(GATEMAN_BUTTON button)
{
    // Enter is locked (no click) while the castle is closed to guests.
    if (button == GATEMAN_BUTTON_ENTER && g_pUIGateKeeper->IsPublic())
    {
        if ((int)CharacterMachine->Gold >= g_pUIGateKeeper->GetEnteranceFee())
        {
            g_pUIGateKeeper->SendEnter();
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
        }
        else
        {
            mu::ui::window::GenericDialogConfig cfg;
            cfg.lines = {
                { I18N::Game::EnteringIsNotAllowed, false },
                { I18N::Game::InsufficientZenForEntering, false },
            };
            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
        }
    }
}

void CGatemanWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "gateman",
        [this](Rml::DataModelConstructor& c, GatemanRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            c.Bind("label_top", &model.labelTop);
            auto lineType = c.RegisterStruct<GatemanLine>();
            lineType.RegisterMember("text", &GatemanLine::text);
            lineType.RegisterMember("text_px", &GatemanLine::textPx);
            c.Bind("title", &model.title);
            c.Bind("restriction", &model.restriction);
            c.Bind("members_line1", &model.membersLine1);
            c.Bind("members_line2", &model.membersLine2);
            c.Bind("members_line3", &model.membersLine3);
            c.Bind("open_to_non_members", &model.openToNonMembers);
            c.Bind("entrance_fee", &model.entranceFee);
            c.Bind("fee_setting_label", &model.feeSettingLabel);
            c.Bind("view_fee", &model.viewFee);
            c.Bind("fee_range", &model.feeRange);
            c.Bind("fee_range_for", &model.feeRangeFor);
            c.Bind("fee_increment", &model.feeIncrement);
            c.Bind("member_question", &model.memberQuestion);
            c.Bind("guest_fee", &model.guestFee);
            c.Bind("guest_pay_prompt", &model.guestPayPrompt);
            c.Bind("guest_question", &model.guestQuestion);
            c.Bind("denied_line1", &model.deniedLine1);
            c.Bind("denied_line2", &model.deniedLine2);
            c.Bind("denied_line3", &model.deniedLine3);
            c.Bind("denied_line4", &model.deniedLine4);
            auto actionButton = c.RegisterStruct<GatemanActionButton>();
            actionButton.RegisterMember("label", &GatemanActionButton::label);
            actionButton.RegisterMember("shown", &GatemanActionButton::shown);
            actionButton.RegisterMember("locked", &GatemanActionButton::locked);
            c.Bind("confirm_button", &model.confirmButton);
            c.Bind("enter_button", &model.enterButton);
            c.Bind("page", &model.page);
            c.Bind("is_public", &model.isPublic);
            c.Bind("guest_can_afford", &model.guestCanAfford);
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.BindEventCallback("gateman_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingButton = static_cast<GATEMAN_BUTTON>(arguments[0].Get<int>(-1));
                                });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc =
        UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/gateman.rml");
}

void CGatemanWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CGatemanWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncContent();
}

void CGatemanWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();

    // One of the window's own lines: the document places it, so only what it says and the size the
    // native renderer would have shrunk it to for its box travel through the model.
    auto line = [&](const wchar_t* text, bool boldFont, float boxWidth) -> GatemanLine
    {
        if (text == nullptr || text[0] == L'\0')
            return {};
        g_pRenderText->SetFont(boldFont ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = boldFont ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            boxWidth > 0.f
                ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), boxWidth)
                : UI::Scaling::NativeTextPixelSize(role, transform);
        return {StringUtils::WideToNarrow(text), px};
    };
    // A page's line: the original's 190-unit centring box.
    auto pageLine = [&](const wchar_t* text, bool boldFont) { return line(text, boldFont, 190.f); };

    // RenderFrame(): the title, in the bold font, which the page then inherited.
    GatemanLine title = line(I18N::Game::GuardNPC, true, 160.f);

    GatemanLine restriction, membersLine1, membersLine2, membersLine3;
    GatemanLine openToNonMembers, entranceFee, feeSettingLabel, viewFee;
    GatemanLine feeRange, feeRangeFor, feeIncrement;
    GatemanLine memberQuestion;
    GatemanLine guestFee, guestPayPrompt, guestQuestion;
    GatemanLine deniedLine1, deniedLine2, deniedLine3, deniedLine4;
    GatemanActionButton confirmButton, enterButton;
    bool guestCanAfford = true;

    wchar_t szText[256] = {};
    wchar_t szGold[64] = {};
    const BYTE type = g_pUIGateKeeper->GetType();
    int page = 0;
    if (type == TOUCH_TYPE_GUILD_MASTER)
    {
        // RenderGuildMasterMode().
        page = 1;
        restriction = pageLine(I18N::Game::EntranceRestriction, true);
        membersLine1 = pageLine(I18N::Game::OnlyTheGuildMembers, false);
        membersLine2 = pageLine(I18N::Game::AreAllowedToEnter, false);
        membersLine3 = pageLine(I18N::Game::IsAllowed, false);
        openToNonMembers = line(I18N::Game::OpenItToNonMembers, false, 0.f);
        ConvertGold(g_pUIGateKeeper->GetEnteranceFee(), szGold);
        mu_swprintf(szText, I18N::Game::EntranceFeeSZen, szGold);
        entranceFee = line(szText, false, 0.f);
        feeSettingLabel = pageLine(I18N::Game::EntranceFeeSetting, true);
        ConvertGold(g_pUIGateKeeper->GetViewEnteranceFee(), szGold);
        mu_swprintf(szText, L"%ls %ls", szGold, I18N::Game::Zen);
        viewFee = line(szText, false, 200.f);
        confirmButton = {StringUtils::WideToNarrow(I18N::Game::Confirm), true, false};
        ConvertGold(g_pUIGateKeeper->GetMaxEnteranceFee(), szGold);
        mu_swprintf(szText, I18N::Game::EntranceFeeRange0SZen, szGold);
        feeRange = pageLine(szText, false);
        feeRangeFor = pageLine(I18N::Game::ForSetting, false);
        ConvertGold(g_pUIGateKeeper->GetAddEnteranceFee(), szGold);
        mu_swprintf(szText, I18N::Game::IncreaseUnitSZen, szGold);
        feeIncrement = pageLine(szText, false);
        enterButton = {StringUtils::WideToNarrow(I18N::Game::Enter), true, false};
    }
    else if (type == TOUCH_TYPE_GUILD_STAFF)
    {
        // RenderGuildMemberMode(): the title's bold font was still set.
        page = 2;
        memberQuestion = pageLine(I18N::Game::WouldYouLikeToEnter, true);
        enterButton = {StringUtils::WideToNarrow(I18N::Game::Enter), true, false};
    }
    else if (type == TOUCH_TYPE_PERSON)
    {
        // RenderGuestMode(): likewise still bold.
        page = 3;
        if (g_pUIGateKeeper->IsPublic())
        {
            ConvertGold(g_pUIGateKeeper->GetEnteranceFee(), szGold);
            mu_swprintf(szText, I18N::Game::EntranceFeeSzen, szGold);
            guestCanAfford = g_pUIGateKeeper->GetEnteranceFee() <= (int)CharacterMachine->Gold;
            guestFee = pageLine(szText, true);
            guestPayPrompt = pageLine(I18N::Game::PayEntranceFeeToEnter, true);
            guestQuestion = pageLine(I18N::Game::WouldYouLikeToEnter, true);
        }
        else
        {
            deniedLine1 = pageLine(I18N::Game::EnteringIsNotAllowed, true);
            deniedLine2 = pageLine(I18N::Game::ApprovalFromTheLordOfACastleIsRequired, true);
            deniedLine3 = pageLine(I18N::Game::ForEntering, true);
            deniedLine4 = pageLine(I18N::Game::PleaseGoBack, true);
        }
        enterButton = {StringUtils::WideToNarrow(I18N::Game::Enter), true,
                       g_pUIGateKeeper->IsPublic() == FALSE};
    }

    SyncField(m_RmlBinder, &GatemanRmlModel::title, "title", std::move(title));
    SyncField(m_RmlBinder, &GatemanRmlModel::restriction, "restriction", std::move(restriction));
    SyncField(m_RmlBinder, &GatemanRmlModel::membersLine1, "members_line1", std::move(membersLine1));
    SyncField(m_RmlBinder, &GatemanRmlModel::membersLine2, "members_line2", std::move(membersLine2));
    SyncField(m_RmlBinder, &GatemanRmlModel::membersLine3, "members_line3", std::move(membersLine3));
    SyncField(m_RmlBinder, &GatemanRmlModel::openToNonMembers, "open_to_non_members", std::move(openToNonMembers));
    SyncField(m_RmlBinder, &GatemanRmlModel::entranceFee, "entrance_fee", std::move(entranceFee));
    SyncField(m_RmlBinder, &GatemanRmlModel::feeSettingLabel, "fee_setting_label", std::move(feeSettingLabel));
    SyncField(m_RmlBinder, &GatemanRmlModel::viewFee, "view_fee", std::move(viewFee));
    SyncField(m_RmlBinder, &GatemanRmlModel::feeRange, "fee_range", std::move(feeRange));
    SyncField(m_RmlBinder, &GatemanRmlModel::feeRangeFor, "fee_range_for", std::move(feeRangeFor));
    SyncField(m_RmlBinder, &GatemanRmlModel::feeIncrement, "fee_increment", std::move(feeIncrement));
    SyncField(m_RmlBinder, &GatemanRmlModel::memberQuestion, "member_question", std::move(memberQuestion));
    SyncField(m_RmlBinder, &GatemanRmlModel::guestFee, "guest_fee", std::move(guestFee));
    SyncField(m_RmlBinder, &GatemanRmlModel::guestPayPrompt, "guest_pay_prompt", std::move(guestPayPrompt));
    SyncField(m_RmlBinder, &GatemanRmlModel::guestQuestion, "guest_question", std::move(guestQuestion));
    SyncField(m_RmlBinder, &GatemanRmlModel::deniedLine1, "denied_line1", std::move(deniedLine1));
    SyncField(m_RmlBinder, &GatemanRmlModel::deniedLine2, "denied_line2", std::move(deniedLine2));
    SyncField(m_RmlBinder, &GatemanRmlModel::deniedLine3, "denied_line3", std::move(deniedLine3));
    SyncField(m_RmlBinder, &GatemanRmlModel::deniedLine4, "denied_line4", std::move(deniedLine4));
    SyncField(m_RmlBinder, &GatemanRmlModel::confirmButton, "confirm_button", std::move(confirmButton));
    SyncField(m_RmlBinder, &GatemanRmlModel::enterButton, "enter_button", std::move(enterButton));
    SyncField(m_RmlBinder, &GatemanRmlModel::page, "page", page);
    SyncField(m_RmlBinder, &GatemanRmlModel::isPublic, "is_public", g_pUIGateKeeper->IsPublic() == TRUE);
    SyncField(m_RmlBinder, &GatemanRmlModel::guestCanAfford, "guest_can_afford", guestCanAfford);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlBinder, &GatemanRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    SyncField(m_RmlBinder, &GatemanRmlModel::labelTop, "label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &GatemanRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
