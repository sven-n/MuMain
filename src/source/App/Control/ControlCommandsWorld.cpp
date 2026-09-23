#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "App/Control/ControlEvents.h"
#include "App/Control/ControlObjects.h"
#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Automation/Attack.h"
#include "GameLogic/Automation/Movement.h"
#include "GameLogic/Automation/Pickup.h"
#include "GameLogic/Automation/Skill.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Core/WindowSystem.h"
#include "World/MapInfra/MapManager.h"

#include "json.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <chrono>
#include <cmath>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace
{
using App::Control::Act;
using App::Control::ErrorCode;
using App::Control::Request;
using nlohmann::json;
namespace Automation = GameLogic::Automation;

// How far beyond a weapon's reach the path finder may look for a way to
// the target. The auto-helper's own hunting range, in tiles.
constexpr int HuntingDistance = 10;
// How far a drop may be and still be walked to. The client's own view is
// about this wide, so anything `nearby` reports can be fetched; the
// auto-helper keeps its own, tighter limit.
constexpr int PickupDistance = 32;

constexpr std::chrono::milliseconds MoveDeadline{30000};
constexpr std::chrono::milliseconds AttackDeadline{60000};
constexpr std::chrono::milliseconds SkillDeadline{15000};
constexpr std::chrono::milliseconds PickupDeadline{30000};
constexpr std::chrono::milliseconds WarpDeadline{30000};
// As long as `warp`: a teleport to another map is the same relocation and
// the same world load, and the load is paced by rendered frames.
constexpr std::chrono::milliseconds TeleportDeadline{30000};
// A refused game-master move is silent on the wire: when the character has
// not arrived within this window, the server did not allow it.
constexpr std::chrono::milliseconds TeleportRefusalWindow{3000};

// A move counts as arrived within this many tiles of the target, matching
// the path finder's own tolerance.
constexpr int ArrivalTiles = 1;

// Shortest gap between two walk packets. The server's own speed check
// expects about 278 ms per tile, and a path is at least one tile, so
// planning more often than this looks like a speed hack to it — it bans
// the account after four warnings in an hour.
constexpr std::chrono::milliseconds WalkRepeatInterval{300};

// Cadence of the automation primitives when an act drives them across
// frames. The auto-helper's own timer interval: attacking, casting and
// picking up all walk into range first, and a step per frame is what the
// server reads as a speed hack.
constexpr std::chrono::milliseconds PrimitiveInterval{250};

// A map index written as text, for the callers that send `map` as a string.
// Digits only: a map name cannot be resolved back to an index here.
bool ParseMapIndex(const std::string& text, int& map)
{
    if (text.empty() ||
        !std::all_of(text.begin(), text.end(), [](unsigned char character) { return std::isdigit(character) != 0; }))
    {
        return false;
    }

    try
    {
        map = std::stoi(text);
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}

// True while the character is still walking a planned path. Re-issuing a
// move before it ends restarts the walk from the current tile, so an act
// that walks into range would crawl.
bool WalkInProgress()
{
    return Hero != nullptr && Hero->Path.PathNum > 0 && Hero->Path.CurrentPath < Hero->Path.PathNum - 1;
}

// True when the next step of a per-frame act is due, remembering when the
// previous one was taken.
bool StepIsDue(std::chrono::steady_clock::time_point& last, std::chrono::milliseconds interval = PrimitiveInterval)
{
    const auto now = std::chrono::steady_clock::now();
    if (last.time_since_epoch().count() != 0 && now - last < interval)
    {
        return false;
    }
    last = now;
    return true;
}

// Longest chat line the client sends: the wire field is char[MAX_CHAT_SIZE]
// (WSclient.h:485) and the chat box stops one short of it, so anything
// longer would be cut off on its way out rather than sent.
constexpr std::size_t ChatTextLength = MAX_CHAT_SIZE;
// Longest character name the whisper packet carries, terminator included.
constexpr std::size_t CharacterNameLength = 11;

json PositionObject()
{
    return Hero != nullptr ? json::array({Hero->PositionX, Hero->PositionY}) : json::array({-1, -1});
}

// Drops the queued path so the character stops at the tile it reached.
// The server is told nothing: this is the same as releasing the mouse.
void StopWalking()
{
    if (SceneFlag != MAIN_SCENE || Hero == nullptr)
    {
        return;
    }

    Hero->Path.Lock.lock();
    Hero->Path.PathNum = 0;
    Hero->Path.CurrentPath = 0;
    Hero->Path.CurrentPathFloat = 0;
    Hero->Path.Lock.unlock();
}

// What a `target` field turned out to be. A malformed one is a different
// answer from one the client simply does not see, and the commands say so.
enum class TargetLookup : std::uint8_t
{
    Missing,
    Malformed,
    NotInView,
    Found,
};

// Server-assigned id of the object a command names, by id or by character
// name.
TargetLookup ResolveTarget(const Request& request, int& key)
{
    key = -1;
    if (!request.Has("target"))
    {
        return TargetLookup::Missing;
    }

    int id = 0;
    if (request.GetInt("target", id))
    {
        if (FindCharacterIndex(id) >= MAX_CHARACTERS_CLIENT)
        {
            return TargetLookup::NotInView;
        }
        key = id;
        return TargetLookup::Found;
    }

    std::string name;
    if (!request.GetString("target", name) || name.empty())
    {
        return TargetLookup::Malformed;
    }

    const std::wstring wanted = Core::Text::FromUtf8(name);
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (character.Object.Live && wcscmp(character.ID, wanted.c_str()) == 0)
        {
            key = character.Key;
            return TargetLookup::Found;
        }
    }
    return TargetLookup::NotInView;
}

// The error a lookup that did not find an object deserves, or an empty
// string when it did. `required` says whether the command needs one.
std::string TargetError(const Request& request, TargetLookup lookup, std::string_view command, bool required)
{
    switch (lookup)
    {
    case TargetLookup::Found:
        return {};
    case TargetLookup::Missing:
        return required ? App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::BadRequest,
                                                    "`" + std::string(command) + "` needs a `target`")
                        : std::string{};
    case TargetLookup::Malformed:
        return App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::BadRequest,
                                         "`target` is an object id or a character name");
    case TargetLookup::NotInView:
        break;
    }
    return App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::NotInView,
                                     "the client does not see that object");
}

