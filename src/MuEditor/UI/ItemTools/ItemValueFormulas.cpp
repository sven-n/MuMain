#include "stdafx.h"

#ifdef _EDITOR

#include "ItemValueFormulas.h"

#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <algorithm>

namespace MuEditor::Items
{
namespace
{
using Data::Items::WrittenStatTerm;

void LayTerm(const WrittenStatTerm& written, const FormulaSource& source,
             SourcedTerm<std::optional<Data::Items::StatBonusTerm>>& term)
{
    switch (written.state)
    {
    case WrittenStatTerm::State::NotWritten:
        return;
    case WrittenStatTerm::State::None:
        term.value.reset();
        break;
    case WrittenStatTerm::State::Values:
        term.value = written.values;
        break;
    }
    term.source = source;
}

template <typename TValue>
void LayValue(const std::optional<TValue>& written, const FormulaSource& source, SourcedTerm<TValue>& term)
{
    if (written)
    {
        term.value = *written;
        term.source = source;
    }
}

void LaySections(const std::array<Data::Items::WrittenStatSection, Data::Items::ItemStatCount>& sections,
                 const Data::Items::WrittenRequirementSection& requirements, const FormulaSource& source,
                 ExpandedStatProfile& profile)
{
    for (size_t stat = 0; stat < Data::Items::ItemStatCount; ++stat)
    {
        const Data::Items::WrittenStatSection& section = sections[stat];
        ExpandedStatTerms& terms = profile.stats[stat];
        if (section.table)
        {
            terms.table.value = *section.table;
            terms.table.source = source;
        }
        LayTerm(section.excellent, source, terms.excellent);
        LayTerm(section.ancient, source, terms.ancient);
        LayValue(section.perLevelAfterHalving, source, terms.perLevelAfterHalving);
    }
    ExpandedRequirementTerms& terms = profile.requirements;
    LayValue(requirements.levelPerItemLevel, source, terms.levelPerItemLevel);
    LayValue(requirements.excellentLevelAdd, source, terms.excellentLevelAdd);
    for (size_t stat = 0; stat < Data::Items::ItemRequirementStatCount; ++stat)
    {
        LayValue(requirements.stats[stat], source, terms.stats[stat]);
    }
    LayValue(requirements.strengthPerOptionLevel, source, terms.strengthPerOptionLevel);
}
} // namespace

ExpandedStatProfile ExpandStatProfile(const Data::Items::ItemStatFile& file, std::string_view name)
{
    ExpandedStatProfile profile;
    profile.name = Data::Items::StatProfileNames::Default;
    profile.excellentDropLevelBonus = file.excellentDropLevelBonus;
    profile.ancientDropLevelBonus = file.ancientDropLevelBonus;
    LaySections(file.defaults, file.defaultRequirements, {FormulaSource::Kind::Default, {}}, profile);

    const auto written = std::ranges::find_if(file.profiles, [name](const Data::Items::WrittenStatProfile& entry)
                                              { return entry.name == name; });
    if (written != file.profiles.end())
    {
        profile.name = written->name;
        LaySections(written->stats, written->requirements, {FormulaSource::Kind::Profile, written->name}, profile);
    }
    return profile;
}
} // namespace MuEditor::Items

#endif // _EDITOR
