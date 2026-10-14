#include "stdafx.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Dialogs/GenericMenuDialog.h"
#include "Audio/DSPlaySound.h"
#include "UI/Social/SocialWindowBase.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameLogic/Events/MatchEvent.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "GameLogic/Items/MixMgr.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "Engine/Object/ZzzOpenData.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "UI/Core/WindowSystem.h"
#include "MUHelper/MuHelper.h"
#include "Core/Text/TextLineWrap.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Text/CUIRenderText.h"

extern int DeleteIndex;
extern int AppointStatus;
extern bool LogOut;

char AppointType;

#define SUBGUILDMASTER	64
#define BATTLEMASTER	32

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
template <typename AppendLine>
int AppendWrappedMessageLines(const std::wstring& text, BYTE fontType, int maxWidth, AppendLine appendLine)
{
    g_pRenderText->SetFont(fontType == MSGBOX_FONT_BOLD ? g_hFontBold : g_hFont);
    auto lines = WrapTextToWidth(text, maxWidth, [](const wchar_t* line, size_t length)
    {
        return g_pRenderText->MeasureText(line, static_cast<int>(length)).cx;
    });
    if (lines.empty())
    {
        lines.emplace_back();
    }

    for (const auto& line : lines)
    {
        appendLine(line);
    }
    return static_cast<int>(lines.size());
}

// The event result boxes' MessageBoxView: the lines RenderMatchResult() draws, each text shrunk to
// the box the original gave it, and the OK button's newui_button_ok art. The theme places them by
// their roles and the box's kind.
void SyncMatchResultView(MessageBoxView& view)
{
    std::vector<MatchResultLine> results;
    matchEvent::CollectResult(results);

    std::vector<MessageBoxView::Line> lines;
    for (const MatchResultLine& result : results)
    {
        MessageBoxView::Line line{result.role, {}};
        for (const MatchResultCell& text : result.cells)
        {
            const bool bold = text.font == MatchResultCell::Font::Bold;
            // A box the text does not fit shrinks it (the Devil Square headers' box height of 3).
            float textPx = 0.f;
            if (text.boxWidth > 0 || text.boxHeight > 0)
            {
                g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
                const SIZE size = g_pRenderText->MeasureText(text.text.c_str(), static_cast<int>(text.text.size()));
                const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
                const float fittedPx = UI::RmlBridge::NativeTextPxInBounds(
                    role, static_cast<float>(size.cx), static_cast<float>(size.cy), static_cast<float>(text.boxWidth),
                    static_cast<float>(text.boxHeight));
                if (fittedPx != UI::RmlBridge::NativeTextPx(role))
                    textPx = fittedPx;
            }
            line.cells.push_back({text.text, bold, text.color, textPx});
        }
        lines.push_back(std::move(line));
    }

    MessageBoxView::Button button{L"OK"};
    button.okArt = true;
    view.Sync(lines, {button});
}

// A box's text lines (MSGBOX_TEXTDATA), each centred in its font; the theme stacks them.
std::vector<MessageBoxView::Line> MessageLines(const type_vector_msgdata& messages)
{
    std::vector<MessageBoxView::Line> lines;
    for (const MSGBOX_TEXTDATA* message : messages)
        lines.push_back({"message", {{message->strMsg, message->byFontType == MSGBOX_FONT_BOLD, message->dwColor}}});
    return lines;
}

// A progress notice's MessageBoxView (CProgressMsgBox, CCursedTempleProgressMsgBox): their
// RenderFrame() -- a middle strip per line past two --, RenderTexts() and RenderProgress() -- the
// elapsed fraction.
void SyncProgressView(MessageBoxView& view, const type_vector_msgdata& messages, DWORD startTime, DWORD elapseTime)
{
    const int middles = messages.size() > 2 ? static_cast<int>(messages.size()) - 2 : 0;
    view.SetFrame(middles);
    const float fraction = static_cast<float>(timeGetTime() - startTime) / static_cast<float>(elapseTime);
    view.SetProgress(fraction);
    view.Sync(MessageLines(messages), {});
}

// A click on the OK button RmlUi reported, sent as the box's OK event.
bool TakeMatchResultOk(MessageBoxView& view, CMessageBoxBase* box)
{
    if (!view.IsShown() || view.TakePressedButton() != 0)
        return false;
    g_MessageBox->SendEvent(box, MSGBOX_EVENT_USER_COMMON_OK);
    return true;
}
} // namespace

//////////////////////////////////////////////////////////////////////////

mu::ui::window::CGemIntegrationDisjointMsgBox::CGemIntegrationDisjointMsgBox()
{
    m_iMiddleFrameCount = 0;
}

mu::ui::window::CGemIntegrationDisjointMsgBox::~CGemIntegrationDisjointMsgBox()
{
    Release();
}