// Sends one chat line the way the chat box does.
// The caller must have checked `Hero`: the scene gate only says the world
// is on screen, and the character exists a few frames later.
void SendChat(const std::string& text)
{
    const std::wstring wide = Core::Text::FromUtf8(text);
    SocketClient->ToGameServer()->SendPublicChatMessage(MU_C16(Hero->ID), MU_C16(wide.c_str()));
}

// Whether a tile pair addresses a square of the map at all. The path
// finder's index folds an out-of-range x into the next row, so the
// commands that take tiles refuse one before it gets that far.
bool IsTileOnMap(int tileX, int tileY)
{
    return tileX >= 0 && tileX < TERRAIN_SIZE && tileY >= 0 && tileY < TERRAIN_SIZE;
}

// move: walk to a tile, re-planning each frame like a held mouse button.
class MoveAct : public Act
{
public:
    MoveAct(int tileX, int tileY) : m_tileX(tileX), m_tileY(tileY) {}

    [[nodiscard]] std::string_view Name() const override
    {
        return "move";
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return MoveDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["position"] = PositionObject();
        progress["target"] = json::array({m_tileX, m_tileY});
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (Hero == nullptr)
        {
            return Status::Running;
        }

        if (std::abs(Hero->PositionX - m_tileX) <= ArrivalTiles && std::abs(Hero->PositionY - m_tileY) <= ArrivalTiles)
        {
            json result;
            result["position"] = PositionObject();
            response = App::Control::EncodeResult(EncodedId(), result.dump());
            return Status::Finished;
        }

        // Let the planned path play out before planning the next one:
        // re-planning every frame would restart the walk mid-step.
        if (WalkInProgress())
        {
            return Status::Running;
        }

        // Pace the walk packets like a player's clicks; the server treats a
        // burst of them as a speed hack.
        if (!StepIsDue(m_lastWalkAt, WalkRepeatInterval))
        {
            return Status::Running;
        }

        const Automation::MoveResult result = Automation::WalkTo(m_tileX, m_tileY);
        if (result == Automation::MoveResult::NoPath)
        {
            response =
                App::Control::EncodeError(EncodedId(), ErrorCode::NoPath, "no path to that tile", ProgressObject());
            return Status::Finished;
        }

        if (result == Automation::MoveResult::Arrived)
        {
            json arrived;
            arrived["position"] = PositionObject();
            response = App::Control::EncodeResult(EncodedId(), arrived.dump());
            return Status::Finished;
        }

        return Status::Running;
    }

private:
    int m_tileX;
    int m_tileY;
    std::chrono::steady_clock::time_point m_lastWalkAt{};
};

