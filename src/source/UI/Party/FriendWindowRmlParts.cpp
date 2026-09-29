// What each window of the friends family draws, as RmlUi parts (FriendWindowView.h): the
// counterparts of CUIBaseWindow::Render() and the RenderSub() functions in UIWindows.cpp, from
// the same geometry, in the same order. The parts are named by what they are; the legacy theme
// (friend_window.rcss) gives them the original's SetLineColor() colours and window art.

#include "stdafx.h"

#include "UI/Party/FriendWindowView.h"
#include "UI/Party/UIWindows.h"
#include "UI/Party/FriendWindow.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

#include <algorithm>

namespace
{
const DWORD TextNormal = RGBA(230, 220, 200, 255);
const DWORD TextHighlight = RGBA(255, 255, 255, 255);
const DWORD TextSelectedRow = RGBA(0, 0, 0, 255);

float TextWidth(const wchar_t* text)
{
    g_pRenderText->SetFont(g_hFont);
    return static_cast<float>(g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx);
}

float TextHeight(const wchar_t* text)
{
    g_pRenderText->SetFont(g_hFont);
    return static_cast<float>(g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cy);
}

// A list line's selection box: RenderColor() in whatever colour the list's render left current --
// the scroll track's SetLineColor(2), or SetLineColor(0) once a line's RenderCheckBox() ran.
const char* SelectedRowRole(bool afterCheckBox)
{
    return afterCheckBox ? "row-selected after-check-box" : "row-selected";
}

// CUIChatPalListBox::Render(): its scroll bar, then its lines (a chat room's members and the
// friends it can invite).
void CollectChatPalList(FriendWindowRmlBuilder& view, CUIChatPalListBox& list)
{
    view.ListScrollBar(list);
    const TextListScrollBarGeometry bar = list.ComputeLegacyScrollBar();
    const auto listX = static_cast<float>(list.GetPosition_x());
    list.ForEachRenderLine(
        [&](int line, GUILDLIST_TEXT& item, bool selected)
        {
            const auto lineY = static_cast<float>(list.GetRenderLinePos_y(line));
            if (selected)
                view.Fill(SelectedRowRole(false), listX, lineY - 3, list.GetWidth() - bar.barWidth + 1, 13);
            const DWORD color = selected ? TextSelectedRow : TextNormal;
            view.Text(item.m_szID, listX + 3 + list.GetColumnPos_x(0), lineY, color);
            if (list.GetLayout() == 1)
            {
                wchar_t server[MAX_TEXT_LENGTH + 1] = {0};
                if (item.m_Server == 0xFF || item.m_Server == 0xFE || item.m_Server == 0xFC)
                    mu_swprintf(server, I18N::Game::Offline1039);
                else if (item.m_Server == 0xFD)
                    mu_swprintf(server, I18N::Game::CannotUse);
                else
                    mu_swprintf(server, I18N::Game::_2dServer, item.m_Server + 1);
                view.Text(server, listX + 3 + 4 + list.GetColumnPos_x(1), lineY, color);
            }
        });
}

// RenderWindowVLine(): a vertical divider, a dark band between two lines.
void CollectWindowVLine(FriendWindowRmlBuilder& view, float x, float y, float height)
{
    view.Fill("separator", x, y, 1, height);
    view.Fill("separator", x + 4, y, 1, height);
    view.Fill("frame-band", x + 1, y, 3, height);
}

// CUISimpleChatListBox::RenderDataLine()'s colours by message type: the sender's, then the text's.
DWORD ChatNameColor(int type)
{
    switch (type)
    {
    case 1:
        return RGBA(100, 150, 255, 255);
    case 2:
        return RGBA(255, 30, 0, 255);
    case 3:
        return RGBA(239, 220, 205, 255);
    default:
        return RGBA(0, 0, 0, 255);
    }
}

DWORD ChatTextColor(int type)
{
    switch (type)
    {
    case 1:
        return RGBA(70, 165, 210, 255);
    case 2:
        return RGBA(255, 30, 0, 255);
    case 3:
        return TextNormal;
    default:
        return RGBA(0, 0, 0, 255);
    }
}
} // namespace

