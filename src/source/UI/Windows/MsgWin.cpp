//*****************************************************************************
// File: MsgWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/MsgWin.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/ServerSelWin.h"
#include "UI/Windows/LoginWin.h"
#include "Character/CharMakeWin.h"
#include "Character/CharSelMainWin.h"
#include "Character/CharInfoBalloonMng.h"
#include "Core/Platform/CrtDbg.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/AI/GOBoid.h"
#include "Scenes/SceneCore.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "UI/Widgets/UIControls.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneCommon.h"
#include "Core/Utilities/Log/ErrorReport.h"
#include "Core/Globals/_enum.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#define	MW_OK		0
#define	MW_CANCEL	1

extern int g_iChatInputType;

CMsgWin g_MsgWin;

CMsgWin::CMsgWin()
{
}

CMsgWin::~CMsgWin()
{
    Release();
}

void CMsgWin::Create()
{
    Release();

    m_sprBack.Create(352, 113, BITMAP_MESSAGE_WIN);

    m_sprInput.Create(171, 23, BITMAP_MSG_WIN_INPUT);

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
    }

    memset(m_aszMsg[0], 0, sizeof(char) * MW_MSG_LINE_MAX * MW_MSG_ROW_MAX);

    m_eType = MWT_NON;
    m_nMsgLine = 0;
    m_nMsgCode = -1;
    m_nGameExit = -1;
    m_dDeltaTickSum = 0.0;

    // RmlUi migration -- see this class's header comment. Guarded like every other hybrid
    // window's Create() (CSceneUICoordinator::RepositionSceneUI() re-runs Create() on resolution change), so
    // the document/model are created once, ever.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "msg_win",
            [this](Rml::DataModelConstructor& c, MsgWinRmlModel& model)
            {
                c.Bind("line1", &model.line1);
                c.Bind("line2", &model.line2);
                c.Bind("line2_hidden", &model.line2Hidden);
                c.Bind("no_buttons", &model.noButtons);
                c.Bind("mode_cancel_only", &model.modeCancelOnly);
                c.Bind("mode_ok_only", &model.modeOkOnly);
                c.Bind("mode_both", &model.modeBoth);
                c.Bind("mode_input", &model.modeInput);
                c.Bind("ok_label", &model.okLabel);
                c.Bind("cancel_label", &model.cancelLabel);

                c.BindEventCallback("msgwin_ok_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOk(); });
                c.BindEventCallback("msgwin_cancel_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCancel(); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/msg_win.rml");
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_MSG_WINDOW, this);
    Show(false);
}

void CMsgWin::Release()
{
    m_aBtn[0].Release();
    m_aBtn[1].Release();
    m_sprInput.Release();
    m_sprBack.Release();

    // See CLoginWin::PreRelease()'s identical comment -- each migrated window's Release() is called
    // explicitly at every scene transition, not swept automatically by any shared list, and this
    // class has no base-class knowledge of m_pRmlDoc.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CMsgWin::SetPosition(int nXCoord, int nYCoord)
{
    m_sprBack.SetPosition(nXCoord, nYCoord);
    SetCtrlPosition();
}

void CMsgWin::SetCtrlPosition()
{
    int nBaseXPos = m_sprBack.GetXPos();
    int nBtnYPos = m_sprBack.GetYPos() + 72;

    switch (m_eType)
    {
    case MWT_BTN_CANCEL:
        m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 149, nBtnYPos);
        break;
    case MWT_BTN_OK:
        m_aBtn[MW_OK].SetPosition(nBaseXPos + 149, nBtnYPos);
        break;
    case MWT_BTN_BOTH:
        m_aBtn[MW_OK].SetPosition(nBaseXPos + 98, nBtnYPos);
        m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 200, nBtnYPos);
        break;
    case MWT_STR_INPUT:
        m_sprInput.SetPosition(nBaseXPos + 32, nBtnYPos + 4);
        m_aBtn[MW_OK].SetPosition(nBaseXPos + 209, nBtnYPos);
        m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 264, nBtnYPos);
        if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
            if (g_iChatInputType == 1)
                // Real pixels, not divided by g_fScreenRate_x/y -- CUITextInputBox::SetPosition()
                // stores this raw and only rescales it at Render() time via ConvertPositionX/Y's
                // *ambient* active transform (UIControls.cpp/ZzzOpenglUtil.cpp). The old division
                // here relied on that ambient transform being the same one active when this ran --
                // true when this only ever ran unscoped (CWin days), but this call chain now
                // reaches here through ManageOKClick()->PopUp()->SetMsg(), inside CMsgWin::Update()
                // itself, which CNewUIManager::Update() wraps in a LayoutMode::Legacy (identity)
                // ScopedActiveTransform -- dividing by that identity is a no-op, so the position
                // got stored as real pixels while still being *labeled* reference-space, then
                // rescaled a second time by RenderTextOnTop()'s own (correctly ambient,
                // non-identity) transform at render time. Storing real pixels directly and
                // forcing identity at the one consuming Render() call (below) instead keeps both
                // sides consistent regardless of which context triggered this.
                g_pSinglePasswdInputBox->SetPosition(m_sprInput.GetXPos() + 10, m_sprInput.GetYPos() + 8);
        break;
    }
}

