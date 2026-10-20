#pragma once

namespace mu::ui::window
{
enum MESSAGE_TYPE
{
    TYPE_ALL_MESSAGE = 0,
    TYPE_CHAT_MESSAGE,
    TYPE_WHISPER_MESSAGE,
    TYPE_SYSTEM_MESSAGE,
    TYPE_ERROR_MESSAGE,
    TYPE_PARTY_MESSAGE,
    TYPE_GUILD_MESSAGE,
    TYPE_UNION_MESSAGE,
    TYPE_GENS_MESSAGE,
    TYPE_GM_MESSAGE,
    // A view, not a message type: the messages from Discord. Each of them
    // keeps the type of its scope (guild, alliance, chat) for its colours.
    TYPE_DISCORD_MESSAGE,

    NUMBER_OF_TYPES,
    TYPE_UNKNOWN = 0xFFFFFFFF
};
}