bool mu::ui::window::CGemIntegrationDisjointMsgBox::Create(float fPriority)
{
    int width, height;

    SetAddCallbackFunc();

    width = MSGBOX_WIDTH;
    m_iMiddleFrameCount = 1;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    AddMsg(I18N::Game::DismantleJewel, RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    SetButtonInfo();
    ChangeMiddleFrameBig();
    AddMsg(L" ", RGBA(255, 128, 0, 255), MSGBOX_FONT_BOLD);
    AddMsg(I18N::Game::SelectAJewelToDissolve, CLRDW_YELLOW, MSGBOX_FONT_BOLD);

    m_View.Create(m_iMiddleFrameCount, "gem-disjoint");
    SyncView();

    return true;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::Release()
{
    m_View.Destroy();
    CMessageBoxBase::Release();

    auto vi = m_MsgDataList.begin();
    for (; vi != m_MsgDataList.end(); vi++)
    {
        SAFE_DELETE(*vi);
    }
    m_MsgDataList.clear();
}

bool mu::ui::window::CGemIntegrationDisjointMsgBox::Update()
{
    COMGEM::RefreshWantedList();
    const int row = m_View.TakePressedListRow();
    if (row >= 0 && row < static_cast<int>(m_DisplayedItems.size()))
        COMGEM::SelectWantedItem(m_DisplayedItems[row]);
    m_BtnDisjoint.SetEnable(COMGEM::GetWantedList().Selected().has_value());

    const int pressed = m_View.TakePressedButton();
    if (pressed == 0 || (pressed == 1 && m_BtnDisjoint.IsEnabled()))
    {
        const DWORD event = pressed == 0 ? MSGBOX_EVENT_USER_COMMON_CANCEL :
                                          MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT;
        g_MessageBox->SendEvent(this, event);
        return true;
    }
    SyncView();
    return true;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::SyncView()
{
    m_View.SetFrame(m_iMiddleFrameCount);

    // RenderButtons() then RenderGemList(): Close, then Dissolve (grey until a line is selected).
    const std::vector<MessageBoxView::Button> buttons = {{I18N::Game::Close388, m_BtnCancel.IsEnabled()},
                                                         {I18N::Game::Disband, m_BtnDisjoint.IsEnabled()}};
    m_View.Sync(MessageLines(m_MsgDataList), buttons);

    SyncGemList();
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::SyncGemList()
{
    const auto& state = COMGEM::GetWantedList();
    const auto entries = state.Entries();
    if (!std::equal(m_DisplayedItems.begin(), m_DisplayedItems.end(), entries.begin(), entries.end()))
    {
        m_DisplayedItems.assign(entries.begin(), entries.end());
        m_ListRows.clear();
        static constexpr int kJewelsPerBundleLevel = 10;
        for (const auto& entry : m_DisplayedItems)
        {
            wchar_t text[MAX_GLOBAL_TEXT_STRING] = {};
            const int jewel = COMGEM::Check_Jewel(entry.type);
            mu_swprintf(text, L"%ls,  %d", I18N::Game::Lookup(COMGEM::GetJewelIndex(jewel, COMGEM::eGEM_NAME)),
                        (entry.level + 1) * kJewelsPerBundleLevel);
            m_ListRows.push_back({StringUtils::WideToNarrow(text), static_cast<int>(m_ListRows.size()), false});
        }
    }
    for (auto& row : m_ListRows)
        row.selected = state.Selected() && *state.Selected() == m_DisplayedItems[row.index];
    m_View.SyncList(&m_ListRows);
}

bool mu::ui::window::CGemIntegrationDisjointMsgBox::Render()
{
    return true;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::ChangeMiddleFrameBig()
{
    int height = 0;

    m_iMiddleFrameCount = 10;
    height = MSGBOX_TOP_HEIGHT + (m_iMiddleFrameCount * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    SetSize(GetSize().cx, height);

    m_BtnCancel.SetPos(m_BtnCancel.GetPosX(), GetSize().cy - 50);

    m_BtnDisjoint.SetPos(m_BtnDisjoint.GetPosX(), GetSize().cy - 85);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    auto* pMsg = new MSGBOX_TEXTDATA;
    pMsg->strMsg = strMsg;
    pMsg->dwColor = dwColor;
    pMsg->byFontType = byFontType;
    m_MsgDataList.push_back(pMsg);
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::DisjointBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_DISJOINT_DISJOINT);
    AddCallbackFunc(mu::ui::window::CGemIntegrationDisjointMsgBox::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::DisjointBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    if (!COMGEM::PrepareUnmix())
        return CALLBACK_BREAK;

    const int iGemLevel = COMGEM::GetUnMixGemLevel() + 1;
    const int nIdx = COMGEM::m_cGemType;

    wchar_t strText[256] = { 0, };
    mu_swprintf(strText, I18N::Game::AreYouSureToDisbandSD, I18N::Game::Lookup(COMGEM::GetJewelIndex(nIdx, COMGEM::eGEM_NAME)), iGemLevel);
    mu::ui::window::GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.lines.push_back({ strText, true });
    mu_swprintf(strText, I18N::Game::DissolvingCostDZen, COMGEM::m_iValue);
    cfg.lines.push_back({ strText, true });
    cfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
    cfg.onCancel = []
    {
        COMGEM::GetBack();
        mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationDisjointMsgBoxLayout));
    };
    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGemIntegrationDisjointMsgBox::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

void mu::ui::window::CGemIntegrationDisjointMsgBox::SetButtonInfo()
{
    float x, y, width, height;
    float msgboxhalfwidth = (GetSize().cx / 2.f);
    float btnhalfwidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH + 20;
    height = MSGBOX_BTN_EMPTY_HEIGHT;
    btnhalfwidth = width / 2.f;

    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    btnhalfwidth = width / 2.f;
    x = msgboxhalfwidth - btnhalfwidth;
    y = 40;
    m_BtnCancel.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnCancel.SetText(I18N::Game::Close388);

    x = msgboxhalfwidth - btnhalfwidth;
    width = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
    m_BtnDisjoint.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_EMPTY_SMALL);
    m_BtnDisjoint.SetText(I18N::Game::Disband);
    m_BtnDisjoint.SetEnable(false);
}

//////////////////////////////////////////////////////////////////////////

// Uses CGenericMenuDialog (UI/Dialogs/GenericMenuDialog.h), the sibling N-button-menu primitive
// to CGenericConfirmDialog. Declared in WindowCommon.h (its own two call sites,
// UI/HUD/HotKey.cpp's Esc handler and UI/Dialogs/WindowMenu.cpp's menu item 0, only need the
// declaration); implemented here rather than in WindowCommon.cpp since every global this touches
// (Hero, g_pNewUIHotKey, g_ErrorReport, MUHelper::g_MuHelper, M34CryWolf1st, ...) was already
// reachable from this file, unlike the genuinely generic CreateOkMessageBox()/
// CreateOkMessageBoxWithTitle() helpers that do live in WindowCommon.cpp.
//
// All 5 buttons keep native's own uniform 108x29 size (MSGBOX_BTN_EMPTY_WIDTH/_HEIGHT) -- unlike
// most other CustomMessageBox.h menu classes, this one does NOT give Cancel the smaller
// MSGBOX_BTN_EMPTY_SMALL_WIDTH treatment, so no button here sets compact=true.
void mu::ui::window::ShowSystemMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "system-menu";
    cfg.systemMenu = true;

    GenericMenuConfig::MenuButton btnGameOver;
    btnGameOver.label = I18N::Game::ExitGame;
    btnGameOver.onClick = []
    {
        g_ErrorReport.Write(L"> Menu - Exit game. ");
        g_ErrorReport.WriteCurrentTime();

        SaveOptions();
        SaveMacro(L"Data\\Macro.txt");

        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY))
        {
            g_pSystemLogBox->AddText(I18N::Game::ExitGameAfterClosingTheChaosInterface, mu::ui::window::TYPE_ERROR_MESSAGE);
        }
        else
        {
            MUHelper::g_MuHelper.TriggerStop();
            LogOut = true;
            SocketClient->ToGameServer()->SendLogOut(LogOutType::CloseGame);
            PostMessage(g_hWnd, WM_CLOSE, 0, 0);
            g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 0");
        }

        if (!Hero->PK) g_pNewUIHotKey->SetStateGameOver(true);
    };
    cfg.buttons.push_back(std::move(btnGameOver));

    GenericMenuConfig::MenuButton btnChooseServer;
    btnChooseServer.label = I18N::Game::SelectServer;
    btnChooseServer.onClick = []
    {
        View_End_Result = false;
        Suc_Or_Fail = -1;
        M34CryWolf1st::CryWolfMVPInit();

        g_ErrorReport.Write(L"> Menu - Join another server. ");
        g_ErrorReport.WriteCurrentTime();

        SaveOptions();
        SaveMacro(L"Data\\Macro.txt");

        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY))
        {
            g_pSystemLogBox->AddText(I18N::Game::ExitGameAfterClosingTheChaosInterface, mu::ui::window::TYPE_ERROR_MESSAGE);
        }
        else
        {
            MUHelper::g_MuHelper.TriggerStop();
            g_pNewUIMng->ResetActiveUIObj();
            LogOut = true;
            SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToServerSelection);
            g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 2");
        }

        if (!Hero->PK) g_pNewUIHotKey->SetStateGameOver(true);
    };
    cfg.buttons.push_back(std::move(btnChooseServer));

    GenericMenuConfig::MenuButton btnChooseCharacter;
    btnChooseCharacter.label = I18N::Game::SwitchCharacter;
    btnChooseCharacter.onClick = []
    {
        View_End_Result = false;
        Suc_Or_Fail = -1;
        M34CryWolf1st::CryWolfMVPInit();

        g_ErrorReport.Write(L"> Menu - Join with another character. ");
        g_ErrorReport.WriteCurrentTime();

        SaveOptions();
        SaveMacro(L"Data\\Macro.txt");

        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY))
        {
            g_pSystemLogBox->AddText(I18N::Game::ExitGameAfterClosingTheChaosInterface, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        }
        else
        {
            MUHelper::g_MuHelper.TriggerStop();
            g_pNewUIMng->ResetActiveUIObj();
            LogOut = true;
            SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToCharacterSelection);
            g_ConsoleDebug->Write(MCD_SEND, L"0xF1 [SendRequestLogOut] 1");
        }

        if (!Hero->PK) g_pNewUIHotKey->SetStateGameOver(true);
    };
    cfg.buttons.push_back(std::move(btnChooseCharacter));

    GenericMenuConfig::MenuButton btnOption;
    btnOption.label = I18N::Game::Option385;
    btnOption.onClick = [] { g_pNewUISystem->Show(mu::ui::window::INTERFACE_OPTION); };
    cfg.buttons.push_back(std::move(btnOption));

    GenericMenuConfig::MenuButton btnCancel;
    btnCancel.label = I18N::Game::Cancel;
    // No onClick -- native's own CancelBtnDown has no side effect beyond closing the box.
    btnCancel.dismiss = true;
    cfg.buttons.push_back(std::move(btnCancel));

    g_pGenericMenuDialog->Show(std::move(cfg));
}

