#include "stdafx.h"

#ifdef _EDITOR

#include "ItemStatTermFields.h"

#include "ItemFieldWidgets.h"

#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <optional>
#include <string>

namespace MuEditor::Items::FormulaFields
{
namespace
{
using Data::Items::RequirementFormula;
using Data::Items::StatBonusTerm;
using Data::Items::WrittenStatTerm;

// The reader's ranges.
constexpr int MaxValue = 65535;
constexpr int MaxByte = 255;
// Pixel widths before the editor UI scale.
constexpr float NumberWidth = 70.0f;
constexpr float ChoiceWidth = 180.0f;

bool Number(const char* label, int& value, int minimum, int maximum)
{
    ImGui::SetNextItemWidth(Fields::ScaledWidth(NumberWidth));
    if (!ImGui::InputInt(label, &value, 0, 0))
    {
        return false;
    }
    value = std::clamp(value, minimum, maximum);
    return true;
}

// The first column of a term's row.
void BeginRow(const char* label)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
}

// The last column of a profile's row: Override for an inherited term, Reset
// to default for a written one. Returns what was clicked.
enum class SourceAction
{
    None,
    Override,
    Reset,
};

SourceAction RenderSource(bool profile, bool written)
{
    if (!profile)
    {
        return SourceAction::None;
    }
    ImGui::TableSetColumnIndex(2);
    if (!written)
    {
        return ImGui::SmallButton(I18N::Editor::Override) ? SourceAction::Override : SourceAction::None;
    }
    return ImGui::SmallButton(I18N::Editor::ResetToDefault) ? SourceAction::Reset : SourceAction::None;
}

std::string DescribeTerm(const WrittenStatTerm& term)
{
    if (term.state != WrittenStatTerm::State::Values)
    {
        return I18N::Editor::None;
    }
    const StatBonusTerm& values = term.values;
    std::string text =
        I18N::Format(I18N::Editor::Times0Plus1, {std::to_string(values.times), std::to_string(values.add)});
    if (values.dropLevelDivisor != 0)
    {
        text += ", " + I18N::Format(I18N::Editor::DropLevelDividedBy0, {std::to_string(values.dropLevelDivisor)});
    }
    if (values.afterLevelBonus)
    {
        text += std::string(", ") + I18N::Editor::AfterTheLevelBonus;
    }
    return text;
}

bool TermValues(StatBonusTerm& values, bool ancient)
{
    bool changed = Number(I18N::Editor::Times, values.times, 0, MaxValue);
    ImGui::SameLine();
    changed |= Number(I18N::Editor::Plus, values.add, 0, MaxValue);
    ImGui::SameLine();
    changed |= Number(I18N::Editor::DropLevelDivisor, values.dropLevelDivisor, 0, MaxValue);
    if (ancient)
    {
        ImGui::SameLine();
        changed |= ImGui::Checkbox(I18N::Editor::AfterTheLevelBonus, &values.afterLevelBonus);
    }
    return changed;
}

bool TermRow(const char* label, WrittenStatTerm& term, const WrittenStatTerm* inherited, bool ancient)
{
    ImGui::PushID(label);
    BeginRow(label);
    bool changed = false;
    const bool written = term.state != WrittenStatTerm::State::NotWritten;
    if (!written)
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::Inherited0, {DescribeTerm(*inherited)}).c_str());
    }
    else
    {
        const bool hasValues = term.state == WrittenStatTerm::State::Values;
        ImGui::SetNextItemWidth(Fields::ScaledWidth(ChoiceWidth * 0.6f));
        if (ImGui::BeginCombo("##State", hasValues ? I18N::Editor::Values : I18N::Editor::None))
        {
            if (ImGui::Selectable(I18N::Editor::None, !hasValues) && hasValues)
            {
                term.state = WrittenStatTerm::State::None;
                changed = true;
            }
            if (ImGui::Selectable(I18N::Editor::Values, hasValues) && !hasValues)
            {
                term.state = WrittenStatTerm::State::Values;
                changed = true;
            }
            ImGui::EndCombo();
        }
        if (term.state == WrittenStatTerm::State::Values)
        {
            ImGui::SameLine();
            changed |= TermValues(term.values, ancient);
        }
    }
    switch (RenderSource(inherited != nullptr, written))
    {
    case SourceAction::Override:
        term = *inherited;
        changed = true;
        break;
    case SourceAction::Reset:
        term = WrittenStatTerm{};
        changed = true;
        break;
    case SourceAction::None:
        break;
    }
    ImGui::PopID();
    return changed;
}

template <typename TValue>
bool OptionalRow(const char* label, std::optional<TValue>& value, const std::optional<TValue>* inherited,
                 const auto& render, const auto& describe)
{
    ImGui::PushID(label);
    BeginRow(label);
    bool changed = false;
    if (!value)
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::Inherited0,
                                               {inherited != nullptr && *inherited ? describe(**inherited) : ""})
                                      .c_str());
    }
    else
    {
        changed = render(*value);
    }
    switch (RenderSource(inherited != nullptr, value.has_value()))
    {
    case SourceAction::Override:
        value = *inherited;
        changed = true;
        break;
    case SourceAction::Reset:
        value.reset();
        changed = true;
        break;
    case SourceAction::None:
        break;
    }
    ImGui::PopID();
    return changed;
}

bool IntRow(const char* label, std::optional<int>& value, const std::optional<int>* inherited, int maximum)
{
    return OptionalRow(
        label, value, inherited, [&](int& number) { return Number("##Value", number, 0, maximum); },
        [](int number) { return std::to_string(number); });
}

enum class FormulaKind
{
    Base,
    Weighted,
    FromLevelRequirement,
};

