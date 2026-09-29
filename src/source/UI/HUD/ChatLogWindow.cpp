#include "stdafx.h"
#include "I18N/All.h"

#include "UI/HUD/ChatLogWindow.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Audio/DSPlaySound.h"
#include "UI/Widgets/UIControls.h"
#include "Engine/Object/ZzzInterface.h"

// RmlUi migration -- see ChatLogRmlModel (ChatLogWindow.h).
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>

using namespace SEASON3B;
using namespace mu::ui::window;



mu::ui::window::CChatLogWindow::CChatLogWindow()
{
    Init();
}

mu::ui::window::CChatLogWindow::~CChatLogWindow()
{
    Release();
}

void mu::ui::window::CChatLogWindow::Init()
{
    m_pNewUIMng = nullptr;
    m_WndPos.x = m_WndPos.y = 0;
    m_WndSize.cx = WND_WIDTH; m_WndSize.cy = 0;
    m_nShowingLines = 6;
    m_iCurrentRenderEndLine = -1;
    m_fBackAlpha = 0.6f;

    m_EventState = EVENT_NONE;

    m_bShowFrame = false;

    m_CurrentRenderMsgType = TYPE_ALL_MESSAGE;
    m_bShowChatLog = true;

}

bool mu::ui::window::CChatLogWindow::Create(CManager* pNewUIMng, int x, int y, int nShowingLines /* = 6 */)
{
    Release();

    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CHATLOGWINDOW, this);
    m_WndPos.x = x; m_WndPos.y = y;
    SetNumberOfShowingLines(nShowingLines);
    // No LoadImages() any more: every sprite this window used is referenced by chat_log.rcss and
    // loaded through RmlUi's own exclusive-slot path instead. CGuildInfoWindow/CGuardWindow alias
    // this class's IMAGE_LIST values but load their own copies, so nothing depended on it -- the
    // enum itself stays in the header for those aliases.
    return true;
}

void mu::ui::window::CChatLogWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    ResetFilter();
    ClearAll();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    Init();
}

void mu::ui::window::CChatLogWindow::SetPosition(int x, int y)
{
    m_WndPos.x = x;
    m_WndPos.y = y;
}

void mu::ui::window::CChatLogWindow::AddText(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType, MESSAGE_TYPE ErrMsgType /*= TYPE_ALL_MESSAGE*/)
{
    if (strID.empty() && strText.empty())
    {
        return;
    }

    if (GetNumberOfLines(MsgType) >= MAX_NUMBER_OF_LINES)
    {
        RemoveFrontLine(MsgType);
    }

    if (GetNumberOfLines(TYPE_ALL_MESSAGE) >= MAX_NUMBER_OF_LINES)
    {
        RemoveFrontLine(TYPE_ALL_MESSAGE);
    }

    if (m_vecFilters.empty())
    {
        ProcessAddText(strID, strText, MsgType, ErrMsgType);
    }
    else
    {
        if (MsgType != TYPE_CHAT_MESSAGE)
        {
            ProcessAddText(strID, strText, MsgType, ErrMsgType);
        }
        else if (CheckFilterText(strID) || CheckFilterText(strText))
        {
            ProcessAddText(strID, strText, MsgType, ErrMsgType);

            if (g_pOption->IsWhisperSound())
            {
                PlayBuffer(SOUND_WHISPER);
            }
        }
    }
}

void mu::ui::window::CChatLogWindow::ProcessAddText(const type_string& strID, const type_string& strText, MESSAGE_TYPE MsgType, MESSAGE_TYPE ErrMsgType)
{
    m_bLinesDirty = true;
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message");
        return;
    }

    int nScrollLines = 0;
    if (strText.size() >= 20)
    {
        type_string	strText1, strText2;
        SeparateText(strID, strText, MsgType, strText1, strText2);
        if (!strText1.empty())
        {
            const auto pMsgText = new CMessageText;
            if (!pMsgText->Create(strID, strText, MsgType))
                delete pMsgText;
            else
            {
                pvecMsgs->push_back(pMsgText);
            }

            const auto pAllMsgText = new CMessageText;
            if (!pAllMsgText->Create(strID, strText1, MsgType))
            {
                delete pAllMsgText;
            }
            else
            {
                m_vecAllMsgs.push_back(pAllMsgText);
            }

            if ((MsgType == TYPE_ERROR_MESSAGE) && (ErrMsgType != TYPE_ERROR_MESSAGE && ErrMsgType != TYPE_ALL_MESSAGE))
            {
                type_vector_msgs* pErrvecMsgs = GetMsgs(ErrMsgType);
                if (pErrvecMsgs == nullptr)
                {
                    assert(!"Error Chat");
                    return;
                }

                const auto pErrMsgText = new CMessageText;
                if (!pErrMsgText->Create(strID, strText1, MsgType))
                    delete pErrMsgText;
                else
                {
                    pErrvecMsgs->push_back(pErrMsgText);
                }
            }

            if (GetCurrentMsgType() == TYPE_ALL_MESSAGE || GetCurrentMsgType() == MsgType)
            {
                nScrollLines++;
            }
        }
        if (!strText2.empty())
        {
            const auto pMsgText = new CMessageText;
            if (!pMsgText->Create(L"", strText2, MsgType))
                delete pMsgText;
            else
            {
                pvecMsgs->push_back(pMsgText);
            }

            const auto pAllMsgText = new CMessageText;
            if (!pAllMsgText->Create(L"", strText2, MsgType))
                delete pAllMsgText;
            else
            {
                m_vecAllMsgs.push_back(pAllMsgText);
            }

            if ((MsgType == TYPE_ERROR_MESSAGE) && (ErrMsgType != TYPE_ERROR_MESSAGE && ErrMsgType != TYPE_ALL_MESSAGE))
            {
                type_vector_msgs* pErrvecMsgs = GetMsgs(ErrMsgType);
                if (pErrvecMsgs == nullptr)
                {
                    assert(!"Error chat 2");
                    return;
                }

                const auto pErrMsgText = new CMessageText;
                if (!pErrMsgText->Create(L"", strText2, MsgType))
                    delete pErrMsgText;
                else
                {
                    pErrvecMsgs->push_back(pErrMsgText);
                }
            }

            if (GetCurrentMsgType() == TYPE_ALL_MESSAGE || GetCurrentMsgType() == MsgType)
            {
                nScrollLines++;
            }
        }
    }
    else
    {
        const auto pMsgText = new CMessageText;
        if (!pMsgText->Create(strID, strText, MsgType))
            delete pMsgText;
        else
        {
            pvecMsgs->push_back(pMsgText);
        }

        const auto pAllMsgText = new CMessageText;
        if (!pAllMsgText->Create(strID, strText, MsgType))
            delete pAllMsgText;
        else
        {
            m_vecAllMsgs.push_back(pAllMsgText);
        }

        if ((MsgType == TYPE_ERROR_MESSAGE)
            && (ErrMsgType != TYPE_ERROR_MESSAGE && ErrMsgType != TYPE_ALL_MESSAGE))
        {
            type_vector_msgs* pErrvecMsgs = GetMsgs(ErrMsgType);
            if (pErrvecMsgs == nullptr)
            {
                assert(!"Error chat 3");
                return;
            }

            const auto pErrMsgText = new CMessageText;
            if (!pErrMsgText->Create(strID, strText, MsgType))
                delete pErrMsgText;
            else
            {
                pErrvecMsgs->push_back(pErrMsgText);
            }
        }

        if (GetCurrentMsgType() == TYPE_ALL_MESSAGE || GetCurrentMsgType() == MsgType)
        {
            nScrollLines++;
        }
    }

    pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
    {
        assert(!"Error chat 4");
        return;
    }

    //. Auto Scrolling
    if (nScrollLines > 0 && ((pvecMsgs->size() - (m_iCurrentRenderEndLine + 1) - nScrollLines) < 3))
        m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
    else if (!m_bShowFrame)
        m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

