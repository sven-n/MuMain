#include "stdafx.h"

#include "Data/GameData/MonsterData/MonsterSoundMoments.h"

namespace Data::Monsters
{
std::string_view GetMonsterSoundMomentName(MonsterSoundMoment moment)
{
    switch (moment)
    {
    case MonsterSoundMoment::Attack:
        return "attack";
    case MonsterSoundMoment::Hit:
        return "hit";
    case MonsterSoundMoment::Death:
        return "death";
    case MonsterSoundMoment::Stand:
        return "stand";
    case MonsterSoundMoment::Walk:
        return "walk";
    }
    return "";
}

std::optional<MonsterSoundMoment> FindMonsterSoundMoment(std::string_view name)
{
    for (const MonsterSoundMoment moment : AllMonsterSoundMoments)
    {
        if (GetMonsterSoundMomentName(moment) == name)
        {
            return moment;
        }
    }
    return std::nullopt;
}
} // namespace Data::Monsters
