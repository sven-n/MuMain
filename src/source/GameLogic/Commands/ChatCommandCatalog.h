#pragma once

#include "Core/Platform/WinCompat.h"

#include <cstdint>
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

// What the value of a parameter refers to, so that the client can help with it,
// e.g. by naming the map of a map number. The server only appends new kinds, so
// the values stay the same; a kind we don't know yet is treated as None.
enum class ChatCommandValueReference : BYTE
{
    None = 0,
    CharacterName = 1,
    AccountName = 2,
    GuildName = 3,
    Map = 4,
    MapCoordinateX = 5,
    MapCoordinateY = 6,
    ItemGroup = 7,
    ItemNumber = 8,
    MonsterNumber = 9,
    ObjectId = 10,
    SkillNumber = 11,
    LanguageIsoCode = 12,
};

struct ChatCommandParameter
{
    // The index which GroupWithIndex has when the parameter isn't grouped.
    static constexpr int NoGroup = -1;

    std::wstring Name;
    // The name used in the "shortName=value" notation. Empty when the
    // parameter can only be passed by its position.
    std::wstring ShortName;
    // The accepted values, separated by a pipe. Empty when not limited.
    std::wstring ValidValues;
    bool IsRequired = false;
    ChatCommandParameterType Type = ChatCommandParameterType::Text;

    // The hints below are only sent by newer servers. Without them, they keep
    // their defaults.
    ChatCommandValueReference ValueReference = ChatCommandValueReference::None;
    // The parameter which identifies the referenced object together with this
    // one, e.g. the group of an item number.
    int GroupWithIndex = NoGroup;
    bool HasRange = false;
    int64_t Minimum = 0;
    int64_t Maximum = 0;

    // False for a value the server would reject because it's not a number or
    // out of the range. An empty value is accepted - whether it may be left
    // out depends on IsRequired.
    bool Accepts(const std::wstring& value) const;
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

    const std::vector<ChatCommand>& GetCommands() const
    {
        return m_commands;
    }

    // True when the server told us about the commands. Until then, the
    // feature stays hidden - the server may be an older one which doesn't
    // know the request.
    bool IsAvailable() const
    {
        return m_isComplete && !m_commands.empty();
    }

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
    BYTE m_expectedCount = 0;
    bool m_isComplete = false;
    bool m_wasRequested = false;
};

inline ChatCommandCatalog& Catalog()
{
    return ChatCommandCatalog::Instance();
}
} // namespace GameLogic::Commands