void CUIBaseWindow::CollectRmlView(FriendWindowRmlBuilder& view)
{
    std::call_once(_controlsInitialized, [this]() { InitControls(); });

    const auto x = static_cast<float>(m_iPos_x);
    const auto y = static_cast<float>(m_iPos_y);
    const auto w = static_cast<float>(m_iWidth);
    const auto h = static_cast<float>(m_iHeight);

    if (m_iOptions != UIWINDOWSTYLE_NULL)
    {
        const float backTop = y + 5;
        const float backHeight = CheckOption(UIWINDOWSTYLE_FRAME) ? h - 10 : h;
        CUIPhotoViewer* photo = GetRmlPhoto();
        if (photo == nullptr)
        {
            view.Fill("window-back", x, backTop, w, backHeight);
        }
        else
        {
            // The photo viewer's 3D character is drawn natively after the RmlUi background layer
            // and before this document: its box's share of the back goes under it, the rest stays
            // here around it.
            const float px = std::clamp(static_cast<float>(photo->GetPosition_x()), x, x + w);
            const float pr = std::clamp(static_cast<float>(photo->GetPosition_x() + photo->GetWidth()), px, x + w);
            const float py = std::clamp(static_cast<float>(photo->GetPosition_y()), backTop, backTop + backHeight);
            const float pb =
                std::clamp(static_cast<float>(photo->GetPosition_y() + photo->GetHeight()), py, backTop + backHeight);
            view.Fill("window-back", x, backTop, w, py - backTop);
            view.Fill("window-back", x, py, px - x, pb - py);
            view.Fill("window-back", pr, py, x + w - pr, pb - py);
            view.Fill("window-back", x, pb, w, backTop + backHeight - pb);
            view.UnderlayFill("window-back", px, py, pr - px, pb - py);
        }
    }

    CollectRmlContent(view);

    const bool backWindow = g_pWindowMgr->GetTopWindowUIID() != GetUIID();
    if (CheckOption(UIWINDOWSTYLE_FRAME))
    {
        if (CheckOption(UIWINDOWSTYLE_TITLEBAR))
        {
            view.Sprite("title-bar", x + 6, y + 5, w - 12, 15);
            view.Fill(backWindow ? "title-edge-back" : "title-edge-light", x + 5, y + 5, 1, 1);
            view.Fill(backWindow ? "title-edge-back" : "title-edge", x + 5, y + 6, 1, 13);
            view.Fill("title-edge-back", x + 5, y + 19, 1, 1);
            view.Fill("title-edge-right-top", x + w - 6, y + 5, 1, 1);
            view.Fill("title-edge-right", x + w - 6, y + 6, 1, 13);
            view.Fill("title-edge-right-bottom", x + w - 6, y + 19, 1, 1);
        }

        // DrawOutLine()
        view.Fill("frame-outer", x, y, w, 1);
        view.Fill("frame-band", x, y + 1, w, 3);
        view.Fill("frame-inner", x, y + 4, w, 1);
        view.Fill("frame-inner", x, y + h - 5, w, 1);
        view.Fill("frame-band", x, y + h - 4, w, 3);
        view.Fill("frame-outer", x, y + h - 1, w, 1);
        view.Fill("frame-outer", x, y, 1, h);
        view.Fill("frame-band", x + 1, y + 5, 3, h - 10);
        view.Fill("frame-inner", x + 4, y + 1, 1, h - 2);
        view.Fill("frame-inner", x + w - 5, y + 1, 1, h - 2);
        view.Fill("frame-band", x + w - 4, y + 5, 3, h - 10);
        view.Fill("frame-outer", x + w - 1, y, 1, h);
        view.Fill("frame-corner", x + 2, y + 2, 1, 1);
        view.Fill("frame-corner", x + w - 3, y + 2, 1, 1);
        view.Fill("frame-corner", x + 2, y + h - 3, 1, 1);
        if (!CheckOption(UIWINDOWSTYLE_RESIZEABLE))
            view.Fill("frame-corner", x + w - 3, y + h - 3, 1, 1);
    }
    if (CheckOption(UIWINDOWSTYLE_TITLEBAR))
    {
        view.Fill("frame-inner", x + 5, y + 20, w - 10, 1);

        g_pRenderText->SetFont(g_hFontBold);
        wchar_t title[256] = {0};
        CutText3(m_strTitle.c_str(), title, m_iWidth - 50, 1, 256);
        view.Text(title, x + 9, y + 8, backWindow ? RGBA(115, 110, 100, 255) : RGBA(230, 220, 200, 255), true);
        g_pRenderText->SetFont(g_hFont);

        if (CheckOption(UIWINDOWSTYLE_MINBUTTON))
            view.Sprite("control-minimize", x + w - (CheckOption(UIWINDOWSTYLE_MAXBUTTON) ? 38 : 27), y + 8, 9, 9);
        if (CheckOption(UIWINDOWSTYLE_MAXBUTTON))
            view.Sprite(m_bIsMaximize ? "control-restore" : "control-maximize", x + w - 27, y + 8, 9, 9);
        view.Sprite("control-close", x + w - 16, y + 8, 9, 9);
    }
    if (CheckOption(UIWINDOWSTYLE_RESIZEABLE))
        view.Sprite("resize-grip", x + w - 10, y + h - 10, 9, 9);
}

