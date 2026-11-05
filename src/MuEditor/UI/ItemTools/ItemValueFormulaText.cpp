#include "stdafx.h"

#ifdef _EDITOR

#include "ItemValueFormulaText.h"

#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "I18N/All.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace MuEditor::Items
{
namespace
{
using Data::Items::ItemDefinition;
using Data::Items::ItemRequirement;
using Data::Items::ItemStat;

// Defense rate, which the game keeps in a byte.
constexpr int MaxByteStat = 255;

// A part of a line and where it comes from; a part without a source states a
// rule that stays code.
struct Part
{
    std::string text;
    std::optional<FormulaSource> source;
};

FormulaSource ItemField(const char* name)
{
    return {FormulaSource::Kind::ItemField, name};
}

// The parts joined by "; ", with the source after the last part of each run
// of parts from the same source.
std::string Join(const std::vector<Part>& parts)
{
    std::string text;
    for (size_t index = 0; index < parts.size(); ++index)
    {
        text += index == 0 ? "" : "; ";
        text += parts[index].text;
        const bool lastOfRun =
            parts[index].source && (index + 1 == parts.size() || parts[index + 1].source != parts[index].source);
        if (lastOfRun)
        {
            text += " [" + DescribeFormulaSource(*parts[index].source) + "]";
        }
    }
    return text;
}

// The terms that are there, joined by " + ".
std::string Sum(const std::vector<std::string>& terms)
{
    std::string text;
    for (const std::string& term : terms)
    {
        if (!term.empty())
        {
            text += (text.empty() ? "" : " + ") + term;
        }
    }
    return text;
}

std::string Number(int value)
{
    return std::to_string(value);
}

int DropLevelPart(const Data::Items::StatBonusTerm& term, int dropLevel)
{
    return term.dropLevelDivisor > 0 ? dropLevel / term.dropLevelDivisor : 0;
}

// "base × times / d + add + d / dropLevelDivisor" with the parts the term has.
std::string TermExpression(const Data::Items::StatBonusTerm& term, const std::string& base, int dropLevel)
{
    return Sum({term.times != 0 ? base + " × " + Number(term.times) + " / " + Number(dropLevel) : "",
                term.add != 0 ? Number(term.add) : "",
                term.dropLevelDivisor != 0 ? Number(dropLevel) + " / " + Number(term.dropLevelDivisor) : ""});
}

// "20 (+0 to +2), 50 (+3 to +15)".
std::string DescribeByLevel(const Data::Items::RequirementByLevel& values)
{
    std::string text;
    size_t first = 0;
    for (size_t level = 1; level <= values.size(); ++level)
    {
        if (level < values.size() && values[level] == values[first])
        {
            continue;
        }
        text += text.empty() ? "" : ", ";
        const std::string value = Number(values[first]);
        text += level - 1 == first
                    ? I18N::Format(I18N::Editor::FormulaValueAtOneItemLevel, {value, Number(static_cast<int>(first))})
                    : I18N::Format(I18N::Editor::FormulaValueAtItemLevels,
                                   {value, Number(static_cast<int>(first)), Number(static_cast<int>(level - 1))});
        first = level;
    }
    return I18N::Format(I18N::Editor::FormulaGivenByItemLevel, {text});
}

int BaseOf(const Data::Items::ItemRequirements& requirements, ItemRequirement requirement)
{
    switch (requirement)
    {
    case ItemRequirement::Strength:
        return requirements.strength;
    case ItemRequirement::Dexterity:
        return requirements.dexterity;
    case ItemRequirement::Energy:
        return requirements.energy;
    case ItemRequirement::Vitality:
        return requirements.vitality;
    case ItemRequirement::Leadership:
        return requirements.leadership;
    default:
        return requirements.level;
    }
}

// How many of the values are above 0.
size_t CountAbove0(const Data::Items::RequirementByLevel& values)
{
    return static_cast<size_t>(std::count_if(values.begin(), values.end(), [](WORD value) { return value > 0; }));
}

std::string RequirementFormulaText(const ItemDefinition& item, const Data::Items::RequirementFormula& formula,
                                   const std::string& weight)
{
    const std::string add = Number(formula.add);
    const std::string percent = Number(formula.percent);
    const std::string weighted = I18N::Format(I18N::Editor::FormulaWeightedRequirement,
                                              {add, weight, Number(item.level), Number(formula.levelWeight), percent});
    const Data::Items::RequirementByLevel* levelByLevel = item.requirementsByLevel.Find(ItemRequirement::Level);
    // The level requirement counts where it is above 0; elsewhere the drop level does.
    if (formula.levelRequirementBasis && levelByLevel != nullptr && CountAbove0(*levelByLevel) > 0)
    {
        const std::string byLevel =
            I18N::Format(I18N::Editor::FormulaRequirementFromTheLevelRequirementByLevel, {add, weight, percent});
        return CountAbove0(*levelByLevel) == levelByLevel->size()
                   ? byLevel
                   : I18N::Format(I18N::Editor::FormulaLevelRequirementOrDropLevel, {byLevel, weighted});
    }
    if (formula.levelRequirementBasis && levelByLevel == nullptr && item.requirements.level > 0)
    {
        return I18N::Format(I18N::Editor::FormulaRequirementFromTheLevelRequirement,
                            {add, weight, Number(item.requirements.level), percent});
    }
    return formula.levelRequirementBasis ? I18N::Format(I18N::Editor::FormulaWithoutALevelRequirement, {weighted})
                                         : weighted;
}
} // namespace

std::string DescribeFormulaSource(const FormulaSource& source)
{
    switch (source.kind)
    {
    case FormulaSource::Kind::Profile:
        return source.name;
    case FormulaSource::Kind::ItemField:
        return I18N::Format(I18N::Editor::FormulaSourceItemField, {source.name});
    case FormulaSource::Kind::DropLevelBonus:
        return "dropLevelBonus";
    case FormulaSource::Kind::Default:
    default:
        return Data::Items::StatProfileNames::Default;
    }
}

FormulaLine DescribeStatFormula(const ItemDefinition& item, ItemStat stat, bool maxDamage,
                                const ExpandedStatProfile& profile)
{
    int base = 0;
    int excellentBase = 0;
    // The fixed excellent bonus takes the place of the damage and magic power terms.
    bool fixedBonusCounts = false;
    switch (stat)
    {
    case ItemStat::Damage:
        base = maxDamage ? item.damageMax : item.damageMin;
        excellentBase = item.damageMin;
        fixedBonusCounts = true;
        break;
    case ItemStat::MagicPower:
        base = excellentBase = item.magicPower;
        fixedBonusCounts = true;
        break;
    case ItemStat::BlockRate:
        base = excellentBase = item.blockRate;
        break;
    case ItemStat::Defense:
        base = excellentBase = item.defense;
        break;
    case ItemStat::MagicDefense:
        base = excellentBase = item.magicDefense;
        break;
    default:
        break;
    }
    if (base == 0)
    {
        return {I18N::Editor::FormulaNone, {}};
    }

    const ExpandedStatTerms& terms = profile.Get(stat);
    const int dropLevel = item.level;
    const bool wing = item.slot == Data::Items::ItemSlot::Wings;
    std::vector<Part> parts;
    parts.push_back({terms.table.value ? Number(base) + " + " + *terms.table.value
                                       : I18N::Format(I18N::Editor::FormulaNoLevelBonus, {Number(base)}),
                     terms.table.source});

    // The excellent part, which an ancient item has too.
    int excellent = 0;
    if (!terms.excellent.value)
    {
        parts.push_back({I18N::Editor::FormulaNoExcellentPart, terms.excellent.source});
    }
    else if (dropLevel == 0)
    {
        parts.push_back({I18N::Editor::FormulaNoExcellentPartAtDropLevel0, std::nullopt});
    }
    else if (fixedBonusCounts && item.excellentBonus != 0)
    {
        excellent = item.excellentBonus;
        parts.push_back(
            {I18N::Format(I18N::Editor::FormulaExcellentPlus, {Number(excellent)}), ItemField("excellentBonus")});
    }
    else
    {
        const Data::Items::StatBonusTerm& term = *terms.excellent.value;
        excellent = excellentBase * term.times / dropLevel + term.add + DropLevelPart(term, dropLevel);
        const std::string expression = TermExpression(term, Number(excellentBase), dropLevel);
        parts.push_back(
            {I18N::Format(wing ? I18N::Editor::FormulaExcellentPartOnlyWhenAncient : I18N::Editor::FormulaExcellentPart,
                          {Number(excellent), expression}),
             terms.excellent.source});
    }

    if (!terms.ancient.value)
    {
        parts.push_back({I18N::Editor::FormulaNoAncientPart, terms.ancient.source});
    }
    else
    {
        const Data::Items::StatBonusTerm& term = *terms.ancient.value;
        const int ancientDropLevel = dropLevel + profile.ancientDropLevelBonus;
        std::vector<std::string> valueParts = {Number(base), excellent != 0 ? Number(excellent) : ""};
        if (term.afterLevelBonus && terms.table.value)
        {
            valueParts.push_back(*terms.table.value);
        }
        const std::string sum = Sum(valueParts);
        const bool several = sum.find(" + ") != std::string::npos;
        const std::string value = several ? "(" + sum + ")" : sum;
        const std::string expression = TermExpression(term, value, ancientDropLevel);
        if (term.afterLevelBonus)
        {
            parts.push_back(
                {I18N::Format(I18N::Editor::FormulaAncientPartWithTheLevelBonus, {expression}), terms.ancient.source});
        }
        else
        {
            const int ancient = (ancientDropLevel > 0 ? (base + excellent) * term.times / ancientDropLevel : 0) +
                                term.add + DropLevelPart(term, ancientDropLevel);
            parts.push_back(
                {I18N::Format(I18N::Editor::FormulaAncientPart, {Number(ancient), expression}), terms.ancient.source});
        }
    }

    std::string text = Join(parts);
    if (stat == ItemStat::MagicPower)
    {
        const SourcedTerm<int>& growth = terms.perLevelAfterHalving;
        text = (growth.value != 0 ? I18N::Format(I18N::Editor::FormulaHalved, {text, Number(growth.value)})
                                  : "(" + text + ") / 2") +
               " [" + DescribeFormulaSource(growth.source) + "]";
    }
    if (stat == ItemStat::BlockRate)
    {
        text = I18N::Format(I18N::Editor::FormulaAtMost, {text, Number(MaxByteStat)});
    }
    return {text, {}};
}

FormulaLine DescribeRequirementFormula(const ItemDefinition& item, ItemRequirement requirement,
                                       const ExpandedStatProfile& profile)
{
    const ExpandedRequirementTerms& terms = profile.requirements;
    const Data::Items::RequirementByLevel* byLevel = item.requirementsByLevel.Find(requirement);
    const int base = BaseOf(item.requirements, requirement);
    std::vector<Part> parts;
    if (requirement == ItemRequirement::Level)
    {
        // Excellent options raise only a level requirement above 0.
        const size_t above0 = byLevel != nullptr ? CountAbove0(*byLevel) : 0;
        if (byLevel != nullptr)
        {
            parts.push_back({DescribeByLevel(*byLevel), ItemField("requirementsByLevel")});
            if (above0 == 0)
            {
                return {Join(parts), {}};
            }
        }
        else if (base == 0)
        {
            return {I18N::Editor::FormulaNone, {}};
        }
        else
        {
            const SourcedTerm<int>& growth = terms.levelPerItemLevel;
            parts.push_back({growth.value > 0
                                 ? I18N::Format(I18N::Editor::FormulaLevelGrowth, {Number(base), Number(growth.value)})
                                 : I18N::Format(I18N::Editor::FormulaTheSameAtEveryItemLevel, {Number(base)}),
                             growth.source});
        }
        if (item.slot == Data::Items::ItemSlot::Wings)
        {
            parts.push_back({I18N::Editor::FormulaWingOptionsDoNotRaiseIt, std::nullopt});
        }
        else if (!item.excellentRaisesLevel)
        {
            parts.push_back({I18N::Editor::FormulaExcellentOptionsDoNotRaiseIt, ItemField("excellentRaisesLevel")});
        }
        else if (terms.excellentLevelAdd.value > 0)
        {
            const bool everyLevel = byLevel == nullptr || above0 == byLevel->size();
            parts.push_back({I18N::Format(everyLevel ? I18N::Editor::FormulaExcellentPlus
                                                     : I18N::Editor::FormulaExcellentPlusWhereItIsAbove0,
                                          {Number(terms.excellentLevelAdd.value)}),
                             terms.excellentLevelAdd.source});
        }
        return {Join(parts), {}};
    }

    std::string tooltip;
    const SourcedTerm<Data::Items::RequirementFormula>& formula =
        terms.stats[static_cast<size_t>(requirement) - static_cast<size_t>(ItemRequirement::Strength)];
    if (byLevel != nullptr)
    {
        parts.push_back({DescribeByLevel(*byLevel), ItemField("requirementsByLevel")});
    }
    else if (formula.value.keepBase)
    {
        parts.push_back({base == 0 ? std::string(I18N::Editor::FormulaNone)
                                   : I18N::Format(I18N::Editor::FormulaKeptAsIs, {Number(base)}),
                         formula.source});
    }
    else
    {
        parts.push_back({base == 0
                             ? I18N::Format(I18N::Editor::FormulaNoneWithWeight0,
                                            {RequirementFormulaText(item, formula.value, I18N::Editor::FormulaWeight)})
                             : RequirementFormulaText(item, formula.value, Number(base)),
                         formula.source});
        tooltip = I18N::Format(I18N::Editor::FormulaDropLevelBonus,
                               {Number(profile.excellentDropLevelBonus), Number(profile.ancientDropLevelBonus)});
    }
    if (requirement == ItemRequirement::Strength && terms.strengthPerOptionLevel.value > 0)
    {
        parts.push_back(
            {I18N::Format(I18N::Editor::FormulaPerOptionLevel, {Number(terms.strengthPerOptionLevel.value)}),
             terms.strengthPerOptionLevel.source});
    }
    return {Join(parts), tooltip};
}
} // namespace MuEditor::Items

#endif // _EDITOR