void mu::ui::window::CChatLogWindow::RemoveFrontLine(MESSAGE_TYPE MsgType)
{
    m_bLinesDirty = true;
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);

    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message RemoveFrontLine");
        return;
    }

    auto vi = pvecMsgs->begin();
    if (vi != pvecMsgs->end())
    {
        delete (*vi);
        vi = pvecMsgs->erase(vi);
    }

    if (MsgType == GetCurrentMsgType())
    {
        Scrolling(GetCurrentRenderEndLine());
    }
}

void mu::ui::window::CChatLogWindow::Clear(MESSAGE_TYPE MsgType)
{
    m_bLinesDirty = true;
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message CChatLogWindow");
        return;
    }

    auto vi_msg = pvecMsgs->begin();
    for (; vi_msg != pvecMsgs->end(); vi_msg++)
        delete (*vi_msg);
    pvecMsgs->clear();

    if (MsgType == GetCurrentMsgType())
    {
        m_iCurrentRenderEndLine = -1;
    }
}

size_t mu::ui::window::CChatLogWindow::GetNumberOfLines(MESSAGE_TYPE MsgType)
{
    type_vector_msgs* pvecMsgs = GetMsgs(MsgType);
    if (pvecMsgs == nullptr)
    {
        return 0;
    }

    return pvecMsgs->size();
}

int mu::ui::window::CChatLogWindow::GetCurrentRenderEndLine() const
{
    return m_iCurrentRenderEndLine;
}

void mu::ui::window::CChatLogWindow::Scrolling(int nRenderEndLine)
{
    type_vector_msgs* pvecMsgs = GetMsgs(m_CurrentRenderMsgType);
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty message Scrolling");
        return;
    }

    if ((int)pvecMsgs->size() <= m_nShowingLines)
    {
        m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
    }
    else
    {
        if (nRenderEndLine < m_nShowingLines)
            m_iCurrentRenderEndLine = m_nShowingLines - 1;

        else if (nRenderEndLine >= (int)pvecMsgs->size())
            m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
        else
            m_iCurrentRenderEndLine = nRenderEndLine;
    }

    // Nothing renders from m_iCurrentRenderEndLine any more, so a caller that moves it (the chat
    // input box's PageUp/PageDown) has to have it pushed into RmlUi's own scroll offset.
    m_bScrollRequest = true;
}

void mu::ui::window::CChatLogWindow::SetFilterText(const type_string& strFilterText)
{
    m_bLinesDirty = true;
    bool bPrevFilter = false;

    if (!m_vecFilters.empty())
    {
        bPrevFilter = true;
        ResetFilter();
    }

    wchar_t szTemp[MAX_CHAT_BUFFER_SIZE + 1] = { 0, };
    strFilterText.copy(szTemp, MAX_CHAT_BUFFER_SIZE);
    szTemp[MAX_CHAT_BUFFER_SIZE] = '\0';

    wchar_t* context = nullptr;
	wchar_t* token = wcstok_s(szTemp, L" ", &context);
	token = wcstok_s(nullptr, L" ", &context);

    if (token == nullptr)
    {
        ResetFilter();
        AddText(L"", I18N::Game::FilteringHasBeenCanceled, TYPE_SYSTEM_MESSAGE);
    }
    else
    {
        for (int i = 0; i < 5; i++)
        {
            if (nullptr == token)
            {
                break;
            }
            AddFilterWord(token);
            token = wcstok_s(nullptr, L" ", &context);
        }

        AddText(L"", I18N::Game::FilteringHasBeenActivated, TYPE_SYSTEM_MESSAGE);
    }
}

