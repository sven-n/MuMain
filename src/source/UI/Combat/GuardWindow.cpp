
#include "stdafx.h"
#include "UI/Combat/GuardWindow.h"
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
#include "Guild/GuildTypes.h"
#include "GameLogic/Events/SiegeRegistration.h"

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
#include "Render/Text/CUIRenderText.h"

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
bool IsGuildMark(const ITEM* item)
{
    return item->Type == ITEM_POTION + 21 && item->Level == 3;
}

// Guild marks in the hero's main inventory.
DWORD CountGuildMarks()
{
    DWORD count = 0;
    CInventoryCtrl* inventory = g_pMyInventory->GetInventoryCtrl();
    for (int i = 0; i < (int)inventory->GetNumberOfItems(); ++i)
    {
        ITEM* item = inventory->GetItem(i);
        if (IsGuildMark(item))
            count += item->Durability;
    }
    return count;
}

int FindGuildMarkSlot()
{
    CInventoryCtrl* inventory = g_pMyInventory->GetInventoryCtrl();
    for (int i = 0; i < (int)inventory->GetNumberOfItems(); ++i)
    {
        ITEM* item = inventory->GetItem(i);
        if (IsGuildMark(item))
            return item->y * COLUMN_INVENTORY + item->x;
    }
    return -1;
}
} // namespace


CGuardWindow::CGuardWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iNumCurOpenTab = TAB_SIEGE_INFO;
}

CGuardWindow::~CGuardWindow()
{
    Release();
}

bool CGuardWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GUARDSMAN, this);

    SetPos(x, y);

    SetCurOpenTab(m_iNumCurOpenTab);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CGuardWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    if (RmlUiRuntime::Instance().IsCreated())
    {
        auto* context = RmlUiRuntime::Instance().GetContext();
        if (m_pRmlDoc)
            context->UnloadDocument(m_pRmlDoc);
        m_RmlBinder.Destroy(context);
    }
    m_pRmlDoc = nullptr;

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CGuardWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

// The page and the tab highlight read the same value.
void CGuardWindow::SetCurOpenTab(int iTab)
{
    m_iNumCurOpenTab = iTab;
}

bool CGuardWindow::UpdateMouseEvent()
{
    // Page keys use the active pane's theme-defined height.
    if (m_iNumCurOpenTab == TAB_REGISTER_INFO)
        UpdateRegisterInfoLists();

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

bool CGuardWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GUARDSMAN) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GUARDSMAN);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

bool CGuardWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const GUARD_BUTTON button = m_PendingButton;
    m_PendingButton = GUARD_BUTTON_NONE;
    if (IsVisible() && button == GUARD_BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GUARDSMAN);
    }
    else if (IsVisible() && button != GUARD_BUTTON_NONE)
    {
        if (m_iNumCurOpenTab == TAB_REGISTER)
            UpdateRegisterTab(button);
        else if (m_iNumCurOpenTab == TAB_REGISTER_INFO)
            UpdateRegisterInfoTab(button);
    }

    const int tab = m_PendingTab;
    m_PendingTab = -1;
    if (IsVisible() && tab >= TAB_SIEGE_INFO && tab <= TAB_REGISTER_INFO)
    {
        SetCurOpenTab(tab);
        if (tab == TAB_REGISTER_INFO)
        {
            if (m_eTimeType == CASTLESIEGE_STATE_REGSIEGE || m_eTimeType == CASTLESIEGE_STATE_REGMARK)
                SocketClient->ToGameServer()->SendCastleSiegeRegisteredGuildsListRequest();
            else if (m_eTimeType == CASTLESIEGE_STATE_NOTIFY || m_eTimeType == CASTLESIEGE_STATE_READYSIEGE)
                SocketClient->ToGameServer()->SendCastleOwnerListRequest();
        }
    }

    SyncRmlModel();
    return true;
}

bool CGuardWindow::Render()
{
    // Nothing native left: the frame, the tabs, the pages, the lists' lines and the buttons are
    // RmlUi. Kept because CObject requires the override.
    return true;
}

void CGuardWindow::OpeningProcess()
{
    SetCurOpenTab(TAB_SIEGE_INFO);

    SocketClient->ToGameServer()->SendCastleSiegeRegistrationStateRequest();
}

