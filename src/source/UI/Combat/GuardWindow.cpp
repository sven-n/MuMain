
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
#include "UI/Events/UIGuardsMan.h"

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
#include "UI/Party/UIWindows.h"
#include "Render/Text/CUIRenderText.h"

using namespace SEASON3B;
using namespace mu::ui::window;

extern DWORD g_dwActiveUIID;

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

    LoadScrollBarImages();

    // The tabs stay a native radio group for their hit tests; guard_window.rml draws them.
    m_TabBtn.CreateRadioGroup(3, BITMAP_GUILDINFO_BEGIN);
    m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 12.f, m_Pos.y + 84.f, 56, 22);
    SetCurOpenTab(m_iNumCurOpenTab);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CGuardWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    UnloadScrollBarImages();

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

// The only writer of the open tab: the radio group follows it, never the other way round, so the
// highlight the document draws and the page it draws cannot disagree.
void CGuardWindow::SetCurOpenTab(int iTab)
{
    m_iNumCurOpenTab = iTab;
    m_TabBtn.ChangeFrame(iTab);
}

bool CGuardWindow::UpdateMouseEvent()
{
    // The guild lists keep their native scrolling and line clicks; the buttons are RmlUi's (see
    // Update()).
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

    if (IsVisible())
    {
        const int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();
        if (iNumCurOpenTab != RADIOGROUPEVENT_NONE)
        {
            SetCurOpenTab(iNumCurOpenTab);

            if (iNumCurOpenTab == TAB_REGISTER_INFO)
            {
                if (m_eTimeType == CASTLESIEGE_STATE_REGSIEGE || m_eTimeType == CASTLESIEGE_STATE_REGMARK)
                {
                    SocketClient->ToGameServer()->SendCastleSiegeRegisteredGuildsListRequest();
                }
                else if (m_eTimeType == CASTLESIEGE_STATE_NOTIFY || m_eTimeType == CASTLESIEGE_STATE_READYSIEGE)
                {
                    SocketClient->ToGameServer()->SendCastleOwnerListRequest();
                }
            }
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
    m_DeclareGuildListBox.Clear();
    m_GuildListBox.Clear();

    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CGuardWindow::GetLayerDepth()
{
    return 5.0f;
}

void CGuardWindow::LoadScrollBarImages()
{
    LoadBitmap(L"Interface\\newui_scrollbar_up.tga", IMAGE_GUARDWINDOW_SCROLL_TOP);
    LoadBitmap(L"Interface\\newui_scrollbar_m.tga", IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
    LoadBitmap(L"Interface\\newui_scrollbar_down.tga", IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
    LoadBitmap(L"Interface\\newui_scroll_on.tga", IMAGE_GUARDWINDOW_SCROLLBAR_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_scroll_off.tga", IMAGE_GUARDWINDOW_SCROLLBAR_OFF, GL_LINEAR);
}

void CGuardWindow::UnloadScrollBarImages()
{
    DeleteBitmap(IMAGE_GUARDWINDOW_SCROLL_TOP);
    DeleteBitmap(IMAGE_GUARDWINDOW_SCROLL_MIDDLE);
    DeleteBitmap(IMAGE_GUARDWINDOW_SCROLL_BOTTOM);
    DeleteBitmap(IMAGE_GUARDWINDOW_SCROLLBAR_ON);
    DeleteBitmap(IMAGE_GUARDWINDOW_SCROLLBAR_OFF);
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
        if (button == GUARD_BUTTON_PROCLAIM && Hero->GuildStatus == G_MASTER && !g_GuardsMan.HasRegistered() &&
            !ProclaimLocked())
        {
            if (g_GuardsMan.IsSufficentDeclareLevel())
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
        if (button == GUARD_BUTTON_REGISTER && g_GuardsMan.HasRegistered() && g_GuardsMan.GetMyMarkCount() > 0)
        {
            int nMarkSlot = g_GuardsMan.GetMyMarkSlotIndex();
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
    if (m_eTimeType == CASTLESIEGE_STATE_REGSIEGE || m_eTimeType == CASTLESIEGE_STATE_REGMARK)
    {
        m_DeclareGuildListBox.DoAction();
        if (PressKey(VK_PRIOR))
            m_DeclareGuildListBox.Scrolling(-1 * m_DeclareGuildListBox.GetBoxSize());
        if (PressKey(VK_NEXT))
            m_DeclareGuildListBox.Scrolling(m_DeclareGuildListBox.GetBoxSize());
    }
    else if (m_eTimeType == CASTLESIEGE_STATE_NOTIFY || m_eTimeType == CASTLESIEGE_STATE_READYSIEGE)
    {
        m_GuildListBox.DoAction();
        if (PressKey(VK_PRIOR))
            m_GuildListBox.Scrolling(-1 * m_GuildListBox.GetBoxSize());
        if (PressKey(VK_NEXT))
            m_GuildListBox.Scrolling(m_GuildListBox.GetBoxSize());
    }
}

void CGuardWindow::UpdateRegisterInfoTab(GUARD_BUTTON button)
{
    if (button == GUARD_BUTTON_GIVE_UP && g_GuardsMan.HasRegistered() && CASTLESIEGE_STATE_REGSIEGE <= m_eTimeType &&
        m_eTimeType <= CASTLESIEGE_STATE_REGMARK && Hero->GuildStatus == G_MASTER)
    {
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines.push_back({I18N::Game::AreYouReallyWantToQuitTheSiegeWargare, false});
        cfg.onPrimary = [] { SocketClient->ToGameServer()->SendCastleSiegeUnregisterRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

void CGuardWindow::SetData(LPPMSG_ANS_CASTLESIEGESTATE Info)
{
    if (!Info)	return;

    // Whole buffers: the original cleared 9 and 11 bytes of these wchar_t arrays, so a name
    // converted without room for its terminator kept stack garbage after it (the original lost
    // the "Official seal of king" line to a ten-character guild master).
    memset(m_szOwnerGuild, 0, sizeof(m_szOwnerGuild));
    memset(m_szOwnerGuildMaster, 0, sizeof(m_szOwnerGuildMaster));

    m_eTimeType = (CASTLESIEGE_STATE)Info->cCastleSiegeState;
    CMultiLanguage::ConvertFromUtf8(m_szOwnerGuild, Info->cOwnerGuild, MAX_GUILDNAME);
    CMultiLanguage::ConvertFromUtf8(m_szOwnerGuildMaster, Info->cOwnerGuildMaster, MAX_USERNAME_SIZE);

    m_wStartYear = MAKEWORD(Info->btStartYearL, Info->btStartYearH);
    m_byStartMonth = Info->btStartMonth;
    m_byStartDay = Info->btStartDay;
    m_byStartHour = Info->btStartHour;
    m_byStartMinute = Info->btStartMinute;
    m_wEndYear = MAKEWORD(Info->btEndYearL, Info->btEndYearH);
    m_byEndMonth = Info->btEndMonth;
    m_byEndDay = Info->btEndDay;
    m_byEndHour = Info->btEndHour;
    m_byEndMinute = Info->btEndMinute;
    m_wSiegeStartYear = MAKEWORD(Info->btSiegeStartYearL, Info->btSiegeStartYearH);
    m_bySiegeStartMonth = Info->btSiegeStartMonth;
    m_bySiegeStartDay = Info->btSiegeStartDay;
    m_bySiegeStartHour = Info->btSiegeStartHour;
    m_bySiegeStartMinute = Info->btSiegeStartMinute;
    m_dwStateLeftSec = MAKELONG(MAKEWORD(Info->btStateLeftSec4, Info->btStateLeftSec3), MAKEWORD(Info->btStateLeftSec2, Info->btStateLeftSec1));
    //m_dwStateLeftSec = Info->btStateLeftSec1<<24 | Info->btStateLeftSec2<<16 | Info->btStateLeftSec3<<8 | Info->btStateLeftSec4;
}

void CGuardWindow::AddDeclareGuildList(wchar_t* szGuildName, int nMarkCount, BYTE byIsGiveUP, BYTE bySeqNum)
{
    m_DeclareGuildListBox.AddText(szGuildName, nMarkCount, byIsGiveUP, bySeqNum);
}

void CGuardWindow::ClearDeclareGuildList()
{
    m_DeclareGuildListBox.Clear();
}

void CGuardWindow::SortDeclareGuildList()
{
    m_DeclareGuildListBox.Sort();
}

void CGuardWindow::AddGuildList(wchar_t* szGuildName, BYTE byCsJoinSide, BYTE byGuildInvolved, int iGuildScore)
{
    m_GuildListBox.AddText(szGuildName, byCsJoinSide, byGuildInvolved, iGuildScore);
}

void CGuardWindow::ClearGuildList()
{
    m_GuildListBox.Clear();
}

void CGuardWindow::RenderScrollBarFrame(int iPos_x, int iPos_y, int iHeight)
{
    RenderImage(IMAGE_GUARDWINDOW_SCROLL_TOP, iPos_x, iPos_y, 7, 3);
#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP

    BITMAP_t* pImage = &Bitmaps[IMAGE_GUARDWINDOW_SCROLL_MIDDLE];
    float _Temp = pImage->Height - 1;
    float _fMiddle_Cnt = (iHeight - 6) / _Temp;
    int _iMiddle_Cnt = (int)_fMiddle_Cnt;
    float _Middle_rest = _fMiddle_Cnt - _iMiddle_Cnt;

    for (int i = 0; i < _iMiddle_Cnt; i++)
        RenderImage(IMAGE_GUARDWINDOW_SCROLL_MIDDLE, iPos_x, iPos_y + (float)(i * _Temp + 3), 7, _Temp);

    RenderImage(IMAGE_GUARDWINDOW_SCROLL_MIDDLE, iPos_x, iPos_y + (float)(_iMiddle_Cnt * _Temp + 3), 7, _Temp * _Middle_rest);
#else //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
    RenderBitmap(IMAGE_GUARDWINDOW_SCROLL_MIDDLE, iPos_x, iPos_y + 3, 7.f, iHeight - 6, 0, 0, 7.f / 8.f, 15.f / 16.f);
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
    RenderImage(IMAGE_GUARDWINDOW_SCROLL_BOTTOM, iPos_x, iPos_y + iHeight - 3, 7, 3);
}

void CGuardWindow::RenderScrollBar(int iPos_x, int iPos_y, BOOL bIsClicked)
{
    const DWORD scrollBarColor = bIsClicked
        ? RGBA(200, 200, 200, 255)
        : RGBA(255, 255, 255, 255);
    RenderImage(IMAGE_GUARDWINDOW_SCROLLBAR_ON, iPos_x, iPos_y, 15, 30, 0.f, 0.f, scrollBarColor);
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
            declareRow.RegisterMember("top", &GuardDeclareRow::top);
            declareRow.RegisterMember("selected", &GuardDeclareRow::selected);
            c.RegisterArray<std::vector<GuardDeclareRow>>();
            c.Bind("declare_rows", &model.declareRows);
            auto siegeRow = c.RegisterStruct<GuardSiegeRow>();
            siegeRow.RegisterMember("name", &GuardSiegeRow::name);
            siegeRow.RegisterMember("side", &GuardSiegeRow::side);
            siegeRow.RegisterMember("involvement", &GuardSiegeRow::involvement);
            siegeRow.RegisterMember("top", &GuardSiegeRow::top);
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
            c.Bind("scroll_shown", &model.scrollShown);
            c.Bind("scroll_top", &model.scrollTop);
            c.Bind("scroll_height", &model.scrollHeight);
            c.Bind("thumb_top", &model.thumbTop);
            c.Bind("thumb_dragged", &model.thumbDragged);
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
    std::vector<GuardDeclareRow> declareRows;
    std::vector<GuardSiegeRow> siegeRows;
    Rml::String scoreValue;
    TextListScrollBarGeometry scroll{};
    bool scrollShown = false;
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
                if (!g_GuardsMan.HasRegistered())
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
            if (g_GuardsMan.HasRegistered())
            {
                registerMessage = pageLine(I18N::Game::RegisterTheAcquiredSign);
                const int nMarkCount = g_GuardsMan.GetMyMarkCount();
                mu_swprintf(szText, I18N::Game::AcquiredNoOfSignU, nMarkCount);
                registerAcquired = pageLine(szText);
                mu_swprintf(szText, I18N::Game::RegisteredNoOfSignU, g_GuardsMan.GetRegMarkCount());
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
        // RenderRegisterInfoTab() and the lists' RenderInterface()/RenderDataLine(). The native
        // list boxes still hold the rows, their scrolling and their selection, so a row's own
        // `top` follows their scroll position -- everything else about a row is the theme's.
        if (m_eTimeType == CASTLESIEGE_STATE_REGSIEGE || m_eTimeType == CASTLESIEGE_STATE_REGMARK)
        {
            listKind = 1;
            CUIBCDeclareGuildListBox& list = m_DeclareGuildListBox;
            scroll = list.GetScrollBarGeometry();
            scrollShown = true;
            list.ForEachRenderLine(
                [&](int line, const BCDECLAREGUILD_TEXT& item, bool selected)
                {
                    // Only the hero's own guild or alliance is listed.
                    if (wcscmp(GuildMark[Hero->GuildMarkIndex].UnionName, item.szName) != 0 &&
                        wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, item.szName) != 0)
                        return false;
                    wchar_t cell[64] = {};
                    GuardDeclareRow row;
                    row.name = StringUtils::WideToNarrow(item.szName);
                    mu_swprintf(cell, L"%d", item.nCount);
                    row.markCount = StringUtils::WideToNarrow(cell);
                    row.state = StringUtils::WideToNarrow(item.byIsGiveUp ? I18N::Game::Failed
                                                                          : I18N::Game::Processing);
                    mu_swprintf(cell, L"%u", item.bySeqNum);
                    row.order = StringUtils::WideToNarrow(cell);
                    row.top = static_cast<float>(list.GetRenderLinePos_y(line)) - 3.f - y0;
                    row.selected = selected;
                    declareRows.push_back(std::move(row));
                    return true;
                });
        }
        else if (m_eTimeType == CASTLESIEGE_STATE_NOTIFY || m_eTimeType == CASTLESIEGE_STATE_READYSIEGE)
        {
            listKind = 2;
            CUIBCGuildListBox& list = m_GuildListBox;
            scroll = list.GetScrollBarGeometry();
            scrollShown = true;
            list.ForEachRenderLine(
                [&](int line, const BCGUILD_TEXT& item, bool selected)
                {
                    GuardSiegeRow row;
                    row.name = StringUtils::WideToNarrow(item.szName);
                    row.side = StringUtils::WideToNarrow(item.byJoinSide == 1 ? I18N::Game::DefendingTeam
                                                                             : I18N::Game::InvadingTeam);
                    row.involvement = StringUtils::WideToNarrow(
                        item.byGuildInvolved == 1 ? I18N::Game::Maintain : I18N::Game::Assist);
                    row.top = static_cast<float>(list.GetRenderLinePos_y(line)) - 3.f - y0;
                    row.selected = selected;
                    row.defending = item.byJoinSide == 1;
                    siegeRows.push_back(std::move(row));
                    // The summary row shows the guild the list has picked out, not the selected
                    // line: a defending guild has no score to show.
                    if (list.Select_Guild == line)
                    {
                        wchar_t info[300] = {};
                        if (item.byJoinSide == 1)
                            mu_swprintf(info, L"--");
                        else
                            mu_swprintf(info, L"%ls :     %d", item.szName, item.iGuildScore);
                        scoreValue = StringUtils::WideToNarrow(info);
                    }
                });
        }
        else if (m_eTimeType == CASTLESIEGE_STATE_ENDSIEGE)
        {
            listMessage = pageLine(I18N::Game::TrucePeriod, true);
        }

        if (g_GuardsMan.HasRegistered() && CASTLESIEGE_STATE_REGSIEGE <= m_eTimeType &&
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
    if (model.declareRows != declareRows)
    {
        model.declareRows = std::move(declareRows);
        m_RmlBinder.MarkDirty("declare_rows");
    }
    if (model.siegeRows != siegeRows)
    {
        model.siegeRows = std::move(siegeRows);
        m_RmlBinder.MarkDirty("siege_rows");
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
    SyncField(m_RmlBinder, &GuardWindowRmlModel::scoreValue, "score_value", std::move(scoreValue));
    // RenderScrollBarFrame() at the list's right edge - 8 over the track, the thumb at - 12.
    SyncField(m_RmlBinder, &GuardWindowRmlModel::scrollShown, "scroll_shown", scrollShown);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::scrollTop, "scroll_top", scroll.rangeTop - y0);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::scrollHeight, "scroll_height", scroll.rangeBottom - scroll.rangeTop);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::thumbTop, "thumb_top", scroll.thumbTop - y0);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::thumbDragged, "thumb_dragged", scroll.dragged && MouseLButtonPush);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    SyncField(m_RmlBinder, &GuardWindowRmlModel::buttonLabelTop, "button_label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::tabLabelTop, "tab_label_top", static_cast<float>(22 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &GuardWindowRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
