#include "stdafx.h"
#include "NewUIChatInputBox.h"

#include "DSPlaySound.h"
#include "NewUIChatLogWindow.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "ZzzOpenData.h"
#include "MapManager.h"
#include "ZzzInterface.h"

using namespace SEASON3B;

SEASON3B::CNewUIChatInputBox::CNewUIChatInputBox()
{
    Init();
}

SEASON3B::CNewUIChatInputBox::~CNewUIChatInputBox()
{
    Release();
}

void SEASON3B::CNewUIChatInputBox::Init()
{
    m_pNewUIMng = nullptr;
    m_pNewUIChatLogWnd = nullptr;
    m_pNewUISystemLogWnd = nullptr;
    m_pChatInputBox = nullptr;
    m_pWhsprIDInputBox = nullptr;
    m_WndPos = {};
    m_WndSize = {};
    m_WndPos.x = m_WndPos.y = 0;
    m_WndSize.cx = m_WndSize.cy = 0;
    m_iCurChatHistory = 0;
    m_iCurWhisperIDHistory = 0;

    m_iTooltipType = INPUT_TOOLTIP_NOTHING;
    m_iInputMsgType = INPUT_CHAT_MESSAGE;
    m_bBlockWhisper = false;
    m_bShowSystemMessages = true;
    m_bShowChatLog = true;
    m_bWhisperSend = true;

    m_bShowMessageElseNormal = false;
}

void SEASON3B::CNewUIChatInputBox::LoadImages()
{
    LoadBitmap(L"Interface\\newui_chat_back.jpg", IMAGE_INPUTBOX_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_normal_on.jpg", IMAGE_INPUTBOX_NORMAL_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_party_on.jpg", IMAGE_INPUTBOX_PARTY_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_guild_on.jpg", IMAGE_INPUTBOX_GUILD_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_gens_on.jpg", IMAGE_INPUTBOX_GENS_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_whisper_on.jpg", IMAGE_INPUTBOX_WHISPER_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_system_on.jpg", IMAGE_INPUTBOX_SYSTEM_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_chat_on.jpg", IMAGE_INPUTBOX_CHATLOG_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_frame_on.jpg", IMAGE_INPUTBOX_FRAME_ON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_btn_size.jpg", IMAGE_INPUTBOX_BTN_SIZE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_chat_btn_alpha.jpg", IMAGE_INPUTBOX_BTN_TRANSPARENCY, GL_LINEAR);
}

void SEASON3B::CNewUIChatInputBox::UnloadImages()
{
    DeleteBitmap(IMAGE_INPUTBOX_BTN_TRANSPARENCY);
    DeleteBitmap(IMAGE_INPUTBOX_BTN_SIZE);
    DeleteBitmap(IMAGE_INPUTBOX_FRAME_ON);
    DeleteBitmap(IMAGE_INPUTBOX_SYSTEM_ON);
    DeleteBitmap(IMAGE_INPUTBOX_CHATLOG_ON);
    DeleteBitmap(IMAGE_INPUTBOX_WHISPER_ON);
    DeleteBitmap(IMAGE_INPUTBOX_GUILD_ON);
    DeleteBitmap(IMAGE_INPUTBOX_GENS_ON);
    DeleteBitmap(IMAGE_INPUTBOX_PARTY_ON);
    DeleteBitmap(IMAGE_INPUTBOX_NORMAL_ON);
    DeleteBitmap(IMAGE_INPUTBOX_BACK);
}