// CUIFriendWindow::RenderSub(): the current tab, then the tab strip over it.
void CUIFriendWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    SyncTabLayout();

    switch (m_iTabIndex)
    {
    case 0:
        m_FriendListWnd.CollectRmlView(view);
        break;
    case 1:
        m_LetterBoxWnd.CollectRmlView(view);
        break;
    case 2:
        m_ChatRoomListWnd.CollectRmlView(view);
        break;
    default:
        break;
    }

    // RenderTabStrip()
    view.Fill("panel", RPos_x(0), RPos_y(0), RWidth(), 20);
    const auto stripY = static_cast<float>(RPos_y(2));
    for (int i = 0; i < 3; ++i)
    {
        const auto tabX = static_cast<float>(RPos_x(0) + i * 53);
        if (i == m_iTabIndex)
        {
            view.Fill("separator", tabX, stripY, 53, 1);
            view.Fill("separator", tabX - 1, stripY, 1, 19);
            view.Fill("separator", tabX + 52, stripY, 1, 19);
            view.Fill("tab-on", tabX, stripY + 1, 52, 18);
        }
        else
        {
            view.Fill("separator", tabX, stripY + 1, 53, 1);
            view.Fill("separator", tabX + 52, stripY + 1, 1, 18);
            view.Fill("separator", tabX, stripY + 18, 53, 1);
            view.Fill("tab-off", tabX, stripY + 2, 52, 16);
        }
    }
    view.Fill("separator", RPos_x(53 * 3), stripY + 18, RWidth() - 53 * 3, 1);

    const std::pair<const wchar_t*, int> labels[] = {
        {m_FriendListWnd.GetTitle(), 0}, {m_LetterBoxWnd.GetTitle(), 54}, {m_ChatRoomListWnd.GetTitle(), 107}};
    for (int i = 0; i < 3; ++i)
    {
        const wchar_t* label = labels[i].first;
        if (label == nullptr)
            continue;
        const DWORD color = (m_iTabIndex == i || m_iTabMouseOverIndex == i) ? TextHighlight : TextNormal;
        view.Text(label, RPos_x(labels[i].second) + (52 - TextWidth(label) + 0.5f) / 2,
                  RPos_y(0) + (24 - TextHeight(label) + 0.5f) / 2, color);
    }

    const wchar_t* refuse = I18N::Game::RefuseChat;
    const float refuseWidth = TextWidth(refuse);
    const float refuseY = RPos_y(0) + (24 - TextHeight(refuse) + 0.5f) / 2;
    view.Text(refuse, RPos_x(0) + RWidth() - refuseWidth - 2, refuseY, TextNormal);
    const float checkX = RPos_x(0) + RWidth() - refuseWidth - 2 - 14;
    view.CheckBox(static_cast<float>(static_cast<int>(checkX - 1)), static_cast<float>(static_cast<int>(refuseY - 1)),
                  g_pWindowMgr->GetChatReject() == TRUE);
}

