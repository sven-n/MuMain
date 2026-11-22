#pragma once

#include "UI/HUD/Skills/SkillTooltipModel.h"

#include "GameLogic/Buffs/SkillBuffValues.h"

// The skill tooltip's lines for a buff skill (buffs design B9): one white line per value of the buff it casts and the
// duration, each followed by gray lines for its parts above 0, naming their rules ("energy 2000: +66 (1 per 30)"),
// the maximum when it cut the value, and the learned master skills. Texts from Data/Buffs/BuffTexts.json; a value
// whose attribute has no text there shows no lines.
namespace UI::Skills::Tooltip
{
// Adds the lines of `values` (GameLogic::Buffs::ComputeSkillBuff) to `model`.
void AddSkillBuffLines(Model& model, const GameLogic::Buffs::SkillBuffValues& values,
                       const GameLogic::Buffs::BuffCasterStats& stats);
} // namespace UI::Skills::Tooltip