void CGuardWindow::ClosingProcess()
{
    m_GuildLists.ClearDeclarations();
    m_GuildLists.ClearSiegeGuilds();

    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CGuardWindow::GetLayerDepth()
{
    return 5.0f;
}

bool CGuardWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GUARDSMAN);

    return false;
}

bool CGuardWindow::ProclaimLocked() const
{
    // RenderRegisterTab(): the owner guild (or its alliance) cannot announce.
    return !wcscmp(GuildMark[Hero->GuildMarkIndex].UnionName, m_szOwnerGuild) ||
           !wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, m_szOwnerGuild);
}

void CGuardWindow::UpdateRegisterTab(GUARD_BUTTON button)
{
    switch (m_eTimeType)
    {
    case CASTLESIEGE_STATE_REGSIEGE:
        // The button exists (and is unlocked) only for a guild master not yet registered.
        if (button == GUARD_BUTTON_PROCLAIM && Hero->GuildStatus == G_MASTER && !g_SiegeRegistration.HasRegistered() &&
            !ProclaimLocked())
        {
            if (g_SiegeRegistration.IsSufficientDeclareLevel())
            {
                SocketClient->ToGameServer()->SendCastleSiegeRegistrationRequest();
            }
            else
            {
                mu::ui::window::GenericDialogConfig cfg;
                cfg.lines = {
                    { I18N::Game::YouHaveNoAbility, false },
                    { I18N::Game::ToAttackTheCastle, false },
                };
                mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
            }
        }
        break;
    case CASTLESIEGE_STATE_REGMARK:
        if (button == GUARD_BUTTON_REGISTER && g_SiegeRegistration.HasRegistered() && CountGuildMarks() > 0)
        {
            int nMarkSlot = FindGuildMarkSlot();
            if (nMarkSlot != -1)
            {
                SocketClient->ToGameServer()->SendCastleSiegeMarkRegistration(nMarkSlot);
            }
        }
        break;
    }
}

void CGuardWindow::UpdateRegisterInfoLists()
{
    if (!m_pRmlDoc)
        return;
    const int kind = m_RmlBinder.GetModel().listKind;
    if (kind == 0)
        return;
    auto* pane = m_pRmlDoc->GetElementById(kind == 1 ? "guard_declare_list" : "guard_siege_list");
    if (!pane)
        return;
    if (PressKey(VK_PRIOR))
        pane->SetScrollTop(pane->GetScrollTop() - pane->GetClientHeight());
    if (PressKey(VK_NEXT))
        pane->SetScrollTop(pane->GetScrollTop() + pane->GetClientHeight());
}