FormulaKind KindOf(const RequirementFormula& formula)
{
    return formula.keepBase                ? FormulaKind::Base
           : formula.levelRequirementBasis ? FormulaKind::FromLevelRequirement
                                           : FormulaKind::Weighted;
}

const char* KindLabel(FormulaKind kind)
{
    switch (kind)
    {
    case FormulaKind::Base:
        return I18N::Editor::BaseValue;
    case FormulaKind::FromLevelRequirement:
        return I18N::Editor::FromTheLevelRequirement;
    case FormulaKind::Weighted:
    default:
        return I18N::Editor::WeightFormula;
    }
}

std::string DescribeFormula(const RequirementFormula& formula)
{
    if (formula.keepBase)
    {
        return I18N::Editor::BaseValue;
    }
    return std::string(KindLabel(KindOf(formula))) + ": " +
           I18N::Format(
               I18N::Editor::Plus0LevelWeight1Percent2,
               {std::to_string(formula.add), std::to_string(formula.levelWeight), std::to_string(formula.percent)});
}

bool FormulaFields(RequirementFormula& formula)
{
    bool changed = false;
    const FormulaKind kind = KindOf(formula);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(ChoiceWidth));
    if (ImGui::BeginCombo("##Kind", KindLabel(kind)))
    {
        for (const FormulaKind option : {FormulaKind::Base, FormulaKind::Weighted, FormulaKind::FromLevelRequirement})
        {
            if (ImGui::Selectable(KindLabel(option), option == kind) && option != kind)
            {
                // A formula starts as the shipped default's weighted one.
                if (kind == FormulaKind::Base)
                {
                    formula = RequirementFormula{false, 20, 3, 3, false};
                }
                formula.keepBase = option == FormulaKind::Base;
                formula.levelRequirementBasis = option == FormulaKind::FromLevelRequirement;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    if (!formula.keepBase)
    {
        ImGui::SameLine();
        changed |= Number(I18N::Editor::Plus, formula.add, 0, MaxValue);
        ImGui::SameLine();
        changed |= Number(I18N::Editor::LevelWeight, formula.levelWeight, 0, MaxValue);
        ImGui::SameLine();
        changed |= Number(I18N::Editor::Percent, formula.percent, 0, MaxValue);
    }
    return changed;
}

bool BeginTerms(bool profile)
{
    const int columns = profile ? 3 : 2;
    return ImGui::BeginTable("##Terms", columns, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);
}
} // namespace

bool StatSection(Data::Items::ItemStat stat, Data::Items::WrittenStatSection& section,
                 const Data::Items::WrittenStatSection* defaults, std::span<const Data::Items::LevelBonusTable> tables)
{
    const bool profile = defaults != nullptr;
    if (!BeginTerms(profile))
    {
        return false;
    }
    bool changed = OptionalRow(
        I18N::Editor::LevelBonusTable, section.table, profile ? &defaults->table : nullptr,
        [&](std::string& name)
        {
            bool picked = false;
            ImGui::SetNextItemWidth(Fields::ScaledWidth(ChoiceWidth * 1.4f));
            if (ImGui::BeginCombo("##Table", name.c_str()))
            {
                for (const Data::Items::LevelBonusTable& table : tables)
                {
                    if (ImGui::Selectable(table.name.c_str(), table.name == name) && table.name != name)
                    {
                        name = table.name;
                        picked = true;
                    }
                }
                ImGui::EndCombo();
            }
            return picked;
        },
        [](const std::string& name) { return name; });
    changed |= TermRow(I18N::Editor::ExcellentPart, section.excellent, profile ? &defaults->excellent : nullptr, false);
    changed |= TermRow(I18N::Editor::AncientPart, section.ancient, profile ? &defaults->ancient : nullptr, true);
    if (stat == Data::Items::ItemStat::MagicPower)
    {
        changed |= IntRow(I18N::Editor::PerLevelAfterHalving, section.perLevelAfterHalving,
                          profile ? &defaults->perLevelAfterHalving : nullptr, MaxByte);
    }
    ImGui::EndTable();
    return changed;
}

bool RequirementSection(Data::Items::WrittenRequirementSection& section,
                        const Data::Items::WrittenRequirementSection* defaults)
{
    const bool profile = defaults != nullptr;
    if (!BeginTerms(profile))
    {
        return false;
    }
    bool changed = IntRow(I18N::Editor::LevelRequirementPerItemLevel, section.levelPerItemLevel,
                          profile ? &defaults->levelPerItemLevel : nullptr, MaxValue);
    changed |= IntRow(I18N::Editor::LevelAddedForExcellentOptions, section.excellentLevelAdd,
                      profile ? &defaults->excellentLevelAdd : nullptr, MaxValue);
    const char* labels[] = {I18N::Metadata::ReqStrength, I18N::Metadata::ReqDexterity, I18N::Metadata::ReqEnergy,
                            I18N::Metadata::ReqVitality, I18N::Metadata::ReqLeadership};
    for (size_t stat = 0; stat < Data::Items::ItemRequirementStatCount; ++stat)
    {
        changed |= OptionalRow(
            labels[stat], section.stats[stat], profile ? &defaults->stats[stat] : nullptr,
            [](RequirementFormula& formula) { return FormulaFields(formula); }, &DescribeFormula);
    }
    changed |= IntRow(I18N::Editor::StrengthPerOptionLevel, section.strengthPerOptionLevel,
                      profile ? &defaults->strengthPerOptionLevel : nullptr, MaxByte);
    ImGui::EndTable();
    return changed;
}
} // namespace MuEditor::Items::FormulaFields

#endif // _EDITOR