mu::ui::window::CBloodCastleResultMsgBox::CBloodCastleResultMsgBox()
{
}

mu::ui::window::CBloodCastleResultMsgBox::~CBloodCastleResultMsgBox()
{
}

bool mu::ui::window::CBloodCastleResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(mu::ui::window::CBloodCastleResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    x = (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    m_View.Create(static_cast<int>(MIDDLE_COUNT), "blood-castle-result");

    return true;
}

void mu::ui::window::CBloodCastleResultMsgBox::Release()
{
    m_View.Destroy();
    CMessageBoxBase::Release();
}

bool mu::ui::window::CBloodCastleResultMsgBox::Update()
{
    if (TakeMatchResultOk(m_View, this))
        return true;

    if (m_View.IsShown())
        SyncMatchResultView(m_View);
    return true;
}

bool mu::ui::window::CBloodCastleResultMsgBox::Render()
{
    // Nothing native left: MessageBoxView draws the box. Kept because the base requires it.
    return true;
}

CALLBACK_RESULT mu::ui::window::CBloodCastleResultMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::CDevilSquareRankMsgBox::CDevilSquareRankMsgBox()
{
}

mu::ui::window::CDevilSquareRankMsgBox::~CDevilSquareRankMsgBox()
{
}

bool mu::ui::window::CDevilSquareRankMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(mu::ui::window::CDevilSquareRankMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT1 * MSGBOX_MIDDLE_HEIGHT)
        + (MIDDLE_COUNT2 * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_LINE_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    x = (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);

    // RenderFrame(): 11 middle strips, the divider, 3 more; the table's four rules are the theme's.
    const auto middles = static_cast<int>(MIDDLE_COUNT1 + MIDDLE_COUNT2);
    m_View.Create(middles, "devil-square-rank");
    m_View.SetFrame(middles, static_cast<int>(MIDDLE_COUNT1));

    return true;
}

void mu::ui::window::CDevilSquareRankMsgBox::Release()
{
    m_View.Destroy();
    CMessageBoxBase::Release();
}

bool mu::ui::window::CDevilSquareRankMsgBox::Update()
{
    if (TakeMatchResultOk(m_View, this))
        return true;

    if (m_View.IsShown())
        SyncMatchResultView(m_View);
    return true;
}

bool mu::ui::window::CDevilSquareRankMsgBox::Render()
{
    // Nothing native left: MessageBoxView draws the box. Kept because the base requires it.
    return true;
}

CALLBACK_RESULT mu::ui::window::CDevilSquareRankMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

mu::ui::window::CChaosCastleResultMsgBox::CChaosCastleResultMsgBox()
{
}

mu::ui::window::CChaosCastleResultMsgBox::~CChaosCastleResultMsgBox()
{
}

bool mu::ui::window::CChaosCastleResultMsgBox::Create(float fPriority)
{
    int x, y, width, height;

    AddCallbackFunc(mu::ui::window::CChaosCastleResultMsgBox::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);

    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    x = (GetSize().cx / 2) - (MSGBOX_BTN_WIDTH / 2);
    y = GetSize().cy - (MSGBOX_BTN_HEIGHT + MSGBOX_BTN_BOTTOM_BLANK);
    width = MSGBOX_BTN_WIDTH;
    height = MSGBOX_BTN_HEIGHT;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height, CMessageBoxButton::MSGBOX_BTN_SIZE_OK);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_BtnOk.SetInfo(CMessageBoxMng::IMAGE_MSGBOX_BTN_OK, x, y, width, height);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    m_View.Create(static_cast<int>(MIDDLE_COUNT), "chaos-castle-result");

    return true;
}

void mu::ui::window::CChaosCastleResultMsgBox::Release()
{
    m_View.Destroy();
    CMessageBoxBase::Release();
}

bool mu::ui::window::CChaosCastleResultMsgBox::Update()
{
    if (TakeMatchResultOk(m_View, this))
        return true;

    if (m_View.IsShown())
        SyncMatchResultView(m_View);
    return true;
}

bool mu::ui::window::CChaosCastleResultMsgBox::Render()
{
    // Nothing native left: MessageBoxView draws the box. Kept because the base requires it.
    return true;
}

CALLBACK_RESULT mu::ui::window::CChaosCastleResultMsgBox::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    return CALLBACK_BREAK;
}

//////////////////////////////////////////////////////////////////////////