void CGuardWindow::UpdateRegisterInfoTab(GUARD_BUTTON button)
{
    if (button == GUARD_BUTTON_GIVE_UP && g_SiegeRegistration.HasRegistered() && CASTLESIEGE_STATE_REGSIEGE <= m_eTimeType &&
        m_eTimeType <= CASTLESIEGE_STATE_REGMARK && Hero->GuildStatus == G_MASTER)
    {
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines.push_back({I18N::Game::AreYouReallyWantToQuitTheSiegeWargare, false});
        cfg.onPrimary = [] { SocketClient->ToGameServer()->SendCastleSiegeUnregisterRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

void CGuardWindow::SetData(const UI::Siege::GuardStatus& status)
{
    std::fill(std::begin(m_szOwnerGuild), std::end(m_szOwnerGuild), L'\0');
    std::fill(std::begin(m_szOwnerGuildMaster), std::end(m_szOwnerGuildMaster), L'\0');
    std::copy_n(status.ownerGuild.begin(),
                std::min(status.ownerGuild.size(), std::size(m_szOwnerGuild) - 1), m_szOwnerGuild);
    std::copy_n(status.ownerGuildMaster.begin(),
                std::min(status.ownerGuildMaster.size(), std::size(m_szOwnerGuildMaster) - 1), m_szOwnerGuildMaster);

    m_eTimeType = status.phase;
    m_wStartYear = status.registrationStart.year;
    m_byStartMonth = status.registrationStart.month;
    m_byStartDay = status.registrationStart.day;
    m_byStartHour = status.registrationStart.hour;
    m_byStartMinute = status.registrationStart.minute;
    m_wEndYear = status.registrationEnd.year;
    m_byEndMonth = status.registrationEnd.month;
    m_byEndDay = status.registrationEnd.day;
    m_byEndHour = status.registrationEnd.hour;
    m_byEndMinute = status.registrationEnd.minute;
    m_wSiegeStartYear = status.battleStart.year;
    m_bySiegeStartMonth = status.battleStart.month;
    m_bySiegeStartDay = status.battleStart.day;
    m_bySiegeStartHour = status.battleStart.hour;
    m_bySiegeStartMinute = status.battleStart.minute;
    m_dwStateLeftSec = status.secondsRemaining;
}

void CGuardWindow::AddDeclareGuildList(std::wstring_view name, int markCount, bool gaveUp, BYTE sequence)
{
    if (name.empty())
        return;
    m_GuildLists.AddDeclaration({std::wstring(name.substr(0, MAX_GUILDNAME)), markCount, gaveUp, sequence});
}

void CGuardWindow::ClearDeclareGuildList()
{
    m_GuildLists.ClearDeclarations();
}

void CGuardWindow::SortDeclareGuildList()
{
    m_GuildLists.SortDeclarations();
}

void CGuardWindow::AddGuildList(std::wstring_view name, BYTE side, BYTE involvement, int score)
{
    if (name.empty())
        return;
    m_GuildLists.AddSiegeGuild({std::wstring(name.substr(0, MAX_GUILDNAME)), side, involvement, score});
}

void CGuardWindow::ClearGuildList()
{
    m_GuildLists.ClearSiegeGuilds();
}

void CGuardWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "guard_window",
        [this](Rml::DataModelConstructor& c, GuardWindowRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            c.Bind("button_label_top", &model.buttonLabelTop);
            c.Bind("tab_label_top", &model.tabLabelTop);
            auto tab = c.RegisterStruct<GuardTabEntry>();
            tab.RegisterMember("label", &GuardTabEntry::label);
            tab.RegisterMember("selected", &GuardTabEntry::selected);
            c.RegisterArray<std::vector<GuardTabEntry>>();
            c.Bind("tabs", &model.tabs);
            c.Bind("active_tab", &model.activeTab);
            c.BindEventCallback("guard_tab",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                                {
                                    if (args.size() == 1)
                                        m_PendingTab = args[0].Get<int>(-1);
                                });
            auto lineType = c.RegisterStruct<GuardLine>();
            lineType.RegisterMember("text", &GuardLine::text);
            lineType.RegisterMember("text_px", &GuardLine::textPx);
            c.Bind("title", &model.title);
            c.Bind("owner_master", &model.ownerMaster);
            c.Bind("owner_guild", &model.ownerGuild);
            c.Bind("status_start", &model.statusStart);
            c.Bind("status_end", &model.statusEnd);
            c.Bind("status_period", &model.statusPeriod);
            c.Bind("status_expected_label", &model.statusExpectedLabel);
            c.Bind("status_expected_time", &model.statusExpectedTime);
            c.Bind("status_next_stage", &model.statusNextStage);
            c.Bind("register_message", &model.registerMessage);
            c.Bind("register_message2", &model.registerMessage2);
            c.Bind("register_acquired", &model.registerAcquired);
            c.Bind("register_registered", &model.registerRegistered);
            c.Bind("register_bold", &model.registerBold);
            c.Bind("list_message", &model.listMessage);
            auto actionButton = c.RegisterStruct<GuardActionButton>();
            actionButton.RegisterMember("label", &GuardActionButton::label);
            actionButton.RegisterMember("shown", &GuardActionButton::shown);
            actionButton.RegisterMember("locked", &GuardActionButton::locked);
            c.Bind("proclaim_button", &model.proclaimButton);
            c.Bind("register_button", &model.registerButton);
            c.Bind("give_up_button", &model.giveUpButton);
            c.Bind("list_kind", &model.listKind);
            auto declareRow = c.RegisterStruct<GuardDeclareRow>();
            declareRow.RegisterMember("name", &GuardDeclareRow::name);
            declareRow.RegisterMember("mark_count", &GuardDeclareRow::markCount);
            declareRow.RegisterMember("state", &GuardDeclareRow::state);
            declareRow.RegisterMember("order", &GuardDeclareRow::order);
            declareRow.RegisterMember("selected", &GuardDeclareRow::selected);
            c.RegisterArray<std::vector<GuardDeclareRow>>();
            c.Bind("declare_rows", &model.declareRows);
            auto siegeRow = c.RegisterStruct<GuardSiegeRow>();
            siegeRow.RegisterMember("name", &GuardSiegeRow::name);
            siegeRow.RegisterMember("side", &GuardSiegeRow::side);
            siegeRow.RegisterMember("involvement", &GuardSiegeRow::involvement);
            siegeRow.RegisterMember("selected", &GuardSiegeRow::selected);
            siegeRow.RegisterMember("defending", &GuardSiegeRow::defending);
            c.RegisterArray<std::vector<GuardSiegeRow>>();
            c.Bind("siege_rows", &model.siegeRows);
            c.Bind("header_name", &model.headerName);
            c.Bind("header_mark_count", &model.headerMarkCount);
            c.Bind("header_state", &model.headerState);
            c.Bind("header_order", &model.headerOrder);
            c.Bind("header_side", &model.headerSide);
            c.Bind("header_involvement", &model.headerInvolvement);
            c.Bind("score_label", &model.scoreLabel);
            c.Bind("score_value", &model.scoreValue);
            c.BindEventCallback("guard_declare_select",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        SelectListGuild(true, args[0].Get<Rml::String>());
                });
            c.BindEventCallback("guard_siege_select",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.size() == 1)
                        SelectListGuild(false, args[0].Get<Rml::String>());
                });
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.BindEventCallback("guard_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingButton = static_cast<GUARD_BUTTON>(arguments[0].Get<int>(-1));
                                });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/guard_window.rml");
}

