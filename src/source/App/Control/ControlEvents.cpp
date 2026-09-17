#include "App/Control/ControlEvents.h"

#include "Core/Text/Utf8.h"

#include "json.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <utility>

namespace
{
using nlohmann::json;

struct RingState
{
    std::array<App::Control::Events::Record, App::Control::Events::RingCapacity> entries;
    std::size_t next = 0;
    std::size_t count = 0;
    std::uint64_t lastSequence = 0;
    bool enabled = false;
};

struct SubscriberEntry
{
    std::size_t token = 0;
    App::Control::Events::Subscriber callback;
};

RingState& Ring()
{
    static RingState state;
    return state;
}

App::Control::Events::ObjectResolver& Resolver()
{
    static App::Control::Events::ObjectResolver resolver;
    return resolver;
}

std::vector<SubscriberEntry>& Subscribers()
{
    static std::vector<SubscriberEntry> subscribers;
    return subscribers;
}

std::string UtcTimestamp()
{
    using namespace std::chrono;

    const auto now = system_clock::now();
    const auto seconds = time_point_cast<std::chrono::seconds>(now);
    const auto milliseconds = duration_cast<std::chrono::milliseconds>(now - seconds).count();

    const std::time_t asTime = system_clock::to_time_t(seconds);
    std::tm utc = {};
#ifdef _WIN32
    ::gmtime_s(&utc, &asTime);
#else
    ::gmtime_r(&asTime, &utc);
#endif

    char buffer[32] = {};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &utc);

    char stamp[48] = {};
    std::snprintf(stamp, sizeof(stamp), "%s.%03dZ", buffer, static_cast<int>(milliseconds));
    return stamp;
}

// {id, name, kind} for an object the client knows, or a placeholder for one
// it does not (out of view, already deleted, or no resolver installed).
json DescribeObject(int key)
{
    App::Control::Events::ObjectDescription description;
    description.id = key;
    if (key >= 0 && Resolver())
    {
        description = Resolver()(key);
    }

    json described;
    described["id"] = description.id;
    described["name"] = description.name;
    described["kind"] = description.kind;
    if (description.known)
    {
        described["position"] = json::array({description.x, description.y});
    }
    return described;
}

void Push(std::string_view name, const json& fields)
{
    RingState& ring = Ring();

    App::Control::Events::Record record;
    record.seq = ++ring.lastSequence;
    record.utc = UtcTimestamp();
    record.name = name;
    record.fields = fields.dump();

    ring.entries[ring.next] = record;
    ring.next = (ring.next + 1) % App::Control::Events::RingCapacity;
    if (ring.count < App::Control::Events::RingCapacity)
    {
        ++ring.count;
    }

    for (const SubscriberEntry& subscriber : Subscribers())
    {
        if (subscriber.callback)
        {
            subscriber.callback(record);
        }
    }
}
} // namespace

namespace App::Control::Events
{
void SetObjectResolver(ObjectResolver resolver)
{
    Resolver() = std::move(resolver);
}

void SetEnabled(bool enabled)
{
    Ring().enabled = enabled;
}

bool IsEnabled()
{
    return Ring().enabled;
}

std::uint64_t LastSequence()
{
    return Ring().lastSequence;
}

std::vector<Record> Since(std::uint64_t seq)
{
    const RingState& ring = Ring();

    std::vector<Record> selected;
    selected.reserve(ring.count);

    const std::size_t oldest = (ring.next + RingCapacity - ring.count) % RingCapacity;
    for (std::size_t offset = 0; offset < ring.count; ++offset)
    {
        const Record& record = ring.entries[(oldest + offset) % RingCapacity];
        if (record.seq > seq)
        {
            selected.push_back(record);
        }
    }
    return selected;
}

void Reset()
{
    RingState& ring = Ring();
    ring.entries = {};
    ring.next = 0;
    ring.count = 0;
    ring.lastSequence = 0;
}

std::size_t Subscribe(Subscriber subscriber)
{
    static std::size_t nextToken = 0;

    ++nextToken;
    Subscribers().push_back({nextToken, std::move(subscriber)});
    return nextToken;
}

void Unsubscribe(std::size_t token)
{
    std::vector<SubscriberEntry>& subscribers = Subscribers();
    for (auto entry = subscribers.begin(); entry != subscribers.end(); ++entry)
    {
        if (entry->token == token)
        {
            subscribers.erase(entry);
            return;
        }
    }
}

std::size_t SubscriberCount()
{
    return Subscribers().size();
}

void RecordHit(Direction direction, int attackerKey, int targetKey, int damage, int shieldDamage, bool critical,
               bool missed)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["direction"] = direction == Direction::Dealt ? "dealt" : "received";
    fields["attacker"] = DescribeObject(attackerKey);
    fields["target"] = DescribeObject(targetKey);
    fields["damage"] = damage;
    fields["shield_damage"] = shieldDamage;
    fields["critical"] = critical;
    fields["missed"] = missed;
    Push("hit", fields);
}

void RecordDeath(int victimKey, int killerKey)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["victim"] = DescribeObject(victimKey);
    fields["killer"] = DescribeObject(killerKey);
    Push("killed", fields);
}

void RecordChat(const wchar_t* sender, const wchar_t* text, std::string_view kind)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["sender"] = Core::Text::ToUtf8(sender);
    fields["text"] = Core::Text::ToUtf8(text);
    fields["kind"] = kind;
    Push("chat", fields);
}

void RecordStat(std::string_view stat, long long value, long long maximum)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["stat"] = stat;
    fields["value"] = value;
    if (maximum >= 0)
    {
        fields["max"] = maximum;
    }
    Push("stat", fields);
}

void RecordDrop(int itemKey, const wchar_t* itemName, int x, int y)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["id"] = itemKey;
    fields["item"] = Core::Text::ToUtf8(itemName);
    fields["position"] = json::array({x, y});
    Push("drop", fields);
}

void RecordDropGone(int itemKey, std::string_view reason)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["id"] = itemKey;
    fields["reason"] = reason;
    Push("drop_gone", fields);
}

void RecordScene(std::string_view scene)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["scene"] = scene;
    Push("scene", fields);
}

void RecordMap(int mapNumber, const wchar_t* mapName, int x, int y)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["map"] = mapNumber;
    fields["map_name"] = Core::Text::ToUtf8(mapName);
    fields["position"] = json::array({x, y});
    Push("map", fields);
}

void RecordViewEnter(int key)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["object"] = DescribeObject(key);
    Push("view_enter", fields);
}

void RecordViewLeave(int key)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["object"] = DescribeObject(key);
    Push("view_leave", fields);
}

void RecordParty(std::string_view change, const wchar_t* name)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["change"] = change;
    fields["name"] = Core::Text::ToUtf8(name);
    Push("party", fields);
}

void RecordDisconnect(std::string_view reason)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["reason"] = reason;
    Push("disconnect", fields);
}

void RecordError(std::string_view command, std::string_view code, std::string_view message)
{
    if (!IsEnabled())
    {
        return;
    }

    json fields;
    fields["command"] = command;
    fields["error"] = code;
    fields["message"] = message;
    Push("error", fields);
}
} // namespace App::Control::Events