void mu::ui::window::ShowChaosMixMenuDialog()
{
    wchar_t szText[256] = { 0, };

    GenericMenuConfig cfg;
    // Native CChaosMixMenuMsgBox: lines 15 apart, each recipe's lines above its button.
    cfg.kind = "chaos-mix";
    cfg.title = I18N::Game::SelectMethodOfCombination;
    cfg.highlightTitle = true;
    // No shared summary lines -- native interleaves each recipe's own blurb directly above its
    // button instead of grouping all the body text above the whole list; see MenuButton::lines.

    GenericMenuConfig::MenuButton btnGeneralMix;
    btnGeneralMix.label = I18N::Game::RegularCombination;
    mu_swprintf(szText, I18N::Game::Wings7TypesFruitDevilSInvitation);
    btnGeneralMix.lines.push_back({ szText, false });
    mu_swprintf(szText, I18N::Game::Dinorant1015ItemsCloakOfInvisibility, Hero->ID);
    btnGeneralMix.lines.push_back({ szText, false });
    mu_swprintf(szText, I18N::Game::FenrirSHornScrollOfBloodCondorSFeather, Hero->ID);
    btnGeneralMix.lines.push_back({ szText, false });
    btnGeneralMix.onClick = [] { g_MixRecipeMgr.SetMixType(0); };
    cfg.buttons.push_back(std::move(btnGeneralMix));

    GenericMenuConfig::MenuButton btnChaosMix;
    btnChaosMix.label = I18N::Game::ChaosWeaponCombination;
    mu_swprintf(szText, I18N::Game::ChaosDragonAxeChaosLightningStaff, Hero->ID);
    btnChaosMix.lines.push_back({ szText, false });
    mu_swprintf(szText, I18N::Game::ChaosNatureBow, Hero->ID);
    btnChaosMix.lines.push_back({ szText, false });
    btnChaosMix.onClick = [] { g_MixRecipeMgr.SetMixType(1); };
    cfg.buttons.push_back(std::move(btnChaosMix));

    GenericMenuConfig::MenuButton btnMix380;
    btnMix380.label = I18N::Game::ItemOptionCombination;
    mu_swprintf(szText, I18N::Game::Add380ItemOption, Hero->ID);
    btnMix380.lines.push_back({ szText, false });
    btnMix380.onClick = [] { g_MixRecipeMgr.SetMixType(2); };
    cfg.buttons.push_back(std::move(btnMix380));

    // Native's own CancelBtnDown side effect, mirrored onto Esc too (cfg.onCancel below) -- this
    // dialog sits on top of an already-open MixInventory, so just closing it without also
    // clearing/hiding that would leave it stuck half-configured.
    auto cancelFn = []
    {
        g_MixRecipeMgr.ClearCheckRecipeResult();
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    GenericMenuConfig::MenuButton btnCancel;
    btnCancel.label = I18N::Game::Close388;
    btnCancel.compact = true;
    btnCancel.onClick = cancelFn;
    cfg.buttons.push_back(std::move(btnCancel));
    cfg.onCancel = cancelFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

mu::ui::window::CProgressMsgBox::CProgressMsgBox()
{
    m_dwStartTime = 0;
    m_dwEndTime = 0;
    m_dwElapseTime = 0;
}

mu::ui::window::CProgressMsgBox::~CProgressMsgBox()
{
    Release();
}

bool mu::ui::window::CProgressMsgBox::Create(DWORD dwElapseTime, float fPriority)
{
    int width, height;

    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    SetAddCallbackFunc();
    m_View.Create(0, "progress");

    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;

    SetCanMove(true);

    return true;
}

void mu::ui::window::CProgressMsgBox::Release()
{
    m_View.Destroy();
}

void mu::ui::window::CProgressMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CProgressMsgBox::ClosingProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
}

void mu::ui::window::CProgressMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
    }
}

void mu::ui::window::CProgressMsgBox::SetElapseTime(DWORD dwElapseTime)
{
    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;
}

int mu::ui::window::CProgressMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    return AppendWrappedMessageLines(strMsg, byFontType, static_cast<int>(MSGBOX_TEXT_MAXWIDTH),
        [&](const std::wstring& line)
        {
            auto* message = new MSGBOX_TEXTDATA;
            message->strMsg = line;
            message->dwColor = dwColor;
            message->byFontType = byFontType;
            m_MsgDataList.push_back(message);
        });
}

bool mu::ui::window::CProgressMsgBox::Update()
{
    g_pMainFrame->UpdateItemHotKey();

    DWORD dwTime = timeGetTime();
    if (dwTime >= m_dwEndTime)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    }

    SyncView();
    return true;
}

bool mu::ui::window::CProgressMsgBox::Render()
{
    // Nothing native left: MessageBoxView draws the box. Kept because the base requires it.
    return true;
}

void mu::ui::window::CProgressMsgBox::SyncView()
{
    SyncProgressView(m_View, m_MsgDataList, m_dwStartTime, m_dwElapseTime);
}

CALLBACK_RESULT mu::ui::window::CProgressMsgBox::ClosingProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_CONTINUE;
}

mu::ui::window::CCursedTempleProgressMsgBox::CCursedTempleProgressMsgBox()
{
    m_dwStartTime = 0;
    m_dwEndTime = 0;
    m_dwElapseTime = 0;
}

mu::ui::window::CCursedTempleProgressMsgBox::~CCursedTempleProgressMsgBox()
{
    Release();
}

bool mu::ui::window::CCursedTempleProgressMsgBox::Create(DWORD dwElapseTime, float fPriority)
{
    int width, height;

    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    SetAddCallbackFunc();
    m_View.Create(0, "progress");

    m_dwElapseTime = dwElapseTime;
    m_dwStartTime = timeGetTime();
    m_dwEndTime = m_dwStartTime + m_dwElapseTime;

    SetCanMove(true);

    return true;
}

void mu::ui::window::CCursedTempleProgressMsgBox::Release()
{
    m_View.Destroy();
}

void mu::ui::window::CCursedTempleProgressMsgBox::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CCursedTempleProgressMsgBox::ClosingProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    AddCallbackFunc(mu::ui::window::CCursedTempleProgressMsgBox::CompleteProcess, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS);
}

void mu::ui::window::CCursedTempleProgressMsgBox::AddMsg(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    int iOrigSize = m_MsgDataList.size();
    int iLine = SeparateText(strMsg, dwColor, byFontType);
    int iSize = m_MsgDataList.size();

    if (iSize > 2)
    {
        float height = GetSize().cy;

        if (iOrigSize < 2)
        {
            iLine = iLine + iOrigSize - 2;
        }

        height += (MSGBOX_MIDDLE_HEIGHT * iLine);
        SetSize(GetSize().cx, height);
    }
}

int mu::ui::window::CCursedTempleProgressMsgBox::SeparateText(const type_string& strMsg, DWORD dwColor, BYTE byFontType)
{
    return AppendWrappedMessageLines(strMsg, byFontType, static_cast<int>(MSGBOX_TEXT_MAXWIDTH),
        [&](const std::wstring& line)
        {
            auto* message = new MSGBOX_TEXTDATA;
            message->strMsg = line;
            message->dwColor = dwColor;
            message->byFontType = byFontType;
            m_MsgDataList.push_back(message);
        });
}

bool mu::ui::window::CCursedTempleProgressMsgBox::Update()
{
    g_pMainFrame->UpdateItemHotKey();

    DWORD dwTime = timeGetTime();
    if (dwTime >= m_dwEndTime)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_COMPLETEPROCESS);
    }

    if (CheckHeroAction() == false || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_GAMESYSTEM) == false)
    {
        g_MessageBox->SendEvent(this, MSGBOX_EVENT_USER_CUSTOM_PROGRESS_CLOSINGPROCESS);
    }

    if (m_View.IsShown())
        SyncView();
    return true;
}

void mu::ui::window::CCursedTempleProgressMsgBox::SyncView()
{
    SyncProgressView(m_View, m_MsgDataList, m_dwStartTime, m_dwElapseTime);
}