// CUIFriendListTabWindow::RenderSub() and CUIChatPalListBox::Render().
void CUIFriendListTabWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    SyncControlLayout();

    CUIChatPalListBox& list = m_PalListBox;
    const int listHeight = list.GetHeight();
    view.Fill("panel", RPos_x(0), RPos_y(18 + listHeight), RWidth(), RHeight() - listHeight - 18);

    view.ListScrollBar(list);
    const TextListScrollBarGeometry bar = list.ComputeLegacyScrollBar();
    const auto listX = static_cast<float>(list.GetPosition_x());
    list.ForEachRenderLine(
        [&](int line, GUILDLIST_TEXT& item, bool selected)
        {
            const auto lineY = static_cast<float>(list.GetRenderLinePos_y(line));
            if (selected)
                view.Fill(SelectedRowRole(false), listX, lineY - 3, list.GetWidth() - bar.barWidth + 1, 13);
            const DWORD color = selected ? TextSelectedRow : TextNormal;
            view.Text(item.m_szID, listX + 3 + list.GetColumnPos_x(0), lineY, color);
            if (list.GetLayout() == 1)
            {
                wchar_t server[MAX_TEXT_LENGTH + 1] = {0};
                if (item.m_Server == 0xFF || item.m_Server == 0xFE || item.m_Server == 0xFC)
                    mu_swprintf(server, I18N::Game::Offline1039);
                else if (item.m_Server == 0xFD)
                    mu_swprintf(server, I18N::Game::CannotUse);
                else
                    mu_swprintf(server, I18N::Game::_2dServer, item.m_Server + 1);
                view.Text(server, listX + 3 + 4 + list.GetColumnPos_x(1), lineY, color);
            }
        });

    view.Button(m_AddFriendButton);
    view.Button(m_DelFriendButton);
    view.Button(m_TalkButton);
    view.Button(m_LetterButton);

    view.Fill("separator", RPos_x(0), RPos_y(16), RWidth(), 1);
    view.Fill("list-header", RPos_x(0), RPos_y(0), RWidth(), 16);
    view.Fill("separator", RPos_x(0), RPos_y(17 + listHeight), RWidth(), 1);
    view.Fill("separator", RPos_x(0) + list.GetColumnPos_x(1), RPos_y(17), 1, RHeight() - 22 - 17);
    view.Fill("header-divider", RPos_x(0) + list.GetColumnPos_x(1), RPos_y(3), 1, 10);

    const auto headerColor = [&](int column)
    {
        const bool hover =
            CheckMouseIn(RPos_x(0) + list.GetColumnPos_x(column), RPos_y(0), list.GetColumnWidth(column), 19) == TRUE;
        return (hover || g_pFriendList->GetCurrentSortType() == column) ? TextHighlight : TextNormal;
    };
    view.Text(I18N::Game::FriendSName, RPos_x(4) + list.GetColumnPos_x(0), RPos_y(3), headerColor(0));
    view.Text(I18N::Game::Server, RPos_x(4) + list.GetColumnPos_x(1), RPos_y(3), headerColor(1));
}

