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

namespace
{
    // Same cap the native password box was given (SetTextLimit(20)). Set on #msgwin_input from C++
    // so it stays one rule rather than a literal duplicated per theme.
    constexpr int kResidentPasswordMaxLength = 20;
}

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


    memset(m_aszMsg[0], 0, sizeof(char) * MW_MSG_LINE_MAX * MW_MSG_ROW_MAX);

    m_eType = MWT_NON;
    m_nMsgLine = 0;
    m_nMsgCode = -1;
    m_nGameExit = -1;
    m_dDeltaTickSum = 0.0;

    // Guarded so the document/model are created once, since Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_MSG_WINDOW, this);
    Show(false);
}

void CMsgWin::BuildRmlUi()
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
            c.Bind("password_input", &model.residentPassword);

            c.BindEventCallback("msgwin_ok_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOk(); });
            c.BindEventCallback("msgwin_cancel_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCancel(); });
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/msg_win.rml");
}

void CMsgWin::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    // See CLoginWin::ReloadRmlTheme()'s comment on why this reads m_pRmlDoc directly.
    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    if (wasVisible) { SyncRmlModel(); if (m_pRmlDoc) m_pRmlDoc->Show(); }
}

void CMsgWin::Release()
{
    m_sprBack.Release();

    // Called explicitly at each scene transition; no base-class auto-release for m_pRmlDoc.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();

    // Base-class visibility reset, NOT the full CMsgWin::Show(false) override (same reasoning as
    // CServerMsgWin::Release()/CCharMakeWin::Release()) -- without this, a message box open at the
    // exact instant of the character-select -> main-scene transition leaves IsVisible() stuck true,
    // so Winmain.cpp's post-RmlUi callback keeps calling RenderTextOnTop() (and CManager's own
    // sweep keeps calling Update()/Render()) against this already-released window every MAIN_SCENE
    // frame afterward -- a stray flicker at the message box's last position.
    mu::ui::window::CObject::Show(false);
}

void CMsgWin::SetPosition(int nXCoord, int nYCoord)
{
    m_sprBack.SetPosition(nXCoord, nYCoord);
    SetCtrlPosition();
}

void CMsgWin::SetCtrlPosition()
{
    if (m_eType != MWT_STR_INPUT)
        return;

    // Nothing left to position here: the resident-password field is an RmlUi element placed by each
    // theme's own .msgwin-input-field rule, inside #input_frame.
}

void CMsgWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    m_sprBack.Show(bShow);

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

    CInput& rInput = CInput::Instance();

    // Update() takes no parameters, so reconstruct the per-frame delta tick locally.
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
    else if (m_bRmlOkClicked)
    {
        m_bRmlOkClicked = false;
        ManageOKClick();
    }
    else if (m_bRmlCancelClicked)
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
    // RmlUi's #panel owns this dialog's visuals, the resident-password field included; m_sprBack
    // only tracks this window's position.
    SyncRmlModel();
    return true;
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

int CMsgWin::PendingMessageCode() const
{
    return IsVisible() ? m_nMsgCode : -1;
}

bool CMsgWin::DismissMessage()
{
    if (!IsVisible())
        return false;

    // Confirming these on a caller's behalf would end the process or delete
    // a character: the two exits, and the two steps of the delete-character
    // confirmation, which a second dismissal would carry through.
    if (m_nMsgCode == RECEIVE_LOG_IN_FAIL_VERSION || m_nMsgCode == MESSAGE_SERVER_LOST ||
        m_nMsgCode == MESSAGE_DELETE_CHARACTER_CONFIRM || m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
        return false;

    ManageOKClick();
    return true;
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
        g_LoginWin.FocusUsername(/*selectAll=*/true);
        CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
        break;
    case MESSAGE_INPUT_PASSWORD:
    case RECEIVE_LOG_IN_FAIL_PASSWORD:
        g_LoginWin.Show(true);
        g_LoginWin.FocusPassword(/*selectAll=*/true);
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
    if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
    {
        m_RmlBinder.GetModel().residentPassword.clear();
        m_RmlBinder.MarkDirty("password_input");
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

    m_RmlBinder.GetModel().residentPassword.clear();
    m_RmlBinder.MarkDirty("password_input");

    if (m_pRmlDoc)
    {
        if (Rml::Element* field = m_pRmlDoc->GetElementById("msgwin_input"))
        {
            field->SetAttribute("maxlength", kResidentPasswordMaxLength);
            // Explicit focus, not an autofocus attribute: this document is reused by every other
            // MSG_WIN_TYPE, and only this one mode has a field to focus.
            field->Focus();
        }
    }
}

std::wstring CMsgWin::GetResidentPasswordInput() const
{
    return StringUtils::NarrowToWide(m_RmlBinder.GetModel().residentPassword);
}

void CMsgWin::RequestDeleteCharacter()
{
    const std::wstring typed = GetResidentPasswordInput();
    wcsncpy(InputText[0], typed.c_str(), kResidentPasswordMaxLength);
    InputText[0][kResidentPasswordMaxLength] = L'\0';
    m_RmlBinder.GetModel().residentPassword.clear();
    m_RmlBinder.MarkDirty("password_input");

    InputEnable = false;
    CurrentProtocolState = REQUEST_DELETE_CHARACTER;
    SocketClient->ToGameServer()->SendDeleteCharacter(MU_C16(CharactersClient[SelectedHero].ID), MU_C16(InputText[0]));
}