void mu::ui::window::CCursedTempleProgressMsgBox::SetNpcIndex(DWORD dwIndex)
{
    m_dwNpcIndex = dwIndex;
}

DWORD mu::ui::window::CCursedTempleProgressMsgBox::GetNpcIndex()
{
    return m_dwNpcIndex;
}

CALLBACK_RESULT mu::ui::window::CCursedTempleProgressMsgBox::ClosingProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    g_CursedTemple->SetGaugebarEnabled(false);
    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CCursedTempleProgressMsgBox::CompleteProcess(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    auto* pMsgBox = dynamic_cast<CCursedTempleProgressMsgBox*>(pOwner);
    if (pMsgBox == nullptr)
    {
        return CALLBACK_CONTINUE;
    }

    SocketClient->ToGameServer()->SendTalkToNpcRequest(pMsgBox->GetNpcIndex());

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);
    g_CursedTemple->SetGaugebarCloseTimer();
    return CALLBACK_CONTINUE;
}

bool mu::ui::window::CCursedTempleProgressMsgBox::Render()
{
    // Nothing native left: MessageBoxView draws the box. Kept because the base requires it.
    return true;
}

bool mu::ui::window::CCursedTempleProgressMsgBox::CheckHeroAction()
{
    if (g_isCharacterBuff((&Hero->Object), eDeBuff_Harden)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_Stun)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_CursedTempleRestraint)
        || g_isCharacterBuff((&Hero->Object), eDeBuff_Sleep)
        )
    {
        return false;
    }

    int action = Hero->Object.CurrentAction;

    if (!(action >= PLAYER_SET && action <= PLAYER_STOP_RIDE_WEAPON)
        && !(action == PLAYER_SHOCK)
        && !(action == PLAYER_FENRIR_STAND)
        && !(action == PLAYER_FENRIR_STAND_TWO_SWORD)
        && !(action == PLAYER_FENRIR_STAND_ONE_RIGHT)
        && !(action == PLAYER_FENRIR_STAND_ONE_LEFT)
        && !(action == PLAYER_DARKLORD_STAND)
        && !(action == PLAYER_STOP_RIDE_HORSE)
        && !(action == PLAYER_ATTACK_STRIKE)
        && !(action == PLAYER_STOP_TWO_HAND_SWORD_TWO)
        && !(action >= PLAYER_RAGE_FENRIR_STAND && action <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
        && !(action == PLAYER_RAGE_UNI_STOP_ONE_RIGHT))
    {
        return false;
    }

    return true;
}

// CDuelMsgBox/CDuelResultMsgBox are now CGenericConfirmDialog's portrait2D field.

void mu::ui::window::ShowCherryBlossomMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "cherry-blossom";

    wchar_t title[256];
    mu_swprintf(title, L"%ls", MonsterScript[450].Name);
    cfg.title = title;
    cfg.lines.push_back({ I18N::Game::GoldenCherryBlossomsBranches, true });

    // Native's own 3 color buttons are no-ops beyond closing the box (WhiteCBBtnDown/RedCBBtnDown/
    // GodCBBtnDown never set a mix type or open an interface) -- faithfully ported as-is, not a
    // bug introduced here. Combined with there being no live CreateMessageBox() call site for this
    // class either (see WindowCommon.h's own comment), this dialog looks like unfinished/parked
    // native content, not a working feature.
    GenericMenuConfig::MenuButton btnWhite;
    btnWhite.label = I18N::Game::Lookup(2542);
    cfg.buttons.push_back(std::move(btnWhite));

    GenericMenuConfig::MenuButton btnRed;
    btnRed.label = I18N::Game::Lookup(2543);
    cfg.buttons.push_back(std::move(btnRed));

    GenericMenuConfig::MenuButton btnGold;
    btnGold.label = I18N::Game::GoldenCherryBlossomsBranches;
    cfg.buttons.push_back(std::move(btnGold));

    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    cfg.buttons.push_back(std::move(btnExit));

    g_pGenericMenuDialog->Show(std::move(cfg));
}

// CGemIntegrationMsgBox/CGemIntegrationUnityMsgBox ported as 3 chained free functions -- see
// WindowCommon.h's own comment on ShowGemIntegrationMenuDialog() for why. COMGEM
// (GameLogic/Items/CComGem.h) is the shared state; every onClick below reads/writes it exactly
// like the native handlers it replaces.
void mu::ui::window::ShowGemIntegrationMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "gem-integration";
    cfg.lines.push_back({I18N::Game::JewelCombination, true, RGBA(255, 128, 0, 255)});
    cfg.lines.push_back({ I18N::Game::YouCanCombineOrDissolve, false });
    cfg.lines.push_back({ I18N::Game::VariousJewels, false });

    auto exitFn = [] { COMGEM::Exit(); };

    GenericMenuConfig::MenuButton btnUnity;
    btnUnity.label = I18N::Game::JewelCombination;
    btnUnity.narrow = true;
    btnUnity.onClick = []
    {
        COMGEM::SetMode(COMGEM::ATTACH);
        mu::ui::window::ShowGemIntegrationJewelDialog();
    };
    cfg.buttons.push_back(std::move(btnUnity));

    GenericMenuConfig::MenuButton btnDisjoint;
    btnDisjoint.label = I18N::Game::DismantleJewel;
    btnDisjoint.narrow = true;
    btnDisjoint.onClick = []
    {
        COMGEM::SetMode(COMGEM::DETACH);
        if (!COMGEM::FindWantedList())
        {
            // Same check native's own DisjointBtnDown makes -- nothing eligible to dismantle.
            // Native just logs and leaves its dialog open; re-Show()-ing this same config from
            // inside the click (the same reentrant-queue mechanism ShowTrainerMenuDialog()/
            // ShowTrainerRecoverDialog() already prove) reproduces that faithfully, since
            // CGenericMenuDialog buttons always close on click.
            g_pSystemLogBox->AddText(I18N::Game::CanTBeDismantled, mu::ui::window::TYPE_ERROR_MESSAGE);
            mu::ui::window::ShowGemIntegrationMenuDialog();
            return;
        }
        // CGemIntegrationDisjointMsgBox stays native -- embedded live inventory list-selection
        // widget, out of scope for this primitive.
        mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGemIntegrationDisjointMsgBoxLayout));
    };
    cfg.buttons.push_back(std::move(btnDisjoint));

    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