void CMsgWin::Show(bool bShow)
{
    mu::ui::window::CNewUIObj::Show(bShow);

    m_sprBack.Show(bShow);

    switch (m_eType)
    {
    case MWT_BTN_CANCEL:
        m_aBtn[MW_OK].Show(false);
        m_aBtn[MW_CANCEL].Show(bShow);
        m_sprInput.Show(false);
        break;
    case MWT_BTN_OK:
        m_aBtn[MW_OK].Show(bShow);
        m_aBtn[MW_CANCEL].Show(false);
        m_sprInput.Show(false);
        break;
    case MWT_BTN_BOTH:
        m_aBtn[MW_OK].Show(bShow);
        m_aBtn[MW_CANCEL].Show(bShow);
        m_sprInput.Show(false);
        break;
    case MWT_STR_INPUT:
        m_aBtn[MW_OK].Show(bShow);
        m_aBtn[MW_CANCEL].Show(bShow);
        m_sprInput.Show(bShow);
        break;
    default:
        m_aBtn[MW_OK].Show(false);
        m_aBtn[MW_CANCEL].Show(false);
        m_sprInput.Show(false);
    }

    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

bool CMsgWin::Update()
{
    if (!IsVisible())
        return true;

    m_aBtn[MW_OK].Update();
    m_aBtn[MW_CANCEL].Update();

    if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT && g_iChatInputType == 1 &&
        g_pSinglePasswdInputBox != nullptr && g_pSinglePasswdInputBox->GetState() == UISTATE_NORMAL)
    {
        g_pSinglePasswdInputBox->DoAction();
    }

    CInput& rInput = CInput::Instance();

    // dDeltaTick previously threaded through from CWin::UpdateWhileActive(double); this window's
    // migrated Update() takes no parameters, same as every other CNewUIObj window -- read the
    // same clamped expression CSceneUICoordinator::Update(dDeltaTick) itself resolves to in steady state (see
    // docs/newui-legacy-merger.md's g_pTimer gotcha).
    extern float FPS_ANIMATION_FACTOR;
    const double dDeltaTick = 200.0 * static_cast<double>(FPS_ANIMATION_FACTOR);

    if (rInput.IsKeyDown(VK_RETURN))
    {
        if (m_eType > MWT_BTN_CANCEL)
        {
            ::PlayBuffer(SOUND_CLICK01);
            ManageOKClick();
        }
        else if (m_eType == MWT_BTN_CANCEL)
        {
            ::PlayBuffer(SOUND_CLICK01);
            ManageCancelClick();
        }
    }
    else if (rInput.IsKeyDown(VK_ESCAPE))
    {
        if (m_eType == MWT_BTN_OK)
        {
            ::PlayBuffer(SOUND_CLICK01);
            ManageOKClick();
        }
        else if (m_eType > MWT_NON)
        {
            ::PlayBuffer(SOUND_CLICK01);
            ManageCancelClick();
        }
    }
    else if (m_aBtn[MW_OK].IsClick() || m_bRmlOkClicked)
    {
        m_bRmlOkClicked = false;
        ManageOKClick();
    }
    else if (m_aBtn[MW_CANCEL].IsClick() || m_bRmlCancelClicked)
    {
        m_bRmlCancelClicked = false;
        ManageCancelClick();
    }
    else if (m_nMsgCode == MESSAGE_GAME_END_COUNTDOWN)
    {
        if (m_nGameExit != -1)
        {
            m_dDeltaTickSum += dDeltaTick;
            if (m_dDeltaTickSum > 1000.0)
            {
                m_dDeltaTickSum = 0.0;
                if (--m_nGameExit == 0)
                {
                    g_ErrorReport.Write(L"> Menu - Exit game.");
                    g_ErrorReport.WriteCurrentTime();
                    ::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
                }
                else
                {
                    wchar_t szMsg[64]{};
                    mu_swprintf(szMsg, I18N::Game::YouWillExitGameInDSeconds, m_nGameExit);
                    SetMsg(m_eType, szMsg, L"");
                }
            }
        }
    }

    return true;
}

