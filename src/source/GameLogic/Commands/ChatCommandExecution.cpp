#include "stdafx.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"
#include "UI/NewUI/NewUISystem.h"

namespace GameLogic::Commands
{
void ChatCommandCatalog::RequestOnce()
{
    if (this->m_wasRequested)
    {
        return;
    }

    this->m_wasRequested = true;
    if (SocketClient != nullptr)
    {
        SocketClient->ToGameServer()->SendChatCommandListRequest();
    }
}

void ChatCommandCatalog::Execute(const std::wstring& commandLine)
{
    if (commandLine.empty() || SocketClient == nullptr)
    {
        return;
    }

    SocketClient->ToGameServer()->SendPublicChatMessage(MU_C16(Hero->ID), MU_C16(commandLine.c_str()));

    // Put it into the chat history as if it had been typed, so that it can be
    // repeated with the arrow keys without going through the window again.
    if (g_pChatInputBox != nullptr)
    {
        g_pChatInputBox->AddChatHistory(commandLine);
    }
}
} // namespace GameLogic::Commands
