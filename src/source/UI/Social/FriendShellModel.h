#pragma once

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Types.h>
#include <vector>

namespace UI::Social
{
struct FriendShellModel
{
    struct FriendRow { Rml::String name, serverLabel; int server = 0; };
    struct LetterRow { int id = 0; Rml::String sender, date, title; bool read = false, checked = false; };
    struct WindowRow { int id = 0; Rml::String title; };
    std::vector<FriendRow> friends;
    std::vector<LetterRow> letters;
    std::vector<WindowRow> windows;
    Rml::String selectedFriend, title;
    int selectedLetter = 0, selectedWindow = 0, tab = 0;
    // window_shell's own contract: a titled panel placed at a final device-pixel top-left.
    float rootX = 0, rootY = 0;
    int friendSort = 0, letterSort = 0;
    bool rejectChat = false, checkAll = false;
    bool hasTitle = true, positioned = true, maximized = false;
    void Bind(Rml::DataModelConstructor& constructor);
};
}