void mu::ui::window::CChatLogWindow::ResetFilter()
{
    m_bLinesDirty = true;
    m_vecFilters.clear();
}

void mu::ui::window::CChatLogWindow::SetSizeAuto()
{
    SetNumberOfShowingLines(GetNumberOfShowingLines() + 3);
}

void mu::ui::window::CChatLogWindow::SetNumberOfShowingLines(int nShowingLines, OUT LPSIZE lpBoxSize/* = nullptr*/)
{
    m_nShowingLines = (int)(nShowingLines / 3) * 3;
    if (m_nShowingLines < 3)
        m_nShowingLines = 3;
    if (m_nShowingLines > 15)
        m_nShowingLines = 3;

    if (m_nShowingLines > GetCurrentRenderEndLine())
        Scrolling(m_nShowingLines - 1);

    UpdateWndSize();
    UpdateScrollPos();

    if (lpBoxSize)
    {
        lpBoxSize->cx = WND_WIDTH;
        lpBoxSize->cy = (SCROLL_MIDDLE_PART_HEIGHT * GetNumberOfShowingLines()) + (SCROLL_TOP_BOTTOM_PART_HEIGHT * 2) + (WND_TOP_BOTTOM_EDGE * 2);
    }
}
size_t mu::ui::window::CChatLogWindow::GetNumberOfShowingLines() const
{
    return m_nShowingLines;
}

void mu::ui::window::CChatLogWindow::SetBackAlphaAuto()
{
    m_fBackAlpha += 0.2f;

    if (m_fBackAlpha > 0.9f)
    {
        m_fBackAlpha = 0.2f;
    }
}

void mu::ui::window::CChatLogWindow::SetBackAlpha(float fAlpha)
{
    if (fAlpha < 0.f)
        m_fBackAlpha = 0.f;
    else if (fAlpha > 1.f)
        m_fBackAlpha = 1.f;
    else
        m_fBackAlpha = fAlpha;
}

float mu::ui::window::CChatLogWindow::GetBackAlpha() const
{
    return m_fBackAlpha;
}

void mu::ui::window::CChatLogWindow::ShowFrame()
{
    m_bShowFrame = true;
}

void mu::ui::window::CChatLogWindow::HideFrame()
{
    m_bShowFrame = false;
}

bool mu::ui::window::CChatLogWindow::IsShowFrame()
{
    return m_bShowFrame;
}

bool mu::ui::window::CChatLogWindow::UpdateMouseEvent()
{
    // Almost everything this used to do now belongs to RmlUi: the mouse wheel, the scrollbar drag
    // and the per-line hover/right-click hit test are all handled by #lines (base.rcss's
    // .scroll-pane) and the line elements themselves. What survives is the part RmlUi has no
    // concept of -- native's 3-line-step window resize, which is driven by the pointer's absolute
    // Y against the whole screen, not by any element's own box.

    if (!m_bShowFrame)
    {
        m_EventState = EVENT_NONE;
        return true;
    }

    if (m_EventState != EVENT_RESIZING_BTN_DOWN)
        return true;

    // Armed by #resize_handle's data-event-mousedown (chat_resize_begin).
    if (false == MouseLButtonPush || true == MouseLButtonPop)
    {
        m_EventState = EVENT_NONE;
        return true;
    }

    // Native's own stepping, unchanged: the screen is divided into 3-line bands above and below
    // the handle's resting position, and the pointer's band picks the new line count.
    const LONG resizeTop = (LONG)(m_WndPos.y - m_WndSize.cy - RESIZING_BTN_HEIGHT);
    const int nTopSections = (15 - (int)GetNumberOfShowingLines()) / 3;
    const int nBottomSections = ((int)GetNumberOfShowingLines() - 3) / 3;

    for (int i = 0; i < nTopSections; i++)
    {
        if (mu::ui::window::CheckMouseIn(0, resizeTop - RESIZING_BTN_HEIGHT - ((i + 1) * SCROLL_MIDDLE_PART_HEIGHT * 3 * 2),
            REFERENCE_WIDTH, SCROLL_MIDDLE_PART_HEIGHT * 3 + RESIZING_BTN_HEIGHT))
        {
            SetNumberOfShowingLines((int)GetNumberOfShowingLines() + (i + 1) * 3);
            return false;
        }
    }
    for (int i = 0; i < nBottomSections; i++)
    {
        if (mu::ui::window::CheckMouseIn(0, resizeTop + RESIZING_BTN_HEIGHT + ((i + 1) * SCROLL_MIDDLE_PART_HEIGHT * 3),
            REFERENCE_WIDTH, RESIZING_BTN_HEIGHT + SCROLL_MIDDLE_PART_HEIGHT * 3))
        {
            SetNumberOfShowingLines((int)GetNumberOfShowingLines() - (i + 1) * 3);
            return false;
        }
    }
    if (mu::ui::window::CheckMouseIn(0, 0, REFERENCE_WIDTH,
        m_WndPos.y - (SCROLL_MIDDLE_PART_HEIGHT * 15 + RESIZING_BTN_HEIGHT + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2)))
    {
        SetNumberOfShowingLines(15);
    }
    if (mu::ui::window::CheckMouseIn(0, m_WndPos.y - (SCROLL_MIDDLE_PART_HEIGHT * 3 + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2),
        REFERENCE_WIDTH, SCROLL_MIDDLE_PART_HEIGHT * 3 + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2))
    {
        SetNumberOfShowingLines(3);
    }
    return false;
}

bool mu::ui::window::CChatLogWindow::UpdateKeyEvent()
{
    return true;
}

