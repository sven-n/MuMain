#pragma once

#include <string>
#include <vector>

// A chat command together with the values which were entered for it, so that it
// can be sent again without filling anything in a second time.
//
// Templates belong to the installation, not to a character - they are kept in
// the client config.
namespace GameLogic::Commands
{
    struct ChatCommandTemplate
    {
        // What the player sees in the list, which is the command line itself.
        std::wstring Label;
        // The command this template belongs to, e.g. "/item".
        std::wstring Command;
        // The values in the order of the parameters of the command.
        std::vector<std::wstring> Values;
    };

    namespace Templates
    {
        std::vector<ChatCommandTemplate> GetAll();

        void Add(const ChatCommandTemplate& entry);

        void RemoveAt(size_t index);
    }
}
