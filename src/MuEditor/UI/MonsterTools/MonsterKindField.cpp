#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterKindField.h"

#include "Data/GameData/MonsterData/LegacyMonsterKind.h"
#include "I18N/All.h"
#include "imgui.h"

#include <array>

namespace MuEditor::Monsters::KindField
{
namespace
{
using Data::Monsters::MonsterKind;

constexpr std::array<MonsterKind, 4> Kinds = {MonsterKind::Monster, MonsterKind::Npc, MonsterKind::Trap,
                                              MonsterKind::Other};
} // namespace

std::span<const MonsterKind> GetKinds()
{
    return Kinds;
}

const char* GetLabel(MonsterKind kind)
{
    switch (kind)
    {
    case MonsterKind::Npc:
        return I18N::Editor::KindNPC;
    case MonsterKind::Trap:
        return I18N::Editor::KindTrap;
    case MonsterKind::Other:
        return I18N::Editor::KindOther;
    case MonsterKind::Monster:
    default:
        return I18N::Editor::KindMonster;
    }
}

MonsterKind GetLegacyKind(int number)
{
    // The old rules only give kinds that a monster kind has.
    return Data::Monsters::FromObjectKind(Data::Monsters::GetLegacyObjectKind(number)).value_or(MonsterKind::Monster);
}

bool RenderCombo(const char* label, MonsterKind& kind)
{
    if (!ImGui::BeginCombo(label, GetLabel(kind)))
    {
        return false;
    }
    bool changed = false;
    for (const MonsterKind choice : Kinds)
    {
        if (ImGui::Selectable(GetLabel(choice), choice == kind) && choice != kind)
        {
            kind = choice;
            changed = true;
        }
    }
    ImGui::EndCombo();
    return changed;
}
} // namespace MuEditor::Monsters::KindField

#endif // _EDITOR
