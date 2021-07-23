

#include "stdafx.h"
#include "NewUIChatInputBox.h"
#include "NewUIChatLogWindow.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "CSChaosCastle.h"
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#include "ZzzOpenData.h"
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
using namespace SEASON3B;

#ifdef PSW_FIX_INPUTTEXTMACRO
#ifdef CSK_LUCKY_SEAL////////////////////////////////////////////////////////////		
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
namespace
{
	bool CheckLuckySeal( char *Text )
	{
		string isLuckychattext = Text;
		if( isLuckychattext.find( GlobalText[260] ) != string::npos 
			|| isLuckychattext.find( "/move" ) != string::npos )  
		{
			if( g_pMoveCommandWindow->IsMapMove( Text ) == false )
			{
				unicode::t_char szText[1024];
				unicode::_sprintf(szText, GlobalText[2558]);
				g_pChatListBox->AddText("", szText, SEASON3B::TYPE_ERROR_MESSAGE);
				return true;
			}
			else {
				return false;
			}
		}

		return false;
	}
};
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#endif //CSK_LUCKY_SEAL////////////////////////////////////////////////////////////
#endif //PSW_FIX_INPUTTEXTMACRO

SEASON3B::CNewUIChatInputBox::CNewUIChatInputBox() 
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	:	MAX_CHAT_SIZE_UTF16((int)(MAX_CHAT_SIZE/(g_pMultiLanguage->GetNumByteForOneCharUTF8())))
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
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
	//. Loading Image
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
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_pChatInputBox->Init(g_hWnd, 176, 14, MAX_CHAT_SIZE_UTF16-1);
#else //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	m_pChatInputBox->Init(g_hWnd, 176, 14, MAX_CHAT_SIZE-2);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
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
#ifdef _VS2008PORTING
	if(index >= 0 && index < (int)m_vecChatHistory.size())
#else // _VS2008PORTING
	if(index >= 0 && index < m_vecChatHistory.size())
#endif // _VS2008PORTING
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
#ifdef _VS2008PORTING
	if(index >= 0 && index < (int)m_vecWhsprIDHistory.size())
#else // _VS2008PORTING
	if(index >= 0 && index < m_vecWhsprIDHistory.size())
