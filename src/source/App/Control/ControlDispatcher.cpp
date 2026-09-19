#include "stdafx.h"
#include "App/Control/ControlDispatcher.h"

#include "App/Control/ControlCommands.h"
#include "App/Control/ControlEvents.h"
#include "Scenes/SceneCore.h"

#include <chrono>
#include <optional>
#include <utility>

namespace
{
using App::Control::CommandEntry;
using App::Control::SceneRequirement;
namespace Commands = App::Control::Commands;

// `halt` is the control plane's own command: besides stopping the
// character it cancels whatever act is in flight.
constexpr std::string_view HaltCommand = "halt";

const std::vector<CommandEntry>& CommandTable()
{
    static const std::vector<CommandEntry> table = {
        {"ping", SceneRequirement::Any, &Commands::Ping},
        {"scene", SceneRequirement::Any, &Commands::Scene},
        {"state", SceneRequirement::Any, &Commands::State},
        {"nearby", SceneRequirement::World, &Commands::Nearby},
        {"events", SceneRequirement::Any, &Commands::EventsSince},
        {"wait-for", SceneRequirement::Any, &Commands::WaitFor},
        {"screenshot", SceneRequirement::Any, &Commands::Screenshot},
        {"hotkey", SceneRequirement::Any, &Commands::Hotkey},
        {"click-ui", SceneRequirement::Any, &Commands::ClickUi},
        {"type", SceneRequirement::Any, &Commands::Type},
        {"login", SceneRequirement::PreGame, &Commands::Login},
        {"select-char", SceneRequirement::PreGame, &Commands::SelectCharacter},
        {"logout", SceneRequirement::World, &Commands::Logout},
        {"quit", SceneRequirement::Any, &Commands::Quit},
        {"move", SceneRequirement::World, &Commands::Move},
        {"warp", SceneRequirement::World, &Commands::Warp},
        {"teleport", SceneRequirement::World, &Commands::Teleport},
        {"attack", SceneRequirement::World, &Commands::Attack},
        {"skill", SceneRequirement::World, &Commands::Skill},
        {"pickup", SceneRequirement::World, &Commands::Pickup},
        {"use", SceneRequirement::World, &Commands::UseItem},
        {"equip", SceneRequirement::World, &Commands::EquipItem},
        {"say", SceneRequirement::World, &Commands::Say},
        {"whisper", SceneRequirement::World, &Commands::Whisper},
        {"party", SceneRequirement::World, &Commands::Party},
        {"halt", SceneRequirement::Any, &Commands::Halt},
    };
    return table;
}
} // namespace

