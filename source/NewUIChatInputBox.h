
#ifndef _NEWUICHATINPUTBOX_H_
#define _NEWUICHATINPUTBOX_H_

#pragma once

#include "NewUIBase.h"
#include "ZzzTexture.h"
#include "NewUIButton.h"

#pragma warning(disable : 4786)
#include <string>
#include <vector>

class CUITextInputBox;	//. 기존 클래스사용

namespace SEASON3B
{
	class CNewUIManager;
	class CNewUIChatLogWindow;
	class CNewUIChatInputBox : public CNewUIObj
	{
	public:
		enum INPUT_MESSAGE_TYPE
		{
			INPUT_NOTHING = -1,			
			INPUT_CHAT_MESSAGE,
			INPUT_PARTY_MESSAGE,
			INPUT_GUILD_MESSAGE,

		};

		enum INPUT_TOOLTIP_TYPE
		{
			INPUT_TOOLTIP_NOTHING = -1,

			INPUT_TOOLTIP_NORMAL, 
			INPUT_TOOLTIP_PARTY, 
			INPUT_TOOLTIP_GUILD, 

			INPUT_TOOLTIP_WHISPER,
			INPUT_TOOLTIP_SYSTEM,
			INPUT_TOOLTIP_CHAT,

			INPUT_TOOLTIP_FRAME,
			INPUT_TOOLTIP_SIZE,
			INPUT_TOOLTIP_TRANSPARENCY,
		};

		enum IMAGE_LIST
		{	//. max: 12개
			IMAGE_INPUTBOX_BACK = BITMAP_INTERFACE_NEW_CHATINPUTBOX_BEGIN,	//. newui_chat_back.jpg
			IMAGE_INPUTBOX_NORMAL_ON,
			IMAGE_INPUTBOX_PARTY_ON,
			IMAGE_INPUTBOX_GUILD_ON,
			IMAGE_INPUTBOX_WHISPER_ON,
			IMAGE_INPUTBOX_SYSTEM_ON,
			IMAGE_INPUTBOX_CHATLOG_ON,
			IMAGE_INPUTBOX_FRAME_ON,
			IMAGE_INPUTBOX_BTN_SIZE,
			IMAGE_INPUTBOX_BTN_TRANSPARENCY,
		};

	private:
		enum 
		{
			MAIN_FRAME_HEIGHT = 51,
			WND_WIDTH = 254,
			WND_HEIGHT = 47,
			WHISPERBOX_WIDTH = 71,
			WHISPERBOX_HEIGHT = 20,
		};
		enum EVENT_STATE
		{
			EVENT_NONE = 0,
			EVENT_CLIENT_WND_HOVER,
		};

		typedef std::basic_string<unicode::t_char, std::char_traits<unicode::t_char>, 
			std::allocator<unicode::t_char> > type_string;
		typedef std::vector<type_string>	type_vec_history;

		CNewUIManager*			m_pNewUIMng;
		CNewUIChatLogWindow*	m_pNewUIChatLogWnd;
		POINT	m_WndPos;
		SIZE	m_WndSize;

		CUITextInputBox*	m_pChatInputBox, * m_pWhsprIDInputBox;
		type_vec_history	m_vecChatHistory, m_vecWhsprIDHistory;

		int m_iCurChatHistory, m_iCurWhisperIDHistory;

		int m_iTooltipType;			// 채팅 버튼들 툴팁 타입
		int m_iInputMsgType;		// 채팅 타입
		bool m_bBlockWhisper;		// 귓속말 차단 On/Off
		bool m_bOnlySystemMessage;	// 시스템 메시지만 표시 On/Off
		bool m_bShowChatLog;		// 채팅로그창 On/Off
		bool m_bWhisperSend;		// 귓속말 보내기 On/Off
		bool m_bShowMessageElseNormal;		// 일반 채팅만 빼고 나머지 다 표시
		
		CNewUIButton m_BtnSize;				// 크기조절 버튼
		CNewUIButton m_BtnTransparency;		// 투명도조절 버튼
		
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		const int MAX_CHAT_SIZE_UTF16;
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

		void Init();
		
		void LoadImages();
		void UnloadImages();

		void SetButtonInfo();
		void SetInputMsgType(int iInputMsgType);
		int GetInputMsgType() const;

		bool RenderFrame();
		void RenderButtons();
		void RenderTooltip();

	public:
		CNewUIChatInputBox();
		virtual ~CNewUIChatInputBox();

		bool Create(CNewUIManager* pNewUIMng, CNewUIChatLogWindow* pNewUIChatLogWnd, int x, int y);
		void Release();

		void SetWndPos(int x, int y);
		
		void SetFont(HFONT hFont);

		bool HaveFocus();	// 커서를 가지고 있는가

		void AddChatHistory(const type_string& strText);	// 히스토리에 추가한다
		void RemoveChatHistory(int index);	// 히스토리 제거
		void RemoveAllChatHIstory();

		void AddWhsprIDHistory(const type_string& strWhsprID);	// 히스토리에 추가한다
		void RemoveWhsprIDHistory(int index);	// 히스토리 제거
		void RemoveAllWhsprIDHIstory();

		bool IsBlockWhisper();
		void SetBlockWhisper(bool bBlockWhisper);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		float GetLayerDepth();	//. 6.2f
		float GetKeyEventOrder();	//. 9.0f

		void OpenningProcess();
		void ClosingProcess();
		
		void SetWhsprID(const char* strWhsprID);

	protected:
		void GetChatText(type_string& strText);	// 텍스트 가져오기
		void GetWhsprID(type_string& strWhsprID);

		void SetTextPosition(int x, int y);
		void SetBuddyPosition(int x, int y);
	};
}

#endif // _NEWUICHATINPUTBOX_H_