

#include "stdafx.h"
#include "NewUIChatInputBox.h"
#include "NewUIChatLogWindow.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "CSChaosCastle.h"
#include "ZzzOpenData.h"
#include "MapManager.h"
using namespace SEASON3B;

SEASON3B::CNewUIChatInputBox::CNewUIChatInputBox() : MAX_CHAT_SIZE_UTF16((int)(MAX_CHAT_SIZE/(g_pMultiLanguage->GetNumByteForOneCharUTF8())))

{ 
	Init(); 
}

SEASON3B::CNewUIChatInputBox::~CNewUIChatInputBox() 
{ 
	Release(); 
}

void SEASON3B::CNewUIChatInputBox::Init()
{
	m_pNewUIMng = NULL;
	m_pNewUIChatLogWnd = NULL;
	m_WndPos.x = m_WndPos.y = 0;
	m_WndSize.cx = m_WndSize.cy = 0;
	m_pChatInputBox = m_pWhsprIDInputBox = NULL;
	m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

	m_iTooltipType = INPUT_TOOLTIP_NOTHING;
	m_iInputMsgType = INPUT_CHAT_MESSAGE;
	m_bBlockWhisper = false;
	m_bOnlySystemMessage = false;
	m_bShowChatLog = true;
	m_bWhisperSend = true;

	m_bShowMessageElseNormal = false;
}

void SEASON3B::CNewUIChatInputBox::LoadImages()
{
	LoadBitmap("Interface\\newui_chat_back.jpg", IMAGE_INPUTBOX_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_normal_on.jpg", IMAGE_INPUTBOX_NORMAL_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_party_on.jpg", IMAGE_INPUTBOX_PARTY_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_guild_on.jpg", IMAGE_INPUTBOX_GUILD_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_whisper_on.jpg", IMAGE_INPUTBOX_WHISPER_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_system_on.jpg", IMAGE_INPUTBOX_SYSTEM_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_chat_on.jpg", IMAGE_INPUTBOX_CHATLOG_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_frame_on.jpg", IMAGE_INPUTBOX_FRAME_ON, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_btn_size.jpg", IMAGE_INPUTBOX_BTN_SIZE, GL_LINEAR);
	LoadBitmap("Interface\\newui_chat_btn_alpha.jpg", IMAGE_INPUTBOX_BTN_TRANSPARENCY, GL_LINEAR);
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
	DeleteBitmap(IMAGE_INPUTBOX_PARTY_ON);
	DeleteBitmap(IMAGE_INPUTBOX_NORMAL_ON);
	DeleteBitmap(IMAGE_INPUTBOX_BACK);
}

bool SEASON3B::CNewUIChatInputBox::Create(CNewUIManager* pNewUIMng, CNewUIChatLogWindow* pNewUIChatLogWnd, int x, int y)
{
	Release();
	
	if(NULL == pNewUIChatLogWnd || NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_CHATINPUTBOX, this);
	
	m_pNewUIChatLogWnd = pNewUIChatLogWnd;
	SetWndPos(x, y);

	m_pChatInputBox = new CUITextInputBox;
	m_pChatInputBox->Init(g_hWnd, 176, 14, MAX_CHAT_SIZE_UTF16-1);
	m_pChatInputBox->SetPosition(m_WndPos.x+72, m_WndPos.y+32);
	m_pChatInputBox->SetTextColor(255, 255, 230, 210);
	m_pChatInputBox->SetBackColor(0, 0, 0, 25);
	m_pChatInputBox->SetFont(g_hFont);
	m_pChatInputBox->SetState(UISTATE_HIDE);

	m_pWhsprIDInputBox = new CUITextInputBox;
	m_pWhsprIDInputBox->Init(g_hWnd, 50, 14, 10);
	m_pWhsprIDInputBox->SetPosition(m_WndPos.x+5, m_WndPos.y+32);
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

	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
	
	Init();
}

void SEASON3B::CNewUIChatInputBox::SetButtonInfo()
{
	m_BtnSize.ChangeButtonImgState( true, IMAGE_INPUTBOX_BTN_SIZE, false );
	m_BtnSize.ChangeButtonInfo(m_WndPos.x+200, m_WndPos.y, 27, 26);

	m_BtnTransparency.ChangeButtonImgState( true, IMAGE_INPUTBOX_BTN_TRANSPARENCY, false );
	m_BtnTransparency.ChangeButtonInfo(m_WndPos.x+227, m_WndPos.y, 27, 26);
}