// attack: N plain attacks on one object, walking into range as needed.
class AttackAct : public Act
{
public:
    AttackAct(int targetKey, int times, std::chrono::milliseconds interval)
        : m_targetKey(targetKey), m_times(times), m_interval(interval)
    {
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return "attack";
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return AttackDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["attacks"] = m_issued;
        progress["target"] = m_targetKey;
        progress["position"] = PositionObject();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (m_issued >= m_times)
        {
            json result;
            result["attacks"] = m_issued;
            result["target"] = m_targetKey;
            response = App::Control::EncodeResult(EncodedId(), result.dump());
            return Status::Finished;
        }

        const auto now = std::chrono::steady_clock::now();
        if (m_issued > 0 && now - m_lastAttack < m_interval)
        {
            return Status::Running;
        }

        if (WalkInProgress() || !StepIsDue(m_lastStep))
        {
            return Status::Running;
        }

        const Automation::AttackResult result = Automation::AttackObject(m_targetKey, true, HuntingDistance);

        switch (result)
        {
        case Automation::AttackResult::Attacked:
            ++m_issued;
            m_lastAttack = now;
            return Status::Running;
        case Automation::AttackResult::Approaching:
        case Automation::AttackResult::Busy:
            return Status::Running;
        case Automation::AttackResult::NotInView:
            return Fail(response, ErrorCode::NotInView, "the client no longer sees that object");
        case Automation::AttackResult::NotAttackable:
        {
            // A target that died under the last swing is a finished
            // attack, not a refused one — but only if this command ever
            // swung at it: a target that was already a corpse when the
            // request arrived was not killed by it.
            const int index = FindCharacterIndex(m_targetKey);
            if (m_issued > 0 && index != MAX_CHARACTERS_CLIENT && CharactersClient[index].Dead > 0)
            {
                json killed;
                killed["attacks"] = m_issued;
                killed["target"] = m_targetKey;
                killed["killed"] = true;
                response = App::Control::EncodeResult(EncodedId(), killed.dump());
                return Status::Finished;
            }
            return Fail(response, ErrorCode::NotAttackable, "that object cannot be attacked");
        }
        case Automation::AttackResult::NoPath:
            return Fail(response, ErrorCode::NoPath, "no path into attack range");
        case Automation::AttackResult::Blocked:
            return Fail(response, ErrorCode::NoPath, "a wall stands between the two");
        case Automation::AttackResult::NoArrows:
            return Fail(response, ErrorCode::Failed, "the quiver is empty");
        case Automation::AttackResult::SafeZone:
            return Fail(response, ErrorCode::NotAllowed,
                        "the character stands in a safe zone, where attacks are refused");
        case Automation::AttackResult::NoTarget:
            return Fail(response, ErrorCode::NotInView, "no target");
        }

        return Status::Running;
    }

private:
    Status Fail(std::string& response, ErrorCode code, const std::string& message)
    {
        App::Control::Events::RecordError("attack", App::Control::ErrorCodeName(code), message);
        response = App::Control::EncodeError(EncodedId(), code, message, ProgressObject());
        return Status::Finished;
    }

    int m_targetKey;
    int m_times;
    std::chrono::milliseconds m_interval;
    int m_issued = 0;
    std::chrono::steady_clock::time_point m_lastAttack{};
    std::chrono::steady_clock::time_point m_lastStep{};
};

// skill: one cast on a target or at the caster's own tile.
class SkillAct : public Act
{
public:
    SkillAct(ActionSkillType skill, bool targetRequired, int targetKey)
        : m_skill(skill), m_targetRequired(targetRequired), m_targetKey(targetKey)
    {
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return "skill";
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return SkillDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["skill"] = static_cast<int>(m_skill);
        progress["target"] = m_targetKey;
        progress["position"] = PositionObject();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (WalkInProgress() || !StepIsDue(m_lastStep))
        {
            return Status::Running;
        }

        const Automation::SkillResult result =
            Automation::CastSkill(m_skill, m_targetRequired, m_targetKey, true, HuntingDistance);

        switch (result)
        {
        case Automation::SkillResult::Cast:
        {
            json result_object;
            result_object["skill"] = static_cast<int>(m_skill);
            result_object["target"] = m_targetKey;
            response = App::Control::EncodeResult(EncodedId(), result_object.dump());
            return Status::Finished;
        }
        case Automation::SkillResult::Approaching:
        case Automation::SkillResult::Busy:
        case Automation::SkillResult::NotReady:
            return Status::Running;
        case Automation::SkillResult::NotInView:
            return Fail(response, ErrorCode::NotInView, "the client no longer sees that object");
        case Automation::SkillResult::NotAttackable:
            return Fail(response, ErrorCode::NotAttackable, "that object cannot be attacked");
        case Automation::SkillResult::NoPath:
            return Fail(response, ErrorCode::NoPath, "no path into skill range");
        case Automation::SkillResult::Blocked:
            return Fail(response, ErrorCode::NoPath, "a wall stands between the two");
        case Automation::SkillResult::NoTarget:
            return Fail(response, ErrorCode::NotInView, "that skill needs a target");
        case Automation::SkillResult::Refused:
            return Fail(response, ErrorCode::SkillRefused, "the client refused the cast");
        }

        return Status::Running;
    }

private:
    Status Fail(std::string& response, ErrorCode code, const std::string& message)
    {
        App::Control::Events::RecordError("skill", App::Control::ErrorCodeName(code), message);
        response = App::Control::EncodeError(EncodedId(), code, message, ProgressObject());
        return Status::Finished;
    }

    ActionSkillType m_skill;
    bool m_targetRequired;
    int m_targetKey;
    std::chrono::steady_clock::time_point m_lastStep{};
};

