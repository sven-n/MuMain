// The one-line calls the packet receive functions make into the event stream.
//
// They take what the packet already decoded and resolve names, kinds and the
// character's own stats here, so the receive monolith carries no formatting and
// no knowledge of the control socket beyond a single call per packet.
//
// Every recorder is a no-op while the control socket is off, and the recorders
// exist only in builds that enable it (ENABLE_CONTROL_SOCKET in
// src/CMakeLists.txt): a player build gets inline empty bodies below, so the
// receive functions keep the same one-line calls and the compiler drops them.
//
// Upstream sync checklist: when merging upstream changes into
// Network/Server/WSclient.cpp, keep these calls. They are all named
// `App::Control::Events::Record…` and each sits at the end of the receive
// function it belongs to.
#pragma once

#if MU_ENABLE_CONTROL_SOCKET

#include <cstdint>

namespace App::Control::Events
{
// A damage packet: `attackerIndex` is the client's own character slot of
// the attacker (the `AttackPlayer` global), `targetKey` the server id that
// took the hit.
void RecordAttackDamage(int attackerIndex, int targetKey, int damage, int shieldDamage, int damageType);

// A death packet. `killerKey` may be -1 when the packet does not name one.
void RecordDeathOf(int victimKey, int killerKey);

// Experience and level changes of the controlled character.
void RecordExperienceGain(std::uint64_t experience, int damage);
void RecordLevelUp(int level);

// Life/mana/shield/ability of the controlled character, as they stand now.
void RecordHeroStats();

// Chat lines, by kind: public, whisper, party, guild, system.
void RecordChatLine(const wchar_t* sender, const wchar_t* text, const char* kind);

// Drops appearing and vanishing. `itemSlot` is the client's slot in the
// dropped-item table.
void RecordDropAppeared(int itemSlot);
void RecordDropVanished(int itemSlot, const char* reason);

// The map the character is on, after a warp or a map-change packet.
void RecordMapChange();

// Objects entering and leaving the client's view.
void RecordViewEnterKey(int key);
void RecordViewLeaveKey(int key);

// Party membership changes: joined, left, list.
void RecordPartyChange(const char* change, const wchar_t* name);

// The server closed the session.
void RecordDisconnected(const char* reason);
} // namespace App::Control::Events

#else

#include <cstdint>

namespace App::Control::Events
{
inline void RecordAttackDamage(int, int, int, int, int) {}
inline void RecordDeathOf(int, int) {}
inline void RecordExperienceGain(std::uint64_t, int) {}
inline void RecordLevelUp(int) {}
inline void RecordHeroStats() {}
inline void RecordChatLine(const wchar_t*, const wchar_t*, const char*) {}
inline void RecordDropAppeared(int) {}
inline void RecordDropVanished(int, const char*) {}
inline void RecordMapChange() {}
inline void RecordViewEnterKey(int) {}
inline void RecordViewLeaveKey(int) {}
inline void RecordPartyChange(const char*, const wchar_t*) {}
inline void RecordDisconnected(const char*) {}
} // namespace App::Control::Events

#endif // MU_ENABLE_CONTROL_SOCKET
