
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
#include "Guild/UIGuildInfo.h"
#include "UI/Events/UIGuardsMan.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

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
            tab.RegisterMember("label_left", &GuardTabEntry::labelLeft);
            tab.RegisterMember("selected", &GuardTabEntry::selected);
            c.RegisterArray<std::vector<GuardTabEntry>>();
            c.Bind("tabs", &model.tabs);
            c.Bind("list_shown", &model.listShown);
            c.Bind("list_has_footer", &model.listHasFooter);
            c.Bind("scroll_shown", &model.scrollShown);
            c.Bind("scroll_top", &model.scrollTop);
            c.Bind("scroll_height", &model.scrollHeight);
            c.Bind("thumb_top", &model.thumbTop);
            c.Bind("thumb_dragged", &model.thumbDragged);
            auto box = c.RegisterStruct<GuardBoxEntry>();
            box.RegisterMember("left", &GuardBoxEntry::left);
            box.RegisterMember("top", &GuardBoxEntry::top);
            box.RegisterMember("width", &GuardBoxEntry::width);
            box.RegisterMember("height", &GuardBoxEntry::height);
            box.RegisterMember("color", &GuardBoxEntry::color);
            c.RegisterArray<std::vector<GuardBoxEntry>>();
            c.Bind("boxes", &model.boxes);
            auto text = c.RegisterStruct<GuardTextEntry>();
            text.RegisterMember("text", &GuardTextEntry::text);
            text.RegisterMember("left", &GuardTextEntry::left);
            text.RegisterMember("top", &GuardTextEntry::top);
            text.RegisterMember("width", &GuardTextEntry::width);
            text.RegisterMember("text_px", &GuardTextEntry::textPx);
            text.RegisterMember("align", &GuardTextEntry::align);
            text.RegisterMember("bold", &GuardTextEntry::bold);
            text.RegisterMember("color", &GuardTextEntry::color);
            c.RegisterArray<std::vector<GuardTextEntry>>();
            c.Bind("texts", &model.texts);
            auto button = c.RegisterStruct<GuardButtonEntry>();
            button.RegisterMember("label", &GuardButtonEntry::label);
            button.RegisterMember("id", &GuardButtonEntry::id);
            button.RegisterMember("top", &GuardButtonEntry::top);
            button.RegisterMember("locked", &GuardButtonEntry::locked);
            c.RegisterArray<std::vector<GuardButtonEntry>>();
            c.Bind("buttons", &model.buttons);
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
    std::vector<GuardTextEntry> texts;
    std::vector<GuardBoxEntry> boxes;
    std::vector<GuardButtonEntry> buttons;
    // RenderText(x, y, text, width, 0, sort) in window coordinates, in the font and colour the
    // original had set at that point (its draws leak them from one call to the next).
    bool bold = true;
    DWORD color = RGBA(220, 220, 220, 255);
    auto addText = [&](const wchar_t* text, float x, float y, float width, int align)
    {
        if (text == nullptr || text[0] == L'\0')
            return;
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            width > 0.f ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), width)
                        : UI::Scaling::NativeTextPixelSize(role, transform);
        texts.push_back(
            {StringUtils::WideToNarrow(text), x - x0, y - y0, width, px, align, bold, UI::RmlBridge::RgbaToCss(color)});
    };
    // RT3_WRITE_RIGHT_TO_LEFT: the text ends at x.
    auto addTextEndingAt = [&](const wchar_t* text, float x, float y) { addText(text, x - 200, y, 200, 2); };
    auto addCentred = [&](const wchar_t* text, float y) { addText(text, x0, y, 190, 1); };
    auto addBox = [&](float x, float y, float width, float height, DWORD rgba)
    { boxes.push_back({x - x0, y - y0, width, height, UI::RmlBridge::RgbaToCss(rgba)}); };
    auto addButton = [&](GUARD_BUTTON id, const wchar_t* label, float top, bool locked)
    { buttons.push_back({StringUtils::WideToNarrow(label), id, top, locked}); };
    wchar_t szText[256] = {};

    // RenderFrame(): the title and the owner lines, bold (220, 220, 220).
    addText(I18N::Game::GuardNPC, x0 + 15, y0 + 13, 160, 1);
    mu_swprintf(szText, I18N::Game::OfficialSealOfKingS,
                m_szOwnerGuildMaster[0] ? m_szOwnerGuildMaster : I18N::Game::None);
    addCentred(szText, y0 + 50);
    mu_swprintf(szText, I18N::Game::AffiliatedGuildS, m_szOwnerGuild[0] ? m_szOwnerGuild : I18N::Game::None);
    addCentred(szText, y0 + 65);

    // The tabs: the second one's label follows the period. (The original pushed the labels into a
    // static list every frame and so kept the first frame's for the whole session.)
    const wchar_t* tabLabels[] = {
        I18N::Game::Status, m_eTimeType == CASTLESIEGE_STATE_REGSIEGE ? I18N::Game::Announce : I18N::Game::Register,
        I18N::Game::List};
    std::vector<GuardTabEntry> tabs;
    g_pRenderText->SetFont(g_hFont);
    for (int i = 0; i < 3; ++i)
    {
        const SIZE size = g_pRenderText->MeasureText(tabLabels[i], static_cast<int>(wcslen(tabLabels[i])));
        tabs.push_back({StringUtils::WideToNarrow(tabLabels[i]), static_cast<float>(56 / 2 - size.cx / 2),
                        i == m_iNumCurOpenTab});
    }

    const float y = y0 + 125;
    bool listShown = false;
    bool listHasFooter = false;
    TextListScrollBarGeometry scroll{};
    bool scrollShown = false;
    switch (m_iNumCurOpenTab)
    {
    case TAB_SIEGE_INFO:
    {
        // RenderSeigeInfoTab().
        bold = false;
        wchar_t szTemp[256] = {};
        mu_swprintf(szTemp, I18N::Game::StartingUUUUU, m_wStartYear, m_byStartMonth, m_byStartDay, m_byStartHour,
                    m_byStartMinute);
        addCentred(szTemp, y);
        mu_swprintf(szTemp, I18N::Game::UntillUUUUU, m_wEndYear, m_byEndMonth, m_byEndDay, m_byEndHour, m_byEndMinute);
        addCentred(szTemp, y + 14);
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
        addCentred(period, y + 28);
        if (m_eTimeType < CASTLESIEGE_STATE_STARTSIEGE)
        {
            addCentred(I18N::Game::ExpectedSiegePeriodIs, y + 63);
            mu_swprintf(szTemp, I18N::Game::UUUUU, m_wSiegeStartYear, m_bySiegeStartMonth, m_bySiegeStartDay,
                        m_bySiegeStartHour, m_bySiegeStartMinute);
            addCentred(szTemp, y + 77);
            mu_swprintf(szTemp, I18N::Game::UUURemainedForTheNextStage, m_dwStateLeftSec / 3600,
                        (m_dwStateLeftSec % 3600) / 60, (m_dwStateLeftSec % 3600) % 60);
            addCentred(szTemp, y + 112);
        }
        break;
    }
    case TAB_REGISTER:
    {
        // RenderRegisterTab().
        bold = false;
        switch (m_eTimeType)
        {
        case CASTLESIEGE_STATE_NONE:
        case CASTLESIEGE_STATE_IDLE_1:
            addCentred(I18N::Game::SiegePeriodIsOver, y);
            break;
        case CASTLESIEGE_STATE_REGSIEGE:
            if (Hero->GuildStatus == G_MASTER)
            {
                if (!g_GuardsMan.HasRegistered())
                    addButton(GUARD_BUTTON_PROCLAIM, I18N::Game::Announce, 120, ProclaimLocked());
                else
                    addCentred(I18N::Game::Announced, y);
            }
            else
            {
                addCentred(I18N::Game::NotAGuildMaster, y);
            }
            break;
        case CASTLESIEGE_STATE_IDLE_2:
            addCentred(I18N::Game::StandbyPeriodForSignRegistration, y);
            break;
        case CASTLESIEGE_STATE_REGMARK:
            if (g_GuardsMan.HasRegistered())
            {
                addCentred(I18N::Game::RegisterTheAcquiredSign, y);
                const int nMarkCount = g_GuardsMan.GetMyMarkCount();
                mu_swprintf(szText, I18N::Game::AcquiredNoOfSignU, nMarkCount);
                addCentred(szText, y + 30);
                mu_swprintf(szText, I18N::Game::RegisteredNoOfSignU, g_GuardsMan.GetRegMarkCount());
                addCentred(szText, y + 44);
                addButton(GUARD_BUTTON_REGISTER, I18N::Game::Register, 200, nMarkCount <= 0);
            }
            else
            {
                addCentred(I18N::Game::ThisGuildIsNotRegisteredInCastleSiege, y);
            }
            break;
        case CASTLESIEGE_STATE_IDLE_3:
            bold = true;
            addCentred(I18N::Game::AnnouncementAndRegistrationPeriod, y);
            addCentred(I18N::Game::HasEnded, y + 14);
            break;
        case CASTLESIEGE_STATE_NOTIFY:
            addCentred(I18N::Game::AnnouncementPeriod, y);
            break;
        case CASTLESIEGE_STATE_READYSIEGE:
            addCentred(I18N::Game::SiegePreparationPeriod, y);
            break;
        case CASTLESIEGE_STATE_STARTSIEGE:
            addCentred(I18N::Game::SiegePeriod, y);
            break;
        case CASTLESIEGE_STATE_ENDSIEGE:
            bold = true;
            addCentred(I18N::Game::TrucePeriod, y);
            break;
        case CASTLESIEGE_STATE_ENDCYCLE:
            addCentred(I18N::Game::SiegeIsOver, y);
            break;
        default:
            break;
        }
        break;
    }
    case TAB_REGISTER_INFO:
    {
        // RenderRegisterInfoTab() and the lists' RenderInterface()/RenderDataLine(): the list's
        // backdrop and selected lines black at 40 % (SetLineColor(7, 0.4f), the colour the render
        // left set), the headers in the tab's bold (220, 220, 220), the lines in the normal font.
        const DWORD black40 = RGBA(0, 0, 0, 102);
        const DWORD textColor = RGBA(230, 220, 200, 255);
        if (m_eTimeType == CASTLESIEGE_STATE_REGSIEGE || m_eTimeType == CASTLESIEGE_STATE_REGMARK)
        {
            listShown = true;
            CUIBCDeclareGuildListBox& list = m_DeclareGuildListBox;
            const float lx = static_cast<float>(list.GetPosition_x());
            const float ly = static_cast<float>(list.GetPosition_y());
            const float lw = static_cast<float>(list.GetWidth());
            const float lh = static_cast<float>(list.GetHeight());
            addBox(lx - 1, ly - lh - 1, lw + 1, lh + 2, black40);
            scroll = list.GetScrollBarGeometry();
            scrollShown = true;
            addText(I18N::Game::NAME, lx + 5, ly - lh - 12, 0, 0);
            addText(I18N::Game::NoReg, lx + 50, ly - lh - 12, 0, 0);
            addText(I18N::Game::Stat, lx + 98, ly - lh - 12, 0, 0);
            addText(I18N::Game::Order, lx + 123, ly - lh - 12, 0, 0);
            bold = false;
            list.ForEachRenderLine(
                [&](int line, const BCDECLAREGUILD_TEXT& item, bool selected)
                {
                    // Only the hero's own guild or alliance is listed.
                    if (wcscmp(GuildMark[Hero->GuildMarkIndex].UnionName, item.szName) != 0 &&
                        wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, item.szName) != 0)
                        return false;
                    const float ry = static_cast<float>(list.GetRenderLinePos_y(line));
                    if (selected)
                        addBox(lx, ry - 3, lw - 13 + 1, 13, black40);
                    color = selected ? RGBA(0, 0, 0, 255) : textColor;
                    addText(item.szName, lx + 6, ry, 0, 0);
                    wchar_t cell[64] = {};
                    mu_swprintf(cell, L"%d", item.nCount);
                    addTextEndingAt(cell, lx + 74, ry);
                    addTextEndingAt(item.byIsGiveUp ? I18N::Game::Failed : I18N::Game::Processing, lx + 124, ry);
                    mu_swprintf(cell, L"%u", item.bySeqNum);
                    addTextEndingAt(cell, lx + 144, ry);
                    return true;
                });
        }
        else if (m_eTimeType == CASTLESIEGE_STATE_NOTIFY || m_eTimeType == CASTLESIEGE_STATE_READYSIEGE)
        {
            listShown = true;
            listHasFooter = true;
            CUIBCGuildListBox& list = m_GuildListBox;
            const float lx = static_cast<float>(list.GetPosition_x());
            const float ly = static_cast<float>(list.GetPosition_y());
            const float lw = static_cast<float>(list.GetWidth());
            const float lh = static_cast<float>(list.GetHeight());
            addBox(lx - 1, ly - lh - 1, lw + 1, lh + 2, black40);
            addBox(lx - 1, ly + 25 - 1, lw - 100 + 1, 20, RGBA(146, 134, 121, 102));
            addBox(lx - 1 + lw - 100 + 1, ly + 25 - 1, lw - 60, 20, black40);
            scroll = list.GetScrollBarGeometry();
            scrollShown = true;
            addText(I18N::Game::NAME, lx + 5, ly - lh - 12, 0, 0);
            addText(I18N::Game::Camp, lx + 80, ly - lh - 12, 0, 0);
            addText(I18N::Game::Maintain, lx + 120, ly - lh - 12, 0, 0);
            addText(I18N::Game::Score, lx + 18, ly + 31 - 1, 0, 0);
            bold = false;
            list.ForEachRenderLine(
                [&](int line, const BCGUILD_TEXT& item, bool selected)
                {
                    const float ry = static_cast<float>(list.GetRenderLinePos_y(line));
                    if (selected || item.byJoinSide == 1)
                        addBox(lx, ry - 3, lw - 13 + 1, 13, black40);
                    color = selected ? RGBA(0, 0, 0, 255) : textColor;
                    addText(item.szName, lx + 6, ry, 0, 0);
                    addTextEndingAt(item.byJoinSide == 1 ? I18N::Game::DefendingTeam : I18N::Game::InvadingTeam,
                                    lx + 104, ry);
                    addTextEndingAt(item.byGuildInvolved == 1 ? I18N::Game::Maintain : I18N::Game::Assist, lx + 141,
                                    ry);
                    if (list.Select_Guild == line)
                    {
                        color = textColor;
                        wchar_t info[300] = {};
                        if (item.byJoinSide == 1)
                            mu_swprintf(info, L"--");
                        else
                            mu_swprintf(info, L"%ls :     %d", item.szName, item.iGuildScore);
                        addText(info, lx + 60, ly + 31 - 1, 0, 0);
                    }
                });
        }
        else if (m_eTimeType == CASTLESIEGE_STATE_ENDSIEGE)
        {
            addCentred(I18N::Game::TrucePeriod, y);
        }

        if (g_GuardsMan.HasRegistered() && CASTLESIEGE_STATE_REGSIEGE <= m_eTimeType &&
            m_eTimeType <= CASTLESIEGE_STATE_REGMARK && Hero->GuildStatus == G_MASTER)
            addButton(GUARD_BUTTON_GIVE_UP, I18N::Game::AbandonCastleSiege, 370, false);
        break;
    }
    default:
        break;
    }

    GuardWindowRmlModel& model = m_RmlBinder.GetModel();
    auto sync = [&](auto field, const char* name, auto value)
    {
        if (!(model.*field == value))
        {
            model.*field = std::move(value);
            m_RmlBinder.MarkDirty(name);
        }
    };
    const bool sameTabs =
        model.tabs.size() == tabs.size() &&
        std::equal(model.tabs.begin(), model.tabs.end(), tabs.begin(),
                   [](const GuardTabEntry& a, const GuardTabEntry& b)
                   { return a.label == b.label && a.labelLeft == b.labelLeft && a.selected == b.selected; });
    if (!sameTabs)
    {
        model.tabs = std::move(tabs);
        m_RmlBinder.MarkDirty("tabs");
    }
    const bool sameBoxes = model.boxes.size() == boxes.size() &&
                           std::equal(model.boxes.begin(), model.boxes.end(), boxes.begin(),
                                      [](const GuardBoxEntry& a, const GuardBoxEntry& b)
                                      {
                                          return a.left == b.left && a.top == b.top && a.width == b.width &&
                                                 a.height == b.height && a.color == b.color;
                                      });
    if (!sameBoxes)
    {
        model.boxes = std::move(boxes);
        m_RmlBinder.MarkDirty("boxes");
    }
    const bool sameTexts = model.texts.size() == texts.size() &&
                           std::equal(model.texts.begin(), model.texts.end(), texts.begin(),
                                      [](const GuardTextEntry& a, const GuardTextEntry& b)
                                      {
                                          return a.text == b.text && a.left == b.left && a.top == b.top &&
                                                 a.width == b.width && a.textPx == b.textPx && a.align == b.align &&
                                                 a.bold == b.bold && a.color == b.color;
                                      });
    if (!sameTexts)
    {
        model.texts = std::move(texts);
        m_RmlBinder.MarkDirty("texts");
    }
    const bool sameButtons =
        model.buttons.size() == buttons.size() &&
        std::equal(model.buttons.begin(), model.buttons.end(), buttons.begin(),
                   [](const GuardButtonEntry& a, const GuardButtonEntry& b)
                   { return a.label == b.label && a.id == b.id && a.top == b.top && a.locked == b.locked; });
    if (!sameButtons)
    {
        model.buttons = std::move(buttons);
        m_RmlBinder.MarkDirty("buttons");
    }
    sync(&GuardWindowRmlModel::listShown, "list_shown", listShown);
    sync(&GuardWindowRmlModel::listHasFooter, "list_has_footer", listHasFooter);
    // RenderScrollBarFrame() at the list's right edge - 8 over the track, the thumb at - 12.
    sync(&GuardWindowRmlModel::scrollShown, "scroll_shown", scrollShown);
    sync(&GuardWindowRmlModel::scrollTop, "scroll_top", scroll.rangeTop - y0);
    sync(&GuardWindowRmlModel::scrollHeight, "scroll_height", scroll.rangeBottom - scroll.rangeTop);
    sync(&GuardWindowRmlModel::thumbTop, "thumb_top", scroll.thumbTop - y0);
    sync(&GuardWindowRmlModel::thumbDragged, "thumb_dragged", scroll.dragged && MouseLButtonPush);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    sync(&GuardWindowRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    sync(&GuardWindowRmlModel::buttonLabelTop, "button_label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    sync(&GuardWindowRmlModel::tabLabelTop, "tab_label_top", static_cast<float>(22 / 2 - lineHeight / 2));
    sync(&GuardWindowRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