bool CMsgWin::Render()
{
    // RmlUi's #panel now owns 100% of this dialog's visuals (background frame, message text,
    // OK/Cancel, the resident-password input frame's background) in every theme -- see this
    // class's header comment. The legacy CSprite/CButton objects stay alive purely as bookkeeping
    // (redundant click detection), never rendered. SyncRmlModel() is the only thing this override
    // still needs to do; RenderTextOnTop() (the resident-password live text) is called from
    // Winmain.cpp's SetPostRmlUiCallback instead of here, so it isn't drawn twice.
    SyncRmlModel();
    return true;
}

void CMsgWin::RenderTextOnTop()
{
    if (m_nMsgCode != MESSAGE_DELETE_CHARACTER_RESIDENT)
        return;

    if (g_iChatInputType == 1)
    {
        // Force identity to match SetCtrlPosition()'s now-real-pixel SetPosition() call (see its
        // own comment) -- this runs unscoped (Winmain.cpp's post-RmlUi callback), so without this
        // it would rescale by whatever the ambient/default transform happens to be instead.
        const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Legacy, WindowWidth, WindowHeight);
        UI::Scaling::ScopedActiveTransform identity(transform);
        g_pSinglePasswdInputBox->Render();
    }
    else if (g_iChatInputType == 0)
    {
        InputTextWidth = 100;
        ::RenderInputText(
            int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
            int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y), 0, 0);
        InputTextWidth = 256;
    }
}

void CMsgWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto syncLabel = [this](Rml::String MsgWinRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncBool = [this](bool MsgWinRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };

    syncLabel(&MsgWinRmlModel::line1, "line1", m_nMsgLine > 0 ? m_aszMsg[0] : L"");
    syncLabel(&MsgWinRmlModel::line2, "line2", m_nMsgLine > 1 ? m_aszMsg[1] : L"");
    syncBool(&MsgWinRmlModel::line2Hidden, "line2_hidden", m_nMsgLine <= 1);
    syncBool(&MsgWinRmlModel::noButtons, "no_buttons", m_eType == MWT_NON);
    syncBool(&MsgWinRmlModel::modeCancelOnly, "mode_cancel_only", m_eType == MWT_BTN_CANCEL);
    syncBool(&MsgWinRmlModel::modeOkOnly, "mode_ok_only", m_eType == MWT_BTN_OK);
    syncBool(&MsgWinRmlModel::modeBoth, "mode_both", m_eType == MWT_BTN_BOTH);
    syncBool(&MsgWinRmlModel::modeInput, "mode_input", m_eType == MWT_STR_INPUT);
    syncLabel(&MsgWinRmlModel::okLabel, "ok_label", I18N::Game::OK);
    syncLabel(&MsgWinRmlModel::cancelLabel, "cancel_label", I18N::Game::Cancel);
}

