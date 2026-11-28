#pragma once

#include "Data/GameData/BuffData/BuffAttributes.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <vector>

// The time and the finished values the server sends for a buff on the hero (buffs design section 5, B3, B22 to B25):
// OpenMU's extended C1 07 (MUnique/OpenMU#1476) for the open source client.
namespace GameLogic::Buffs
{
// How a sent value changes its attribute: OpenMU's aggregate type.
enum class SentValueKind : std::uint8_t
{
    AddRaw = 0,
    Multiplicate = 1, // a factor: 1.22 raises the attribute by 22 %
    AddFinal = 2,
    Maximum = 3,
};

struct SentValue
{
    int attribute = Data::Buffs::NoBuffAttribute; // the index in Data::Buffs::BuffAttributes
    SentValueKind kind = SentValueKind::AddRaw;
    float value = 0.0f;
};

// A sent value the client can't use (B23): its attribute's id, its kind, and whether the client knows the attribute
// (then the kind is the unknown part).
struct UnknownSentValue
{
    Data::Buffs::OpenMuAttributeId id{};
    std::uint8_t kind = 0;
    bool knownAttribute = false;
};

// One extended C1 07, read.
struct ExtendedBuffStatus
{
    int playerKey = 0;
    int buff = 0;
    std::uint32_t seconds = 0; // the time left; 0 = the server sends no time for this buff (B25)
    std::vector<SentValue> values;
    std::vector<UnknownSentValue> unknown; // values the client can't use (B23)
};

// The packet's length without values: header 3, active 1, player 2, buff 1, seconds 4, value count 1.
inline constexpr std::size_t ExtendedBuffStatusHeaderSize = 12;
// One value: attribute id 16, kind 1, float 4.
inline constexpr std::size_t SentValueSize = 21;

// Reads the extended C1 07 from the bytes of a C1 07 packet; nullopt for the short one (7 bytes, an older server or
// someone else's buff), an inactive one, or one whose length doesn't fit its value count.
std::optional<ExtendedBuffStatus> ReadExtendedBuffStatus(std::span<const std::uint8_t> packet);

// The values the server sent for the hero's buffs, by buff number. They are what the buff was at its last activation,
// recast or update; the buff tooltip and the character stats read them for buffs whose values come from the caster
// (B24).
class ServerBuffValues
{
public:
    void Set(int buff, std::vector<SentValue> values);
    void Clear(int buff);
    void ClearAll();

    // The sent values of the buff; nullptr when the server sent none.
    const std::vector<SentValue>* Find(int buff) const;

    // All sent values, by buff number.
    const std::map<int, std::vector<SentValue>>& All() const
    {
        return m_values;
    }

    // The sent value of the buff for that attribute; nullopt when there is none.
    std::optional<SentValue> FindValue(int buff, int attribute) const;

    // Counts every change, so a reader can keep what it built from the values until they change.
    std::uint32_t Version() const
    {
        return m_version;
    }

    // Whether the id hasn't been reported yet; true once per id (B23 logs each unknown id once).
    bool NoteUnknownId(const Data::Buffs::OpenMuAttributeId& id);

private:
    std::map<int, std::vector<SentValue>> m_values;
    std::uint32_t m_version = 0;
    std::set<Data::Buffs::OpenMuAttributeId> m_reportedIds;
};

// The hero's.
ServerBuffValues& HeroServerBuffValues();

// Forgets the times and values the server sent for the hero's buffs: the hero joins a world (login, character switch,
// another server), and OpenMU removed every effect when the player left (buffs design section 4).
void ForgetHeroSentBuffs();

// How much a sent value raises its attribute, as a number of the attribute's unit: a factor as the fraction it adds
// (1.22 gives 0.22), everything else in the client's unit (BuffAttribute::openMuScale: OpenMU's critical damage chance
// 0.2 is the client's 20).
double SentValueAmount(const SentValue& value);

// The sum of what the server sent for `attribute` over the buffs `isActive` names whose values come from their caster
// (Buffs.json `valuesFrom`), as SentValueAmount gives it; nullopt when none sent one. The data says which buffs count,
// so a master skill's buff (Expansion of Wizardry's 138 and 139) counts like its base skill's (B24).
std::optional<double> SumSentValues(const ServerBuffValues& values, int attribute,
                                    const std::function<bool(int buff)>& isActive);

// The same over the hero's buffs: the character window adds it to the stat; without it, the caller falls back to the
// client's formula (B3).
std::optional<double> GetHeroSentTotal(Data::Buffs::BuffAttributeName attribute);
} // namespace GameLogic::Buffs