void SEASON3B::CNewUIChatInputBox::SetWndPos(int x, int y)
{
	m_WndPos.x = x; m_WndPos.y = y;
	m_WndSize.cx = WND_WIDTH; m_WndSize.cy = WND_HEIGHT;

	if(m_pChatInputBox && m_pWhsprIDInputBox)
	{
		m_pChatInputBox->SetPosition(m_WndPos.x+72, m_WndPos.y+32);
		m_pWhsprIDInputBox->SetPosition(m_WndPos.x+5, m_WndPos.y+32);
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
	type_vec_history::iterator vi = std::find(m_vecChatHistory.begin(), m_vecChatHistory.end(), strText);
	if(vi != m_vecChatHistory.end())
		m_vecChatHistory.erase(vi);
	else if(m_vecChatHistory.size() > 12)
		m_vecChatHistory.erase(m_vecChatHistory.begin());
	m_vecChatHistory.push_back(strText);
}

void SEASON3B::CNewUIChatInputBox::RemoveChatHistory(int index)
{
	if(index >= 0 && index < (int)m_vecChatHistory.size())
		m_vecChatHistory.erase(m_vecChatHistory.begin() + index);
}

void SEASON3B::CNewUIChatInputBox::RemoveAllChatHIstory()
{
	m_vecChatHistory.clear();
}

void SEASON3B::CNewUIChatInputBox::AddWhsprIDHistory(const type_string& strWhsprID)
{
	type_vec_history::iterator vi = std::find(m_vecWhsprIDHistory.begin(), m_vecWhsprIDHistory.end(), strWhsprID);
	if(vi != m_vecWhsprIDHistory.end())
		m_vecWhsprIDHistory.erase(vi);
	else if(m_vecWhsprIDHistory.size() > 5)
		m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin());
	m_vecWhsprIDHistory.push_back(strWhsprID);
}