// CUIChatRoomListTabWindow::RenderSub() and CUIWindowListBox::Render().
void CUIChatRoomListTabWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    SyncControlLayout();

    CUIWindowListBox& list = m_WindowListBox;
    const int listHeight = list.GetHeight();
    view.Fill("panel", RPos_x(0), RPos_y(18 + listHeight), RWidth(), RHeight() - listHeight - 18);

    view.ListScrollBar(list);
    const TextListScrollBarGeometry bar = list.ComputeLegacyScrollBar();
    const auto listX = static_cast<float>(list.GetPosition_x());
    list.ForEachRenderLine(
        [&](int line, WINDOWLIST_TEXT& item, bool selected)
        {
            const auto lineY = static_cast<float>(list.GetRenderLinePos_y(line));
            if (selected)
                view.Fill(SelectedRowRole(false), listX, lineY - 3, list.GetWidth() - bar.barWidth + 1, 13);
            view.Text(item.m_szTitle, listX + 8, lineY, selected ? TextSelectedRow : TextNormal);
        });

    view.Button(m_HideAllButton);

    view.Fill("separator", RPos_x(0), RPos_y(0) + 16, RWidth(), 1);
    view.Fill("list-header", RPos_x(0), RPos_y(0), RWidth(), 16);
    view.Fill("separator", RPos_x(0), RPos_y(17 + listHeight), RWidth(), 1);
    view.Text(I18N::Game::WindowTitle, RPos_x(8), RPos_y(3), TextNormal);
}

// CUILetterBoxTabWindow::RenderSub() and CUILetterListBox::Render().
void CUILetterBoxTabWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    SyncControlLayout();

    CUILetterListBox& list = m_LetterListBox;
    const int listHeight = list.GetHeight();
    view.Fill("panel", RPos_x(0), RPos_y(18 + listHeight), RWidth(), RHeight() - listHeight - 18);

    view.ListScrollBar(list);
    const TextListScrollBarGeometry bar = list.ComputeLegacyScrollBar();
    const auto listX = static_cast<float>(list.GetPosition_x());
    bool checkBoxDrawn = false;
    list.ForEachRenderLine(
        [&](int line, LETTERLIST_TEXT& item, bool selected)
        {
            const auto lineY = static_cast<float>(list.GetRenderLinePos_y(line));
            if (selected)
                view.Fill(SelectedRowRole(checkBoxDrawn), listX, lineY - 3, list.GetWidth() - bar.barWidth + 1, 13);
            const DWORD color = selected ? TextSelectedRow : TextNormal;
            view.CheckBox(listX + 1, lineY - 1, item.m_bIsSelected == TRUE);
            checkBoxDrawn = true;
            if (item.m_bIsRead == TRUE)
                view.Sprite("letter-read", listX + 1 + 10, lineY - 2, 13, 10);
            else
                view.Sprite("letter-unread", listX + 1 + 10, lineY - 1, 13, 9);
            view.Text(item.m_szID, listX + 4 + list.GetColumnPos_x(1), lineY, color, false,
                      static_cast<float>(list.GetColumnWidth(1) - 4), 0);
            view.Text(item.m_szDate, listX + list.GetColumnPos_x(2), lineY, color, false,
                      static_cast<float>(list.GetColumnWidth(2)), 1);
            const float maxWidth = list.GetWidth() - bar.barWidth - list.GetColumnPos_x(3) - 4;
            view.Text(item.m_szText, listX + 4 + list.GetColumnPos_x(3), lineY, color, false, maxWidth, 0);
        });

    view.Fill("separator", RPos_x(0), RPos_y(16), RWidth(), 1);
    view.Fill("list-header", RPos_x(0), RPos_y(0), RWidth(), 16);
    view.Fill("separator", RPos_x(0), RPos_y(17 + listHeight), RWidth(), 1);
    for (int column = 1; column <= 3; ++column)
        view.Fill("separator", RPos_x(0) + list.GetColumnPos_x(column), RPos_y(17), 1, RHeight() - 22 - 17);
    for (int column = 1; column <= 3; ++column)
        view.Fill("header-divider", RPos_x(0) + list.GetColumnPos_x(column), RPos_y(3), 1, 10);

    view.CheckBox(RPos_x(1), RPos_y(3), m_bCheckAllState == TRUE);
    view.Sprite("letter-unread", RPos_x(1 + 10), RPos_y(3), 13, 9);

    const auto headerColor = [&](int column)
    {
        const bool hover =
            CheckMouseIn(RPos_x(0) + list.GetColumnPos_x(column), RPos_y(0), list.GetColumnWidth(column), 19) == TRUE;
        return (hover || g_pLetterList->GetCurrentSortType() == column) ? TextHighlight : TextNormal;
    };
    view.Text(I18N::Game::Sender, RPos_x(4) + list.GetColumnPos_x(1), RPos_y(3), headerColor(1));
    view.Text(I18N::Game::DateRcvd, RPos_x(4) + list.GetColumnPos_x(2), RPos_y(3), headerColor(2));
    view.Text(I18N::Game::Title1030, RPos_x(4) + list.GetColumnPos_x(3), RPos_y(3), headerColor(3));

    view.Button(m_WriteButton);
    view.Button(m_ReadButton);
    view.Button(m_ReplyButton);
    view.Button(m_DeleteButton);
}