// pickup: walk to a drop and ask for it.
class PickupAct : public Act
{
public:
    // What the drop was when the caller named it, kept beside the slot: a
    // drop that vanishes frees its slot for the next one, and the act must
    // not walk to a different item that happens to land in it. The client
    // gives dropped items no key of their own, so the identity is what does
    // not change while a drop lies there — its type and its tile.
    PickupAct(int itemSlot, int itemType, std::pair<int, int> tile)
        : m_itemSlot(itemSlot), m_itemType(itemType), m_tile(tile)
    {
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return "pickup";
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return PickupDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["slot"] = m_itemSlot;
        progress["position"] = PositionObject();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (!Items[m_itemSlot].Object.Live || Items[m_itemSlot].Item.Type != m_itemType ||
            App::Control::DropTile(m_itemSlot) != m_tile)
        {
            response = EncodeError(EncodedId(), ErrorCode::NotInView, "the drop is gone", ProgressObject());
            return Status::Finished;
        }

        if (WalkInProgress() || !StepIsDue(m_lastStep))
        {
            return Status::Running;
        }

        const Automation::PickupResult result = Automation::PickUpItem(m_itemSlot, PickupDistance);

        switch (result)
        {
        case Automation::PickupResult::Requested:
        {
            json picked;
            picked["slot"] = m_itemSlot;
            response = App::Control::EncodeResult(EncodedId(), picked.dump());
            return Status::Finished;
        }
        case Automation::PickupResult::Approaching:
        case Automation::PickupResult::Busy:
            return Status::Running;
        case Automation::PickupResult::TooFar:
            response = App::Control::EncodeError(EncodedId(), ErrorCode::NotPickable, "the drop is out of reach",
                                                 ProgressObject());
            return Status::Finished;
        case Automation::PickupResult::Gone:
            response = App::Control::EncodeError(EncodedId(), ErrorCode::NotInView, "the drop is no longer there",
                                                 ProgressObject());
            return Status::Finished;
        case Automation::PickupResult::NoPath:
            response =
                App::Control::EncodeError(EncodedId(), ErrorCode::NoPath, "no path to the drop", ProgressObject());
            return Status::Finished;
        }

        return Status::Running;
    }

private:
    int m_itemSlot;
    int m_itemType;
    std::pair<int, int> m_tile;
    std::chrono::steady_clock::time_point m_lastStep{};
};

// warp: the warp list entry, answered once the character has been moved.
class WarpAct : public Act
{
public:
    // A warp across servers leaves the world scene as its own work: the
    // dispatcher's scene guard must not end it for doing so.
    [[nodiscard]] bool ChangesScene() const override
    {
        return true;
    }

    WarpAct(int mapIndex, std::string name) : m_mapIndex(mapIndex), m_name(std::move(name)) {}

    [[nodiscard]] std::string_view Name() const override
    {
        return "warp";
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return WarpDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["warp"] = m_name;
        progress["map"] = gMapManager.WorldActive;
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (!m_sent)
        {
            m_teleportsBefore = App::Control::Events::TeleportCount();
            // The click path saves the options when the destination lives on
            // another server (NewUIMoveCommandWindow.cpp:414): the session
            // ends there, and what is not saved now is lost.
            if (g_pMoveCommandWindow->IsTheMapInDifferentServer(gMapManager.WorldActive, m_mapIndex))
            {
                SaveOptions();
            }
            SocketClient->ToGameServer()->SendWarpCommandRequest(g_pMoveCommandWindow->GetMoveCommandKey(),
                                                                 static_cast<uint16_t>(m_mapIndex));
            m_sent = true;
            return Status::Running;
        }

        if (SceneFlag != MAIN_SCENE)
        {
            return Status::Running;
        }

        // What a landed warp looks like from here is the teleport packet:
        // the server answers every warp — to another map or to a gate on
        // this one — by putting the character down with it, and the handler
        // has already written the new map and position by the time this
        // runs. Inferring it instead from a jump in position both misses a
        // character already standing on the gate and fires on an ordinary
        // walk, and inferring it from the world load behind the packet
        // fails on a client that is not rendering: that load is counted in
        // rendered frames, so at one frame a second it outlasts the whole
        // deadline.
        if (App::Control::Events::TeleportCount() == m_teleportsBefore)
        {
            return Status::Running;
        }

        json result;
        result["map"] = gMapManager.WorldActive;
        result["position"] = PositionObject();
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        return Status::Finished;
    }

private:
    int m_mapIndex;
    std::string m_name;
    bool m_sent = false;
    std::uint64_t m_teleportsBefore = 0;
};

