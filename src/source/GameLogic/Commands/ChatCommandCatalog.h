#pragma once

#include <string>
#include <vector>

// The chat commands which the server offers to this player.
//
// The server sends one AvailableChatCommand message per command after we asked
// for them, which lets the client offer the commands without the player having
// to know or type any of them.
namespace GameLogic::Commands
{
    // The kind of value a parameter expects, so that a fitting input can be offered.
    enum class ChatCommandParameterType : BYTE
    {
        Text = 0,
        Number = 1,
        Boolean = 2,
    };

    struct ChatCommandParameter
    {
        std::wstring Name;
        // The name used in the "shortName=value" notation. Empty when the
        // parameter can only be passed by its position.
        std::wstring ShortName;
        // The accepted values, separated by a pipe. Empty when not limited.
        std::wstring ValidValues;
        bool IsRequired = false;
        ChatCommandParameterType Type = ChatCommandParameterType::Text;
    };

    struct ChatCommand
    {
        std::wstring Command;
        std::wstring Name;
        std::wstring Description;
        BYTE MinimumCharacterStatus = 0;
        std::vector<ChatCommandParameter> Parameters;

        // A command without required parameters can be executed right away.
        bool CanExecuteDirectly() const;

        // The syntax as it's shown to the player, e.g. "/item {Group} {Number}".
        std::wstring GetUsage() const;
    };

    class ChatCommandCatalog
    {
    public:
        static ChatCommandCatalog& Instance();

        const std::vector<ChatCommand>& GetCommands() const { return m_commands; }

        // True when the server told us about the commands. Until then, the
        // feature stays hidden - the server may be an older one which doesn't
        // know the request.
        bool IsAvailable() const { return m_isComplete && !m_commands.empty(); }

        // Asks the server for the commands, at most once per session.
        void RequestOnce();

        // Forgets everything, e.g. when the character changed.
        void Reset();

        // Takes one AvailableChatCommand message. Returns false when the data
        // isn't plausible, so that the caller can ignore it.
        bool AddFromPacket(const BYTE* data, int32_t size);

        // Builds the line which executes the command with the given values, in
        // the order of ChatCommand::Parameters. Values which are empty are left
        // out, which is only valid for optional parameters.
        static std::wstring BuildCommandLine(const ChatCommand& command, const std::vector<std::wstring>& values);

        // Sends the line as a chat message. The server routes messages which
        // start with a slash to the command instead of broadcasting them.
        static void Execute(const std::wstring& commandLine);

    private:
        std::vector<ChatCommand> m_commands;
        bool m_isComplete = false;
        bool m_wasRequested = false;
    };

    inline ChatCommandCatalog& Catalog() { return ChatCommandCatalog::Instance(); }
}