void CMsgWin::SetMsg(MSG_WIN_TYPE eType, std::wstring lpszMsg, std::wstring lpszMsg2)
{
    m_eType = eType;

    SetCtrlPosition();

    if (lpszMsg2.empty())
    {
        m_nMsgLine = ::SeparateTextIntoLines(lpszMsg.c_str(), m_aszMsg[0], MW_MSG_LINE_MAX, MW_MSG_ROW_MAX);
    }
    else
    {
        lpszMsg.copy(m_aszMsg[0], MW_MSG_ROW_MAX - 1);
        lpszMsg2.copy(m_aszMsg[1], MW_MSG_ROW_MAX - 1);
        m_nMsgLine = 2;
    }
}

void CMsgWin::PopUp(int nMsgCode, wchar_t* pszMsg)
{
    std::wstring lpszMsg = L"";
    std::wstring lpszMsg2 = L"";
    MSG_WIN_TYPE eType = MWT_BTN_OK;
    m_nMsgCode = nMsgCode;
    wchar_t szTempMsg[128];

    switch (m_nMsgCode)
    {
    case MESSAGE_FREE_MSG_NOT_BTN:
        lpszMsg = pszMsg;
        eType = MWT_NON;
        break;
    case MESSAGE_GAME_END_COUNTDOWN:
        m_nGameExit = 5;
        mu_swprintf(szTempMsg, I18N::Game::YouWillExitGameInDSeconds, m_nGameExit);
        lpszMsg = szTempMsg;
        eType = MWT_NON;
        break;
    case MESSAGE_WAIT:
        lpszMsg = I18N::Game::PleaseWait;
        eType = MWT_NON;
        break;
    case MESSAGE_SERVER_BUSY:
    case RECEIVE_LOG_IN_FAIL_SERVER_BUSY:
        lpszMsg = I18N::Game::TheServerIsFull;
        break;
    case RECEIVE_JOIN_SERVER_WAITING:
        g_ServerSelWin.Show(true);
        lpszMsg = I18N::Game::TheServerIsFull;
        break;
    case MESSAGE_SERVER_LOST:
        lpszMsg = I18N::Game::YouAreDisconnectedFromTheServer;
        break;
    case MESSAGE_VERSION:
    case RECEIVE_LOG_IN_FAIL_VERSION:
        lpszMsg = I18N::Game::NewVersionOfGameIsRequired;
        lpszMsg2 = I18N::Game::PleaseDownloadTheNewVersion;
        break;
    case MESSAGE_INPUT_ID:
        lpszMsg = I18N::Game::EnterYourAccount;
        break;
    case MESSAGE_INPUT_PASSWORD:
        lpszMsg = I18N::Game::EnterYourPassword;
        break;
    case RECEIVE_LOG_IN_FAIL_ID:
        lpszMsg = I18N::Game::YourAccountIsInvalid;
        break;
    case RECEIVE_LOG_IN_FAIL_PASSWORD:
        lpszMsg = I18N::Game::PasswordIsIncorrect;
        break;
    case RECEIVE_LOG_IN_FAIL_ID_CONNECTED:
        lpszMsg = I18N::Game::YourAccountIsAlreadyConnected;
        break;
    case RECEIVE_LOG_IN_FAIL_ID_BLOCK:
    case MESSAGE_DELETE_CHARACTER_ID_BLOCK:
        lpszMsg = I18N::Game::ThisAccountIsBlocked;
        break;
    case RECEIVE_LOG_IN_FAIL_CONNECT:
        lpszMsg = I18N::Game::ConnectionError;
        break;
    case RECEIVE_LOG_IN_FAIL_ERROR:
        lpszMsg = I18N::Game::ConnectionClosedDueTo3FailedAttempts;
        break;
    case RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO:
        lpszMsg = I18N::Game::NoChargeInfo;
        break;
    case RECEIVE_LOG_IN_FAIL_USER_TIME1:
        lpszMsg = I18N::Game::YourIndividualSubscriptionTermIsOver;
        break;
    case RECEIVE_LOG_IN_FAIL_USER_TIME2:
        lpszMsg = I18N::Game::YourIndividualSubscriptionTimeIsOver;
        break;
    case RECEIVE_LOG_IN_FAIL_PC_TIME1:
        lpszMsg = I18N::Game::SubscriptionTermIsOverOnYourIP;
        break;
    case RECEIVE_LOG_IN_FAIL_PC_TIME2:
        lpszMsg = I18N::Game::SubscriptionTimeIsOverOnYourIP;
        break;
    case RECEIVE_LOG_IN_FAIL_ONLY_OVER_15:
        lpszMsg = I18N::Game::OnlyPlayersAge18AndOverArePermittedToConnectToThisServer;
        break;
    case RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL:
        lpszMsg = I18N::Game::PleasePurchaseGoldChannelTicketToEnter;
        break;
    case RECEIVE_LOG_IN_FAIL_POINT_DATE:
        lpszMsg = I18N::Game::PointNoMoreDates;
        break;
    case RECEIVE_LOG_IN_FAIL_POINT_HOUR:
        lpszMsg = I18N::Game::PointNoMorePointsLeft;
        break;
    case RECEIVE_LOG_IN_FAIL_INVALID_IP:
        lpszMsg = I18N::Game::YourIPIsNotAllowedToConnect;
        break;
    case MESSAGE_DELETE_CHARACTER_GUILDWARNING:
        lpszMsg = I18N::Game::YouCanTDeleteTheCharacterThatBelongsToTheGuild;
        break;
    case MESSAGE_DELETE_CHARACTER_WARNING:
        mu_swprintf(szTempMsg, I18N::Game::CharacterLevelAboveDCannotBeDeleted, CHAR_DEL_LIMIT_LV);
        lpszMsg = szTempMsg;
        break;
    case MESSAGE_DELETE_CHARACTER_CONFIRM:
        mu_swprintf(szTempMsg, I18N::Game::WouldYouLikeToDeleteSCharacter, CharactersClient[SelectedHero].ID);
        lpszMsg = szTempMsg;
        eType = MWT_BTN_BOTH;
        break;
    case MESSAGE_DELETE_CHARACTER_RESIDENT:
        lpszMsg = I18N::Game::PleaseEnterYourWEBZENCOMPassword;
        eType = MWT_STR_INPUT;
        InitResidentNumInput();
        break;
    case MESSAGE_DELETE_CHARACTER_ITEM_BLOCK:
        lpszMsg = I18N::Game::TheCharacterIsItemBlocked;
        break;
    case MESSAGE_STORAGE_RESIDENTWRONG:
        lpszMsg = I18N::Game::ThePasswordYouHaveEnteredIsIncorrect;
        break;
    case MESSAGE_DELETE_CHARACTER_SUCCESS:
        CharactersClient[SelectedHero].Object.Live = false;
        DeleteMount(&CharactersClient[SelectedHero].Object);
        SelectedHero = -1;
        g_CharSelMainWin.UpdateDisplay();
        g_CharInfoBalloonMng.UpdateDisplay();
        lpszMsg = I18N::Game::CharacterWasDeletedSuccessfully;
        break;
    case MESSAGE_BLOCKED_CHARACTER:
        lpszMsg = I18N::Game::ThisIsABlockedCharacter;
        break;
    case MESSAGE_MIN_LENGTH:
        lpszMsg = I18N::Game::TypeMoreThan4Letters;
        break;
    case MESSAGE_ID_SPACE_ERROR:
        lpszMsg = I18N::Game::ItContainsProhibitedWords;
        break;
    case MESSAGE_SPECIAL_NAME:
        lpszMsg = I18N::Game::CannotUseSymbols;
        break;
    case RECEIVE_CREATE_CHARACTER_FAIL:
        g_CharMakeWin.Show(true);
        lpszMsg = I18N::Game::IncorrectCharacterNameWasEnteredOrSameCharacterNameExists;
        break;
    case RECEIVE_CREATE_CHARACTER_FAIL2:
        g_CharMakeWin.Show(true);
        lpszMsg = I18N::Game::NoMoreCharactersCanBeCreated;
        break;
    default:
        m_nMsgCode = -1;
        return;
    }

    SetMsg(eType, lpszMsg, lpszMsg2);
    Show(true);
}