namespace App::Control
{
const std::vector<CommandEntry>& Dispatcher::Commands()
{
    return CommandTable();
}

const CommandEntry* Dispatcher::FindCommand(std::string_view name)
{
    for (const CommandEntry& entry : CommandTable())
    {
        if (entry.name == name)
        {
            return &entry;
        }
    }
    return nullptr;
}

bool Dispatcher::SceneAllows(SceneRequirement requirement)
{
    switch (requirement)
    {
    case SceneRequirement::Any:
        return true;
    case SceneRequirement::World:
        return SceneFlag == MAIN_SCENE;
    case SceneRequirement::PreGame:
        return SceneFlag != MAIN_SCENE;
    }
    return false;
}

std::string_view Dispatcher::ActName() const
{
    return m_act ? m_act->Name() : std::string_view{};
}

void Dispatcher::Queue(std::size_t connection, std::string line)
{
    if (line.empty())
    {
        return;
    }
    m_responses.push_back({connection, std::move(line)});
}

std::vector<OutgoingResponse> Dispatcher::TakeResponses()
{
    std::vector<OutgoingResponse> taken;
    taken.swap(m_responses);
    return taken;
}

void Dispatcher::Reject(const Request& request, std::size_t connection)
{
    Events::RecordError(request.Command(), ErrorCodeName(request.Error()), request.Message());
    Queue(connection, EncodeError(request.EncodedId(), request.Error(), request.Message()));
}

bool Dispatcher::InterruptAct(std::string_view reason)
{
    if (!m_act)
    {
        return false;
    }

    std::unique_ptr<Act> interrupted = std::move(m_act);
    m_act.reset();

    Queue(m_actConnection,
          EncodeError(interrupted->EncodedId(), ErrorCode::Interrupted, reason, interrupted->ProgressObject()));
    return true;
}

void Dispatcher::AbandonConnection(std::size_t connection)
{
    if (m_act && m_actConnection == connection)
    {
        // Nobody is left to answer; drop the act rather than keep the slot.
        m_act.reset();
    }

    std::erase_if(m_watchers, [connection](const Watcher& watcher) { return watcher.connection == connection; });

    std::erase_if(m_responses,
                  [connection](const OutgoingResponse& response) { return response.connection == connection; });
}

void Dispatcher::Handle(const Request& request, std::size_t connection)
{
    if (!request.IsValid())
    {
        Reject(request, connection);
        return;
    }

    const CommandEntry* entry = FindCommand(request.Command());
    if (entry == nullptr)
    {
        // The parser accepts the vocabulary; a name without a table entry
        // would be a programming error, not caller input.
        Queue(connection, EncodeError(request.EncodedId(), ErrorCode::UnknownCommand,
                                      "command `" + request.Command() + "` has no handler"));
        return;
    }

    if (!SceneAllows(entry->scene))
    {
        const std::string message = "command `" + request.Command() + "` is not available on scene `" +
                                    std::string(Commands::CurrentSceneName()) + "`";
        Events::RecordError(request.Command(), ErrorCodeName(ErrorCode::WrongScene), message);
        Queue(connection, EncodeError(request.EncodedId(), ErrorCode::WrongScene, message));
        return;
    }

    if (request.Command() == HaltCommand)
    {
        InterruptAct("halted");
    }

    std::unique_ptr<Act> act;
    std::string response = entry->handler(request, act);

    if (act && !act->IsAct())
    {
        // Observers run alongside whatever the character is doing.
        act->SetEncodedId(request.EncodedId());
        m_watchers.push_back({std::move(act), connection, std::chrono::steady_clock::now()});
        TickWatchers();
        return;
    }

    if (act)
    {
        // A second act interrupts the one in flight, exactly like `halt`.
        InterruptAct("superseded by `" + request.Command() + "`");

        act->SetEncodedId(request.EncodedId());
        m_act = std::move(act);
        m_actConnection = connection;
        m_actStartedAt = std::chrono::steady_clock::now();

        // An act may still answer at once (already at the target tile).
        Tick();
        return;
    }

    Queue(connection, std::move(response));
}

void Dispatcher::Tick()
{
    TickAct();
    TickWatchers();
}

void Dispatcher::TickWatchers()
{
    for (auto watcher = m_watchers.begin(); watcher != m_watchers.end();)
    {
        std::string response;
        const Act::Status status = watcher->command->Tick(response);

        if (status == Act::Status::Emitted)
        {
            // A streamed line: send it and let the reader keep running.
            Queue(watcher->connection, std::move(response));
            watcher->startedAt = std::chrono::steady_clock::now();
            ++watcher;
            continue;
        }

        const bool finished = status == Act::Status::Finished;
        const std::optional<std::chrono::milliseconds> deadline = watcher->command->Deadline();
        const bool expired =
            !finished && deadline.has_value() && std::chrono::steady_clock::now() - watcher->startedAt >= *deadline;

        if (!finished && !expired)
        {
            ++watcher;
            continue;
        }

        if (expired)
        {
            response = EncodeError(watcher->command->EncodedId(), ErrorCode::Timeout,
                                   "`" + std::string(watcher->command->Name()) + "` did not finish in time",
                                   watcher->command->ProgressObject());
        }

        Queue(watcher->connection, std::move(response));
        watcher = m_watchers.erase(watcher);
    }
}

void Dispatcher::TickAct()
{
    if (!m_act)
    {
        return;
    }

    std::string response;
    if (m_act->Tick(response) == Act::Status::Finished)
    {
        Queue(m_actConnection, std::move(response));
        m_act.reset();
        return;
    }

    const std::optional<std::chrono::milliseconds> deadline = m_act->Deadline();
    const auto elapsed = std::chrono::steady_clock::now() - m_actStartedAt;
    if (!deadline.has_value() || elapsed < *deadline)
    {
        return;
    }

    std::unique_ptr<Act> expired = std::move(m_act);
    m_act.reset();
    Queue(m_actConnection,
          EncodeError(expired->EncodedId(), ErrorCode::Timeout,
                      "`" + std::string(expired->Name()) + "` did not finish in time", expired->ProgressObject()));
}
} // namespace App::Control
