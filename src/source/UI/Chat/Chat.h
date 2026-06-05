#pragma once

class CHARACTER;
class OBJECT;

// Chat balloons: the speech bubbles drawn above characters and the message
// list that feeds them. Extracted from ZzzInterface.cpp.
namespace UI::Chat
{
    void CreateChat(wchar_t* ID, const wchar_t* Text, CHARACTER* c, int Flag = 0, int SetColor = -1);
    int  CreateChat(wchar_t* character_name, const wchar_t* chat_text, OBJECT* Owner, int Flag = 0, int SetColor = -1);
    void AssignChat(wchar_t* ID, const wchar_t* Text, int Flag = 0);
    void MoveChat();
    void SetPlayerColor(BYTE PK);
    void RenderBooleans();
}