void mu::ui::window::ShowGemIntegrationJewelDialog()
{
    GenericMenuConfig cfg;
    cfg.lines.push_back({I18N::Game::JewelCombination, true, RGBA(255, 128, 0, 255)});
    cfg.lines.push_back({ I18N::Game::SelectAJewelToCombine, false });
    // 2-column grid instead of `compact` -- some jewel names ("Higher Refining Stone") don't fit
    // compact's 64dp width even wrapped onto 2 lines; `columns` is a general per-dialog knob (see
    // its own comment, GenericMenuDialog.h) that sizes buttons to fit N per row and supports
    // wrapped labels, decoupled from `compact`'s own "small Close/Cancel button" meaning.
    cfg.columns = 2;

    int nBtnIndex[COMGEM::eGEMTYPE_END] = { 1806, 1807, 3312, 3313, 3314, 2081, 3315, 3316, 3317, 3318 };
    for (int i = 0; i < (int)COMGEM::eGEMTYPE_END; ++i)
    {
        GenericMenuConfig::MenuButton btn;
        btn.label = I18N::Game::Lookup(nBtnIndex[i]);
        btn.onClick = [i]
        {
            COMGEM::SetGem(i * 2);
            mu::ui::window::ShowGemIntegrationMixDialog();
        };
        cfg.buttons.push_back(std::move(btn));
    }

    auto exitFn = [] { COMGEM::Exit(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

void mu::ui::window::ShowGemIntegrationMixDialog()
{
    GenericMenuConfig cfg;
    cfg.lines.push_back({I18N::Game::JewelCombination, true, RGBA(255, 128, 0, 255)});
    cfg.lines.push_back({ I18N::Game::ChooseANumberButtonToCombine, false });

    wchar_t szTemp[256] = { 0, };
    for (int k = 0; k < (int)COMGEM::eCOMTYPE_END; ++k)
    {
        mu_swprintf(szTemp, I18N::Game::CombineDDZenIsRequired, 10 * (k + 1), 500000 * (k + 1));

        GenericMenuConfig::MenuButton btn;
        btn.label = szTemp;
        btn.onClick = [k]
        {
            COMGEM::m_cComType = COMGEM::GetJewelRequireCount(k);
            if (!COMGEM::CheckInv())
            {
                // CheckInv() already logged the error and called COMGEM::GetBack() (resets
                // m_cGemType to NOGEM) -- reopen the jewel-type grid to match that state, instead
                // of leaving the player on a mix-amount grid for a jewel type that's no longer
                // selected (what native's own SelectMixBtnDown literally does).
                mu::ui::window::ShowGemIntegrationJewelDialog();
                return;
            }

            wchar_t strText[256] = { 0, };
            mu::ui::window::GenericDialogConfig confirmCfg;
            confirmCfg.showCancel = true;
            mu_swprintf(strText, I18N::Game::Lookup(COMGEM::GetJewelIndex(COMGEM::m_cGemType, 0)), I18N::Game::JewelOfSoul, COMGEM::m_cCount);
            confirmCfg.lines.push_back({ strText, true });
            mu_swprintf(strText, I18N::Game::CombinationCostDZen, COMGEM::m_iValue);
            confirmCfg.lines.push_back({ strText, true });
            confirmCfg.onPrimary = [] { COMGEM::ProcessCSAction(); COMGEM::Exit(); };
            confirmCfg.onCancel = []
            {
                COMGEM::GetBack();
                mu::ui::window::ShowGemIntegrationJewelDialog();
            };
            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(confirmCfg));
        };
        cfg.buttons.push_back(std::move(btn));
    }

    auto exitFn = [] { COMGEM::Exit(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

bool mu::ui::window::CGemIntegrationDisjointMsgBoxLayout::SetLayout()
{
    CGemIntegrationDisjointMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CBloodCastleResultMsgBoxLayout::SetLayout()
{
    CBloodCastleResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CDevilSquareRankMsgBoxLayout::SetLayout()
{
    CDevilSquareRankMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CChaosCastleResultMsgBoxLayout::SetLayout()
{
    CChaosCastleResultMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CCrownSwitchPopLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::CrownSwitchHasBeenReleased);

    return true;
}

bool mu::ui::window::CCrownSwitchPushLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::CrownSwitchHasBeenActivated);

    return true;
}

bool mu::ui::window::CCrownSwitchOtherPushLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    return true;
}

bool mu::ui::window::CSealRegisterStartLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

bool mu::ui::window::CSealRegisterSuccessLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::OfficialSealRegistrationIsSuccessful);

    return true;
}

bool mu::ui::window::CSealRegisterFailLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    return true;
}

bool mu::ui::window::CSealRegisterOtherLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::AnotherCharacterIsRegisteringTheOfficialSeal);

    return true;
}

bool mu::ui::window::CSealRegisterOtherCampLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::OtherSiegeTeamIsRunningTheCrownSwitch);

    return true;
}

bool mu::ui::window::CCrownDefenseRemoveLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::ShieldOfTheCrownHasBeenRemoved);

    return true;
}

bool mu::ui::window::CCrownDefenseCreateLayout::SetLayout()
{
    CProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(3000))
        return false;

    pMsgBox->AddMsg(I18N::Game::ShieldOfTheCrownHasBeenActivated);

    return true;
}

bool mu::ui::window::CCursedTempleHolicItemGetLayout::SetLayout()
{
    CCursedTempleProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(10000))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouAreCurrentGainingTheSacredItem);

    return true;
}

bool mu::ui::window::CCursedTempleHolicItemSaveLayout::SetLayout()
{
    CCursedTempleProgressMsgBox* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create(10000))
        return false;

    pMsgBox->AddMsg(I18N::Game::YouAreCurrentlyStoringTheSacredItem);

    return true;
}

void mu::ui::window::ShowLuckyTradeMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "npc-menu";
    // Hardcoded Korean, no I18N constant -- faithfully carried over from native's own literal.
    cfg.title = L"럭키아이템 교환NPC"; // "LuckyItem Trade NPC"
    cfg.lines.push_back({ L"럭키아이템으로 교환하거나 제련할 수 있습니?", false });

    auto exitFn = [] { SocketClient->ToGameServer()->SendCraftingDialogCloseRequest(); };

    GenericMenuConfig::MenuButton btnTrade;
    btnTrade.label = L"럭키아이템 교환"; // "GlobalText"
    btnTrade.onClick = [] { g_pLuckyItemWnd->SetAct(eLuckyItemType_Trade); g_pNewUISystem->Show(mu::ui::window::INTERFACE_LUCKYITEMWND); };
    cfg.buttons.push_back(std::move(btnTrade));

    GenericMenuConfig::MenuButton btnRefinery;
    btnRefinery.label = L"럭키아이템 제련"; // "GlobalText"
    btnRefinery.onClick = [] { g_pLuckyItemWnd->SetAct(eLuckyItemType_Refinery); g_pNewUISystem->Show(mu::ui::window::INTERFACE_LUCKYITEMWND); };
    cfg.buttons.push_back(std::move(btnRefinery));

    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

//////////////////////////////////////////////////////////////////////////