void SEASON3B::CNewUIChatInputBox::RemoveWhsprIDHistory(int index)
{
	if(index >= 0 && index < (int)m_vecWhsprIDHistory.size())
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
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
	{
		return true;
	}

	if(SelectedCharacter >= 0)
	{
		if(IsVisible() && SEASON3B::IsRelease(VK_RBUTTON))
		{
			CHARACTER* pCha = &CharactersClient[SelectedCharacter];	

			if(pCha->Object.Kind == KIND_PLAYER && !gMapManager.InChaosCastle()	&& !(::IsStrifeMap(gMapManager.WorldActive) && Hero->m_byGensInfluence != pCha->m_byGensInfluence)

				)
			{
				SetWhsprID(pCha->ID);
			}
		}
	}

	m_iTooltipType = INPUT_TOOLTIP_NOTHING;
	int i, x;
	for(i=0; i<9; ++i)
	{
		x = m_WndPos.x + (i*27) + (i/3*6);
		
		if(CheckMouseIn(x, m_WndPos.y, 27, 26) == true)
		{
			m_iTooltipType = i;
			break;
		}
	}

	int iSelectedInputType = -1;
	for(i=0; i<3; ++i)
	{
		if(CheckMouseIn(m_WndPos.x+(i*27), m_WndPos.y, 27, 26) == true)
		{
			iSelectedInputType = i;
		}
	}
	if(iSelectedInputType > -1 && SEASON3B::IsRelease(VK_LBUTTON))
	{
		SetInputMsgType(INPUT_CHAT_MESSAGE+iSelectedInputType);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}

	if(CheckMouseIn(m_WndPos.x+87, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
	{
		m_bBlockWhisper = !m_bBlockWhisper;
		PlayBuffer(SOUND_CLICK01);
		return false;
	}

	if(m_bShowChatLog)
	{
		if(CheckMouseIn(m_WndPos.x+114, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
		{	
			m_bOnlySystemMessage = !m_bOnlySystemMessage;

			if(m_bOnlySystemMessage)
			{
				m_pNewUIChatLogWnd->ChangeMessage(SEASON3B::TYPE_SYSTEM_MESSAGE);
			}
			else
			{
				m_pNewUIChatLogWnd->ChangeMessage(SEASON3B::TYPE_ALL_MESSAGE);
			}

			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	if(CheckMouseIn(m_WndPos.x+141, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
	{
		m_bShowChatLog = !m_bShowChatLog;

		if(m_bShowChatLog)
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

	if(CheckMouseIn(m_WndPos.x+173, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
	{
		if(m_pNewUIChatLogWnd->IsShowFrame())
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

	if(m_pNewUIChatLogWnd->IsShowFrame())
	{
		if(m_BtnSize.UpdateMouseEvent() == true)
		{
			m_pNewUIChatLogWnd->SetSizeAuto();
			m_pNewUIChatLogWnd->UpdateWndSize();
			m_pNewUIChatLogWnd->UpdateScrollPos();
			PlayBuffer(SOUND_CLICK01);
			return false;
		}

		if(m_BtnTransparency.UpdateMouseEvent() == true)
		{
			m_pNewUIChatLogWnd->SetBackAlphaAuto();
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	if(CheckMouseIn(m_WndPos.x, m_WndPos.y, m_WndSize.cx, m_WndSize.cy) == true)
	{
		return false;
	}

	return true;
}

bool SEASON3B::CNewUIChatInputBox::UpdateKeyEvent()
{
	if(SEASON3B::IsPress(VK_F2))
	{
		m_bShowMessageElseNormal = !m_bShowMessageElseNormal;
		
		if(m_bShowMessageElseNormal)
		{
			m_pNewUIChatLogWnd->ChangeMessage( SEASON3B::TYPE_WHISPER_MESSAGE );
		}
		else
		{
			m_pNewUIChatLogWnd->ChangeMessage( SEASON3B::TYPE_ALL_MESSAGE );
		}

		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	
	if(SEASON3B::IsPress(VK_F3))
	{
		if(m_bWhisperSend == false)
		{
			m_bWhisperSend = true;
			m_pWhsprIDInputBox->SetState(UISTATE_NORMAL);
		}
		else
		{
			m_bWhisperSend = false;
			m_pWhsprIDInputBox->SetState(UISTATE_HIDE);
			if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
			{
				m_pChatInputBox->GiveFocus();
			}
		}

		return false;
	}

	if(m_pNewUIChatLogWnd->IsShowFrame())
	{
		if(SEASON3B::IsPress(VK_F4))
		{	
			m_pNewUIChatLogWnd->SetSizeAuto();
			m_pNewUIChatLogWnd->UpdateWndSize();
			m_pNewUIChatLogWnd->UpdateScrollPos();
			PlayBuffer(SOUND_CLICK01);	
			return false;
		}
	}

	if(SEASON3B::IsPress(VK_F5))
	{
		if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
		{
			if(m_pNewUIChatLogWnd->IsShowFrame())
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
	
	if(m_pNewUIChatLogWnd->IsShowFrame())
	{
		if(IsPress(VK_PRIOR))
		{
			m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() - m_pNewUIChatLogWnd->GetNumberOfShowingLines());
			return false;
		}
		if(IsPress(VK_NEXT))
		{
			m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() + m_pNewUIChatLogWnd->GetNumberOfShowingLines());
			return false;
		}	
	}

	if( false == IsVisible() && SEASON3B::IsPress(VK_RETURN) )
	{
		if(gMapManager.InChaosCastle() == true && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME) == false)
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
		}
		else if(gMapManager.InChaosCastle() == false)
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
		}

		RestoreIMEStatus();
		return false;
	}

	if(IsVisible() && HaveFocus() && SEASON3B::IsPress(VK_RETURN))
	{
		char	szChatText[MAX_CHAT_SIZE+1]	= {'\0'};
		char	szWhisperID[MAX_ID_SIZE+1]	= {'\0'};		
		wchar_t *szReceivedChat = new wchar_t[MAX_CHAT_SIZE_UTF16];
		
		m_pChatInputBox->GetText(szReceivedChat, MAX_CHAT_SIZE_UTF16);
		m_pWhsprIDInputBox->GetText(szWhisperID, MAX_ID_SIZE+1);

		for (int i = 0; i < MAX_CHAT_SIZE_UTF16; i++)
			szReceivedChat[i] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szReceivedChat[i]);

		std::wstring wstrText = L"";
		
		if (szReceivedChat[0] != 0x002F)
		{
			switch(m_iInputMsgType) {
			case INPUT_PARTY_MESSAGE:
				wstrText = L"~";
				break;
			case INPUT_GUILD_MESSAGE:
				wstrText = L"@";
				break;
			default:
				break;
			}
		}
		wstrText.append(szReceivedChat);

		delete [] szReceivedChat;

		std::string strText;

		g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), g_pMultiLanguage->GetCodePage());
		strncpy(szChatText, strText.c_str(), sizeof szChatText);
		
		if(unicode::_strlen(szChatText) != 0)
		{	
			if(!CheckCommand(szChatText))
			{
				{
					if(CheckAbuseFilter(szChatText))
					{
						g_pMultiLanguage->ConvertCharToWideStr(wstrText, GlobalText[570]);
					}
					
					if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL && unicode::_strlen(szChatText) && strlen(szWhisperID) > 0 )
					{
						g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
						strncpy(szChatText, strText.c_str(), sizeof szChatText);
						SendChatWhisper(szWhisperID, szChatText);
						g_pChatListBox->AddText(Hero->ID, szChatText, SEASON3B::TYPE_WHISPER_MESSAGE);
						AddWhsprIDHistory(szWhisperID);
					}
					else if (strncmp(szChatText, GlobalText[260], strlen(GlobalText[260])) == 0)
					{
						char* pszMapName = szChatText + strlen(GlobalText[260])+1;
						int iMapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(pszMapName);

						if (g_pMoveCommandWindow->IsTheMapInDifferentServer(gMapManager.WorldActive, iMapIndex))
						{
							SaveOptions();
						}
						
						SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), iMapIndex);
					}
					else
					{	
						if ( Hero->SafeZone || (Hero->Helper.Type != MODEL_HELPER+2 && Hero->Helper.Type != MODEL_HELPER+3 && Hero->Helper.Type != MODEL_HELPER+4 && Hero->Helper.Type != MODEL_HELPER+37) ) 
						{
							CheckChatText(szChatText);
						}
						
						g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
						strncpy(szChatText, strText.c_str(), sizeof szChatText);
						SendChat(szChatText);
						AddChatHistory(szChatText);
					}
				}
			}
		}
		m_pChatInputBox->SetText("");
		m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

		SaveIMEStatus();

		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
		return false;
	}
	if(IsVisible() && m_pChatInputBox->HaveFocus())
	{
		if(SEASON3B::IsPress(VK_UP) && false == m_vecChatHistory.empty())
		{
			m_iCurChatHistory--;
			if(m_iCurChatHistory < 0)
				m_iCurChatHistory = m_vecChatHistory.size()-1;
			m_pChatInputBox->SetText(m_vecChatHistory[m_iCurChatHistory].c_str());

			return false;
		}
		else if(SEASON3B::IsPress(VK_DOWN) && false == m_vecChatHistory.empty())
		{
			m_iCurChatHistory++;

			if(m_iCurChatHistory >= (int)m_vecChatHistory.size())
				m_iCurChatHistory = 0;

			m_pChatInputBox->SetText(m_vecChatHistory[m_iCurChatHistory].c_str());

			return false;
		}
	}
	
	if(IsVisible() && m_pWhsprIDInputBox->HaveFocus() 
		&& m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
	{
		if(SEASON3B::IsPress(VK_UP) && false == m_vecWhsprIDHistory.empty())
		{
			m_iCurWhisperIDHistory--;
			if(m_iCurWhisperIDHistory < 0)
				m_iCurWhisperIDHistory = m_vecWhsprIDHistory.size()-1;
			m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

			return false;
		}
		else if(SEASON3B::IsPress(VK_DOWN) && false == m_vecWhsprIDHistory.empty())
		{
			m_iCurWhisperIDHistory++;

			if(m_iCurWhisperIDHistory >= (int)m_vecWhsprIDHistory.size())
				m_iCurWhisperIDHistory = 0;

			m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

			return false;
		}
	}
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
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
	if(m_pChatInputBox->HaveFocus() && GetRelatedWnd() != m_pChatInputBox->GetHandle())
	{
		SetRelatedWnd(m_pChatInputBox->GetHandle());
	}
	if(m_pWhsprIDInputBox->HaveFocus() && GetRelatedWnd() != m_pWhsprIDInputBox->GetHandle())
	{
		SetRelatedWnd(m_pWhsprIDInputBox->GetHandle());
	}
	if(!HaveFocus() && GetRelatedWnd() != g_hWnd)
	{
		SetRelatedWnd(g_hWnd);
	}

	if(IsVisible() == false)
	{
		return true;
	}
	
	m_pChatInputBox->DoAction();
	
	if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
	{
		m_pWhsprIDInputBox->DoAction();
	}
	
	m_pChatInputBox->DoAction();
	
	if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL)
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

	if(m_bWhisperSend == false)
	{
		char szWhisperID[32];
		m_pWhsprIDInputBox->GetText(szWhisperID, 32);
		g_pRenderText->SetTextColor(255, 255, 255, 100);
		g_pRenderText->RenderText(m_pWhsprIDInputBox->GetPosition_x(), m_pWhsprIDInputBox->GetPosition_y(), szWhisperID);

		EnableAlphaTest();
		glColor4f(0.5f, 0.2f, 0.2f, 0.2f);
		RenderColor(m_WndPos.x+2, m_WndPos.y+28, 61, 17);
		EndRenderColor();
	}
			
	DisableAlphaBlend();
	
	return true;
}

bool SEASON3B::CNewUIChatInputBox::RenderFrame()
{
	RenderImage(IMAGE_INPUTBOX_BACK, m_WndPos.x, m_WndPos.y, WND_WIDTH, WND_HEIGHT);

	return true;
}

void SEASON3B::CNewUIChatInputBox::RenderButtons()
{
	// 채팅 타입 버튼
	RenderImage(IMAGE_INPUTBOX_NORMAL_ON+m_iInputMsgType, m_WndPos.x+27*m_iInputMsgType, m_WndPos.y, 27, 26);

	// 귓속말 차단 On/Off
	if(m_bBlockWhisper)
	{
		RenderImage(IMAGE_INPUTBOX_WHISPER_ON, m_WndPos.x+87, m_WndPos.y, 27, 26);
	}

	// 채팅 로그 출력 On/Off
	if(m_bShowChatLog)
	{
		RenderImage(IMAGE_INPUTBOX_CHATLOG_ON, m_WndPos.x+141, m_WndPos.y, 27, 26);

		// 시스템 메세지만 출력 On/Off
		if(m_bOnlySystemMessage)
		{
			RenderImage(IMAGE_INPUTBOX_SYSTEM_ON, m_WndPos.x+114, m_WndPos.y, 27, 26);
		}
	}

	// 채팅 로그 출력 On/Off
	if(m_pNewUIChatLogWnd->IsShowFrame())
	{
		RenderImage(IMAGE_INPUTBOX_FRAME_ON, m_WndPos.x+173, m_WndPos.y, 27, 26);

		// 사이즈 변경, 투명도 조절 버튼
		m_BtnSize.Render();
		m_BtnTransparency.Render();
	}
}

void SEASON3B::CNewUIChatInputBox::RenderTooltip()
{
	if(m_iTooltipType == INPUT_TOOLTIP_NOTHING)
		return;

	unicode::t_char strTooltip[256];

	const int iTextIndex[9] = { 1681, 1682, 1683, 1684, 1685, 750, 1686, 751, 752};

	unicode::_sprintf(strTooltip, "%s", GlobalText[iTextIndex[m_iTooltipType]]);

	SIZE Fontsize;
	g_pRenderText->SetFont(g_hFont);
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTooltip, unicode::_strlen(strTooltip), &Fontsize);

	Fontsize.cx = Fontsize.cx / ((float)WindowWidth / 640);
	Fontsize.cy = Fontsize.cy / ((float)WindowHeight / 480);

	int x = m_WndPos.x + (m_iTooltipType*27) + (m_iTooltipType/3*6) + 10 - (Fontsize.cx / 2); 
	if(x < 0) x = 0;
	int y = m_WndPos.y - (Fontsize.cy + 1);

	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 180);
	g_pRenderText->RenderText(x, y, strTooltip, Fontsize.cx+6, 0, RT3_SORT_CENTER);
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
	m_pChatInputBox->SetText("");

	if(m_bWhisperSend == true)
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
	unicode::t_char szChatText[256];
	m_pChatInputBox->GetText(szChatText, 256);
	strText = szChatText;
}
void SEASON3B::CNewUIChatInputBox::GetWhsprID(type_string& strWhsprID)
{
	char szWhisperID[32];
	m_pWhsprIDInputBox->GetText(szWhisperID, 32);
	strWhsprID = szWhisperID;
}

void SEASON3B::CNewUIChatInputBox::SetWhsprID(const char* strWhsprID)
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