namespace
{
    // Lowercase slug per message type -- the RCSS builds .chat-line--<slug> from it. Types native
    // refuses to draw at all never reach the model; they become empty placeholder entries so a
    // line's array index keeps matching its index in the message vector.
    const char* MessageTypeSlug(mu::ui::window::MESSAGE_TYPE type)
    {
        switch (type)
        {
        case mu::ui::window::TYPE_CHAT_MESSAGE:    return "chat";
        case mu::ui::window::TYPE_WHISPER_MESSAGE: return "whisper";
        case mu::ui::window::TYPE_SYSTEM_MESSAGE:  return "system";
        case mu::ui::window::TYPE_ERROR_MESSAGE:   return "error";
        case mu::ui::window::TYPE_PARTY_MESSAGE:   return "party";
        case mu::ui::window::TYPE_GUILD_MESSAGE:   return "guild";
        case mu::ui::window::TYPE_UNION_MESSAGE:   return "union";
        case mu::ui::window::TYPE_GENS_MESSAGE:    return "gens";
        case mu::ui::window::TYPE_GM_MESSAGE:      return "gm";
        default:                                   return nullptr;
        }
    }
}

bool mu::ui::window::CChatLogWindow::Update()
{
    BuildRmlUi();
    SyncRmlModel();

    return true;
}

bool mu::ui::window::CChatLogWindow::Render()
{
    // Nothing native left to draw -- RmlUi owns the fill, every message line and the scrollbar.
    // Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CChatLogWindow::BuildRmlUi()
{
    // Guarded so document/model are created once, even though Update() runs every frame.
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "chat_log",
        [this](Rml::DataModelConstructor& c, ChatLogRmlModel& model)
        {
            // Must re-run in full on every call, including from ReloadRmlTheme() -- same reason
            // CCharMakeWin::BuildRmlUi() documents, so no guard here.
            auto line = c.RegisterStruct<ChatLogLineEntry>();
            line.RegisterMember("text", &ChatLogLineEntry::text);
            line.RegisterMember("kind", &ChatLogLineEntry::kind);
            line.RegisterMember("has_id", &ChatLogLineEntry::hasId);
            c.RegisterArray<Rml::Vector<ChatLogLineEntry>>();

            c.Bind("lines", &model.lines);
            c.Bind("panel_height", &model.panelHeight);
            c.Bind("client_height", &model.clientHeight);
            c.Bind("back_alpha", &model.backAlpha);
            c.Bind("show_frame", &model.showFrame);
            c.Bind("pointed_index", &model.pointedIndex);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_px", &model.linePx);
            c.Bind("row_px", &model.rowPx);

            // Drag the handle above the window to resize it in native's own 3-line steps. The
            // stepping stays in UpdateMouseEvent() where the pointer's absolute Y already lives;
            // this only arms it.
            c.BindEventCallback("chat_resize_begin",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    m_EventState = EVENT_RESIZING_BTN_DOWN;
                });
        });

    if (modelCreated)
    {
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                     "Data/Interface/RmlUi/chat_log.rml");
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }
}

void mu::ui::window::CChatLogWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never built -- BuildRmlUi() picks the new theme up whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    m_bLinesDirty = true; // next SyncRmlModel() repopulates the fresh document
}

void mu::ui::window::CChatLogWindow::SyncDocVisibility(bool sceneAllowsShow)
{
    m_bSceneAllowsShow = sceneAllowsShow;
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible() && m_bShowChatLog && sceneAllowsShow);
}

void mu::ui::window::CChatLogWindow::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible() && m_bShowChatLog && m_bSceneAllowsShow);

    ChatLogRmlModel& model = m_RmlBinder.GetModel();

    // Native's own UpdateWndSize(): 15 per line, plus the 3+3 scroll caps and the 2+2 edges.
    const float clientHeight = SCROLL_MIDDLE_PART_HEIGHT * static_cast<float>(m_nShowingLines);
    const float panelHeight = clientHeight + SCROLL_TOP_BOTTOM_PART_HEIGHT * 2.0f
                            + WND_TOP_BOTTOM_EDGE * 2.0f;
    if (model.clientHeight != clientHeight)
    {
        model.clientHeight = clientHeight;
        m_RmlBinder.MarkDirty("client_height");
    }
    if (model.panelHeight != panelHeight)
    {
        model.panelHeight = panelHeight;
        m_RmlBinder.MarkDirty("panel_height");
    }
    if (model.showFrame != m_bShowFrame)
    {
        model.showFrame = m_bShowFrame;
        m_RmlBinder.MarkDirty("show_frame");
    }

    // The user's cycled transparency setting, nothing else: whether the backdrop paints at all is
    // show_frame's job (.framed in the RCSS) and its colour is the theme's.
    const float backAlpha = std::clamp(GetBackAlpha(), 0.0f, 1.0f);
    if (model.backAlpha != backAlpha)
    {
        model.backAlpha = backAlpha;
        m_RmlBinder.MarkDirty("back_alpha");
    }

    SyncNativeLineGeometry();

    if (m_bLinesDirty)
    {
        // Native's own rule, restated against scroll position: follow the tail while the frame is
        // hidden, or while the user is already at the bottom. ProcessAddText() expressed it as
        // "within 3 lines of the end" because it drove a line index; the DOM equivalent is
        // "scrolled to the bottom", and it has to be sampled HERE, against the pre-append layout,
        // not after the new lines exist.
        m_bFollowTail = !m_bShowFrame || IsScrolledToBottom();

        m_bLinesDirty = false;
        RebuildLineModel();
        m_RmlBinder.MarkDirty("lines");

        // Arms a ONE-SHOT pin, consumed next frame. It must be a latch, not a per-frame call:
        // the new lines only have a resolved scroll height after RmlUi lays them out, so the pin
        // has to happen a frame later -- but pinning on every frame in between re-clamps the view
        // to the bottom continuously, which silently defeats the user's own scrollbar drag and
        // mouse wheel.
        m_bScrollPending = true;
    }

    if (m_bScrollPending)
    {
        m_bScrollPending = false;
        ScrollToBottomIfFollowing();
    }

    // A pending request wins for one frame; otherwise the logical cursor follows the view, so the
    // next PageUp/PageDown starts from wherever the user actually scrolled to.
    if (m_bScrollRequest)
    {
        m_bScrollRequest = false;
        ApplyLogicalScroll();
    }
    else
    {
        SyncLogicalScrollFromView();
    }

    UpdatePointedLine();
}