void mu::ui::window::ShowTrainerMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "npc-menu";
    cfg.title = I18N::Game::Trainer;
    cfg.lines.push_back({ I18N::Game::Hi, false });

    wchar_t szText[256] = { 0, };
    mu_swprintf(szText, I18N::Game::SWhatIsYourCommand, Hero->ID);
    cfg.lines.push_back({ szText, false });

    GenericMenuConfig::MenuButton btnRecover;
    btnRecover.label = I18N::Game::RestoreLifeDurability;
    btnRecover.onClick = [] { mu::ui::window::ShowTrainerRecoverDialog(); };
    cfg.buttons.push_back(std::move(btnRecover));

    GenericMenuConfig::MenuButton btnRevive;
    btnRevive.label = I18N::Game::ResurrectSpirit;
    btnRevive.onClick = []
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_TRAINER);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    cfg.buttons.push_back(std::move(btnRevive));

    auto exitFn = [] { SocketClient->ToGameServer()->SendCloseNpcRequest(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

void mu::ui::window::ShowTrainerRecoverDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "trainer-recover";
    cfg.title = I18N::Game::Trainer;
    cfg.lines.push_back({ I18N::Game::SelectThePetToRecoverLife, false });

    // Native positions each pet's own recovery-cost/status sentence (from CalcRecoveryZen)
    // directly under that pet's own button, in its muted gold -- attached to each MenuButton's own
    // lines (linesBelow) so it renders in the same place, rather than bunched above the list.
    const unsigned long statusColor = RGBA(206, 192, 146, 255);
    wchar_t costText[100] = { 0, };
    auto exitFn = [] { SocketClient->ToGameServer()->SendCloseNpcRequest(); };

    GenericMenuConfig::MenuButton btnDarkHorse;
    btnDarkHorse.label = I18N::Game::DarkHorse;
    npcBreeder::CalcRecoveryZen(REVIVAL_DARKHORSE, costText);
    btnDarkHorse.lines.push_back({costText, false, statusColor});
    btnDarkHorse.linesBelow = true;
    btnDarkHorse.onClick = [] { npcBreeder::RecoverPet(REVIVAL_DARKHORSE); SocketClient->ToGameServer()->SendCloseNpcRequest(); };
    cfg.buttons.push_back(std::move(btnDarkHorse));

    GenericMenuConfig::MenuButton btnDarkSpirit;
    btnDarkSpirit.label = I18N::Game::DarkRaven;
    npcBreeder::CalcRecoveryZen(REVIVAL_DARKSPIRIT, costText);
    btnDarkSpirit.lines.push_back({costText, false, statusColor});
    btnDarkSpirit.linesBelow = true;
    btnDarkSpirit.onClick = [] { npcBreeder::RecoverPet(REVIVAL_DARKSPIRIT); SocketClient->ToGameServer()->SendCloseNpcRequest(); };
    cfg.buttons.push_back(std::move(btnDarkSpirit));

    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

// ShowElpisMenuDialog() (WindowCommon.h), onto CGenericMenuDialog. The "About Refinery"/"About
// Jewel of Harmony" buttons never change the button set, only which info blurb shows above it
// (m_iMessageType-driven) -- so this is just the same 4-button config re-Show()n with different
// `lines`, reusing the same reentrant-Show()-during-click chaining
// ShowGemIntegrationJewelDialog()/ShowGemIntegrationMixDialog() use.
void mu::ui::window::ShowElpisMenuDialog(int iMessageType)
{
    GenericMenuConfig cfg;
    // Native CElpisMsgBox: bold gold text from y+43 (the "about" texts left-aligned), a divider at
    // y+120, buttons at y+145/175/205. Native wraps the about texts 30 in from each side and
    // scrolls them; shown whole here, they wrap 16 in (the frame's inner edge) so they stay above
    // the divider at every size, also where the text is relatively larger (800x600, 1280x720):
    // the theme's "elpis-about".
    const unsigned long textColor = RGBA(220, 183, 131, 255);
    cfg.kind = "elpis";
    cfg.title = I18N::Game::Elpis;

    switch (iMessageType)
    {
    case MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY:
        cfg.lines.push_back({I18N::Game::GemstoneOfJewelOfHarmonyHas, true, textColor});
        cfg.kind = "elpis-about";
        break;
    case MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY:
        cfg.lines.push_back({I18N::Game::NewPowerCanBeGrantedTo, true, textColor});
        cfg.kind = "elpis-about";
        break;
    default:
        cfg.lines.push_back({I18N::Game::WhatWouldYouLikeToKnow, true, textColor});
        break;
    }

    GenericMenuConfig::MenuButton btnAboutRefinary;
    btnAboutRefinary.label = I18N::Game::AboutRefinery;
    btnAboutRefinary.onClick = [] { ShowElpisMenuDialog(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_REFINARY); };
    cfg.buttons.push_back(std::move(btnAboutRefinary));

    GenericMenuConfig::MenuButton btnAboutJewel;
    btnAboutJewel.label = I18N::Game::JewelOfHarmony;
    btnAboutJewel.onClick = [] { ShowElpisMenuDialog(MSGBOX_EVENT_USER_CUSTOM_ELPIS_ABOUT_JEWELOFHARMONY); };
    cfg.buttons.push_back(std::move(btnAboutJewel));

    GenericMenuConfig::MenuButton btnRefine;
    btnRefine.label = I18N::Game::RefineGemstone;
    btnRefine.onClick = []
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_ELPIS);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    cfg.buttons.push_back(std::move(btnRefine));

    auto exitFn = [] { SocketClient->ToGameServer()->SendCraftingDialogCloseRequest(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

void mu::ui::window::ShowSeedMasterMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "npc-menu";
    cfg.title = I18N::Game::SeedMaster;
    cfg.lines.push_back({ I18N::Game::ExtractTheSeedOrTheSeedSphere, false });
    cfg.lines.push_back({ I18N::Game::YouMayAssemblyThemTogether, false });

    GenericMenuConfig::MenuButton btnExtract;
    btnExtract.label = I18N::Game::SeedExtraction;
    btnExtract.onClick = []
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_EXTRACT_SEED);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    cfg.buttons.push_back(std::move(btnExtract));

    GenericMenuConfig::MenuButton btnSphere;
    btnSphere.label = I18N::Game::SeedSphereAssembly;
    btnSphere.onClick = []
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_SEED_SPHERE);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    cfg.buttons.push_back(std::move(btnSphere));

    auto exitFn = [] { SocketClient->ToGameServer()->SendCraftingDialogCloseRequest(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

void mu::ui::window::ShowSeedInvestigatorMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "npc-menu";
    cfg.title = I18N::Game::SeedResearcher;
    cfg.lines.push_back({ I18N::Game::EitherApplyTheSeedSphere, false });
    cfg.lines.push_back({ I18N::Game::OrDestroyTheSeedSphereAccordingly, false });

    GenericMenuConfig::MenuButton btnAttach;
    btnAttach.label = I18N::Game::SeedSphereApplication;
    btnAttach.onClick = []
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_ATTACH_SOCKET);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    cfg.buttons.push_back(std::move(btnAttach));

    GenericMenuConfig::MenuButton btnDetach;
    btnDetach.label = I18N::Game::SeedSphereDestruction;
    btnDetach.onClick = []
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_DETACH_SOCKET);
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
    };
    cfg.buttons.push_back(std::move(btnDetach));

    auto exitFn = [] { SocketClient->ToGameServer()->SendCraftingDialogCloseRequest(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}

void mu::ui::window::ShowResetCharacterPointDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "reset-points";
    cfg.title = I18N::Game::ReInitializationHelper;
    cfg.lines.push_back({ I18N::Game::ClickOnTheButtonToReinitializeAllStatPoints, false });

    GenericMenuConfig::MenuButton btnReset;
    btnReset.label = I18N::Game::StatReInitialization; // "스탯 초기화"
    btnReset.onClick = []
    {
        for (int i = 0; i < MAX_EQUIPMENT; i++)
        {
            if (CharacterMachine->Equipment[i].Type != -1)
            {
                g_pSystemLogBox->AddText(I18N::Game::TheAppliedEquipmentsCannotBeReset, mu::ui::window::TYPE_ERROR_MESSAGE);
                return;
            }
        }
        SocketClient->ToGameServer()->SendResetCharacterPointRequest();
    };
    cfg.buttons.push_back(std::move(btnReset));

    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    // No onClick -- native's own ExitBtnDown has no side effect beyond closing the box.
    cfg.buttons.push_back(std::move(btnExit));

    g_pGenericMenuDialog->Show(std::move(cfg));
}

mu::ui::window::CGuild_ToPerson_Position::CGuild_ToPerson_Position()
{
    COMGEM::m_cGemType = COMGEM::CELE;
    AppointType = SUBGUILDMASTER;
}

mu::ui::window::CGuild_ToPerson_Position::~CGuild_ToPerson_Position()
{
    Release();
}

bool mu::ui::window::CGuild_ToPerson_Position::Create(float fPriority)
{
    int width, height;

    SetAddCallbackFunc();

    width = MSGBOX_WIDTH;
    height = MSGBOX_TOP_HEIGHT + (MIDDLE_COUNT * MSGBOX_MIDDLE_HEIGHT) + MSGBOX_BOTTOM_HEIGHT;

    CMessageBoxBase::Create(width, height, fPriority);

    // The original's RenderFrame(): five middle strips, the back 75 units shorter than the box.
    m_View.Create(5, "guild-appoint");

    return true;
}

void mu::ui::window::CGuild_ToPerson_Position::Release()
{
    m_View.Destroy();
    CMessageBoxBase::Release();
}

bool mu::ui::window::CGuild_ToPerson_Position::Update()
{
    // A button RmlUi reported (the original's LButtonUp() checks), sent as the box's event.
    static constexpr DWORD kButtonEvents[] = {MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING,
                                              MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL, MSGBOX_EVENT_USER_COMMON_OK,
                                              MSGBOX_EVENT_USER_COMMON_CANCEL};
    const int pressed = m_View.TakePressedButton();
    if (pressed >= 0 && pressed < static_cast<int>(std::size(kButtonEvents)))
        g_MessageBox->SendEvent(this, kButtonEvents[pressed]);

    // The original's RenderButtons()/RenderTexts(): the appointment chosen (the assistant by
    // default) sets AppointType and its line, then the question, bold (255, 128, 0), centred
    // from y 97 one line height + 4 apart.
    std::vector<std::wstring> texts;
    wchar_t strText[256] = {};
    if (COMGEM::m_cGemType == COMGEM::CELE)
    {
        mu_swprintf(strText, I18N::Game::SAsAS, GuildList[DeleteIndex].Name, I18N::Game::AssistM);
        AppointType = SUBGUILDMASTER;
        texts.push_back(strText);
    }
    if (COMGEM::m_cGemType == COMGEM::SOUL)
    {
        mu_swprintf(strText, I18N::Game::SAsAS, GuildList[DeleteIndex].Name, I18N::Game::BattleM);
        AppointType = BATTLEMASTER;
        texts.push_back(strText);
    }
    texts.push_back(I18N::Game::DoYouWantToAppoint);

    std::vector<MessageBoxView::Line> lines;
    for (const std::wstring& text : texts)
        lines.push_back({"message", {{text, true, RGBA(255, 128, 0, 255)}}});

    // The two appointments, OK and Close.
    const std::vector<MessageBoxView::Button> buttons = {
        {I18N::Game::AppointAsAssistantGuildMaster},
        {I18N::Game::AppointAsABattleMaster},
        {I18N::Game::OK},
        {I18N::Game::Close388},
    };
    m_View.Sync(lines, buttons);

    return true;
}

bool mu::ui::window::CGuild_ToPerson_Position::Render()
{
    // Nothing native left: MessageBoxView draws the box. Kept because the base requires it.
    return true;
}

void mu::ui::window::CGuild_ToPerson_Position::SetAddCallbackFunc()
{
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::BlessingBtnDown,
                    MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_BLESSING);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::SoulBtnDown, MSGBOX_EVENT_USER_CUSTOM_GEM_UNITY_SOUL);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::OkBtnDown, MSGBOX_EVENT_USER_COMMON_OK);
    AddCallbackFunc(mu::ui::window::CGuild_ToPerson_Position::CancelBtnDown, MSGBOX_EVENT_USER_COMMON_CANCEL);
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::BlessingBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::CELE);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::SoulBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::SetGem(COMGEM::SOUL);

    return CALLBACK_CONTINUE;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::OkBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();
    SocketClient->ToGameServer()->SendGuildRoleAssignRequest(
        AppointType,
        MU_C16(GuildList[DeleteIndex].Name),
        AppointType == G_PERSON ? 0x01 : 0x02);

    SocketClient->ToGameServer()->SendGuildListRequest();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

