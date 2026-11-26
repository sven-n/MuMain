#pragma once

#include "Data/GameData/BuffData/BuffAttributes.h"

#include <cstdint>
#include <string>

// One master skill of Data/Skills/MasterSkills.json (buffs design B18, B20): what it adds to the buff its skill casts,
// in OpenMU's shape (AddMasterSkillDefinition: the target attribute, the way it combines, whether it extends the
// duration). Its value is the one the server sends for the hero's skill level (CSkillTreeInfo::GetSkillValue). Skills
// as data (SK1) takes this file over later. See docs/buff-data.md, "Master skills".
namespace Data::Skills
{
constexpr int MasterSkillsFormatVersion = 1;

// How a master skill's value joins the buff value it raises, after the value's maximum: `add` adds it (a fraction's
// attribute takes it in percent, so 10 adds 0.10), `multiply` multiplies the value by 1 + value / 100 (several add up
// to one factor, as OpenMU sums them).
enum class MasterSkillHow : std::uint8_t
{
    Add,
    Multiply,
};

struct MasterSkillDefinition
{
    int skill = 0;
    // The buff attribute it raises (BuffAttributes); empty when it raises none.
    std::string raises;
    MasterSkillHow how = MasterSkillHow::Add;
    // Its value in seconds is added to the buff's duration, after the duration's maximum.
    bool extendsDuration = false;
    // The buff the skill casts when the skill data's Effect names another (OpenMU casts 6 for 441); 0 for the
    // Effect's.
    int castsBuff = 0;
    // The index of `raises` in BuffAttributes, which the table resolves (MasterSkillDatabase::Build).
    int raisesIndex = Buffs::NoBuffAttribute;

    // The data only: raisesIndex follows from the name.
    bool operator==(const MasterSkillDefinition& other) const
    {
        return skill == other.skill && raises == other.raises && how == other.how &&
               extendsDuration == other.extendsDuration && castsBuff == other.castsBuff;
    }
};
} // namespace Data::Skills
