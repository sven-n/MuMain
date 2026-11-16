#include "GameLogic/Social/GuildSize.h"

#include <algorithm>

namespace GameLogic::Social
{
int GetMaxGuildMembers(bool darkLord, int level, int command)
{
    const int byLevel = level / LevelPerGuildMember;
    if (!darkLord)
    {
        return byLevel;
    }
    return std::min(byLevel + command / CommandPerGuildMember, MaxDarkLordGuildMembers);
}
} // namespace GameLogic::Social