void CMsgWin::ManageOKClick()
{
    Show(false);

    switch (m_nMsgCode)
    {
    case RECEIVE_LOG_IN_FAIL_VERSION:
    case MESSAGE_SERVER_LOST:
        ::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
        break;
    case MESSAGE_VERSION:
    case RECEIVE_LOG_IN_FAIL_ERROR:
    case MESSAGE_INPUT_ID:
    case RECEIVE_LOG_IN_FAIL_ID:
    case RECEIVE_LOG_IN_FAIL_ID_CONNECTED:
    case RECEIVE_LOG_IN_FAIL_SERVER_BUSY:
    case RECEIVE_LOG_IN_FAIL_ID_BLOCK:
    case RECEIVE_LOG_IN_FAIL_CONNECT:
    case RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO:
    case RECEIVE_LOG_IN_FAIL_USER_TIME1:
    case RECEIVE_LOG_IN_FAIL_USER_TIME2:
    case RECEIVE_LOG_IN_FAIL_PC_TIME1:
    case RECEIVE_LOG_IN_FAIL_PC_TIME2:
    case RECEIVE_LOG_IN_FAIL_ONLY_OVER_15:
    case RECEIVE_LOG_IN_FAIL_POINT_DATE:
    case RECEIVE_LOG_IN_FAIL_POINT_HOUR:
    case RECEIVE_LOG_IN_FAIL_INVALID_IP:
    case RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL:
        g_LoginWin.Show(true);
        g_LoginWin.GetUsernameInputBox()->GiveFocus(TRUE);
        CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
        break;
    case MESSAGE_INPUT_PASSWORD:
    case RECEIVE_LOG_IN_FAIL_PASSWORD:
        g_LoginWin.Show(true);
        g_LoginWin.GetPasswordInputBox()->GiveFocus(TRUE);
        CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
        break;
    case MESSAGE_DELETE_CHARACTER_CONFIRM:
        PopUp(MESSAGE_DELETE_CHARACTER_RESIDENT);
        break;
    case MESSAGE_DELETE_CHARACTER_RESIDENT:
        RequestDeleteCharacter();
        PopUp(MESSAGE_WAIT);
        break;
    }
}

