
#ifndef _NEWUICHATINPUTBOX_H_
#define _NEWUICHATINPUTBOX_H_

#pragma once

#include "NewUIBase.h"
#include "ZzzTexture.h"
#include "NewUIButton.h"

#pragma warning(disable : 4786)
#include <string>
#include <vector>

class CUITextInputBox;

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
		{
			IMAGE_INPUTBOX_BACK = BITMAP_INTERFACE_NEW_CHATINPUTBOX_BEGIN,
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

		typedef std::basic_string<unicode::t_char, std::char_traits<unicode::t_char>, std::allocator<unicode::t_char> > type_string;
		typedef std::vector<type_string>	type_vec_history;

		CNewUIManager*			m_pNewUIMng;
		CNewUIChatLogWindow*	m_pNewUIChatLogWnd;
		POINT	m_WndPos;
		SIZE	m_WndSize;

		CUITextInputBox*	m_pChatInputBox, * m_pWhsprIDInputBox;
		type_vec_history	m_vecChatHistory, m_vecWhsprIDHistory;

		int m_iCurChatHistory, m_iCurWhisperIDHistory;

		int m_iTooltipType;
		int m_iInputMsgType;
		bool m_bBlockWhisper;
		bool m_bOnlySystemMessage;
		bool m_bShowChatLog;
		bool m_bWhisperSend;
		bool m_bShowMessageElseNormal;
		
		CNewUIButton m_BtnSize;
		CNewUIButton m_BtnTransparency;
		
		const int MAX_CHAT_SIZE_UTF16;

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

		bool HaveFocus();

		void AddChatHistory(const type_string& strText);
		void RemoveChatHistory(int index);
		void RemoveAllChatHIstory();

		void AddWhsprIDHistory(const type_string& strWhsprID);
		void RemoveWhsprIDHistory(int index);
		void RemoveAllWhsprIDHIstory();

		bool IsBlockWhisper();
		void SetBlockWhisper(bool bBlockWhisper);
		
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();

		float GetLayerDepth();
		float GetKeyEventOrder();

		void OpenningProcess();
		void ClosingProcess();
		
		void SetWhsprID(const char* strWhsprID);

	protected:
		void GetChatText(type_string& strText);
		void GetWhsprID(type_string& strWhsprID);

		void SetTextPosition(int x, int y);
		void SetBuddyPosition(int x, int y);
	};
}

#endif // _NEWUICHATINPUTBOX_H_