#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "App/Control/ControlEvents.h"
#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Automation/Attack.h"
#include "GameLogic/Automation/Movement.h"
#include "GameLogic/Automation/Pickup.h"
#include "GameLogic/Automation/Skill.h"
#include "GameLogic/Items/InventoryUtils.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/NewUI/NewUISystem.h"
#include "World/MapInfra/MapManager.h"

#include "json.hpp"

#include <chrono>
#include <cmath>
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
constexpr std::chrono::milliseconds TeleportDeadline{10000};
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

// Longest chat line the client sends.
constexpr std::size_t ChatTextLength = 128;

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

// Server-assigned id of the object a command names, by id or by character
// name. -1 when the client does not know it.
int ResolveTargetKey(const Request& request)
{
    int id = 0;
    if (request.GetInt("target", id))
    {
        for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
        {
            const CHARACTER& character = CharactersClient[index];
            if (character.Object.Live && character.Key == id)
            {
                return character.Key;
            }
        }
        return -1;
    }

    std::string name;
    if (!request.GetString("target", name) || name.empty())
    {
        return -1;
    }

    const std::wstring wanted = Core::Text::FromUtf8(name);
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (character.Object.Live && wcscmp(character.ID, wanted.c_str()) == 0)
        {
            return character.Key;
        }
    }
    return -1;
}

// Sends one chat line the way the chat box does.
void SendChat(const std::string& text)
{
    const std::wstring wide = Core::Text::FromUtf8(text);
    SocketClient->ToGameServer()->SendPublicChatMessage(MU_C16(Hero->ID), MU_C16(wide.c_str()));
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
        // re-planning every frame would restart the walk mid-step. The
        // engine leaves PathNum alone when it arrives, so the end of a
        // path is CurrentPath reaching its last entry.
        if (Hero->Path.PathNum > 0 && Hero->Path.CurrentPath < Hero->Path.PathNum - 1)
        {
            return Status::Running;
        }

        // Pace the walk packets like a player's clicks; the server treats a
        // burst of them as a speed hack.
        const auto now = std::chrono::steady_clock::now();
        if (m_walking && now - m_lastWalkAt < WalkRepeatInterval)
        {
            return Status::Running;
        }

        m_lastWalkAt = now;
        m_walking = true;
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
    bool m_walking = false;
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
            // attack, not a refused one.
            const int index = FindCharacterIndex(m_targetKey);
            if (index != MAX_CHARACTERS_CLIENT && CharactersClient[index].Dead > 0)
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
    explicit PickupAct(int itemSlot) : m_itemSlot(itemSlot) {}

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
    std::chrono::steady_clock::time_point m_lastStep{};
};

// warp: the warp list entry, answered when the map has changed.
class WarpAct : public Act
{
public:
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
            m_fromMap = gMapManager.WorldActive;
            SocketClient->ToGameServer()->SendWarpCommandRequest(g_pMoveCommandWindow->GetMoveCommandKey(),
                                                                 static_cast<uint16_t>(m_mapIndex));
            m_sent = true;
            return Status::Running;
        }

        if (SceneFlag != MAIN_SCENE || gMapManager.WorldActive == m_fromMap)
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
    int m_fromMap = -1;
};

// teleport: the game master's own move command, answered when the
// character stands on the requested tile.
class TeleportAct : public Act
{
public:
    TeleportAct(std::string command, int tileX, int tileY)
        : m_command(std::move(command)), m_tileX(tileX), m_tileY(tileY)
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
            SendChat(m_command);
            m_sentAt = std::chrono::steady_clock::now();
            m_sent = true;
            return Status::Running;
        }

        if (Hero == nullptr || std::abs(Hero->PositionX - m_tileX) > ArrivalTiles ||
            std::abs(Hero->PositionY - m_tileY) > ArrivalTiles)
        {
            if (std::chrono::steady_clock::now() - m_sentAt < TeleportRefusalWindow)
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
    int m_tileX;
    int m_tileY;
    bool m_sent = false;
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

    const std::wstring wide = Core::Text::FromUtf8(name);
    const int mapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(wide.c_str());
    if (mapIndex < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::WarpRefused, "no warp list entry named `" + name + "`");
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

    std::string map;
    if (!request.GetString("map", map) || map.empty())
    {
        map = std::to_string(gMapManager.WorldActive);
    }

    // The server's own game-master command: /move <character> <map> <x> <y>.
    const std::string command =
        "/move " + Core::Text::ToUtf8(Hero->ID) + " " + map + " " + std::to_string(tileX) + " " + std::to_string(tileY);

    act = std::make_unique<TeleportAct>(command, tileX, tileY);
    return {};
}

std::string Attack(const Request& request, std::unique_ptr<Act>& act)
{
    const int targetKey = ResolveTargetKey(request);
    if (targetKey < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotInView, "the client does not see that object");
    }

    int times = 1;
    (void)request.GetInt("times", times);
    if (times < 1)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`times` must be at least 1");
    }

    int interval = 0;
    (void)request.GetInt("interval", interval);

    act = std::make_unique<AttackAct>(targetKey, times, std::chrono::milliseconds(std::max(interval, 0)));
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

    const int targetKey = ResolveTargetKey(request);
    const bool targetRequired = request.Has("target");
    if (targetRequired && targetKey < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotInView, "the client does not see that object");
    }

    act = std::make_unique<SkillAct>(skill, targetRequired, targetKey);
    return {};
}

std::string Pickup(const Request& request, std::unique_ptr<Act>& act)
{
    // `id` is the protocol's own request identifier, so a drop is named by
    // `item` — the id `nearby` reports for it.
    int id = 0;
    if (!request.GetInt("item", id))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`pickup` needs an item id");
    }

    // `nearby` reports both the server id and the client's own slot; accept
    // either, because the pickup packet carries the slot.
    int slot = -1;
    for (int index = 0; index < MAX_ITEMS; ++index)
    {
        if (Items[index].Object.Live && (Items[index].Key == id || index == id))
        {
            slot = index;
            break;
        }
    }

    if (slot < 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotInView, "no such drop in view");
    }

    act = std::make_unique<PickupAct>(slot);
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
    if (item == nullptr || item->Type <= 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::EmptySlot,
                           "inventory slot " + std::to_string(slot) + " is empty");
    }

    SocketClient->ToGameServer()->SendConsumeItemRequest(static_cast<BYTE>(slot), static_cast<BYTE>(0xFF),
                                                         FruitUsage::AddPoints);

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
    if (item == nullptr || item->Type <= 0)
    {
        return EncodeError(request.EncodedId(), ErrorCode::EmptySlot,
                           "inventory slot " + std::to_string(fromSlot) + " is empty");
    }

    SocketClient->ToGameServer()->SendItemMoveRequestExtended(ItemStorageKind::Inventory, static_cast<BYTE>(fromSlot),
                                                              ItemStorageKind::Inventory, static_cast<BYTE>(toSlot));

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

    const std::wstring wideName = Core::Text::FromUtf8(name);
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
        const int targetKey = ResolveTargetKey(request);
        if (targetKey < 0)
        {
            return EncodeError(request.EncodedId(), ErrorCode::NotInView, "the client does not see that player");
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
        // Leaving is kicking yourself: the member list starts with the
        // character itself, as the party window shows it.
        SocketClient->ToGameServer()->SendPartyPlayerKickRequest(static_cast<BYTE>(0));
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
