#ifndef _NEWUICHATINPUTBOX_H_
#define _NEWUICHATINPUTBOX_H_

#pragma once

#include "NewUIBase.h"
#include "NewUIButton.h"

#pragma warning(disable : 4786)
#include <string>
#include <vector>

class CUITextInputBox;

namespace SEASON3B
{
    class CNewUIManager;
    class CNewUIChatLogWindow;
    class CNewUISystemLogWindow;

    class CNewUIChatInputBox : public CNewUIObj
    {
    public:
        // It's also the size of the graphics IMAGE_INPUTBOX_BACK.
        enum
        {
            CHATBOX_WIDTH = 281,
            CHATBOX_HEIGHT = 47,
        };

        enum INPUT_MESSAGE_TYPE
        {
            INPUT_NOTHING = -1,
            INPUT_CHAT_MESSAGE,
            INPUT_PARTY_MESSAGE,
            INPUT_GUILD_MESSAGE,
            INPUT_GENS_MESSAGE,
        };

        enum INPUT_TOOLTIP_TYPE
        {
            INPUT_TOOLTIP_NOTHING = -1,

            INPUT_TOOLTIP_NORMAL,
            INPUT_TOOLTIP_PARTY,
            INPUT_TOOLTIP_GUILD,
            INPUT_TOOLTIP_GENS,

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
            IMAGE_INPUTBOX_GENS_ON,
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
            INPUT_MESSAGE_TYPE_COUNT = 4,
            BUTTON_WIDTH = 27,
            BUTTON_HEIGHT = 26,

            GROUP_SEPARATING_WIDTH = 6,

            INPUT_TYPE_START_X = 0,
            BLOCK_WHISPER_START_X = INPUT_MESSAGE_TYPE_COUNT * BUTTON_WIDTH + GROUP_SEPARATING_WIDTH,
            SYSTEM_ON_START_X = BLOCK_WHISPER_START_X + BUTTON_WIDTH,
            CHATLOG_ON_START_X = SYSTEM_ON_START_X + BUTTON_WIDTH,

            FRAME_ON_START_X = CHATLOG_ON_START_X + BUTTON_WIDTH + GROUP_SEPARATING_WIDTH,
            FRAME_RESIZE_START_X = FRAME_ON_START_X + BUTTON_WIDTH,
            TRANSPARENCY_START_X = FRAME_RESIZE_START_X + BUTTON_WIDTH,
        };

        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_CLIENT_WND_HOVER,
        };

        typedef std::wstring type_string;
        typedef std::vector<type_string>	type_vec_history;

        const uint64_t ChatCooldownMs = 1000; // 1 Second
        uint64_t  m_lastChatTime = 0;

        CNewUIManager* m_pNewUIMng;
        CNewUIChatLogWindow* m_pNewUIChatLogWnd;
        CNewUISystemLogWindow* m_pNewUISystemLogWnd;
        POINT	m_WndPos{};
        SIZE	m_WndSize{};

        CUITextInputBox* m_pChatInputBox, * m_pWhsprIDInputBox;
        type_vec_history	m_vecChatHistory, m_vecWhsprIDHistory;

        int m_iCurChatHistory, m_iCurWhisperIDHistory;

        int m_iTooltipType;
        int m_iInputMsgType;
        bool m_bBlockWhisper;
        bool m_bShowSystemMessages;
        bool m_bShowChatLog;
        bool m_bWhisperSend;
        bool m_bShowMessageElseNormal;

        CNewUIButton m_BtnSize;
        CNewUIButton m_BtnTransparency;

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

        bool Create(CNewUIManager* pNewUIMng,
            CNewUIChatLogWindow* pNewUIChatLogWnd,
            CNewUISystemLogWindow* pNewUISystemLogWnd,
            int x,
            int y);
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

        void SetWhsprID(const wchar_t* strWhsprID);

    protected:
        void GetChatText(type_string& strText);
        void GetWhsprID(type_string& strWhsprID);

        void SetTextPosition(int x, int y);
        void SetBuddyPosition(int x, int y);
    };
}

#endif // _NEWUICHATINPUTBOX_H_