bool SEASON3B::CNewUIChatInputBox::Create(
    CNewUIManager* pNewUIMng,
    CNewUIChatLogWindow* pNewUIChatLogWnd,
    CNewUISystemLogWindow* pNewUISystemLogWnd,
    int x,
    int y)
{
    Release();

    if (nullptr == pNewUIChatLogWnd || nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CHATINPUTBOX, this);

    m_pNewUIChatLogWnd = pNewUIChatLogWnd;
    m_pNewUISystemLogWnd = pNewUISystemLogWnd;
    SetWndPos(x, y);

    m_pChatInputBox = new CUITextInputBox;
    m_pChatInputBox->Init(g_hWnd, 176, 14, MAX_CHAT_SIZE - 1);
    m_pChatInputBox->SetPosition(m_WndPos.x + 72, m_WndPos.y + 32);
    m_pChatInputBox->SetTextColor(255, 255, 230, 210);
    m_pChatInputBox->SetBackColor(0, 0, 0, 25);
    m_pChatInputBox->SetFont(g_hFont);
    m_pChatInputBox->SetState(UISTATE_HIDE);

    m_pWhsprIDInputBox = new CUITextInputBox;
    m_pWhsprIDInputBox->Init(g_hWnd, 50, 14, 10);
    m_pWhsprIDInputBox->SetPosition(m_WndPos.x + 5, m_WndPos.y + 32);
    m_pWhsprIDInputBox->SetSize(50, 14);
    m_pWhsprIDInputBox->SetTextColor(255, 200, 200, 200);
    m_pWhsprIDInputBox->SetBackColor(0, 0, 0, 25);
    m_pWhsprIDInputBox->SetFont(g_hFont);
    m_pWhsprIDInputBox->SetState(UISTATE_HIDE);

    m_pChatInputBox->SetTabTarget(m_pWhsprIDInputBox);
    m_pWhsprIDInputBox->SetTabTarget(m_pChatInputBox);

    LoadImages();

    SetInputMsgType(INPUT_CHAT_MESSAGE);

    SetButtonInfo();

    Show(false);

    return true;
}

void SEASON3B::CNewUIChatInputBox::Release()
{
    UnloadImages();

    RemoveAllChatHIstory();
    RemoveAllWhsprIDHIstory();

    SAFE_DELETE(m_pChatInputBox);
    SAFE_DELETE(m_pWhsprIDInputBox);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    Init();
}

void SEASON3B::CNewUIChatInputBox::SetButtonInfo()
{
    m_BtnSize.ChangeButtonImgState(true, IMAGE_INPUTBOX_BTN_SIZE, false);
    m_BtnSize.ChangeButtonInfo(m_WndPos.x + FRAME_RESIZE_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT);

    m_BtnTransparency.ChangeButtonImgState(true, IMAGE_INPUTBOX_BTN_TRANSPARENCY, false);
    m_BtnTransparency.ChangeButtonInfo(m_WndPos.x + TRANSPARENCY_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT);
}

void SEASON3B::CNewUIChatInputBox::SetWndPos(int x, int y)
{
    m_WndPos.x = x; m_WndPos.y = y;
    m_WndSize.cx = CHATBOX_WIDTH; m_WndSize.cy = CHATBOX_HEIGHT;

    if (m_pChatInputBox && m_pWhsprIDInputBox)
    {
        m_pChatInputBox->SetPosition(m_WndPos.x + 72, m_WndPos.y + 32);
        m_pWhsprIDInputBox->SetPosition(m_WndPos.x + 5, m_WndPos.y + 32);
    }
}

void SEASON3B::CNewUIChatInputBox::SetInputMsgType(int iInputMsgType)
{
    m_iInputMsgType = iInputMsgType;
}

int SEASON3B::CNewUIChatInputBox::GetInputMsgType() const
{
    return m_iInputMsgType;
}

void SEASON3B::CNewUIChatInputBox::SetFont(HFONT hFont)
{
    m_pChatInputBox->SetFont(hFont);
    m_pWhsprIDInputBox->SetFont(hFont);
}

bool SEASON3B::CNewUIChatInputBox::HaveFocus()
{
    return (m_pChatInputBox->HaveFocus() || m_pWhsprIDInputBox->HaveFocus());
}

void SEASON3B::CNewUIChatInputBox::AddChatHistory(const type_string& strText)
{
    auto vi = std::find(m_vecChatHistory.begin(), m_vecChatHistory.end(), strText);
    if (vi != m_vecChatHistory.end())
        m_vecChatHistory.erase(vi);
    else if (m_vecChatHistory.size() > 12)
        m_vecChatHistory.erase(m_vecChatHistory.begin());
    m_vecChatHistory.push_back(strText);
}