// Native's RenderMessages(): each line drawn with RenderText() at the native text size, its
// background as tall as the measured text (MeasureText("Q").cy, logical), one line every
// SCROLL_MIDDLE_PART_HEIGHT; the row pitch follows the well, which is sized in dp.
void mu::ui::window::CChatLogWindow::SyncNativeLineGeometry()
{
    const auto transform = UI::Scaling::GetActiveTransform();
    g_pRenderText->SetFont(g_hFont);
    const int textHeight = g_pRenderText->MeasureText(L"Q", 1).cy;
    const float dpRatio = RmlUiRuntime::Instance().GetContext()->GetDensityIndependentPixelRatio();

    ChatLogRmlModel& model = m_RmlBinder.GetModel();
    auto syncFloat = [&](float ChatLogRmlModel::* field, const char* name, float value)
    {
        if (model.*field != value)
        {
            model.*field = value;
            m_RmlBinder.MarkDirty(name);
        }
    };
    syncFloat(&ChatLogRmlModel::textPx, "text_px",
              UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform));
    syncFloat(&ChatLogRmlModel::linePx, "line_px", UI::Scaling::SizeY(transform, static_cast<float>(textHeight)));
    syncFloat(&ChatLogRmlModel::rowPx, "row_px", SCROLL_MIDDLE_PART_HEIGHT * dpRatio);
}

void mu::ui::window::CChatLogWindow::RebuildLineModel()
{
    ChatLogRmlModel& model = m_RmlBinder.GetModel();
    model.lines.clear();

    type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
        return;

    model.lines.reserve(pvecMsgs->size());
    for (const CMessageText* pMsgText : *pvecMsgs)
    {
        const char* slug = pMsgText ? MessageTypeSlug(pMsgText->GetType()) : nullptr;
        if (slug == nullptr || pMsgText->GetText().empty())
        {
            // Native skipped drawing these (bRenderMessage stayed false), but an entry still has
            // to exist: chat_line_rightclick() looks its sender up by array index, so the model
            // and the message vector must stay index-aligned.
            model.lines.push_back(ChatLogLineEntry{});
            continue;
        }

        ChatLogLineEntry entry;
        entry.kind = slug;
        entry.hasId = !pMsgText->GetID().empty();
        const type_string composed = entry.hasId
            ? pMsgText->GetID() + L" : " + pMsgText->GetText()
            : pMsgText->GetText();
        entry.text = StringUtils::WideToNarrow(composed.c_str());
        model.lines.push_back(std::move(entry));
    }
}

bool mu::ui::window::CChatLogWindow::IsScrolledToBottom() const
{
    if (!m_pRmlDoc)
        return true;

    Rml::Element* lines = m_pRmlDoc->GetElementById("lines");
    if (lines == nullptr)
        return true;

    // One line of slack, so a partially-scrolled last row still counts as "at the bottom" -- the
    // same forgiveness native's own 3-line window gave.
    const float slack = SCROLL_MIDDLE_PART_HEIGHT;
    return lines->GetScrollTop() + lines->GetClientHeight() >= lines->GetScrollHeight() - slack;
}

void mu::ui::window::CChatLogWindow::UpdatePointedLine()
{
    ChatLogRmlModel& model = m_RmlBinder.GetModel();
    int pointed = -1;

    Rml::Element* lines = m_pRmlDoc ? m_pRmlDoc->GetElementById("lines") : nullptr;
    if (lines != nullptr && IsVisible() && m_bShowChatLog)
    {
        // Raw window pixels, the same space GetAbsoluteOffset() reports in -- deliberately NOT
        // MouseX/MouseY, which CManager has already remapped into this window's layout space.
        // Comparing the two directly is what keeps this immune to the UI scale.
        const float px = g_fWindowMouseX;
        const float py = g_fWindowMouseY;

        // Clip to the scrolling well first, so a line scrolled out of view is never pointed.
        const Rml::Vector2f wellPos = lines->GetAbsoluteOffset();
        const float wellW = lines->GetClientWidth();
        const float wellH = lines->GetClientHeight();
        if (px >= wellPos.x && px < wellPos.x + wellW && py >= wellPos.y && py < wellPos.y + wellH)
        {
            for (int i = 0; i < lines->GetNumChildren(); i++)
            {
                Rml::Element* child = lines->GetChild(i);
                if (child == nullptr)
                    continue;

                // The whole row (native tested SCROLL_MIDDLE_PART_HEIGHT), including the gap under
                // a line whose background is only as tall as its text (legacy chat_log.rml).
                const Rml::Vector2f pos = child->GetAbsoluteOffset();
                const float h =
                    child->GetOffsetHeight() + child->GetBox().GetEdge(Rml::BoxArea::Margin, Rml::BoxEdge::Bottom);
                if (py >= pos.y && py < pos.y + h)
                {
                    // Only a line carrying a sender is a target, matching native -- an
                    // unattributed system line has nothing to whisper to.
                    if (i < (int)model.lines.size() && model.lines[i].hasId)
                        pointed = i;
                    break;
                }
            }
        }
    }

    if (model.pointedIndex != pointed)
    {
        model.pointedIndex = pointed;
        m_RmlBinder.MarkDirty("pointed_index");
    }

    if (pointed >= 0 && mu::ui::window::IsPress(VK_RBUTTON))
    {
        type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
        if (pvecMsgs != nullptr && pointed < (int)pvecMsgs->size())
        {
            const type_string& strID = (*pvecMsgs)[pointed]->GetID();
            if (!strID.empty() && g_pChatInputBox)
                g_pChatInputBox->SetWhsprID(strID.c_str());
        }
    }
}