void CGuardWindow::ReloadRmlTheme()
{
    m_ListsDirty = true;
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CGuardWindow::SyncRmlModel()
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

void CGuardWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    const float x0 = static_cast<float>(m_Pos.x);
    const float y0 = static_cast<float>(m_Pos.y);

    // One of the window's own lines: the document places it, so only what it says and the size
    // the native renderer would have shrunk it to for its box travel through the model.
    auto line = [&](const wchar_t* text, bool boldFont, float boxWidth) -> GuardLine
    {
        if (text == nullptr || text[0] == L'\0')
            return {};
        g_pRenderText->SetFont(boldFont ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = boldFont ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        return {StringUtils::WideToNarrow(text),
                UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), boxWidth)};
    };
    // A page's line: the original's 190-unit centring box.
    auto pageLine = [&](const wchar_t* text, bool boldFont = false) { return line(text, boldFont, 190.f); };

    wchar_t szText[256] = {};

    // RenderFrame(): the heading and the owner lines, in the bold font.
    GuardLine title = line(I18N::Game::GuardNPC, true, 160.f);
    mu_swprintf(szText, I18N::Game::OfficialSealOfKingS,
                m_szOwnerGuildMaster[0] ? m_szOwnerGuildMaster : I18N::Game::None);
    GuardLine ownerMaster = pageLine(szText, true);
    mu_swprintf(szText, I18N::Game::AffiliatedGuildS, m_szOwnerGuild[0] ? m_szOwnerGuild : I18N::Game::None);
    GuardLine ownerGuild = pageLine(szText, true);

    // The tabs: the second one's label follows the period. (The original pushed the labels into a
    // static list every frame and so kept the first frame's for the whole session.)
    const wchar_t* tabLabels[] = {
        I18N::Game::Status, m_eTimeType == CASTLESIEGE_STATE_REGSIEGE ? I18N::Game::Announce : I18N::Game::Register,
        I18N::Game::List};
    std::vector<GuardTabEntry> tabs;
    for (int i = 0; i < 3; ++i)
        tabs.push_back({StringUtils::WideToNarrow(tabLabels[i]), i == m_iNumCurOpenTab});

    // Only the page on screen is filled; the theme hides the other two, so their lines keep
    // whatever they last said.
    GuardLine statusStart, statusEnd, statusPeriod;
    GuardLine statusExpectedLabel, statusExpectedTime, statusNextStage;
    GuardLine registerMessage, registerMessage2, registerAcquired, registerRegistered;
    GuardLine listMessage;
    // The original drew the "period has ended" and truce states in the bold font and every other
    // state's line in the normal one; no rule behind that was recoverable.
    const bool registerBold =
        m_eTimeType == CASTLESIEGE_STATE_IDLE_3 || m_eTimeType == CASTLESIEGE_STATE_ENDSIEGE;
    GuardActionButton proclaimButton, registerButton, giveUpButton;

    int listKind = 0;
    switch (m_iNumCurOpenTab)
    {
    case TAB_SIEGE_INFO:
    {
        // RenderSeigeInfoTab().
        wchar_t szTemp[256] = {};
        mu_swprintf(szTemp, I18N::Game::StartingUUUUU, m_wStartYear, m_byStartMonth, m_byStartDay, m_byStartHour,
                    m_byStartMinute);
        statusStart = pageLine(szTemp);
        mu_swprintf(szTemp, I18N::Game::UntillUUUUU, m_wEndYear, m_byEndMonth, m_byEndDay, m_byEndHour, m_byEndMinute);
        statusEnd = pageLine(szTemp);
        const wchar_t* period = nullptr;
        switch (m_eTimeType)
        {
        case CASTLESIEGE_STATE_NONE:
        case CASTLESIEGE_STATE_IDLE_1:
            period = I18N::Game::SiegePeriodIsOver;
            break;
        case CASTLESIEGE_STATE_REGSIEGE:
            period = I18N::Game::SiegeRegistrationPeriod;
            break;
        case CASTLESIEGE_STATE_IDLE_2:
            period = I18N::Game::StandbyPeriodForSignRegistration;
            break;
        case CASTLESIEGE_STATE_REGMARK:
            period = I18N::Game::PeriodForSignRegistration;
            break;
        case CASTLESIEGE_STATE_IDLE_3:
            period = I18N::Game::StandbyPeriodForAnnouncement;
            break;
        case CASTLESIEGE_STATE_NOTIFY:
            period = I18N::Game::AnnouncementPeriod;
            break;
        case CASTLESIEGE_STATE_READYSIEGE:
            period = I18N::Game::SiegePreparationPeriod;
            break;
        case CASTLESIEGE_STATE_STARTSIEGE:
            period = I18N::Game::SiegePeriod;
            break;
        case CASTLESIEGE_STATE_ENDSIEGE:
            period = I18N::Game::TrucePeriod;
            break;
        case CASTLESIEGE_STATE_ENDCYCLE:
            period = I18N::Game::SiegeIsOver;
            break;
        default:
            break;
        }
        statusPeriod = pageLine(period);
        if (m_eTimeType < CASTLESIEGE_STATE_STARTSIEGE)
        {
            statusExpectedLabel = pageLine(I18N::Game::ExpectedSiegePeriodIs);
            mu_swprintf(szTemp, I18N::Game::UUUUU, m_wSiegeStartYear, m_bySiegeStartMonth, m_bySiegeStartDay,
                        m_bySiegeStartHour, m_bySiegeStartMinute);
            statusExpectedTime = pageLine(szTemp);
            mu_swprintf(szTemp, I18N::Game::UUURemainedForTheNextStage, m_dwStateLeftSec / 3600,
                        (m_dwStateLeftSec % 3600) / 60, (m_dwStateLeftSec % 3600) % 60);
            statusNextStage = pageLine(szTemp);
        }
        break;
    }
    case TAB_REGISTER:
    {
        // RenderRegisterTab().
        switch (m_eTimeType)
        {
        case CASTLESIEGE_STATE_NONE:
        case CASTLESIEGE_STATE_IDLE_1:
            registerMessage = pageLine(I18N::Game::SiegePeriodIsOver);
            break;
        case CASTLESIEGE_STATE_REGSIEGE:
            if (Hero->GuildStatus == G_MASTER)
            {
                if (!g_SiegeRegistration.HasRegistered())
                    proclaimButton = {StringUtils::WideToNarrow(I18N::Game::Announce), true, ProclaimLocked()};
                else
                    registerMessage = pageLine(I18N::Game::Announced);
            }
            else
            {
                registerMessage = pageLine(I18N::Game::NotAGuildMaster);
            }
            break;
        case CASTLESIEGE_STATE_IDLE_2:
            registerMessage = pageLine(I18N::Game::StandbyPeriodForSignRegistration);
            break;
        case CASTLESIEGE_STATE_REGMARK:
            if (g_SiegeRegistration.HasRegistered())
            {
                registerMessage = pageLine(I18N::Game::RegisterTheAcquiredSign);
                const int nMarkCount = CountGuildMarks();
                mu_swprintf(szText, I18N::Game::AcquiredNoOfSignU, nMarkCount);
                registerAcquired = pageLine(szText);
                mu_swprintf(szText, I18N::Game::RegisteredNoOfSignU, g_SiegeRegistration.GetRegMarkCount());
                registerRegistered = pageLine(szText);
                registerButton = {StringUtils::WideToNarrow(I18N::Game::Register), true, nMarkCount <= 0};
            }
            else
            {
                registerMessage = pageLine(I18N::Game::ThisGuildIsNotRegisteredInCastleSiege);
            }
            break;
        case CASTLESIEGE_STATE_IDLE_3:
            registerMessage = pageLine(I18N::Game::AnnouncementAndRegistrationPeriod, true);
            registerMessage2 = pageLine(I18N::Game::HasEnded, true);
            break;
        case CASTLESIEGE_STATE_NOTIFY:
            registerMessage = pageLine(I18N::Game::AnnouncementPeriod);
            break;
        case CASTLESIEGE_STATE_READYSIEGE:
            registerMessage = pageLine(I18N::Game::SiegePreparationPeriod);
            break;
        case CASTLESIEGE_STATE_STARTSIEGE:
            registerMessage = pageLine(I18N::Game::SiegePeriod);
            break;
        case CASTLESIEGE_STATE_ENDSIEGE:
            registerMessage = pageLine(I18N::Game::TrucePeriod, true);
            break;
        case CASTLESIEGE_STATE_ENDCYCLE:
            registerMessage = pageLine(I18N::Game::SiegeIsOver);
            break;
        default:
            break;
        }
        break;
    }
    case TAB_REGISTER_INFO:
    {
        if (m_eTimeType == CASTLESIEGE_STATE_REGSIEGE || m_eTimeType == CASTLESIEGE_STATE_REGMARK)
            listKind = 1;
        else if (m_eTimeType == CASTLESIEGE_STATE_NOTIFY || m_eTimeType == CASTLESIEGE_STATE_READYSIEGE)
            listKind = 2;
        else if (m_eTimeType == CASTLESIEGE_STATE_ENDSIEGE)
        {
            listMessage = pageLine(I18N::Game::TrucePeriod, true);
        }

        if (g_SiegeRegistration.HasRegistered() && CASTLESIEGE_STATE_REGSIEGE <= m_eTimeType &&
            m_eTimeType <= CASTLESIEGE_STATE_REGMARK && Hero->GuildStatus == G_MASTER)
            giveUpButton = {StringUtils::WideToNarrow(I18N::Game::AbandonCastleSiege), true, false};
        break;
    }
    default:
        break;
    }

    GuardWindowRmlModel& model = m_RmlBinder.GetModel();
    const bool sameTabs = model.tabs.size() == tabs.size() &&
                          std::equal(model.tabs.begin(), model.tabs.end(), tabs.begin(),
                                     [](const GuardTabEntry& a, const GuardTabEntry& b)
                                     { return a.label == b.label && a.selected == b.selected; });
    if (!sameTabs)
    {
        model.tabs = std::move(tabs);
        m_RmlBinder.MarkDirty("tabs");
    }


    SyncField(m_RmlBinder, &GuardWindowRmlModel::activeTab, "active_tab", m_iNumCurOpenTab);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::title, "title", std::move(title));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::ownerMaster, "owner_master", std::move(ownerMaster));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::ownerGuild, "owner_guild", std::move(ownerGuild));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::statusStart, "status_start", std::move(statusStart));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::statusEnd, "status_end", std::move(statusEnd));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::statusPeriod, "status_period", std::move(statusPeriod));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::statusExpectedLabel, "status_expected_label",
              std::move(statusExpectedLabel));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::statusExpectedTime, "status_expected_time",
              std::move(statusExpectedTime));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::statusNextStage, "status_next_stage", std::move(statusNextStage));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::registerMessage, "register_message", std::move(registerMessage));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::registerMessage2, "register_message2", std::move(registerMessage2));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::registerAcquired, "register_acquired", std::move(registerAcquired));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::registerRegistered, "register_registered",
              std::move(registerRegistered));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::registerBold, "register_bold", registerBold);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::listMessage, "list_message", std::move(listMessage));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::proclaimButton, "proclaim_button", std::move(proclaimButton));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::registerButton, "register_button", std::move(registerButton));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::giveUpButton, "give_up_button", std::move(giveUpButton));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::listKind, "list_kind", listKind);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::headerName, "header_name",
              StringUtils::WideToNarrow(I18N::Game::NAME));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::headerMarkCount, "header_mark_count",
              StringUtils::WideToNarrow(I18N::Game::NoReg));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::headerState, "header_state",
              StringUtils::WideToNarrow(I18N::Game::Stat));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::headerOrder, "header_order",
              StringUtils::WideToNarrow(I18N::Game::Order));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::headerSide, "header_side",
              StringUtils::WideToNarrow(I18N::Game::Camp));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::headerInvolvement, "header_involvement",
              StringUtils::WideToNarrow(I18N::Game::Maintain));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::scoreLabel, "score_label",
              StringUtils::WideToNarrow(I18N::Game::Score));
    SyncGuildLists();
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::buttonLabelTop, "button_label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::tabLabelTop, "tab_label_top", static_cast<float>(22 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}