void SEASON3B::CNewUIChatInputBox::RemoveChatHistory(int index)
{
    if (index >= 0 && index < (int)m_vecChatHistory.size())
        m_vecChatHistory.erase(m_vecChatHistory.begin() + index);
}

void SEASON3B::CNewUIChatInputBox::RemoveAllChatHIstory()
{
    m_vecChatHistory.clear();
}

void SEASON3B::CNewUIChatInputBox::AddWhsprIDHistory(const type_string& strWhsprID)
{
    auto vi = std::find(m_vecWhsprIDHistory.begin(), m_vecWhsprIDHistory.end(), strWhsprID);
    if (vi != m_vecWhsprIDHistory.end())
        m_vecWhsprIDHistory.erase(vi);
    else if (m_vecWhsprIDHistory.size() > 5)
        m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin());
    m_vecWhsprIDHistory.push_back(strWhsprID);
}

void SEASON3B::CNewUIChatInputBox::RemoveWhsprIDHistory(int index)
{
    if (index >= 0 && index < (int)m_vecWhsprIDHistory.size())
    {
        m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin() + index);
    }
}

void SEASON3B::CNewUIChatInputBox::RemoveAllWhsprIDHIstory()
{
    m_vecWhsprIDHistory.clear();
}

bool SEASON3B::CNewUIChatInputBox::IsBlockWhisper()
{
    return m_bBlockWhisper;
}

void SEASON3B::CNewUIChatInputBox::SetBlockWhisper(bool bBlockWhisper)
{
    m_bBlockWhisper = bBlockWhisper;
}

