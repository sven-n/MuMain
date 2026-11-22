#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterLookUses.h"

#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"

#include <vector>

namespace MuEditor::Looks
{
int MonsterLookUses::Count(std::string_view name, Data::Looks::LookKind kind)
{
    Update();
    const Counts& counts = kind == Data::Looks::LookKind::Effect ? m_effects : m_renderStyles;
    const auto found = counts.find(name);
    return found != counts.end() ? found->second : 0;
}

void MonsterLookUses::Update()
{
    if (m_monsterVersion == g_MonsterDatabase.GetVersion() && m_modelVersion == g_MonsterModelDatabase.GetVersion())
    {
        return;
    }
    m_monsterVersion = g_MonsterDatabase.GetVersion();
    m_modelVersion = g_MonsterModelDatabase.GetVersion();
    const std::vector<const Data::Monsters::MonsterDefinition*> monsters = g_MonsterDatabase.GetAll();
    m_renderStyles = Data::Monsters::CountMonsterLookUses(monsters, g_MonsterModelDatabase.GetAll(), false);
    m_effects = Data::Monsters::CountMonsterLookUses(monsters, g_MonsterModelDatabase.GetAll(), true);
}
} // namespace MuEditor::Looks

#endif // _EDITOR