void CMsgWin::ManageCancelClick()
{
    if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT && g_iChatInputType == 1 &&
        g_pSinglePasswdInputBox != nullptr)
    {
        g_pSinglePasswdInputBox->SetText(NULL);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
    }

    m_nMsgCode = -1;
    Show(false);
}

void CMsgWin::InitResidentNumInput()
{
    ::ClearInput();
    InputEnable = true;
    InputNumber = 1;
    InputTextMax[0] = g_iLengthAuthorityCode;
    InputTextHide[0] = 1;

    if (g_iChatInputType == 1)
    {
        g_pSinglePasswdInputBox->SetState(UISTATE_NORMAL);
        g_pSinglePasswdInputBox->SetOption(UIOPTION_NULL);
        g_pSinglePasswdInputBox->SetBackColor(0, 0, 0, 0);
        g_pSinglePasswdInputBox->SetTextLimit(20);
        g_pSinglePasswdInputBox->SetText(NULL);
        g_pSinglePasswdInputBox->GiveFocus();
    }
}

void CMsgWin::RequestDeleteCharacter()
{
    if (g_iChatInputType == 1)
    {
        g_pSinglePasswdInputBox->GetText(InputText[0]);
        g_pSinglePasswdInputBox->SetText(NULL);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
    }
    InputEnable = false;
    CurrentProtocolState = REQUEST_DELETE_CHARACTER;
    SocketClient->ToGameServer()->SendDeleteCharacter(MU_C16(CharactersClient[SelectedHero].ID), MU_C16(InputText[0]));
}
