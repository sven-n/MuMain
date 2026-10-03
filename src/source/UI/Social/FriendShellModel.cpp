#include "stdafx.h"
#include "UI/Social/FriendShellModel.h"

namespace UI::Social
{
void FriendShellModel::Bind(Rml::DataModelConstructor& c)
{
    auto friendRow = c.RegisterStruct<FriendRow>();
    friendRow.RegisterMember("name", &FriendRow::name);
    friendRow.RegisterMember("server_label", &FriendRow::serverLabel);
    auto letterRow = c.RegisterStruct<LetterRow>();
    letterRow.RegisterMember("id", &LetterRow::id);
    letterRow.RegisterMember("sender", &LetterRow::sender);
    letterRow.RegisterMember("date", &LetterRow::date);
    letterRow.RegisterMember("title", &LetterRow::title);
    letterRow.RegisterMember("read", &LetterRow::read);
    letterRow.RegisterMember("checked", &LetterRow::checked);
    auto windowRow = c.RegisterStruct<WindowRow>();
    windowRow.RegisterMember("id", &WindowRow::id);
    windowRow.RegisterMember("title", &WindowRow::title);
    c.RegisterArray<std::vector<FriendRow>>();
    c.RegisterArray<std::vector<LetterRow>>();
    c.RegisterArray<std::vector<WindowRow>>();
    c.Bind("workspace_height", &workspaceHeight);
    c.Bind("friends", &friends);
    c.Bind("letters", &letters);
    c.Bind("windows", &windows);
    c.Bind("selected_friend", &selectedFriend);
    c.Bind("selected_letter", &selectedLetter);
    c.Bind("selected_window", &selectedWindow);
    c.Bind("active_tab", &tab);
    c.Bind("title", &title);
    c.Bind("reject_chat", &rejectChat);
    c.Bind("check_all", &checkAll);
    c.Bind("friend_sort", &friendSort);
    c.Bind("letter_sort", &letterSort);
    c.Bind("has_title", &hasTitle);
    c.Bind("positioned", &positioned);
    c.Bind("root_x", &rootX);
    c.Bind("root_y", &rootY);
    c.Bind("maximized", &maximized);
}
}