void mu::ui::window::CChatLogWindow::ApplyLogicalScroll()
{
    Rml::Element* lines = m_pRmlDoc ? m_pRmlDoc->GetElementById("lines") : nullptr;
    if (lines == nullptr)
        return;

    // Native's own fPosRate, unchanged -- the fraction of the way down the list the current end
    // line represents.
    float fPosRate = 1.0f;
    const int total = (int)GetNumberOfLines(GetCurrentMsgType());
    const int showing = (int)GetNumberOfShowingLines();
    if (total > showing)
    {
        if (showing > GetCurrentRenderEndLine())
            fPosRate = 0.0f;
        else
            fPosRate = (float)(GetCurrentRenderEndLine() + 1 - showing) / (float)(total - showing);
    }

    const float range = lines->GetScrollHeight() - lines->GetClientHeight();
    if (range > 0.0f)
        lines->SetScrollTop(std::clamp(fPosRate, 0.0f, 1.0f) * range);
}

void mu::ui::window::CChatLogWindow::SyncLogicalScrollFromView()
{
    Rml::Element* lines = m_pRmlDoc ? m_pRmlDoc->GetElementById("lines") : nullptr;
    if (lines == nullptr)
        return;

    const int total = (int)GetNumberOfLines(GetCurrentMsgType());
    const int showing = (int)GetNumberOfShowingLines();
    if (total <= showing)
    {
        m_iCurrentRenderEndLine = total - 1;
        return;
    }

    const float range = lines->GetScrollHeight() - lines->GetClientHeight();
    const float fPosRate = range > 0.0f ? std::clamp(lines->GetScrollTop() / range, 0.0f, 1.0f) : 1.0f;
    m_iCurrentRenderEndLine = showing - 1 + (int)(fPosRate * (float)(total - showing) + 0.5f);
}

void mu::ui::window::CChatLogWindow::ScrollToBottomIfFollowing()
{
    if (!m_bFollowTail || !m_pRmlDoc)
        return;

    if (Rml::Element* lines = m_pRmlDoc->GetElementById("lines"))
        lines->SetScrollTop(lines->GetScrollHeight());
}

float mu::ui::window::CChatLogWindow::GetLayerDepth()
{
    return 6.1f;
}

float mu::ui::window::CChatLogWindow::GetKeyEventOrder()
{
    return 8.0f;
}

void mu::ui::window::CChatLogWindow::SeparateText(IN const type_string& strID, IN const type_string& strText,
                                                 MESSAGE_TYPE MsgType, OUT type_string& strText1,
                                                 OUT type_string& strText2)
{
    g_pRenderText->SetFont(MsgType == TYPE_GM_MESSAGE ? g_hFontBold : g_hFont);
    int max_first_line_size = CLIENT_WIDTH;
    if (!strID.empty())
    {
        const type_string strIDPart = strID + L" : ";
        max_first_line_size -= g_pRenderText->MeasureText(
            strIDPart.c_str(), static_cast<int>(strIDPart.length())).cx;
    }

    int required_size = g_pRenderText->MeasureText(strText.c_str(), static_cast<int>(strText.length())).cx;

    if (required_size <= max_first_line_size)
    {
        strText1 = strText;
        strText2 = L"";
        return;
    }

    BOOL bSpaceExist = (strText.find_last_of(L" ") != std::wstring::npos) ? TRUE : FALSE;
    int iLocToken = strText.length();

    while ((required_size > max_first_line_size) && (iLocToken > -1))
    {
        iLocToken = (bSpaceExist) ? strText.find_last_of(L" ", iLocToken - 1) : iLocToken - 1;
        if (iLocToken <= 0)
        {
            strText1.clear();
            strText2 = strText;
            return;
        }
        required_size = g_pRenderText->MeasureText(strText.c_str(), iLocToken).cx;
    }

    strText1 = strText.substr(0, iLocToken);
    strText2 = strText.substr(iLocToken, strText.length() - iLocToken);
}

bool mu::ui::window::CChatLogWindow::CheckFilterText(const type_string& strTestText)
{
    auto vi_filters = m_vecFilters.begin();
    for (; vi_filters != m_vecFilters.end(); vi_filters++)
    {
        if (vi_filters->find(strTestText))
        {
            return true;
        }
    }

    return false;
}

void mu::ui::window::CChatLogWindow::UpdateWndSize()
{
    m_WndSize.cx = WND_WIDTH;
    m_WndSize.cy = (SCROLL_MIDDLE_PART_HEIGHT * GetNumberOfShowingLines()) + (SCROLL_TOP_BOTTOM_PART_HEIGHT * 2) + (WND_TOP_BOTTOM_EDGE * 2);
}

void mu::ui::window::CChatLogWindow::UpdateScrollPos()
{
    // Used to place the native scroll thumb. RmlUi owns the scrollbar now, so this just asks for
    // the logical position to be re-applied -- which is what its callers (CChatInputBox, right
    // after resizing the window) actually want.
    m_bScrollRequest = true;
}

