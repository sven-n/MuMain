#include "GameLogic/Social/GuildSize.h"

#include <algorithm>

namespace GameLogic::Social
{
int GetMaxGuildMembers(CLASS_TYPE baseClass, int level, int command)
{
    const int byLevel = level / LevelPerGuildMember;
    if (baseClass != CLASS_DARK_LORD)
    {
        return byLevel;
    }
    return std::min(byLevel + command / CommandPerGuildMember, MaxDarkLordGuildMembers);
}
} // namespace GameLogic::Social
