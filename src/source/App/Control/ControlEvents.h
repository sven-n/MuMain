// The control socket's event stream.
//
// The packet receive functions record what a scenario asserts on — hits,
// deaths, stat changes, chat, drops, scene and map changes, view and party
// changes, disconnects — into a fixed ring the dispatcher serves to `events`
// and `wait-for`. Recording is one branch on a flag when the socket is off, so
// an ordinary client does no work here.
//
// Everything runs on the main thread: the recorders are called from the packet
// drain, the readers from the control server's per-frame poll.
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace App::Control::Events
{
// Events retained before the oldest is overwritten. The spec asks for at
// least 1000; 2048 covers a long fight without growing the client's memory
// in any noticeable way.
inline constexpr std::size_t RingCapacity = 2048;

// Which side of a hit the controlled character was on.
enum class Direction : std::uint8_t
{
    Dealt,
    Received,
};

// What the event stream says about an object the client knows. Filled in by
// the resolver the control server installs (App/Control/ControlObjects),
// so this file stays free of the engine's globals and stays testable.
struct ObjectDescription
{
    int id = -1;
    std::string name;
    // player | monster | npc | pet | unknown
    std::string kind = "unknown";
    int x = -1;
    int y = -1;
    bool known = false;
};

using ObjectResolver = std::function<ObjectDescription(int key)>;

// Without a resolver every object is reported as `unknown` with its id.
void SetObjectResolver(ObjectResolver resolver);

struct Record
{
    std::uint64_t seq = 0;
    // UTC, ISO-8601 with milliseconds, e.g. 2026-09-16T11:22:33.456Z.
    std::string utc;
    // Event name, in the wire vocabulary a test script parses.
    std::string name;
    // Encoded JSON object with the event's fields.
    std::string fields;
};

using Subscriber = std::function<void(const Record&)>;

// Recording is off until the control server enables it.
void SetEnabled(bool enabled);
[[nodiscard]] bool IsEnabled();

[[nodiscard]] std::uint64_t LastSequence();
// Visits the events newer than `seq`, oldest first, stopping when `visit`
// returns false. Nothing is copied or allocated, which matters for the
// readers that run on every frame: a `wait-for` usually walks the ring to
// find nothing.
void ForEachSince(std::uint64_t seq, const std::function<bool(const Record&)>& visit);
// The same selection as a vector, for a caller that keeps it. Events already
// overwritten are gone; the caller sees a gap in the sequence numbers, never
// a duplicate.
[[nodiscard]] std::vector<Record> Since(std::uint64_t seq);
// Drops every retained event and resets the sequence. For tests and for a
// fresh session; the client never calls it while a follower is attached.
void Reset();

// Live delivery for `events --follow`. The token unsubscribes.
[[nodiscard]] std::size_t Subscribe(Subscriber subscriber);
void Unsubscribe(std::size_t token);
[[nodiscard]] std::size_t SubscriberCount();

void RecordHit(Direction direction, int attackerKey, int targetKey, int damage, int shieldDamage, bool critical,
               bool missed);
void RecordDeath(int victimKey, int killerKey);
void RecordChat(const wchar_t* sender, const wchar_t* text, std::string_view kind);
// `maximum` may be negative when the stat has no maximum (level, zen, …).
void RecordStat(std::string_view stat, long long value, long long maximum);
void RecordDrop(int itemKey, const wchar_t* itemName, int x, int y);
void RecordDropGone(int itemKey, std::string_view reason);
void RecordScene(std::string_view scene);
void RecordMap(int mapNumber, const wchar_t* mapName, int x, int y);
void RecordViewEnter(int key);
void RecordViewLeave(int key);
void RecordParty(std::string_view change, const wchar_t* name);
void RecordDisconnect(std::string_view reason);
void RecordError(std::string_view command, std::string_view code, std::string_view message);
} // namespace App::Control::Events