void mu::ui::window::CChatLogWindow::AddFilterWord(const type_string& strWord)
{
    if (m_vecFilters.size() > 5)
        return;

    auto vi_filters = m_vecFilters.begin();
    for (; vi_filters != m_vecFilters.end(); vi_filters++)
    {
        if (0 == (*vi_filters).compare(strWord))
        {
            return;
        }
    }

    m_vecFilters.push_back(strWord);
}

void mu::ui::window::CChatLogWindow::ClearAll()
{
    m_bLinesDirty = true;
    for (int i = TYPE_ALL_MESSAGE; i < NUMBER_OF_TYPES; i++)
    {
        Clear((MESSAGE_TYPE)i);
    }

    m_iCurrentRenderEndLine = -1;
}

mu::ui::window::CChatLogWindow::type_vector_msgs* mu::ui::window::CChatLogWindow::GetMsgs(MESSAGE_TYPE MsgType)
{
    switch (MsgType)
    {
    case TYPE_ALL_MESSAGE:
        return &m_vecAllMsgs;
    case TYPE_CHAT_MESSAGE:
        return &m_VecChatMsgs;
    case TYPE_WHISPER_MESSAGE:
        return &m_vecWhisperMsgs;
    case TYPE_SYSTEM_MESSAGE:
        return &m_VecSystemMsgs;
    case TYPE_ERROR_MESSAGE:
        return &m_vecErrorMsgs;
    case TYPE_PARTY_MESSAGE:
        return &m_vecPartyMsgs;
    case TYPE_GUILD_MESSAGE:
        return &m_vecGuildMsgs;
    case TYPE_UNION_MESSAGE:
        return &m_vecUnionMsgs;
    case TYPE_GENS_MESSAGE:
        return &m_vecGensMsgs;
    case TYPE_GM_MESSAGE:
        return &m_vecGMMsgs;
    }

    return nullptr;
}

void mu::ui::window::CChatLogWindow::ChangeMessage(MESSAGE_TYPE MsgType)
{
    m_bLinesDirty = true;
    m_CurrentRenderMsgType = MsgType;

    type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
    {
        return;
    }

    m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

mu::ui::window::MESSAGE_TYPE mu::ui::window::CChatLogWindow::GetCurrentMsgType() const
{
    return m_CurrentRenderMsgType;
}

void mu::ui::window::CChatLogWindow::ShowChatLog()
{
    m_bShowChatLog = true;

    type_vector_msgs* pvecMsgs = GetMsgs(GetCurrentMsgType());
    if (pvecMsgs == nullptr)
    {
        return;
    }

    m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

void mu::ui::window::CChatLogWindow::HideChatLog()
{
    m_bShowChatLog = false;
}

mu::ui::window::CSystemLogWindow::CSystemLogWindow()
{
    Init();
}

mu::ui::window::CSystemLogWindow::~CSystemLogWindow()
{
    Release();
}

void mu::ui::window::CSystemLogWindow::Init()
{
    m_pNewUIMng = nullptr;
    m_WndPos.x = m_WndPos.y = 0;
    m_WndSize.cx = WND_WIDTH; m_WndSize.cy = 0;
    m_nShowingLines = 6;
    m_iCurrentRenderEndLine = -1;
    m_fBackAlpha = 0.6f;
    m_bShowMessages = true;
}


bool mu::ui::window::CSystemLogWindow::Create(CManager* pNewUIMng, int x, int y)
{
    Release();

    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SYSTEMLOGWINDOW, this);
    m_WndPos.x = x;
    m_WndPos.y = y;
    return true;
}

void mu::ui::window::CSystemLogWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    ClearAll();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    Init();
}

void mu::ui::window::CSystemLogWindow::SetPosition(int x, int y)
{
    m_WndPos.x = x;
    m_WndPos.y = y;
}

void mu::ui::window::CSystemLogWindow::AddText(const type_string& strText, MESSAGE_TYPE MsgType)
{
    m_bLinesDirty = true;
    if (strText.empty())
    {
        return;
    }

    type_vector_msgs* pvecMsgs = &m_vecAllMsgs;
    if (pvecMsgs == nullptr)
    {
        assert(!"Empty Message");
        return;
    }

    auto pMsgText = new CMessageText;
    if (!pMsgText->Create(L"", strText, MsgType))
    {
        delete pMsgText;
        return;
    }

    if (pvecMsgs->size() >= MAX_NUMBER_OF_LINES)
    {
        RemoveFrontLine();
    }

    pvecMsgs->push_back(pMsgText);

    m_iCurrentRenderEndLine = pvecMsgs->size() - 1;
}

void mu::ui::window::CSystemLogWindow::RemoveFrontLine()
{
    m_bLinesDirty = true;
    auto vi = m_vecAllMsgs.begin();
    if (vi != m_vecAllMsgs.end())
    {
        delete (*vi);
        vi = m_vecAllMsgs.erase(vi);
    }
}

int mu::ui::window::CSystemLogWindow::GetCurrentRenderEndLine() const
{
    return m_iCurrentRenderEndLine;
}

bool mu::ui::window::CSystemLogWindow::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CSystemLogWindow::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CSystemLogWindow::Update()
{
    BuildRmlUi();
    SyncRmlModel();

    return true;
}

bool mu::ui::window::CSystemLogWindow::Render()
{
    // RmlUi's own document draws every line now; kept because CObject requires the override.
    return true;
}