void CGuardWindow::SelectListGuild(bool declaration, const Rml::String& name)
{
    if (!IsVisible() || m_iNumCurOpenTab != TAB_REGISTER_INFO)
        return;
    const auto selectedName = StringUtils::NarrowToWide(name);
    const int kind = m_RmlBinder.GetModel().listKind;
    if (declaration && kind == 1 && (selectedName == m_ListGuild || selectedName == m_ListAlliance))
        m_GuildLists.SelectDeclaration(selectedName);
    else if (!declaration && kind == 2)
        m_GuildLists.SelectSiegeGuild(selectedName);
}

std::vector<GuardDeclareRow> CGuardWindow::BuildDeclareRows() const
{
    std::vector<GuardDeclareRow> rows;
    for (const auto& entry : m_GuildLists.Declarations())
    {
        if (!UI::Combat::GuardGuildLists::IsOwnDeclaration(entry, m_ListGuild, m_ListAlliance))
            continue;
        rows.push_back({StringUtils::WideToNarrow(entry.name.c_str()), std::to_string(entry.markCount),
                       StringUtils::WideToNarrow(entry.gaveUp ? I18N::Game::Failed : I18N::Game::Processing),
                       std::to_string(entry.order), entry.name == m_GuildLists.SelectedDeclaration()});
    }
    return rows;
}