bool SEASON3B::CNewUIChatInputBox::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
    {
        return true;
    }

    auto const releaseMouse = SEASON3B::IsRelease(VK_LBUTTON);

    if (SelectedCharacter >= 0 && (IsVisible() && releaseMouse))
    {
        auto const character = &CharactersClient[SelectedCharacter];

        if (character->Object.Kind == KIND_PLAYER
            && !gMapManager.InChaosCastle()
            && !(::IsStrifeMap(gMapManager.WorldActive)
                && Hero->m_byGensInfluence != character->m_byGensInfluence))
        {
            SetWhsprID(character->ID);
        }
    }

    m_iTooltipType = INPUT_TOOLTIP_NOTHING;

    int iSelectedInputType = -1;
    for (int i = 0; i < INPUT_MESSAGE_TYPE_COUNT; ++i)
    {
        if (CheckMouseIn(m_WndPos.x + (i * BUTTON_WIDTH), m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
        {
            iSelectedInputType = i;
            m_iTooltipType = INPUT_TOOLTIP_NORMAL + i;
        }
    }

    if (iSelectedInputType > -1 && releaseMouse)
    {
        SetInputMsgType(INPUT_CHAT_MESSAGE + iSelectedInputType);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (CheckMouseIn(m_WndPos.x + BLOCK_WHISPER_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
    {
        m_iTooltipType = INPUT_TOOLTIP_WHISPER;
        if (releaseMouse)
        {
            m_bBlockWhisper = !m_bBlockWhisper;
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }


    if (CheckMouseIn(m_WndPos.x + SYSTEM_ON_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
    {
        m_iTooltipType = INPUT_TOOLTIP_SYSTEM;
        if (releaseMouse)
        {
            m_bShowSystemMessages = !m_bShowSystemMessages;

            if (m_bShowSystemMessages)
            {
                m_pNewUISystemLogWnd->ShowMessages();
            }
            else
            {
                m_pNewUISystemLogWnd->HideMessages();
            }

            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (CheckMouseIn(m_WndPos.x + CHATLOG_ON_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
    {
        m_iTooltipType = INPUT_TOOLTIP_CHAT;
        if (releaseMouse)
        {
            m_bShowChatLog = !m_bShowChatLog;

            if (m_bShowChatLog)
            {
                m_pNewUIChatLogWnd->ShowChatLog();
            }
            else
            {
                m_pNewUIChatLogWnd->HideChatLog();
            }

            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (CheckMouseIn(m_WndPos.x + FRAME_ON_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
    {
        m_iTooltipType = INPUT_TOOLTIP_FRAME;
        if (releaseMouse)
        {
            if (m_pNewUIChatLogWnd->IsShowFrame())
            {
                m_pNewUIChatLogWnd->HideFrame();
            }
            else
            {
                m_pNewUIChatLogWnd->ShowFrame();
            }

            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (CheckMouseIn(m_WndPos.x + FRAME_RESIZE_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
    {
        m_iTooltipType = INPUT_TOOLTIP_SIZE;
    }

    if (CheckMouseIn(m_WndPos.x + TRANSPARENCY_START_X, m_WndPos.y, BUTTON_WIDTH, BUTTON_HEIGHT))
    {
        m_iTooltipType = INPUT_TOOLTIP_TRANSPARENCY;
    }

    if (m_pNewUIChatLogWnd->IsShowFrame())
    {
        if (m_BtnSize.UpdateMouseEvent())
        {
            m_pNewUIChatLogWnd->SetSizeAuto();
            m_pNewUIChatLogWnd->UpdateWndSize();
            m_pNewUIChatLogWnd->UpdateScrollPos();
            PlayBuffer(SOUND_CLICK01);
            return false;
        }

        if (m_BtnTransparency.UpdateMouseEvent())
        {
            m_pNewUIChatLogWnd->SetBackAlphaAuto();
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return !CheckMouseIn(m_WndPos.x, m_WndPos.y, m_WndSize.cx, m_WndSize.cy);
}

bool SEASON3B::CNewUIChatInputBox::UpdateKeyEvent()
{
    if (SEASON3B::IsPress(VK_F2))
    {
        m_bShowMessageElseNormal = !m_bShowMessageElseNormal;

        if (m_bShowMessageElseNormal)
        {
            m_pNewUIChatLogWnd->ChangeMessage(SEASON3B::TYPE_WHISPER_MESSAGE);
        }
        else
        {
            m_pNewUIChatLogWnd->ChangeMessage(SEASON3B::TYPE_ALL_MESSAGE);
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (SEASON3B::IsPress(VK_F3))
    {
        if (m_bWhisperSend == false)
        {
            m_bWhisperSend = true;
            m_pWhsprIDInputBox->SetState(UISTATE_NORMAL);
        }
        else
        {
            m_bWhisperSend = false;
            m_pWhsprIDInputBox->SetState(UISTATE_HIDE);
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
            {
                m_pChatInputBox->GiveFocus();
            }
        }

        return false;
    }

    if (m_pNewUIChatLogWnd->IsShowFrame())
    {
        if (SEASON3B::IsPress(VK_F4))
        {
            m_pNewUIChatLogWnd->SetSizeAuto();
            m_pNewUIChatLogWnd->UpdateWndSize();
            m_pNewUIChatLogWnd->UpdateScrollPos();
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (SEASON3B::IsPress(VK_F5))
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
        {
            if (m_pNewUIChatLogWnd->IsShowFrame())
            {
                m_pNewUIChatLogWnd->HideFrame();
            }
            else
            {
                m_pNewUIChatLogWnd->ShowFrame();
            }
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (m_pNewUIChatLogWnd->IsShowFrame())
    {
        if (IsPress(VK_PRIOR))
        {
            m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() - m_pNewUIChatLogWnd->GetNumberOfShowingLines());
            return false;
        }
        if (IsPress(VK_NEXT))
        {
            m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() + m_pNewUIChatLogWnd->GetNumberOfShowingLines());
            return false;
        }
    }

    if (false == IsVisible() && SEASON3B::IsPress(VK_RETURN))
    {
        if (gMapManager.InChaosCastle() == true && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME) == false)
        {
            g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
        }
        else if (gMapManager.InChaosCastle() == false)
        {
            g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
        }

        RestoreIMEStatus();
        return false;
    }

    const uint64_t currentTickCount = GetTickCount64();
    if (IsVisible() && HaveFocus() && SEASON3B::IsPress(VK_RETURN)
        && m_lastChatTime < currentTickCount - ChatCooldownMs)
    {
        m_lastChatTime = currentTickCount;
        wchar_t	szChatText[MAX_CHAT_SIZE + 1] = { '\0' };
        wchar_t	szWhisperID[MAX_ID_SIZE + 1] = { '\0' };

        m_pChatInputBox->GetText(szChatText, MAX_CHAT_SIZE);
        m_pWhsprIDInputBox->GetText(szWhisperID, MAX_ID_SIZE + 1);

        //for (int i = 0; i < MAX_CHAT_SIZE; i++)
        //    szReceivedChat[i] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szReceivedChat[i]);

        std::wstring wstrText = L"";

        if (szChatText[0] != 0x002F)
        {
            switch (m_iInputMsgType) {
            case INPUT_PARTY_MESSAGE:
                wstrText = L"~";
                break;
            case INPUT_GUILD_MESSAGE:
                wstrText = L"@";
                break;
            case INPUT_GENS_MESSAGE:
                wstrText = L"$";
                break;
            default:
                break;
            }
        }
        wstrText.append(szChatText);

        if (wcslen(szChatText) != 0)
        {
            if (!CheckCommand(szChatText))
            {
                {
                    //if (CheckAbuseFilter(szChatText))
                    //{
                    //    wstrText = GlobalText[570];
                    //}

                    if (m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL && wcslen(szChatText) && wcslen(szWhisperID) > 0)
                    {
                        SocketClient->ToGameServer()->SendWhisperMessage(szWhisperID, wstrText.c_str());
                        g_pChatListBox->AddText(Hero->ID, szChatText, SEASON3B::TYPE_WHISPER_MESSAGE);
                        AddWhsprIDHistory(szWhisperID);
                    }
                    else if (wcsncmp(szChatText, GlobalText[260], GlobalText.GetStringSize(260)) == 0)
                    {
                        wchar_t* pszMapName = szChatText + GlobalText.GetStringSize(260) + 1;
                        int iMapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(pszMapName);

                        if (g_pMoveCommandWindow->IsTheMapInDifferentServer(gMapManager.WorldActive, iMapIndex))
                        {
                            SaveOptions();
                        }

                        SocketClient->ToGameServer()->SendWarpCommandRequest(g_pMoveCommandWindow->GetMoveCommandKey(), iMapIndex);
                    }
                    else
                    {
                        if (Hero->SafeZone || (Hero->Helper.Type != MODEL_HORN_OF_UNIRIA && Hero->Helper.Type != MODEL_HORN_OF_DINORANT && Hero->Helper.Type != MODEL_DARK_HORSE_ITEM && Hero->Helper.Type != MODEL_HORN_OF_FENRIR))
                        {
                            CheckChatText(szChatText);
                        }

                        SocketClient->ToGameServer()->SendPublicChatMessage(Hero->ID, wstrText.c_str());
                        AddChatHistory(wstrText);
                    }
                }
            }
        }
        m_pChatInputBox->SetText(L"");
        m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

        SaveIMEStatus();

        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
        return false;
    }
    if (IsVisible() && m_pChatInputBox->HaveFocus())
    {
        if (SEASON3B::IsPress(VK_UP) && false == m_vecChatHistory.empty())
        {
            m_iCurChatHistory--;
            if (m_iCurChatHistory < 0)
                m_iCurChatHistory = m_vecChatHistory.size() - 1;
            m_pChatInputBox->SetText(m_vecChatHistory[m_iCurChatHistory].c_str());

            return false;
        }
        else if (SEASON3B::IsPress(VK_DOWN) && false == m_vecChatHistory.empty())
        {
            m_iCurChatHistory++;

            if (m_iCurChatHistory >= (int)m_vecChatHistory.size())
                m_iCurChatHistory = 0;

            m_pChatInputBox->SetText(m_vecChatHistory[m_iCurChatHistory].c_str());

            return false;
        }
    }

    if (IsVisible() && m_pWhsprIDInputBox->HaveFocus()
        && m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
    {
        if (SEASON3B::IsPress(VK_UP) && false == m_vecWhsprIDHistory.empty())
        {
            m_iCurWhisperIDHistory--;
            if (m_iCurWhisperIDHistory < 0)
                m_iCurWhisperIDHistory = m_vecWhsprIDHistory.size() - 1;
            m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

            return false;
        }
        else if (SEASON3B::IsPress(VK_DOWN) && false == m_vecWhsprIDHistory.empty())
        {
            m_iCurWhisperIDHistory++;

            if (m_iCurWhisperIDHistory >= (int)m_vecWhsprIDHistory.size())
                m_iCurWhisperIDHistory = 0;

            m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

            return false;
        }
    }
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);

            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIChatInputBox::Update()
{
    if (m_pChatInputBox->HaveFocus() && GetRelatedWnd() != m_pChatInputBox->GetHandle())
    {
        SetRelatedWnd(m_pChatInputBox->GetHandle());
    }
    if (m_pWhsprIDInputBox->HaveFocus() && GetRelatedWnd() != m_pWhsprIDInputBox->GetHandle())
    {
        SetRelatedWnd(m_pWhsprIDInputBox->GetHandle());
    }
    if (!HaveFocus() && GetRelatedWnd() != g_hWnd)
    {
        SetRelatedWnd(g_hWnd);
    }

    if (IsVisible() == false)
    {
        return true;
    }

    m_pChatInputBox->DoAction();

    if (m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
    {
        m_pWhsprIDInputBox->DoAction();
    }

    m_pChatInputBox->DoAction();

    if (m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
    {
        m_pWhsprIDInputBox->DoAction();
    }

    return true;
}

bool SEASON3B::CNewUIChatInputBox::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderButtons();
    RenderTooltip();
    m_pChatInputBox->Render();
    m_pWhsprIDInputBox->Render();

    if (m_bWhisperSend == false)
    {
        wchar_t szWhisperID[32];
        m_pWhsprIDInputBox->GetText(szWhisperID, 32);
        g_pRenderText->SetTextColor(255, 255, 255, 100);
        g_pRenderText->RenderText(m_pWhsprIDInputBox->GetPosition_x(), m_pWhsprIDInputBox->GetPosition_y(), szWhisperID);

        EnableAlphaTest();
        glColor4f(0.5f, 0.2f, 0.2f, 0.2f);
        RenderColor(m_WndPos.x + 2, m_WndPos.y + 28, 61, 17);
        EndRenderColor();
    }

    DisableAlphaBlend();

    return true;
}

bool SEASON3B::CNewUIChatInputBox::RenderFrame()
{
    RenderImage(IMAGE_INPUTBOX_BACK, m_WndPos.x, m_WndPos.y, CHATBOX_WIDTH, CHATBOX_HEIGHT);

    return true;
}

void SEASON3B::CNewUIChatInputBox::RenderButtons()
{
    auto windowX = static_cast<float>(m_WndPos.x);
    auto windowY = static_cast<float>(m_WndPos.y);

    RenderImage(IMAGE_INPUTBOX_NORMAL_ON + m_iInputMsgType, windowX + BUTTON_WIDTH * m_iInputMsgType, windowY, BUTTON_WIDTH, BUTTON_HEIGHT);

    if (m_bBlockWhisper)
    {
        RenderImage(IMAGE_INPUTBOX_WHISPER_ON, windowX + BLOCK_WHISPER_START_X, windowY, BUTTON_WIDTH, BUTTON_HEIGHT);
    }

    if (m_bShowSystemMessages)
    {
        RenderImage(IMAGE_INPUTBOX_SYSTEM_ON, windowX + SYSTEM_ON_START_X, windowY, BUTTON_WIDTH, BUTTON_HEIGHT);
    }

    if (m_bShowChatLog)
    {
        RenderImage(IMAGE_INPUTBOX_CHATLOG_ON, windowX + CHATLOG_ON_START_X, windowY, BUTTON_WIDTH, BUTTON_HEIGHT);
    }

    if (m_pNewUIChatLogWnd->IsShowFrame())
    {
        RenderImage(IMAGE_INPUTBOX_FRAME_ON, windowX + FRAME_ON_START_X, windowY, BUTTON_WIDTH, BUTTON_HEIGHT);

        m_BtnSize.Render();
        m_BtnTransparency.Render();
    }
}

void SEASON3B::CNewUIChatInputBox::RenderTooltip()
{
    if (m_iTooltipType == INPUT_TOOLTIP_NOTHING)
    {
        return;
    }

    wchar_t strTooltip[256];

    const int iTextIndex[10] = {
        1681, 1682, 1683, 3321,
        1684, 1685, 750, 1686, 751, 752 };

    swprintf(strTooltip, L"%s", GlobalText[iTextIndex[m_iTooltipType]]);

    SIZE fontsize;
    g_pRenderText->SetFont(g_hFont);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTooltip, wcslen(strTooltip), &fontsize);

    const auto multiplier = ((float)WindowHeight / 480);
    fontsize.cx = fontsize.cx / multiplier;
    fontsize.cy = fontsize.cy / multiplier;

    int x = m_WndPos.x
        + (m_iTooltipType * BUTTON_WIDTH)
        + (m_iTooltipType / 3 * GROUP_SEPARATING_WIDTH)
        + 10 - (fontsize.cx / 2);
    if (x < 0)
    {
        x = 0;
    }

    int y = m_WndPos.y - (fontsize.cy + 1);

    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 180);
    g_pRenderText->RenderText(x, y, strTooltip, fontsize.cx + 6, 0, RT3_SORT_CENTER);
}

float SEASON3B::CNewUIChatInputBox::GetLayerDepth()
{
    return 6.2f;
}

float SEASON3B::CNewUIChatInputBox::GetKeyEventOrder()
{
    return 9.0f;
}

void SEASON3B::CNewUIChatInputBox::OpenningProcess()
{
    m_pChatInputBox->GiveFocus();
    m_pChatInputBox->SetState(UISTATE_NORMAL);
    m_pChatInputBox->SetText(L"");

    if (m_bWhisperSend == true)
    {
        m_pWhsprIDInputBox->SetState(UISTATE_NORMAL);
    }
    else
    {
        m_pWhsprIDInputBox->SetState(UISTATE_HIDE);
    }
}

void SEASON3B::CNewUIChatInputBox::ClosingProcess()
{
    m_pNewUIChatLogWnd->HideFrame();

    m_pChatInputBox->SetState(UISTATE_HIDE);
    m_pWhsprIDInputBox->SetState(UISTATE_HIDE);

    SetFocus(g_hWnd);
}

void SEASON3B::CNewUIChatInputBox::GetChatText(type_string& strText)
{
    wchar_t szChatText[256];
    m_pChatInputBox->GetText(szChatText, 256);
    strText = szChatText;
}
void SEASON3B::CNewUIChatInputBox::GetWhsprID(type_string& strWhsprID)
{
    wchar_t szWhisperID[32];
    m_pWhsprIDInputBox->GetText(szWhisperID, 32);
    strWhsprID = szWhisperID;
}

void SEASON3B::CNewUIChatInputBox::SetWhsprID(const wchar_t* strWhsprID)
{
    m_pWhsprIDInputBox->SetText(strWhsprID);
}

void SEASON3B::CNewUIChatInputBox::SetTextPosition(int x, int y)
{
    m_pChatInputBox->SetPosition(x, y);
}

void SEASON3B::CNewUIChatInputBox::SetBuddyPosition(int x, int y)
{
    m_pWhsprIDInputBox->SetPosition(x, y);
}