// teleport: the game master's own move command, answered when the
// character stands on the requested tile.
class TeleportAct : public Act
{
public:
    TeleportAct(std::string command, int map, int tileX, int tileY)
        : m_command(std::move(command)), m_map(map), m_tileX(tileX), m_tileY(tileY)
    {
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return "teleport";
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return TeleportDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["position"] = PositionObject();
        progress["target"] = json::array({m_tileX, m_tileY});
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (!m_sent)
        {
            // The handler checked the hero, but an act ticks in later frames:
            // a logout or a map change between the two leaves `SendChat`
            // reading a character that is gone.
            if (Hero == nullptr)
            {
                response = App::Control::EncodeError(EncodedId(), ErrorCode::WrongScene,
                                                     "the character left the world before the command was sent");
                return Status::Finished;
            }

            m_fromMap = gMapManager.WorldActive;
            m_teleportsBefore = App::Control::Events::TeleportCount();
            SendChat(m_command);
            m_sentAt = std::chrono::steady_clock::now();
            m_sent = true;
            return Status::Running;
        }

        // The map is part of arrival, not only the tile: map coordinates
        // overlap across maps, so a character standing near the requested
        // tile on the map it is leaving would otherwise satisfy this on the
        // first tick — before the server could have answered at all — and the
        // result would then report the old map it never left.
        if (Hero == nullptr || gMapManager.WorldActive != m_map || std::abs(Hero->PositionX - m_tileX) > ArrivalTiles ||
            std::abs(Hero->PositionY - m_tileY) > ArrivalTiles)
        {
            // The refusal window catches "the server ignored the command",
            // which is only distinguishable while nothing has happened at
            // all: once the teleport packet has arrived, or the map has
            // changed, the move is under way and the deadline governs — that
            // is the timer meant for "this is taking too long". A cross-map
            // teleport loads a world, which on a cold map, or on a client
            // that is barely rendering, takes longer than the window.
            const bool nothingHappened =
                App::Control::Events::TeleportCount() == m_teleportsBefore && gMapManager.WorldActive == m_fromMap;
            if (!nothingHappened || std::chrono::steady_clock::now() - m_sentAt < TeleportRefusalWindow)
            {
                return Status::Running;
            }

            response = App::Control::EncodeError(EncodedId(), ErrorCode::NotAllowed,
                                                 "the server did not move the character", ProgressObject());
            return Status::Finished;
        }

        json result;
        result["position"] = PositionObject();
        result["map"] = gMapManager.WorldActive;
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        return Status::Finished;
    }

private:
    std::string m_command;
    int m_map;
    int m_tileX;
    int m_tileY;
    bool m_sent = false;
    int m_fromMap = -1;
    std::uint64_t m_teleportsBefore = 0;
    std::chrono::steady_clock::time_point m_sentAt{};
};
} // namespace

namespace App::Control::Commands
{
std::string Move(const Request& request, std::unique_ptr<Act>& act)
{
    int tileX = 0;
    int tileY = 0;
    if (!request.GetInt("x", tileX) || !request.GetInt("y", tileY))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`move` needs `x` and `y`");
    }

    // The path finder indexes tiles as x + y * TERRAIN_SIZE and only checks
    // the combined index, so an x past the edge folds into the next row and
    // walks somewhere the caller did not ask for.
    if (!IsTileOnMap(tileX, tileY))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`x` and `y` are tiles on the map, 0 to " + std::to_string(TERRAIN_SIZE - 1));
    }

    act = std::make_unique<MoveAct>(tileX, tileY);
    return {};
}

std::string Warp(const Request& request, std::unique_ptr<Act>& act)
{
    std::string name;
    if (!request.GetString("gate", name) || name.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`warp` needs a gate name");
    }

    // As the engine's own hero-dependent code writes it (DefaultCamera.cpp:111).
    if (Hero == nullptr)
    {
        return EncodeError(request.EncodedId(), ErrorCode::WrongScene, "the character is not in the world yet");
    }

    const std::wstring wide = Core::Text::FromUtf8(name);
    const int mapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(wide.c_str());
    if (mapIndex < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::WarpRefused, "no warp list entry named `" + name + "`");
    }

    // The click path refuses an entry the character does not qualify for
    // without sending anything. Sending it anyway costs the caller the whole
    // warp deadline for a request the server silently declines, so the same
    // requirements are tested here: level, zen, wings for Icarus, the Uniria
    // rule for Atlans, the ring ban.
    if (!g_pMoveCommandWindow->CanMoveToMap(wide.c_str()))
    {
        return EncodeError(request.EncodedId(), ErrorCode::WarpRefused,
                           "the character does not meet the requirements for `" + name + "`");
    }

    act = std::make_unique<WarpAct>(mapIndex, name);
    return {};
}