#endif // _VS2008PORTING
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
		// 채팅입력창이 열린상태에서는 마우스 오른쪽 클릭으로 캐릭터 선택하면 귓속말에 아이디 입력된다.
		if(IsVisible() && SEASON3B::IsRelease(VK_RBUTTON))
		{
			CHARACTER* pCha = &CharactersClient[SelectedCharacter];	

#ifdef LDK_FIX_EXCEPTION_SETWHISPERID_NOTMODELPLAYER_INCHAOSCASTLE
			//채팅창 열고 마우스우클릭시 귀속말id 입력 예외처리(2008.5.7)
			if(pCha->Object.Kind == KIND_PLAYER && !InChaosCastle()
#ifdef ASG_ADD_GENS_SYSTEM
				// 분쟁지역에서는 타 세력 플래이어의 이름이 노출시키면 안됨.
				&& !(::IsStrifeMap(World) && Hero->m_byGensInfluence != pCha->m_byGensInfluence)
#endif	// ASG_ADD_GENS_SYSTEM
				)
#endif //LDK_FIX_EXCEPTION_SETWHISPERID_NOTMODELPLAYER_INCHAOSCASTLE
			{
				SetWhsprID(pCha->ID);
			}
		}
	}

	// 툴팁타입 설정
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

	// 채팅 타입 Radio 버튼 처리
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

	// 귓속말 차단 On/Off
	if(CheckMouseIn(m_WndPos.x+87, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
	{
		m_bBlockWhisper = !m_bBlockWhisper;
		PlayBuffer(SOUND_CLICK01);
		return false;
	}

	if(m_bShowChatLog)
	{
		// 시스템 메시지만 출력 On/Off
		if(CheckMouseIn(m_WndPos.x+114, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
		{	
			m_bOnlySystemMessage = !m_bOnlySystemMessage;

#ifdef KJH_FIX_UI_CHAT_MESSAGE
			if(m_bOnlySystemMessage)
			{
				m_pNewUIChatLogWnd->ChangeMessage(SEASON3B::TYPE_SYSTEM_MESSAGE);
			}
			else
			{
				m_pNewUIChatLogWnd->ChangeMessage(SEASON3B::TYPE_ALL_MESSAGE);
			}
#else // KJH_FIX_UI_CHAT_MESSAGE					// 정리할 때 지워야 하는 소스
			if(m_bOnlySystemMessage)
			{
				m_pNewUIChatLogWnd->ShowMessage(CNewUIChatLogWindow::SHOW_SYSTEM_MESSAGE);
			}
			else
			{
				m_pNewUIChatLogWnd->ShowMessage(CNewUIChatLogWindow::SHOW_ALL_MESSAGES);
			}
#endif // KJH_FIX_UI_CHAT_MESSAGE					// 정리할 때 지워야 하는 소스

			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	// 채팅 로그 출력 On/Off 
	if(CheckMouseIn(m_WndPos.x+141, m_WndPos.y, 27, 26) == true && SEASON3B::IsRelease(VK_LBUTTON))
	{
		m_bShowChatLog = !m_bShowChatLog;

#ifdef KJH_FIX_UI_CHAT_MESSAGE
		// 기능 : 채팅 On/Off
		// 현재 F2를 눌렀을때 기능과 틀리다. (임시)
		if(m_bShowChatLog)
		{
			m_pNewUIChatLogWnd->ShowChatLog();
		}
		else
		{
			m_pNewUIChatLogWnd->HideChatLog();
		}
#else // KJH_FIX_UI_CHAT_MESSAGE					// 정리할 때 지워야 하는 소스
		
		if(m_bShowChatLog)
		{
			if(m_bOnlySystemMessage)
			{
				m_pNewUIChatLogWnd->ShowMessage(CNewUIChatLogWindow::SHOW_SYSTEM_MESSAGE);
			}
			else
			{
				m_pNewUIChatLogWnd->ShowMessage(CNewUIChatLogWindow::SHOW_ALL_MESSAGES);
			}
		}
		else
		{
			m_pNewUIChatLogWnd->ShowMessage(CNewUIChatLogWindow::SHOW_NOTHING);
		}
#endif // KJH_FIX_UI_CHAT_MESSAGE					// 정리할 때 지워야 하는 소스


		PlayBuffer(SOUND_CLICK01);
		return false;
	}

	// 로그 Frame 출력 On/Off
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
		// 로그 프레임 사이즈 변경 버튼
		if(m_BtnSize.UpdateMouseEvent() == true)
		{
			m_pNewUIChatLogWnd->SetSizeAuto();
			m_pNewUIChatLogWnd->UpdateWndSize();
			m_pNewUIChatLogWnd->UpdateScrollPos();
			PlayBuffer(SOUND_CLICK01);
			return false;
		}

		// 로그 프레임 투명도 조절 버튼
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
	// 채팅메세지 (On/Off)
	if(SEASON3B::IsPress(VK_F2))
	{
#ifdef KJH_FIX_UI_CHAT_MESSAGE
		// 현재 F2기능은 전체채팅<->귓말모드 토글기능
		// 아이콘을 눌렀을때 와 기능이 틀리다.
		m_bShowMessageElseNormal = !m_bShowMessageElseNormal;
		
		if(m_bShowMessageElseNormal)
		{
			// 전체 채팅모드
			m_pNewUIChatLogWnd->ChangeMessage( SEASON3B::TYPE_WHISPER_MESSAGE );
		}
		else
		{
			// 귓말모드
			m_pNewUIChatLogWnd->ChangeMessage( SEASON3B::TYPE_ALL_MESSAGE );
		}
#else // KJH_FIX_UI_CHAT_MESSAGE
		m_bShowMessageElseNormal = !m_bShowMessageElseNormal;

		if(m_bShowMessageElseNormal)
		{
			int flag = CNewUIChatLogWindow::SHOW_ALL_MESSAGES;

			flag ^= (int)CNewUIChatLogWindow::SHOW_CHAT_MESSAGE;

			m_pNewUIChatLogWnd->ShowMessage((CNewUIChatLogWindow::SHOW_MESSAGE_FLAG)flag);
		}
		else	
		{
			m_pNewUIChatLogWnd->ShowMessage(CNewUIChatLogWindow::SHOW_ALL_MESSAGES);
		}
#endif // KJH_FIX_UI_CHAT_MESSAGE

		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	
	if(SEASON3B::IsPress(VK_F3))
	{
		//. 귓속말 아이디 입력창 열기
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

	// 체팅창 열기
	if( false == IsVisible() && SEASON3B::IsPress(VK_RETURN) )
	{
		// 카오스캐슬이고 실제 싸움 시작전에는 채팅창이 열린다.
		// 실제 이벤트가 시작하면 이벤트 타임 UI 보이기 시작한다.
		if(InChaosCastle() == true && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME) == false)

		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
		}
		else if(InChaosCastle() == false)
		{
			g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
		}

		RestoreIMEStatus();
		return false;
	}

	if(IsVisible() && HaveFocus() && SEASON3B::IsPress(VK_RETURN))
	{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		//. 쳇 전송
		char	szChatText[MAX_CHAT_SIZE+1]	= {'\0'};
		char	szWhisperID[MAX_ID_SIZE+1]	= {'\0'};		
		wchar_t *szReceivedChat = new wchar_t[MAX_CHAT_SIZE_UTF16];
		
		m_pChatInputBox->GetText(szReceivedChat, MAX_CHAT_SIZE_UTF16);
		m_pWhsprIDInputBox->GetText(szWhisperID, MAX_ID_SIZE+1);

		// 전각 영문 알파벳과 기본 기호들을 반각으로 변환
		for (int i = 0; i < MAX_CHAT_SIZE_UTF16; i++)
			szReceivedChat[i] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szReceivedChat[i]);

		wstring wstrText = L"";
		
		// 특수 명령은 채팅모드 무시
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

		// delete memory
		delete [] szReceivedChat;

		string strText;
		// 각종 함수를 이용하기 위해서 ANSI로 변환
		g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), g_pMultiLanguage->GetCodePage());
		strncpy(szChatText, strText.c_str(), sizeof szChatText);
		
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		//. 쳇 전송
		unicode::t_char szChatText[256];
		unicode::t_char szChatText2[256];
		char szWhisperID[32];
		
		m_pChatInputBox->GetText(szChatText2, 256);		//. 유니코드: 수정하세요!
		m_pWhsprIDInputBox->GetText(szWhisperID, 32);

		if (szChatText2[0] == '/')	// 특수 명령은 채팅모드 무시
		{
			unicode::_strcpy(szChatText, szChatText2);
		}
		else
		if(m_iInputMsgType == INPUT_PARTY_MESSAGE)
		{
			unicode::_sprintf(szChatText, "~%s", szChatText2);
		}
		else if(m_iInputMsgType == INPUT_GUILD_MESSAGE)
		{
			unicode::_sprintf(szChatText, "@%s", szChatText2);
		}
		else
		{
			unicode::_strcpy(szChatText, szChatText2);
		}	
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

		if(unicode::_strlen(szChatText) != 0)
		{	
			if(!CheckCommand(szChatText))		//. 명령어 검사 및 처리
			{
#ifdef PSW_FIX_INPUTTEXTMACRO
#ifdef CSK_LUCKY_SEAL////////////////////////////////////////////////////////////						
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
				if( CheckLuckySeal(szChatText) == false )
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#endif //CSK_LUCKY_SEAL////////////////////////////////////////////////////////////
#endif //PSW_FIX_INPUTTEXTMACRO
				{
					// 필터링
					if(CheckAbuseFilter(szChatText))		//. 쳇팅 욕 필터
					{
						// 570 "사랑해요"
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
						g_pMultiLanguage->ConvertCharToWideStr(wstrText, GlobalText[570]);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
						strcpy(szChatText,GlobalText[570]);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					}
					
					//. 귓말일 경우
					if(m_pWhsprIDInputBox->GetState() == UISTATE_NORMAL 
						&& unicode::_strlen(szChatText) 
						&& strlen(szWhisperID) > 0 ) //. 주의!: ID는 ANSI
					{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
						g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
						strncpy(szChatText, strText.c_str(), sizeof szChatText);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
						SendChatWhisper(szWhisperID, szChatText);
						g_pChatListBox->AddText(Hero->ID, szChatText, SEASON3B::TYPE_WHISPER_MESSAGE);
						AddWhsprIDHistory(szWhisperID);
					}
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					// /warp 명령어를 써서 맵 이동 할때 SendRequestMoveMap() 사용
					else if (strncmp(szChatText, GlobalText[260], strlen(GlobalText[260])) == 0)
					{
						char* pszMapName = szChatText + strlen(GlobalText[260])+1;
						int iMapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(pszMapName);

						// 현재 위치하고 있는 맵이나 이동하려는 맵이 다른 서버에 존재(로랜협곡, 로랜시장)
						if (g_pMoveCommandWindow->IsTheMapInDifferentServer(World, iMapIndex))
						{
							SaveOptions();
						}
						
						SendRequestMoveMap(g_pMoveCommandWindow->GetMoveCommandKey(), iMapIndex);
					}
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
					//. 일반 쳇팅일 경우
					else
					{	
						if ( Hero->SafeZone || (Hero->Helper.Type != MODEL_HELPER+2 
							&& Hero->Helper.Type != MODEL_HELPER+3
							&& Hero->Helper.Type != MODEL_HELPER+4 
							&& Hero->Helper.Type != MODEL_HELPER+37) ) 
						{
							CheckChatText(szChatText);	//. 사용가능한 이모티콘 체크
						}
						
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
						g_pMultiLanguage->ConvertWideCharToStr(strText, wstrText.c_str(), CP_UTF8);
						strncpy(szChatText, strText.c_str(), sizeof szChatText);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

						SendChat(szChatText);
						AddChatHistory(szChatText);
					}
				}
			}
		}
		//. 인풋버퍼 클리어
		m_pChatInputBox->SetText("");
		m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

		SaveIMEStatus();

		//. 쳇팅창 닫기
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
		return false;
	}
#if SELECTED_LANGUAGE != LANGUAGE_TAIWANESE && SELECTED_LANGUAGE != LANGUAGE_CHINESE
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
#ifdef _VS2008PORTING
			if(m_iCurChatHistory >= (int)m_vecChatHistory.size())
#else // _VS2008PORTING
			if(m_iCurChatHistory >= m_vecChatHistory.size())
#endif // _VS2008PORTING
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
#ifdef _VS2008PORTING
			if(m_iCurWhisperIDHistory >= (int)m_vecWhsprIDHistory.size())
#else // _VS2008PORTING
			if(m_iCurWhisperIDHistory >= m_vecWhsprIDHistory.size())
#endif // _VS2008PORTING
				m_iCurWhisperIDHistory = 0;
			m_pWhsprIDInputBox->SetText(m_vecWhsprIDHistory[m_iCurWhisperIDHistory].c_str());

			return false;
		}
	}
#endif //SELECTED_LANGUAGE != LANGUAGE_TAIWANESE && SELECTED_LANGUAGE != LANGUAGE_CHINESE
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == true)
	{
		if(SEASON3B::IsPress(VK_ESCAPE) == true)
		{
			//. 쳇팅창 닫기
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);

			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIChatInputBox::Update()
{
	//. RelatedWnd 셋팅: UpdateKeyEvent를 호출에 관련된 윈도우 핸들 설정
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
	// 1681 "일반 채팅 On/Off"
	// 1682 "파티 채팅 On/Off"
	// 1683 "길드 채팅 On/Off"
	// 1684 "귓말 차단 On/Off"
	// 1685 "시스템 메시지 출력 On/Off"
	// 1686 "채팅창 배경 출력 On/Off (F5)"
	// 750 "채팅출력 On/Off (F2)"
	// 751 "크기조정 (F4)"
	// 752 "투명도 조정"
	const int iTextIndex[9] = { 1681, 1682, 1683, 1684, 1685, 750, 1686, 751, 752};

	unicode::_sprintf(strTooltip, "%s", GlobalText[iTextIndex[m_iTooltipType]]);

	SIZE Fontsize;
	g_pRenderText->SetFont(g_hFont);
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTooltip, unicode::_strlen(strTooltip), &Fontsize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), strTooltip, unicode::_strlen(strTooltip), &Fontsize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

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
	// 인풋버퍼 초기화
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
	m_pChatInputBox->GetText(szChatText, 256);		//. 유니코드: 수정하세요!
	strText = szChatText;
}
void SEASON3B::CNewUIChatInputBox::GetWhsprID(type_string& strWhsprID)
{
	char szWhisperID[32];
	m_pWhsprIDInputBox->GetText(szWhisperID, 32);
	//. 유니코드: 변환루틴 필요!
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