void mu::ui::window::CSystemLogWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "system_log",
        [this](Rml::DataModelConstructor& c, SystemLogRmlModel& model)
        {
            // Re-registered in full on every call, including from ReloadRmlTheme() -- same reason
            // CCharMakeWin::BuildRmlUi() documents. Registering ChatLogLineEntry here as well as
            // in the chat log's own model is fine: each RmlModelBinder owns its own
            // DataTypeRegister.
            auto line = c.RegisterStruct<ChatLogLineEntry>();
            line.RegisterMember("text", &ChatLogLineEntry::text);
            line.RegisterMember("kind", &ChatLogLineEntry::kind);
            line.RegisterMember("has_id", &ChatLogLineEntry::hasId);
            c.RegisterArray<Rml::Vector<ChatLogLineEntry>>();

            c.Bind("lines", &model.lines);
            c.Bind("back_color", &model.backColor);
            c.Bind("panel_x", &model.panelX);
            c.Bind("panel_y", &model.panelY);
            c.Bind("row_px", &model.rowPx);
            c.Bind("line_px", &model.linePx);
            c.Bind("text_px", &model.textPx);
        });

    if (modelCreated)
    {
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                     "Data/Interface/RmlUi/system_log.rml");
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }
}

void mu::ui::window::CSystemLogWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    m_bLinesDirty = true;
}

void mu::ui::window::CSystemLogWindow::SyncDocVisibility(bool sceneAllowsShow)
{
    m_bSceneAllowsShow = sceneAllowsShow;
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible() && m_bShowMessages && sceneAllowsShow);
}

void mu::ui::window::CSystemLogWindow::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    // m_bShowMessages is the input box's own "system messages" toggle; IsVisible() is the window's.
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible() && m_bShowMessages && m_bSceneAllowsShow);

    SystemLogRmlModel& model = m_RmlBinder.GetModel();

    char backColor[48] = { 0, };
    const int alpha = static_cast<int>(std::clamp(m_fBackAlpha, 0.0f, 1.0f) * 255.0f);
    snprintf(backColor, sizeof(backColor), "rgba(0,0,0,%d)", alpha);
    if (model.backColor != backColor)
    {
        model.backColor = backColor;
        m_RmlBinder.MarkDirty("back_color");
    }

    SyncNativeGeometry();

    if (m_bLinesDirty)
    {
        m_bLinesDirty = false;
        RebuildLineModel();
        m_RmlBinder.MarkDirty("lines");
    }
}

// Native's RenderMessages(): the first line at the window position plus FONT_LEADING on both axes,
// one row every MeasureText("Q").cy * 1.2 (logical), each line's background as tall as the text.
void mu::ui::window::CSystemLogWindow::SyncNativeGeometry()
{
    const auto transform = UI::Scaling::GetActiveTransform();
    g_pRenderText->SetFont(g_hFont);
    const int textHeight = g_pRenderText->MeasureText(L"Q", 1).cy;
    const int rowHeight = std::max(1, static_cast<int>(static_cast<float>(textHeight) * 1.2f));

    SystemLogRmlModel& model = m_RmlBinder.GetModel();
    auto syncFloat = [&](float SystemLogRmlModel::* field, const char* name, float value)
    {
        if (model.*field != value)
        {
            model.*field = value;
            m_RmlBinder.MarkDirty(name);
        }
    };
    syncFloat(&SystemLogRmlModel::panelX, "panel_x",
              UI::Scaling::PositionX(transform, static_cast<float>(m_WndPos.x + FONT_LEADING)));
    syncFloat(&SystemLogRmlModel::panelY, "panel_y",
              UI::Scaling::PositionY(transform, static_cast<float>(m_WndPos.y + FONT_LEADING)));
    syncFloat(&SystemLogRmlModel::rowPx, "row_px", UI::Scaling::SizeY(transform, static_cast<float>(rowHeight)));
    syncFloat(&SystemLogRmlModel::linePx, "line_px", UI::Scaling::SizeY(transform, static_cast<float>(textHeight)));
    syncFloat(&SystemLogRmlModel::textPx, "text_px",
              UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform));
}

void mu::ui::window::CSystemLogWindow::RebuildLineModel()
{
    SystemLogRmlModel& model = m_RmlBinder.GetModel();
    model.lines.clear();
    model.lines.reserve(m_vecAllMsgs.size());

    for (const CMessageText* pMsgText : m_vecAllMsgs)
    {
        if (pMsgText == nullptr)
            continue;

        ChatLogLineEntry entry;
        // Native's own two-way split: system messages blue, EVERYTHING else the error red -- not a
        // per-type mapping like the chat log's.
        entry.kind = (pMsgText->GetType() == TYPE_SYSTEM_MESSAGE) ? "system" : "error";
        entry.text = StringUtils::WideToNarrow(pMsgText->GetText().c_str());
        model.lines.push_back(std::move(entry));
    }
}

float mu::ui::window::CSystemLogWindow::GetLayerDepth()
{
    return 6.05f;
}

float mu::ui::window::CSystemLogWindow::GetKeyEventOrder()
{
    return 8.0f;
}

void mu::ui::window::CSystemLogWindow::ClearAll()
{
    m_bLinesDirty = true;
    auto vi_msg = m_vecAllMsgs.begin();
    for (; vi_msg != m_vecAllMsgs.end(); vi_msg++)
        delete (*vi_msg);
    m_vecAllMsgs.clear();

    m_iCurrentRenderEndLine = -1;
}

bool mu::ui::window::CSystemLogWindow::CheckChatRedundancy(const type_string& strText, int iSearchLine/* = 1*/)
{
    if (m_vecAllMsgs.empty()) return false;
    auto vri_msgs = m_vecAllMsgs.rbegin();
    for (int i = 0; (i < iSearchLine) || (vri_msgs != m_vecAllMsgs.rend()); vri_msgs++, i++)
        if (0 == (*vri_msgs)->GetText().compare(strText)) return true;
    return false;
}