// CUITextInputWindow::RenderSub(): the add-friend dialog.
void CUITextInputWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_AddButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CancelButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    }

    view.Button(m_AddButton);
    view.Button(m_CancelButton);
    view.Field(0, m_TextInputBox);
}

// CUIQuestionWindow::RenderSub(): a yes / no or OK question.
void CUIQuestionWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_AddButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CancelButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    }

    view.Text(m_szCaption[0], RPos_x(5), RPos_y(8), TextHighlight);
    if (m_szCaption[1][0] != L'\0')
    {
        const float firstLineHeight = m_szCaption[0][0] != L'\0' ? TextHeight(m_szCaption[0]) : 0.f;
        view.Text(m_szCaption[1], RPos_x(5), RPos_y(8) + firstLineHeight, TextHighlight);
    }

    view.Button(m_AddButton);
    if (m_iDialogType == 0)
        view.Button(m_CancelButton);
}

// CUIChatWindow::RenderSub(): a chat room -- its lines, the members (and the friends to invite),
// the input line.
void CUIChatWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_InviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CloseInviteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        if (GetState() == UISTATE_RESIZE)
        {
            m_ChatListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_PalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_InvitePalListBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    // CUISimpleChatListBox::Render(): the sender (on a message's first line) and the text after it.
    view.ListScrollBar(m_ChatListBox);
    const auto chatX = static_cast<float>(m_ChatListBox.GetPosition_x());
    const auto chatBottom = static_cast<float>(m_ChatListBox.GetPosition_y());
    m_ChatListBox.ForEachRenderLine(
        [&](int line, WHISPER_TEXT& item, bool)
        {
            const float lineY = chatBottom - 16 - static_cast<float>(line) * 13;
            float nameWidth = 0.f;
            if (item.m_szID[0] != L'\0')
            {
                wchar_t name[MAX_TEXT_LENGTH + 1] = {0};
                mu_swprintf(name, L"%ls: ", item.m_szID);
                view.Text(name, chatX + 8, lineY, ChatNameColor(item.m_iType));
                nameWidth = TextWidth(name);
            }
            view.Text(item.m_szText, chatX + 8 + nameWidth, lineY, ChatTextColor(item.m_iType));
        });

    const bool showMembers = m_PalListBox.GetLineNum() > 2 || m_iShowType >= 2;
    if (showMembers)
        CollectChatPalList(view, m_PalListBox);
    if (m_iShowType >= 2)
    {
        CollectChatPalList(view, m_InvitePalListBox);
        CollectWindowVLine(view, static_cast<float>(RPos_x(0) + RWidth() - 160), static_cast<float>(RPos_y(0)),
                           static_cast<float>(RHeight() - 16));
    }
    if (showMembers)
        CollectWindowVLine(view, static_cast<float>(RPos_x(0) + RWidth() - 80), static_cast<float>(RPos_y(0)),
                           static_cast<float>(RHeight() - 16));

    view.Fill("separator", RPos_x(0), RPos_y(0) + RHeight() - 16, RWidth(), 1);
    view.Fill("panel", RPos_x(0), RPos_y(0) + RHeight() - 15, RWidth(), 15);

    view.Button(m_InviteButton);
    view.Field(0, m_TextInputBox);
    if (m_iShowType >= 2)
        view.Button(m_CloseInviteButton);
}

