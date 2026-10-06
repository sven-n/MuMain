#ifndef _NEWCHATLONGWINDOW_H_
#define _NEWCHATLONGWINDOW_H_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Chat/MessageType.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/HUD/ChatInputBox.h"
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

    template <class T>
    class TMessageText
    {
        typedef std::wstring type_string;

        type_string	m_strID, m_strText;
        MESSAGE_TYPE m_MsgType;
        DWORD m_dwIndentSize;

    public:
        TMessageText() : m_MsgType(TYPE_UNKNOWN), m_dwIndentSize(0) {}
        ~TMessageText() { Release(); }

        bool Create(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType)
        {
            if (MsgType >= NUMBER_OF_TYPES)
                return false;

            m_strID = strID;
            m_strText = strText;
            m_MsgType = MsgType;

            return true;
        }
        void Release()
        {
            m_strID.resize(0);
            m_strText.resize(0);
            m_MsgType = TYPE_UNKNOWN;
        }

        const type_string& GetID() const { return m_strID; }
        const type_string& GetText() const { return m_strText; }
        MESSAGE_TYPE GetType() const { return m_MsgType; }
    };

    typedef TMessageText<wchar_t> CMessageText;

    // One rendered chat line. `kind` is the lowercase message-type slug the RCSS builds its
    // .chat-line--<kind> class from; `hasId` marks a line whose sender can be right-clicked to
    // target a whisper (and which therefore gets the hover highlight).
    struct ChatLogLineEntry
    {
        Rml::String text;
        Rml::String kind;
        bool hasId = false;
    };

    // RmlUi owns this window's whole presentation now: the background fill, every message line,
    // and -- via base.rcss's .scroll-pane on #lines -- the scroll position, mouse wheel and
    // scrollbar drag. C++ keeps the message vectors, the filters and the 3-line-step resize, and
    // pins the view to the bottom when a new message arrives.
    struct ChatLogRmlModel
    {
        float panelHeight = 100.0f;  // dp -- 15 * showing lines + 10, native's own UpdateWndSize()
        float clientHeight = 90.0f;  // dp -- the scrolling well inside it
        // The user's transparency setting on its own, 0..1. Frame state is showFrame below and
        // the backdrop's colour is the theme's -- three separate owners, not one composed string.
        float backAlpha = 0.f;
        bool showFrame = false;
        // Index of the line under the cursor, or -1. Drives .chat-line--pointed; C++ resolves it
        // because the lines themselves are pointer-events:none (see chat_log.rcss).
        int pointedIndex = -1;
        Rml::Vector<ChatLogLineEntry> lines;

        // The native renderer's line geometry in physical px (the legacy theme's chat_log.rml):
        // the text size, each line's background height (the measured text) and the 15-unit row
        // pitch of the dp-sized well.
        float textPx = 0.f;
        float linePx = 0.f;
        float rowPx = 0.f;
    };

    class CChatLogWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_SCROLL_TOP = BITMAP_INTERFACE_NEW_CHATLOGWND_BEGIN,	//. newui_scrollbar_up.tga
            IMAGE_SCROLL_MIDDLE,	//. newui_scrollbar_m.tga
            IMAGE_SCROLL_BOTTOM,	//. newui_scrollbar_down.tga
            IMAGE_SCROLLBAR_ON,		//. newui_scroll_on.tga
            IMAGE_SCROLLBAR_OFF,	//. newui_scroll_off.tga
            IMAGE_DRAG_BTN,			//. newui_scrollbar_stretch.tga
        };

    private:
        static constexpr int MAX_CHAT_BUFFER_SIZE = 60;
        static constexpr int MAX_NUMBER_OF_LINES = 200;
        static constexpr float WND_WIDTH = CChatInputBox::CHATBOX_WIDTH;
        static constexpr float FONT_LEADING = 4.0f;
        static constexpr float WND_TOP_BOTTOM_EDGE = 2.0f;
        static constexpr float WND_LEFT_RIGHT_EDGE = 4.0f;
        static constexpr float RESIZING_BTN_WIDTH = WND_WIDTH;
        static constexpr float RESIZING_BTN_HEIGHT = 10.0f;
        static constexpr float SCROLL_BAR_WIDTH = 7.0f;
        static constexpr float SCROLL_TOP_BOTTOM_PART_HEIGHT = 3.0f;
        static constexpr float SCROLL_MIDDLE_PART_HEIGHT = 15.0f;
        static constexpr float CLIENT_WIDTH = WND_WIDTH - SCROLL_BAR_WIDTH * 2.0f - (WND_LEFT_RIGHT_EDGE * 2.0f);
        // Only the resize drag is still a C++ interaction; hover, wheel and scrollbar dragging
        // all belong to RmlUi now, so the states that tracked them are gone.
        enum EVENT_STATE
        {
            EVENT_NONE = 0,
            EVENT_RESIZING_BTN_DOWN,
        };

        typedef std::wstring type_string;
        typedef std::vector<CMessageText*>	type_vector_msgs;
        typedef std::vector<type_string>	type_vector_filters;

        CManager* m_pNewUIMng;

        type_vector_msgs	m_vecAllMsgs;
        type_vector_msgs	m_VecChatMsgs;
        type_vector_msgs	m_vecWhisperMsgs;
        type_vector_msgs	m_vecPartyMsgs;
        type_vector_msgs	m_vecGuildMsgs;
        type_vector_msgs	m_vecUnionMsgs;
        type_vector_msgs	m_vecGensMsgs;
        type_vector_msgs	m_VecSystemMsgs;
        type_vector_msgs	m_vecErrorMsgs;
        type_vector_msgs	m_vecGMMsgs;
        type_vector_filters	m_vecFilters;

        POINT	m_WndPos;
        SIZE	m_WndSize;
        int		m_nShowingLines;

        MESSAGE_TYPE		m_CurrentRenderMsgType;
        bool				m_bShowChatLog;
        bool m_bSceneAllowsShow = false;
        int		m_iCurrentRenderEndLine;
        float	m_fBackAlpha;

        EVENT_STATE			m_EventState;

        bool m_bShowFrame;

        void Init();

        void BuildRmlUi();
        void SyncRmlModel();
        void SyncNativeLineGeometry();
        // Rebuilds the bound line list from the currently-selected message vector. Kept
        // index-aligned with that vector (undrawable entries become blanks rather than being
        // skipped) because chat_line_rightclick() resolves a sender by array index.
        void RebuildLineModel();
        // Pins #lines to the bottom after the model's line list changes, so a new message scrolls
        // into view exactly as native's own "follow the tail" behaviour did. Skipped while the
        // user has scrolled up, same as native.
        void ScrollToBottomIfFollowing();
        bool IsScrolledToBottom() const;
        // Resolves which rendered line the cursor is over and acts on a right-click, replacing
        // native's own per-line loop in UpdateMouseEvent(). Compares raw window pixels against
        // RmlUi's own element boxes -- both are already in screen space, so no transform
        // conversion enters anywhere (that conversion is the bug class RmlPanelGeometry.h warns
        // about).
        void UpdatePointedLine();
        // Pushes m_iCurrentRenderEndLine into RmlUi's scroll offset (native's own fPosRate math),
        // and, when no request is pending, reads it back the other way so the logical cursor
        // tracks wherever the user dragged or scrolled to.
        void ApplyLogicalScroll();
        void SyncLogicalScrollFromView();

        RmlModelBinder<ChatLogRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        // Where Create() put the window, kept while the theme gives it no slot.
        POINT m_HomePos{};
        bool m_bLinesDirty = true;
        bool m_bFollowTail = true;
        // One-shot, set when the line list changes and consumed on the next frame once RmlUi has
        // laid the new lines out. Never pin outside this latch -- doing so re-clamps the view
        // every frame and makes the scrollbar and wheel look broken.
        bool m_bScrollPending = false;
        // Set by Scrolling()/UpdateScrollPos() -- the logical scroll position still has external
        // drivers (CChatInputBox's PageUp/PageDown and its resize buttons), so those requests are
        // latched and applied to RmlUi's own scroll offset on the next sync rather than fighting
        // it every frame.
        bool m_bScrollRequest = false;

    public:
        void ReloadRmlTheme();

        CChatLogWindow();
        ~CChatLogWindow() override;

        bool Create(CManager* pNewUIMng, int x, int y, int nShowingLines = 6);
        void Release();

        void SetPosition(int x, int y);
        void AddText(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType, MESSAGE_TYPE ErrMsgType = TYPE_ALL_MESSAGE);
        void RemoveFrontLine(MESSAGE_TYPE MsgType);
        void Clear(MESSAGE_TYPE MsgType);
        void ClearAll();

        size_t GetNumberOfLines(MESSAGE_TYPE MsgType);
        MESSAGE_TYPE GetCurrentMsgType() const;

        void ChangeMessage(MESSAGE_TYPE MsgType);

        void ShowChatLog();
        void HideChatLog();

        int GetCurrentRenderEndLine() const;
        void Scrolling(int nRenderEndLine);

        void SetFilterText(const type_string& strFilterText);
        void ResetFilter();

        void SetSizeAuto();
        void SetNumberOfShowingLines(int nShowingLines, OUT LPSIZE lpBoxSize = NULL);
        size_t GetNumberOfShowingLines() const;
        void SetBackAlphaAuto();
        void SetBackAlpha(float fAlpha);
        float GetBackAlpha() const;

        void ShowFrame();
        void HideFrame();
        bool IsShowFrame();

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        // Hides the document outside the main scene (CSystem::SyncMainSceneHudVisibility()): the
        // window is only updated there, but its document would keep drawing in every scene.
        void SyncDocVisibility(bool sceneAllowsShow);

        bool Render() override;

        float GetLayerDepth() override;	//. 6.1f
        float GetKeyEventOrder() override;	//. 8.0f

        void UpdateWndSize();
        void UpdateScrollPos();

    protected:
        type_vector_msgs* GetMsgs(MESSAGE_TYPE MsgType);
        void ProcessAddText(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType, MESSAGE_TYPE ErrMsgType);

        void SeparateText(IN const type_string& strID, IN const type_string& strText, MESSAGE_TYPE MsgType,
                          OUT type_string& strText1, OUT type_string& strText2);

        bool CheckFilterText(const type_string& strTestText);
        void AddFilterWord(const type_string& strWord);
    };

    // Top-left system/error overlay. Reuses ChatLogLineEntry (its `hasId` is simply never set --
    // these lines have no sender), so both log windows share one line shape.
    struct SystemLogRmlModel
    {
        // Native applies the user's transparency setting to every line's own background quad, so
        // this is a per-line colour, not a panel fill.
        float backAlpha = 0.6f; // the player's own transparency setting, 0 .. 1
        Rml::Vector<ChatLogLineEntry> lines;

        // The native renderer's geometry under the window's layout, in physical px (the legacy
        // theme's system_log.rml): the row pitch, each line's background
        // height and the text size.
        float rowPx = 0.f;
        float linePx = 0.f;
        float textPx = 0.f;
    };

    class CSystemLogWindow : public CObject
    {
    private:
        enum
        {
            MAX_MSG_BUFFER_SIZE = 6,
            MAX_NUMBER_OF_LINES = 6,
            WND_WIDTH = CChatInputBox::CHATBOX_WIDTH,
            FONT_LEADING = 4,
            WND_TOP_BOTTOM_EDGE = 2,
            WND_LEFT_RIGHT_EDGE = 4,
            CLIENT_WIDTH = WND_WIDTH - (WND_LEFT_RIGHT_EDGE * 2),
        };


        typedef std::wstring type_string;
        typedef std::vector<CMessageText*>	type_vector_msgs;

        CManager* m_pNewUIMng;

        type_vector_msgs	m_vecAllMsgs;

        POINT	m_WndPos;
        SIZE	m_WndSize;
        int		m_nShowingLines;

        int		m_iCurrentRenderEndLine;
        float	m_fBackAlpha;
        bool    m_bShowMessages;
        bool m_bSceneAllowsShow = false;

        void Init();

        void BuildRmlUi();
        void SyncRmlModel();
        void SyncNativeGeometry();
        void RebuildLineModel();

        RmlModelBinder<SystemLogRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        bool m_bLinesDirty = true;
        float m_LastPanelHeight = -1.f;
        void TrackPanelSize();

        void RemoveFrontLine();
        int GetCurrentRenderEndLine() const;

    public:
        void ReloadRmlTheme();

        CSystemLogWindow();
        ~CSystemLogWindow() override;

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPosition(int x, int y);
        void AddText(const type_string& strText, MESSAGE_TYPE MsgType);

        void ClearAll();
        void ShowMessages() { m_bShowMessages = true; }
        void HideMessages() { m_bShowMessages = false; }

        bool UpdateMouseEvent() override;
        bool UpdateKeyEvent() override;
        bool Update() override;
        // Hides the document outside the main scene (CSystem::SyncMainSceneHudVisibility()): the
        // window is only updated there, but its document would keep drawing in every scene.
        void SyncDocVisibility(bool sceneAllowsShow);

        bool Render() override;

        float GetLayerDepth() override;
        float GetKeyEventOrder() override;

        bool CheckChatRedundancy(const type_string& strText, int iSearchLine = 1);
    };
}

#endif // _NEWCHATLONGWINDOW_H_