CALLBACK_RESULT mu::ui::window::CGuild_ToPerson_Position::CancelBtnDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam)
{
    COMGEM::Exit();

    PlayBuffer(SOUND_CLICK01);
    g_MessageBox->SendEvent(pOwner, MSGBOX_EVENT_DESTROY);

    return CALLBACK_BREAK;
}

bool mu::ui::window::CGuild_ToPerson_PositionLayout::SetLayout()
{
    CGuild_ToPerson_Position* pMsgBox = GetMsgBox();
    if (0 == pMsgBox)
        return false;

    if (false == pMsgBox->Create())
        return false;

    return true;
}

void mu::ui::window::ShowDelgardoMainMenuDialog()
{
    GenericMenuConfig cfg;
    cfg.kind = "delgardo";
    cfg.title = I18N::Game::Delgado;
    cfg.lines.push_back({ I18N::Game::RegisterYourLuckyCoinsOr, false });
    cfg.lines.push_back({ I18N::Game::UseTheLuckyCoinsYouAlreadyHave, false });
    cfg.lines.push_back({ I18N::Game::AndExchangeThemForItems, false });

    GenericMenuConfig::MenuButton btnReg;
    btnReg.label = I18N::Game::LuckyCoinRegistration;
    btnReg.onClick = [] { g_pNewUISystem->Show(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION); };
    cfg.buttons.push_back(std::move(btnReg));

    GenericMenuConfig::MenuButton btnExchange;
    btnExchange.label = I18N::Game::LuckyCoinExchange;
    btnExchange.onClick = [] { g_pNewUISystem->Show(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN); };
    cfg.buttons.push_back(std::move(btnExchange));

    auto exitFn = [] { SocketClient->ToGameServer()->SendCraftingDialogCloseRequest(); };
    GenericMenuConfig::MenuButton btnExit;
    btnExit.label = I18N::Game::Close388;
    btnExit.compact = true;
    btnExit.onClick = exitFn;
    cfg.buttons.push_back(std::move(btnExit));
    cfg.onCancel = exitFn;

    g_pGenericMenuDialog->Show(std::move(cfg));
}