// CUILetterWriteWindow::RenderSub(): receiver, title and text fields and the buttons; the photo
// viewer (RenderOver()) stays native.
void CUILetterWriteWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_SendButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CloseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_MailtoInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TitleInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_TextInputBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_PrevPoseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_NextPoseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_Photo.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
    }

    view.Fill("panel", RPos_x(0), RPos_y(0), RWidth(), 29);
    view.Fill("separator", RPos_x(0), RPos_y(14), RWidth(), 1);
    view.Fill("separator", RPos_x(0), RPos_y(29), RWidth(), 1);
    view.Fill("panel", RPos_x(0), RPos_y(0) + RHeight() - 19, RWidth(), 19);
    view.Fill("separator", RPos_x(0), RPos_y(0) + RHeight() - 19, RWidth(), 1);

    // RT3_SORT_RIGHT in a box as wide as "Receiver": a label that fits is moved to its right edge.
    const float labelBox = TextWidth(I18N::Game::Receiver);
    for (const auto& [label, top] : {std::pair{I18N::Game::Receiver, 3}, std::pair{I18N::Game::Title, 18}})
    {
        const float width = TextWidth(label);
        view.Text(label, RPos_x(3) + (width < labelBox ? labelBox - width : 0.f), RPos_y(top), TextNormal);
    }

    view.Field(0, m_MailtoInputBox);
    view.Field(1, m_TitleInputBox);
    view.Field(2, m_TextInputBox);

    view.Button(m_SendButton);
    view.Button(m_CloseButton);
    if (m_iShowType == 1)
    {
        view.Button(m_PrevPoseButton);
        view.Button(m_NextPoseButton);
    }
}

// CUILetterReadWindow::RenderSub(): the letter's lines, the sender line and the buttons; the photo
// viewer (RenderOver()) stays native.
void CUILetterReadWindow::CollectRmlContent(FriendWindowRmlBuilder& view)
{
    if (GetState() == UISTATE_MOVE || GetState() == UISTATE_RESIZE)
    {
        m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_ReplyButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_DeleteButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_CloseButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_PrevButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_NextButton.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        m_Photo.SendUIMessageDirect(UI_MESSAGE_P_MOVE, 0, 0);
        if (GetState() == UISTATE_RESIZE)
        {
            m_LetterTextBox.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
            m_Photo.SendUIMessageDirect(UI_MESSAGE_P_RESIZE, 0, 0);
        }
    }

    // CUILetterTextListBox::Render()
    view.ListScrollBar(m_LetterTextBox);
    const auto textX = static_cast<float>(m_LetterTextBox.GetPosition_x());
    m_LetterTextBox.ForEachRenderLine(
        [&](int line, LETTER_TEXT& item, bool)
        {
            view.Text(item.m_szText, textX + 10, static_cast<float>(m_LetterTextBox.GetRenderLinePos_y(line)),
                      TextNormal);
        });

    const float bottom = static_cast<float>(RPos_y(0) + RHeight());
    if (m_iShowType >= 2)
    {
        view.Fill("separator", RPos_x(0) + RWidth() - 120, bottom - 19, 1, 19);
        view.Fill("separator", RPos_x(0), bottom - 20, RWidth() - 120, 1);
        view.Fill("panel", RPos_x(0), bottom - 19, RWidth() - 120, 18);
    }
    else
    {
        view.Fill("separator", RPos_x(0), bottom - 20, RWidth(), 1);
        view.Fill("panel", RPos_x(0), bottom - 19, RWidth(), 18);
    }
    view.Fill("separator", RPos_x(0), RPos_y(14), RWidth(), 1);
    view.Fill("panel", RPos_x(0), RPos_y(0), RWidth(), 14);

    wchar_t mailFrom[256] = {0};
    mu_swprintf(mailFrom, I18N::Game::SenderSSS, m_LetterHead.m_szID, m_LetterHead.m_szDate, m_LetterHead.m_szTime);
    view.Text(mailFrom, RPos_x(3), RPos_y(3), TextNormal);

    view.Button(m_ReplyButton);
    view.Button(m_DeleteButton);
    view.Button(m_CloseButton);
    view.Button(m_PrevButton);
    view.Button(m_NextButton);
}