std::vector<GuardSiegeRow> CGuardWindow::BuildSiegeRows() const
{
    std::vector<GuardSiegeRow> rows;
    for (const auto& entry : m_GuildLists.SiegeGuilds())
    {
        rows.push_back({StringUtils::WideToNarrow(entry.name.c_str()),
                       StringUtils::WideToNarrow(entry.joinSide == 1 ? I18N::Game::DefendingTeam : I18N::Game::InvadingTeam),
                       StringUtils::WideToNarrow(entry.involvement == 1 ? I18N::Game::Maintain : I18N::Game::Assist),
                       entry.name == m_GuildLists.SelectedSiegeGuild(), entry.joinSide == 1});
    }
    return rows;
}

void CGuardWindow::SyncGuildLists()
{
    const int markIndex = Hero->GuildMarkIndex;
    const bool hasGuild = markIndex >= 0 && markIndex < MAX_MARKS;
    const std::wstring_view guild = hasGuild ? GuildMark[markIndex].GuildName : L"";
    const std::wstring_view alliance = hasGuild ? GuildMark[markIndex].UnionName : L"";
    if (!m_ListsDirty && m_ListRevision == m_GuildLists.Revision() &&
        m_ListGuild == guild && m_ListAlliance == alliance)
        return;
    m_ListRevision = m_GuildLists.Revision();
    m_ListsDirty = false;
    m_ListGuild = guild;
    m_ListAlliance = alliance;
    SyncField(m_RmlBinder, &GuardWindowRmlModel::declareRows, "declare_rows", BuildDeclareRows());
    SyncField(m_RmlBinder, &GuardWindowRmlModel::siegeRows, "siege_rows", BuildSiegeRows());
    Rml::String score;
    if (const auto* entry = m_GuildLists.ScoreGuild())
    {
        if (entry->joinSide == 1)
            score = "--";
        else
            score = StringUtils::WideToNarrow(entry->name.c_str()) + " :     " + std::to_string(entry->score);
    }
    SyncField(m_RmlBinder, &GuardWindowRmlModel::scoreValue, "score_value", std::move(score));
}