std::string Teleport(const Request& request, std::unique_ptr<Act>& act)
{
    int tileX = 0;
    int tileY = 0;
    if (!request.GetInt("x", tileX) || !request.GetInt("y", tileY))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`teleport` needs `x` and `y`");
    }

    if (Hero == nullptr)
    {
        return EncodeError(request.EncodedId(), ErrorCode::WrongScene, "the character is not in the world yet");
    }

    if (!IsTileOnMap(tileX, tileY))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`x` and `y` are tiles on the map, 0 to " + std::to_string(TERRAIN_SIZE - 1));
    }

    // The map is taken as a number, not a name: the act has to recognise the
    // destination to know the character arrived, and the client has no
    // mapping from a map's (localised) name back to its index. Omitted means
    // the map the character is on.
    int map = gMapManager.WorldActive;
    std::string mapText;
    if (request.GetString("map", mapText) && !mapText.empty())
    {
        if (!ParseMapIndex(mapText, map))
        {
            return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                               "`teleport` takes the map as a number, not `" + mapText + "`");
        }
    }
    else if (request.Has("map") && (!request.GetInt("map", map) || map < 0))
    {
        // A `map` that is not a number this client can hold must not fall
        // back to the map the character is standing on: the command would
        // then teleport somewhere the caller never asked for and report it
        // as a success.
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`teleport` needs a map index of 0 or more");
    }

    // The server's own game-master command: /move <character> <map> <x> <y>.
    const std::string command = "/move " + Core::Text::ToUtf8(Hero->ID) + " " + std::to_string(map) + " " +
                                std::to_string(tileX) + " " + std::to_string(tileY);

    act = std::make_unique<TeleportAct>(command, map, tileX, tileY);
    return {};
}

std::string Attack(const Request& request, std::unique_ptr<Act>& act)
{
    int targetKey = -1;
    const std::string targetFailure = TargetError(request, ResolveTarget(request, targetKey), "attack", true);
    if (!targetFailure.empty())
    {
        return targetFailure;
    }

    int times = 1;
    if (request.Has("times") && (!request.GetInt("times", times) || times < 1))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`times` is a whole number, at least 1");
    }

    int interval = 0;
    if (request.Has("interval") && (!request.GetInt("interval", interval) || interval < 0))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`interval` is a whole number of milliseconds, 0 or more");
    }

    act = std::make_unique<AttackAct>(targetKey, times, std::chrono::milliseconds(interval));
    return {};
}

std::string Skill(const Request& request, std::unique_ptr<Act>& act)
{
    int skillNumber = 0;
    if (!request.GetInt("skill", skillNumber))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`skill` needs a skill number");
    }

    const ActionSkillType skill = static_cast<ActionSkillType>(skillNumber);
    if (g_pSkillList == nullptr || g_pSkillList->GetSkillIndex(skill) == -1)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NoSuchSkill,
                           "the character does not have skill " + std::to_string(skillNumber));
    }

    // `target` is what chooses between the two forms of the command: with
    // one, the skill is aimed at that object; without one, it is cast where
    // the character stands. Which skills accept which form is the server's
    // rule, not a table this client holds.
    int targetKey = -1;
    const TargetLookup lookup = ResolveTarget(request, targetKey);
    const std::string targetFailure = TargetError(request, lookup, "skill", false);
    if (!targetFailure.empty())
    {
        return targetFailure;
    }
    const bool targetRequired = lookup == TargetLookup::Found;

    act = std::make_unique<SkillAct>(skill, targetRequired, targetKey);
    return {};
}

std::string Pickup(const Request& request, std::unique_ptr<Act>& act)
{
    // `id` is the protocol's own request identifier, so a drop is named by
    // `item` — the id `nearby` reports for it.
    int slot = 0;
    if (!request.GetInt("item", slot))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`pickup` needs an item id");
    }

    // The id is the client's own slot in the dropped-item table — what
    // `nearby` and the `drop` events report, and what the pickup packet
    // carries. The drop's server key is a different id space and is not
    // addressable here.
    if (slot < 0 || slot >= MAX_ITEMS || !Items[slot].Object.Live)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotInView, "no such drop in view");
    }

    act = std::make_unique<PickupAct>(slot, Items[slot].Item.Type, App::Control::DropTile(slot));
    return {};
}

