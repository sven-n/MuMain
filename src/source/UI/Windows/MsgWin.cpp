//*****************************************************************************
// File: MsgWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/MsgWin.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include <crtdbg.h>
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/AI/GOBoid.h"
#include "Scenes/SceneCore.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "UI/Legacy/UIControls.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneCommon.h"
#include "Core/Utilities/Log/ErrorReport.h"

#define	MW_OK		0
#define	MW_CANCEL	1



extern int g_iChatInputType;
extern CUITextInputBox* g_pSinglePasswdInputBox;

CMsgWin::CMsgWin()
{
}

CMsgWin::~CMsgWin()
{
}

void CMsgWin::Create()
{
    CInput rInput = CInput::Instance();

    CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

    m_sprBack.Create(352, 113, BITMAP_MESSAGE_WIN);

    m_sprInput.Create(171, 23, BITMAP_MSG_WIN_INPUT);

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    memset(m_aszMsg[0], 0, sizeof(char) * MW_MSG_LINE_MAX * MW_MSG_ROW_MAX);

    m_eType = MWT_NON;
    m_nMsgLine = 0;
    m_nMsgCode = -1;
    m_nGameExit = -1;
    m_dDeltaTickSum = 0.0;
}

void CMsgWin::PreRelease()
{
    m_sprInput.Release();
    m_sprBack.Release();
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
                g_pSinglePasswdInputBox->SetPosition(
                    int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
                    int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y));
        break;
    }
}

void CMsgWin::Show(bool bShow)
{
    CWin::Show(bShow);

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
}

bool CMsgWin::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    switch (nArea)
    {
    case WA_MOVE:
        return false;
    }

    return CWin::CursorInWin(nArea);
}

void CMsgWin::UpdateWhileActive(double dDeltaTick)
{
    CInput& rInput = CInput::Instance();

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
        CUIMng::Instance().SetSysMenuWinShow(false);
    }
    else if (m_aBtn[MW_OK].IsClick())
        ManageOKClick();
    else if (m_aBtn[MW_CANCEL].IsClick())
        ManageCancelClick();
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
}

void CMsgWin::RenderControls()
{
    m_sprBack.Render();

    int nTextPosX, nTextPosY;

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetTextColor(CLRDW_WHITE);
    g_pRenderText->SetBgColor(0);

    if (1 == m_nMsgLine)
    {
        nTextPosX = int(m_sprBack.GetXPos() / g_fScreenRate_x);
        if (MWT_NON != m_eType)
            nTextPosY = int((m_sprBack.GetYPos() + 38) / g_fScreenRate_y);
        else
            nTextPosY = int((m_sprBack.GetYPos() + 54) / g_fScreenRate_y);
        g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[0],
            m_sprBack.GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);
    }
    else if (2 == m_nMsgLine)
    {
        nTextPosX = int((m_sprBack.GetXPos() + 25) / g_fScreenRate_x);
        if (MWT_NON != m_eType)
            nTextPosY = int((m_sprBack.GetYPos() + 32) / g_fScreenRate_y);
        else
            nTextPosY = int((m_sprBack.GetYPos() + 44) / g_fScreenRate_y);
        g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[0]);

        if (MWT_NON != m_eType)
            nTextPosY = int((m_sprBack.GetYPos() + 51) / g_fScreenRate_y);
        else
            nTextPosY = int((m_sprBack.GetYPos() + 66) / g_fScreenRate_y);
        g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[1]);
    }

    m_sprInput.Render();

    if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
    {
        if (g_iChatInputType == 1)
            g_pSinglePasswdInputBox->Render();
        else if (g_iChatInputType == 0)
        {
            InputTextWidth = 100;
            ::RenderInputText(
                int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
                int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y), 0, 0);
            InputTextWidth = 256;
        }
    }

    CWin::RenderButtons();
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
    CUIMng& rUIMng = CUIMng::Instance();
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
        rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
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
        rUIMng.m_CharSelMainWin.UpdateDisplay();
        rUIMng.m_CharInfoBalloonMng.UpdateDisplay();
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
        rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
        lpszMsg = I18N::Game::IncorrectCharacterNameWasEnteredOrSameCharacterNameExists;
        break;
    case RECEIVE_CREATE_CHARACTER_FAIL2:
        rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
        lpszMsg = I18N::Game::NoMoreCharactersCanBeCreated;
        break;
    default:
        m_nMsgCode = -1;
        return;
    }

    SetMsg(eType, lpszMsg, lpszMsg2);
    rUIMng.ShowWin(this);
}

void CMsgWin::ManageOKClick()
{
    CUIMng& rUIMng = CUIMng::Instance();
    rUIMng.HideWin(this);

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
        rUIMng.ShowWin(&rUIMng.m_LoginWin);
        CUIMng::Instance().m_LoginWin.GetUsernameInputBox()->GiveFocus(TRUE);
        CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
        break;
    case MESSAGE_INPUT_PASSWORD:
    case RECEIVE_LOG_IN_FAIL_PASSWORD:
        rUIMng.ShowWin(&rUIMng.m_LoginWin);
        CUIMng::Instance().m_LoginWin.GetPasswordInputBox()->GiveFocus(TRUE);
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
    CUIMng& rUIMng = CUIMng::Instance();
    m_nMsgCode = -1;
    rUIMng.HideWin(this);
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
    SocketClient->ToGameServer()->SendDeleteCharacter(CharactersClient[SelectedHero].ID, InputText[0]);
}