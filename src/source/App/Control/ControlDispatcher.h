// Command table of the control socket: which commands exist, which screen each
// one needs, and which of them keep running across frames.
//
// One act (a command that cannot answer in the frame it arrives) is in flight
// at a time. A second act interrupts the first, whose caller is told how far it
// got — the semantics a script expects from `halt`.
#pragma once

#include "App/Control/ControlProtocol.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace App::Control
{
// Which screen a command needs to be on.
enum class SceneRequirement : std::uint8_t
{
    // Valid everywhere, including the loading screen.
    Any,
    // Login, server list or character list: before the world is entered.
    PreGame,
    // Standing in the game world.
    World,
};

// A command that observes game state over several frames.
class Act
{
public:
    enum class Status : std::uint8_t
    {
        // Still working; nothing to send this frame.
        Running,
        // A line was produced and the command keeps running: how a
        // streaming reader (`events --follow`) delivers events.
        Emitted,
        // Done; `response` holds the last line.
        Finished,
    };

    virtual ~Act() = default;

    [[nodiscard]] virtual std::string_view Name() const = 0;

    // Whether this command drives the character. Acts hold the single
    // in-flight slot and interrupt each other; watchers (`wait-for`,
    // `screenshot`) only observe and run alongside, any number at a time.
    [[nodiscard]] virtual bool IsAct() const
    {
        return true;
    }

    // Advances one frame. When it returns Finished, `response` holds the
    // encoded line to send to the caller.
    [[nodiscard]] virtual Status Tick(std::string& response) = 0;

    // How far the act got, as an encoded JSON object. Reported when the act
    // is interrupted or times out.
    [[nodiscard]] virtual std::string ProgressObject() const
    {
        return "{}";
    }

    // How long the act may run before it answers `timeout`. No value means it
    // never times out: a streaming reader runs until its caller goes away.
    [[nodiscard]] virtual std::optional<std::chrono::milliseconds> Deadline() const = 0;

    [[nodiscard]] const std::string& EncodedId() const
    {
        return m_encodedId;
    }
    void SetEncodedId(std::string encodedId)
    {
        m_encodedId = std::move(encodedId);
    }

private:
    std::string m_encodedId;
};

// A handler answers immediately by returning an encoded line, or hands a
// long-running act over by assigning `act` and returning an empty string.
using CommandHandler = std::string (*)(const Request& request, std::unique_ptr<Act>& act);

struct CommandEntry
{
    std::string_view name;
    SceneRequirement scene;
    CommandHandler handler;
};

// A response waiting to go out on the connection that asked for it.
struct OutgoingResponse
{
    std::size_t connection = 0;
    std::string line;
};

class Dispatcher
{
public:
    // Runs one request. Any response it produces is queued for the given
    // connection, including the `interrupted` answer an ousted act gets.
    void Handle(const Request& request, std::size_t connection);

    // Answers a request that could not even be parsed.
    void Reject(const Request& request, std::size_t connection);

    // Advances the in-flight act by one frame.
    void Tick();

    // Drops the act a closed connection was waiting on.
    void AbandonConnection(std::size_t connection);

    // Stops the in-flight act, answering its caller with `interrupted`.
    // Returns false when nothing was running.
    bool InterruptAct(std::string_view reason);

    [[nodiscard]] bool HasActInFlight() const
    {
        return m_act != nullptr;
    }
    [[nodiscard]] std::string_view ActName() const;

    // Takes everything queued for sending.
    [[nodiscard]] std::vector<OutgoingResponse> TakeResponses();

    [[nodiscard]] static const std::vector<CommandEntry>& Commands();
    [[nodiscard]] static const CommandEntry* FindCommand(std::string_view name);

    // Whether the client's current scene satisfies the requirement.
    [[nodiscard]] static bool SceneAllows(SceneRequirement requirement);

private:
    void Queue(std::size_t connection, std::string line);

    // A command that only observes, with the connection waiting for it.
    struct Watcher
    {
        std::unique_ptr<Act> command;
        std::size_t connection = 0;
        std::chrono::steady_clock::time_point startedAt;
    };

    void TickAct();
    void TickWatchers();

    std::unique_ptr<Act> m_act;
    std::size_t m_actConnection = 0;
    std::chrono::steady_clock::time_point m_actStartedAt;
    std::vector<Watcher> m_watchers;
    std::vector<OutgoingResponse> m_responses;
};
} // namespace App::Control