std::string UseItem(const Request& request, std::unique_ptr<Act>&)
{
    int slot = 0;
    if (!request.GetInt("slot", slot))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`use` needs an inventory slot");
    }

    const ITEM* item = FindInventoryItemBySlot(slot);
    // `-1` is the client's empty marker; type 0 is a real item.
    if (item == nullptr || item->Type < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::EmptySlot,
                           "inventory slot " + std::to_string(slot) + " is empty");
    }

    // Through the client's own path, so the vault/trade refusal and the
    // in-flight latch apply: two `use` commands in consecutive frames must
    // not send two consume requests for the same potion.
    if (!IsCanUseItem())
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotAllowed,
                           "items cannot be used while the vault or a trade is open");
    }
    if (EnableUse > 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "the previous use has not been answered yet");
    }

    // Target 0, as every in-client consume passes (NewUIMainFrameWindow.cpp:1050):
    // a real slot there means "apply this jewel to that item", which is a
    // different request.
    SendRequestUse(slot, 0, true);

    json result;
    result["slot"] = slot;
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string EquipItem(const Request& request, std::unique_ptr<Act>&)
{
    int fromSlot = 0;
    int toSlot = 0;
    if (!request.GetInt("slot", fromSlot) || !request.GetInt("target_slot", toSlot))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`equip` needs an inventory slot and a target slot");
    }

    const ITEM* item = FindInventoryItemBySlot(fromSlot);
    if (item == nullptr || item->Type < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::EmptySlot,
                           "inventory slot " + std::to_string(fromSlot) + " is empty");
    }

    // The target is an equipment slot or another inventory slot; anything
    // beyond that would be truncated by the cast below into a different,
    // valid slot, and the answer would name the slot the caller asked for.
    if (toSlot < 0 || toSlot >= MAX_MY_INVENTORY_EX_INDEX)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "target slot " + std::to_string(toSlot) +
                               " is outside the equipment and inventory range (0-" +
                               std::to_string(MAX_MY_INVENTORY_EX_INDEX - 1) + ")");
    }

    // The move the client itself performs: the item is lifted out of its
    // slot into the picked-item state first, because that is what the reply
    // handler completes the move from (NewUIInventoryCtrl.cpp:950-957,
    // NewUIInventoryActionController.cpp:133, WSclient.cpp:6312). Sending
    // the request without it leaves the source slot holding a copy of an
    // item the server has already moved.
    // The extension slots live in their own control, and the main inventory
    // does not know them: the item is taken from whichever holds the slot.
    const bool inExtension = !IsMainInventorySlot(fromSlot);
    mu::ui::window::CInventoryCtrl* inventory = nullptr;
    ITEM* moving = nullptr;
    if (inExtension)
    {
        inventory =
            g_pMyInventoryExt != nullptr ? g_pMyInventoryExt->TryGetExtensionByInventoryIndex(fromSlot) : nullptr;
        moving = g_pMyInventoryExt != nullptr ? g_pMyInventoryExt->FindItem(fromSlot) : nullptr;
    }
    else
    {
        inventory = g_pMyInventory != nullptr ? g_pMyInventory->GetInventoryCtrl() : nullptr;
        moving = g_pMyInventory != nullptr ? g_pMyInventory->FindItem(fromSlot) : nullptr;
    }

    if (inventory == nullptr || moving == nullptr)
    {
        return EncodeError(request.EncodedId(), ErrorCode::Failed, "the inventory is not available");
    }

    if (mu::ui::window::CInventoryCtrl::GetPickedItem() != nullptr)
    {
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "an item is already being moved");
    }

    // The same lock the player's own pick honours (NewUIInventoryCtrl.cpp:947).
    if (inventory->IsLocked())
    {
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "the inventory is locked");
    }

    if (toSlot == fromSlot)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "the item is already in that slot");
    }

    // What the client's own move tests before it sends, and tested before
    // the item is lifted out, so a refused move leaves the inventory
    // untouched. An equipment slot is a different question from an
    // inventory square: it asks whether the item may be worn there
    // (`IsEquipable`), not whether a rectangle is free.
    if (toSlot < MAX_EQUIPMENT_INDEX)
    {
        if (g_pMyInventory == nullptr || !g_pMyInventory->IsEquipable(toSlot, moving))
        {
            return EncodeError(request.EncodedId(), ErrorCode::NotAllowed, "the item cannot be equipped in that slot");
        }
    }
    // An inventory destination is asked after the item is lifted, below:
    // while it still occupies its own squares, a move that overlaps them
    // would be refused for colliding with itself.

    if (!mu::ui::window::CInventoryCtrl::CreatePickedItem(inventory, moving, true))
    {
        // CreatePickedItem has already new-ed the picked-item singleton by
        // the time Create() can fail (NewUIInventoryCtrl.cpp), so a bare
        // return would leave it non-null: every later move — ours and the
        // player's own picking — would answer "an item is already being
        // moved" for the rest of the session.
        mu::ui::window::CInventoryCtrl::DeletePickedItem();
        return EncodeError(request.EncodedId(), ErrorCode::Failed, "the item could not be picked up");
    }
    inventory->RemoveItem(moving);

    mu::ui::window::CPickedItem* picked = mu::ui::window::CInventoryCtrl::GetPickedItem();
    ITEM* lifted = picked != nullptr ? picked->GetItem() : nullptr;

    // Asked now, not before: the item has left its own squares, so a move
    // that overlaps them is not refused for colliding with itself — the
    // order the client's own move uses (NewUIInventoryCtrl.cpp:950,
    // NewUIInventoryActionController.cpp:123).
    if (toSlot >= MAX_EQUIPMENT_INDEX)
    {
        mu::ui::window::CInventoryCtrl* destination =
            IsMainInventorySlot(toSlot)
                ? (g_pMyInventory != nullptr ? g_pMyInventory->GetInventoryCtrl() : nullptr)
                : (g_pMyInventoryExt != nullptr ? g_pMyInventoryExt->TryGetExtensionByInventoryIndex(toSlot) : nullptr);
        if (destination == nullptr || lifted == nullptr || !destination->CanMove(toSlot, lifted))
        {
            if (lifted != nullptr)
            {
                inventory->AddItem(lifted->x, lifted->y, lifted);
            }
            mu::ui::window::CInventoryCtrl::DeletePickedItem();
            return EncodeError(request.EncodedId(), ErrorCode::NotAllowed, "the item does not fit in that slot");
        }
    }

    if (lifted == nullptr ||
        !SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, fromSlot, lifted, STORAGE_TYPE::INVENTORY, toSlot))
    {
        // Nothing was sent, so the item goes back into its slot directly:
        // BackupPickedItem does nothing while `EquipmentItem` is set, which
        // is the very condition that makes the send fail.
        if (lifted != nullptr)
        {
            inventory->AddItem(lifted->x, lifted->y, lifted);
        }
        mu::ui::window::CInventoryCtrl::DeletePickedItem();
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "another item move has not been answered yet");
    }

    json result;
    result["slot"] = fromSlot;
    result["target_slot"] = toSlot;
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string Say(const Request& request, std::unique_ptr<Act>&)
{
    std::string text;
    if (!request.GetString("text", text) || text.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`say` needs a text");
    }

    if (text.size() >= ChatTextLength)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "the chat line is too long");
    }

    // The world is on screen a few frames before the character exists, and
    // the chat packet carries its name.
    if (Hero == nullptr)
    {
        return EncodeError(request.EncodedId(), ErrorCode::WrongScene, "the character is not in the world yet");
    }

    SendChat(text);

    json result;
    result["text"] = text;
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string Whisper(const Request& request, std::unique_ptr<Act>&)
{
    std::string name;
    std::string text;
    if (!request.GetString("name", name) || !request.GetString("text", text) || name.empty() || text.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`whisper` needs a name and a text");
    }

    // The same bound `say` applies: the chat packet's text field is what
    // overflows, and a whisper goes through it too.
    if (text.size() >= ChatTextLength)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "the chat line is too long");
    }
    const std::wstring wideName = Core::Text::FromUtf8(name);
    if (wideName.size() >= CharacterNameLength)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "the character name is too long");
    }

    const std::wstring wideText = Core::Text::FromUtf8(text);
    SocketClient->ToGameServer()->SendWhisperMessage(MU_C16(wideName.c_str()), MU_C16(wideText.c_str()));

    json result;
    result["name"] = name;
    result["text"] = text;
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string Party(const Request& request, std::unique_ptr<Act>&)
{
    std::string action;
    if (!request.GetString("action", action) || action.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`party` needs one of invite, accept, decline, leave");
    }

    json result;
    result["action"] = action;

    if (action == "invite")
    {
        int targetKey = -1;
        const std::string targetFailure = TargetError(request, ResolveTarget(request, targetKey), "party invite", true);
        if (!targetFailure.empty())
        {
            return targetFailure;
        }

        SocketClient->ToGameServer()->SendPartyInviteRequest(static_cast<uint16_t>(targetKey));
        result["target"] = targetKey;
        return EncodeResult(request.EncodedId(), result.dump());
    }

    if (action == "accept" || action == "decline")
    {
        if (PartyKey == 0)
        {
            return EncodeError(request.EncodedId(), ErrorCode::Failed, "no party invitation is pending");
        }

        SocketClient->ToGameServer()->SendPartyInviteResponse(action == "accept" ? 1 : 0,
                                                              static_cast<uint16_t>(PartyKey));
        // The invitation dialog would stay on screen otherwise: the button
        // handler sends the answer and closes the box in one go.
        if (!g_MessageBox->IsEmpty())
        {
            g_MessageBox->PopMessageBox();
        }
        result["requester"] = PartyKey;
        PartyKey = 0;
        return EncodeResult(request.EncodedId(), result.dump());
    }

    if (action == "leave")
    {
        // Leaving is kicking yourself, and the request carries the
        // server-assigned member number — not the row the character sits in.
        // The window finds its own row by name for the same reason
        // (NewUIPartyInfoWindow.cpp:97, 284): index 0 is the party master.
        if (Hero == nullptr)
        {
            return EncodeError(request.EncodedId(), ErrorCode::WrongScene, "the character is not in the world yet");
        }

        int own = -1;
        for (int member = 0; member < ::PartyNumber; ++member)
        {
            if (wcscmp(::Party[member].Name, Hero->ID) == 0)
            {
                own = member;
                break;
            }
        }

        if (own < 0)
        {
            return EncodeError(request.EncodedId(), ErrorCode::NotAllowed, "the character is not in a party");
        }

        SocketClient->ToGameServer()->SendPartyPlayerKickRequest(static_cast<BYTE>(::Party[own].Number));
        return EncodeResult(request.EncodedId(), result.dump());
    }

    return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "unknown party action `" + action + "`");
}

std::string Halt(const Request& request, std::unique_ptr<Act>&)
{
    // The act in flight was already cancelled by the dispatcher; this stops
    // the character itself.
    StopWalking();

    json result;
    result["scene"] = CurrentSceneName();
    if (SceneFlag == MAIN_SCENE && Hero != nullptr)
    {
        result["position"] = PositionObject();
    }
    return EncodeResult(request.EncodedId(), result.dump());
}
} // namespace App::Control::Commands
