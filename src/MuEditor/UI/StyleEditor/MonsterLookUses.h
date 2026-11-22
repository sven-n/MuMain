#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"

#include <map>
#include <string>
#include <string_view>

namespace MuEditor::Looks
{
// How many monster entries and monster models name each look (MN8), as the monster tools have them: counted in one
// pass over the monsters and models, again when either changed.
class MonsterLookUses
{
public:
    int Count(std::string_view name, Data::Looks::LookKind kind);

private:
    using Counts = std::map<std::string, int, std::less<>>;

    void Update();

    Counts m_renderStyles;
    Counts m_effects;
    int m_monsterVersion = -1;
    int m_modelVersion = -1;
};
} // namespace MuEditor::Looks

#endif // _EDITOR
