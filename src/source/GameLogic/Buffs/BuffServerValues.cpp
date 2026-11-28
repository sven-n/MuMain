#include "stdafx.h"

#include "GameLogic/Buffs/BuffServerValues.h"

#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Buffs/w_BuffTimeControl.h"

#include <algorithm>
#include <bit>
#include <cstring>
#include <utility>

namespace GameLogic::Buffs
{
namespace
{
// Where the fields of the extended C1 07 are (docs/Packets/C1-07-MagicEffectStatusExtended_by-server.md in OpenMU).
constexpr std::uint8_t HeaderC1 = 0xC1;
constexpr std::uint8_t Code = 0x07;
constexpr std::size_t LengthAt = 1;
constexpr std::size_t CodeAt = 2;
constexpr std::size_t IsActiveAt = 3;
constexpr std::size_t PlayerIdAt = 4; // 2 bytes, big endian
constexpr std::size_t EffectAt = 6;
constexpr std::size_t SecondsAt = 7; // 4 bytes, little endian
constexpr std::size_t ValueCountAt = 11;
// In a value: the attribute id (16 bytes), then the kind and the float.
constexpr std::size_t KindAt = 16;
constexpr std::size_t ValueAt = 17;
constexpr std::size_t FieldSize32 = 4;
constexpr int BitsPerByte = 8;

std::uint32_t ReadUInt32LittleEndian(std::span<const std::uint8_t> bytes)
{
    std::uint32_t value = 0;
    for (std::size_t index = 0; index < FieldSize32; ++index)
    {
        value |= static_cast<std::uint32_t>(bytes[index]) << (BitsPerByte * index);
    }
    return value;
}
} // namespace

std::optional<ExtendedBuffStatus> ReadExtendedBuffStatus(std::span<const std::uint8_t> packet)
{
    if (packet.size() < ExtendedBuffStatusHeaderSize || packet[0] != HeaderC1 || packet[CodeAt] != Code ||
        packet[IsActiveAt] == 0)
    {
        return std::nullopt;
    }
    const std::size_t length = packet[LengthAt];
    const std::size_t count = packet[ValueCountAt];
    if (length > packet.size() || length != ExtendedBuffStatusHeaderSize + count * SentValueSize)
    {
        return std::nullopt;
    }

    ExtendedBuffStatus status;
    status.playerKey = (static_cast<int>(packet[PlayerIdAt]) << BitsPerByte) | packet[PlayerIdAt + 1];
    status.buff = packet[EffectAt];
    status.seconds = ReadUInt32LittleEndian(packet.subspan(SecondsAt, FieldSize32));
    for (std::size_t index = 0; index < count; ++index)
    {
        const std::span<const std::uint8_t> entry =
            packet.subspan(ExtendedBuffStatusHeaderSize + index * SentValueSize, SentValueSize);
        Data::Buffs::OpenMuAttributeId id{};
        std::copy_n(entry.begin(), id.size(), id.begin());
        const std::uint8_t kind = entry[KindAt];
        const float value = std::bit_cast<float>(ReadUInt32LittleEndian(entry.subspan(ValueAt, FieldSize32)));

        const int attribute = Data::Buffs::FindBuffAttributeByOpenMuId(id);
        if (attribute == Data::Buffs::NoBuffAttribute || kind > static_cast<std::uint8_t>(SentValueKind::Maximum))
        {
            status.unknown.push_back({id, kind, attribute != Data::Buffs::NoBuffAttribute});
            continue;
        }
        status.values.push_back(SentValue{attribute, static_cast<SentValueKind>(kind), value});
    }
    return status;
}

void ServerBuffValues::Set(int buff, std::vector<SentValue> values)
{
    m_values[buff] = std::move(values);
    ++m_version;
}

void ServerBuffValues::Clear(int buff)
{
    if (m_values.erase(buff) > 0)
    {
        ++m_version;
    }
}

void ServerBuffValues::ClearAll()
{
    m_values.clear();
    ++m_version;
}

const std::vector<SentValue>* ServerBuffValues::Find(int buff) const
{
    const auto found = m_values.find(buff);
    return found != m_values.end() ? &found->second : nullptr;
}

std::optional<SentValue> ServerBuffValues::FindValue(int buff, int attribute) const
{
    const std::vector<SentValue>* values = Find(buff);
    if (values == nullptr)
    {
        return std::nullopt;
    }
    const auto found = std::find_if(values->begin(), values->end(),
                                    [attribute](const SentValue& value) { return value.attribute == attribute; });
    return found != values->end() ? std::optional<SentValue>(*found) : std::nullopt;
}

bool ServerBuffValues::NoteUnknownId(const Data::Buffs::OpenMuAttributeId& id)
{
    return m_reportedIds.insert(id).second;
}

ServerBuffValues& HeroServerBuffValues()
{
    static ServerBuffValues values;
    return values;
}

void ForgetHeroSentBuffs()
{
    TheBuffTimeControl().Clear();
    HeroServerBuffValues().ClearAll();
}

double SentValueAmount(const SentValue& value)
{
    const bool known = value.attribute >= 0 && value.attribute < static_cast<int>(Data::Buffs::BuffAttributes.size());
    const double scale = known ? Data::Buffs::BuffAttributes[value.attribute].openMuScale : 1.0;
    // A factor adds what it raises its attribute by, which is in OpenMU's unit too.
    const double amount = value.kind == SentValueKind::Multiplicate ? static_cast<double>(value.value) - 1.0
                                                                    : static_cast<double>(value.value);
    return amount * scale;
}

std::optional<double> SumSentValues(const ServerBuffValues& values, int attribute,
                                    const std::function<bool(int buff)>& isActive)
{
    std::optional<double> total;
    for (const auto& [buff, sent] : values.All())
    {
        const Data::Buffs::BuffDefinition* entry = g_BuffDatabase.Find(buff);
        if (entry == nullptr || entry->valuesFrom != Data::Buffs::BuffValuesFrom::Caster || !isActive(buff))
        {
            continue;
        }
        for (const SentValue& value : sent)
        {
            if (value.attribute == attribute)
            {
                total = total.value_or(0.0) + SentValueAmount(value);
            }
        }
    }
    return total;
}

std::optional<double> GetHeroSentTotal(Data::Buffs::BuffAttributeName attribute)
{
    if (Hero == nullptr)
    {
        return std::nullopt;
    }
    return SumSentValues(HeroServerBuffValues(), attribute.Index(),
                         [](int buff) { return g_isCharacterBuff((&Hero->Object), static_cast<eBuffState>(buff)); });
}
} // namespace GameLogic::Buffs
