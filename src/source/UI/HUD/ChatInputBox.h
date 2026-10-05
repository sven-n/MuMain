#ifndef _NEWUICHATINPUTBOX_H_
#define _NEWUICHATINPUTBOX_H_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#pragma warning(disable : 4786)
#include <string>
#include <vector>

namespace Rml
{
    class ElementDocument;
}

namespace mu::ui::window
{
    class CManager;
    class CChatLogWindow;
    class CSystemLogWindow;

    // Every piece of this window's presentation is RmlUi's now: the bar art, all ten buttons, the
    // tooltip and both text fields. C++ keeps the chat/whisper history, the send logic and the
    // keyboard handling -- which still has to run while a field is focused, see
    // CChatInputBox::Update()'s SetRelatedWnd() note.
    struct ChatInputRmlModel
    {
        Rml::String chatText;
        Rml::String whisperId;

        int inputMsgType = 0;   // INPUT_CHAT_MESSAGE..INPUT_GENS_MESSAGE, as a 0-based index
        bool blockWhisper = false;
        bool showSystem = true;
        bool showChatLog = true;
        bool showFrame = false;
        bool whisperSend = true;

        int tooltipIndex = -1;  // INPUT_TOOLTIP_* of the hovered button, or -1
        float tooltipLeft = 0.0f;
        Rml::String tooltipText;
    };

    class CChatInputBox : public CObject
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

    private:
        // Both survive the port only because RenderTooltip()'s x formula, reproduced verbatim
        // in SyncRmlModel(), is expressed in them.
        static constexpr float BUTTON_WIDTH = 27.0f;

        static constexpr float GROUP_SEPARATING_WIDTH = 6.0f;

        typedef std::wstring type_string;
        typedef std::vector<type_string>	type_vec_history;

        const uint64_t ChatCooldownMs = 1000; // 1 Second
        uint64_t  m_lastChatTime = 0;

        CManager* m_pNewUIMng;
        CChatLogWindow* m_pNewUIChatLogWnd;
        CSystemLogWindow* m_pNewUISystemLogWnd;
        POINT	m_WndPos{};
        SIZE	m_WndSize{};

        type_vec_history	m_vecChatHistory, m_vecWhsprIDHistory;

        int m_iCurChatHistory, m_iCurWhisperIDHistory;

        int m_iTooltipType;
        int m_iInputMsgType;
        bool m_bBlockWhisper;
        bool m_bShowSystemMessages;
        bool m_bShowChatLog;
        bool m_bWhisperSend;
        bool m_bShowMessageElseNormal;

        void Init();

        void SetInputMsgType(int iInputMsgType);
        int GetInputMsgType() const;

        void BuildRmlUi();
        void SyncRmlModel();
        // Focus/value access for the two <input>s, so the key handler below never has to know they
        // are RmlUi elements.
        Rml::Element* GetField(const char* id) const;
        bool IsFieldFocused(const char* id) const;
        void SetFieldText(const char* id, const type_string& text);
        void FocusField(const char* id);

        RmlModelBinder<ChatInputRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        // Where Create() put the box, kept while the theme gives it no slot.
        POINT m_HomePos{};
        // Set by OpenningProcess(), consumed once the document is actually visible. CSystem::Show()
        // runs OpenningProcess() BEFORE ShowInterface(), so IsVisible() is still false there and
        // focusing the field at that point lands on a hidden document and is lost. It also has to
        // happen after SyncDocumentVisibility()'s own Show(), which defaults to FocusFlag::Auto and
        // would blur the field again.
        bool m_bFocusPending = false;

    public:
        CChatInputBox();
        virtual ~CChatInputBox();

        bool Create(CManager* pNewUIMng,
            CChatLogWindow* pNewUIChatLogWnd,
            CSystemLogWindow* pNewUISystemLogWnd,
            int x,
            int y);
        void Release();

        void SetWndPos(int x, int y);

        void ReloadRmlTheme();

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

        void UpdateWhisperTargetFromRightClick();
    };
}

#endif // _NEWUICHATINPUTBOX_H_