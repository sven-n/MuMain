#include "stdafx.h"

#include "MonsterDataValidation.h"

#include "Core/Text/Utf8.h"

#include <algorithm>
#include <array>
#include <string>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const MonsterDefinition& monster,
              const std::string& message)
{
    issues.push_back({severity, "", ItemDataIssue::NoItem, monster.number,
                      "monster " + std::to_string(monster.number) + " name", message});
}

// The models of the five Crywolf altars, which the altar contract's code
// counts from (CrywolfAltarTalk).
bool IsCrywolfAltarModel(const std::string& model)
{
    static constexpr std::array<const char*, 5> Altars{"MODEL_CRYWOLF_ALTAR1", "MODEL_CRYWOLF_ALTAR2",
                                                       "MODEL_CRYWOLF_ALTAR3", "MODEL_CRYWOLF_ALTAR4",
                                                       "MODEL_CRYWOLF_ALTAR5"};
    return std::ranges::find(Altars, model) != Altars.end();
}

bool IsTooLongForGame(const std::string& text)
{
    std::string cut = text;
    return Core::Text::CutToUtf16Length(cut, MaxMonsterNameLength);
}

void ValidateName(const MonsterDefinition& monster, std::vector<ItemDataIssue>& issues)
{
    const LocalizedString& names = monster.names;
    if (names.GetNeutral().empty())
    {
        // A translation still shows in its own language (LocalizedString::Get).
        AddIssue(issues, ItemDataIssueSeverity::Warning, monster,
                 names.GetTranslations().empty()
                     ? "the English name is empty: the game shows no name"
                     : "the English name is empty: the game shows no name in a language without a translation");
    }

    const auto check = [&](const std::string& locale, const std::string& text)
    {
        if (LocalizedString::ContainsSeparator(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, monster,
                     "the " + locale + " name must not contain \"" + std::string(LocalizedString::Separator) + "\"");
        }
        if (IsTooLongForGame(text))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, monster,
                     "the " + locale + " name is longer than " + std::to_string(MaxMonsterNameLength) +
                         " characters and is cut in the game");
        }
    };

    check(std::string(LocalizedString::NeutralLocale), names.GetNeutral());
    for (const auto& [locale, text] : names.GetTranslations())
    {
        if (!LocalizedString::IsValidLocaleCode(locale))
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, monster,
                     "\"" + locale + "\" is not a language code (letters, digits and \"-\" only)");
            continue;
        }
        check(locale, text);
    }
}
} // namespace

void ValidateMonsterNames(std::span<const MonsterDefinition> monsters, std::vector<ItemDataIssue>& issues)
{
    for (const MonsterDefinition& monster : monsters)
    {
        ValidateName(monster, issues);
    }
}

void ValidateMonsterSetups(std::span<const MonsterDefinition* const> monsters, const std::string& source,
                           std::vector<ItemDataIssue>& issues)
{
    for (const MonsterDefinition* monster : monsters)
    {
        // `where` names the place for live as it always did, `talkWhere` for
        // the talk steps as the other talk checks do ("mapVariants[0].").
        const auto check = [&](const MonsterSetup& setup, const std::string& model, const std::string& where,
                               const std::string& talkWhere)
        {
            if (setup.live == false)
            {
                issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, monster->number,
                                  "monster " + std::to_string(monster->number) + where + " live",
                                  "is false: the monster is dropped when it appears, so it is not drawn, cannot be "
                                  "clicked, and later packets for it find nothing, as with the old code (P24)"});
            }
            const bool altarStep =
                std::ranges::any_of(setup.onTalk, [](const MonsterTalkBlock& block)
                                    { return block.kind == MonsterTalkBlock::Kind::CrywolfAltarContract; });
            if (altarStep && !IsCrywolfAltarModel(model))
            {
                issues.push_back(
                    {ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, monster->number,
                     "monster " + std::to_string(monster->number) + " " + talkWhere + "onTalk",
                     "has a crywolfAltarContract step but its model " + model + " is no Crywolf altar: it only talks"});
            }
        };
        const auto checkStates = [&](const MonsterSetup& setup, const std::string& model, const std::string& where,
                                     const std::string& talkWhere)
        {
            check(setup, model, where, talkWhere);
            for (size_t index = 0; index < setup.stateVariants.size(); ++index)
                check(setup.stateVariants[index].setup, model,
                      where + " (state " + setup.stateVariants[index].state + ")",
                      talkWhere + "stateVariants[" + std::to_string(index) + "].");
        };
        checkStates(monster->setup, monster->model, "", "");
        for (size_t index = 0; index < monster->mapVariants.size(); ++index)
        {
            const MonsterMapVariant& variant = monster->mapVariants[index];
            checkStates(variant.setup, variant.model.value_or(monster->model),
                        " (map variant " + std::to_string(index + 1) + ")",
                        "mapVariants[" + std::to_string(index) + "].");
        }
    }
}

void ValidateMonsterTalkSteps(std::span<const MonsterDefinition* const> monsters, const MonsterTalkTexts& talkTexts,
                              const std::string& source, std::vector<ItemDataIssue>& issues)
{
    for (const MonsterDefinition* monster : monsters)
    {
        const std::string name = "monster " + std::to_string(monster->number) + " ";
        const auto check = [&](const MonsterSetup& setup, const std::string& where)
        {
            for (size_t index = 0; index < setup.onTalk.size(); ++index)
            {
                const MonsterTalkBlock& block = setup.onTalk[index];
                const std::string step = name + where + "onTalk[" + std::to_string(index) + "]";
                if (block.kind == MonsterTalkBlock::Kind::Dialog && block.dialog.ok &&
                    !block.dialog.ok->quest.empty() && block.dialog.ok->questStates.empty())
                    issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, monster->number,
                                      step + ".dialog.ok.if.is", "is empty: the OK sends nothing"});
                if (block.kind == MonsterTalkBlock::Kind::CrywolfAltarContract && !talkTexts.contains(block.altarTexts))
                    issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, monster->number,
                                      step + ".crywolfAltarContract",
                                      "names " + (block.altarTexts.empty() ? std::string("no set") : block.altarTexts) +
                                          ", which the shared talk texts do not have: the altar shows empty texts"});
            }
        };
        const auto checkStates = [&](const MonsterSetup& setup, const std::string& where)
        {
            check(setup, where);
            for (size_t index = 0; index < setup.stateVariants.size(); ++index)
                check(setup.stateVariants[index].setup, where + "stateVariants[" + std::to_string(index) + "].");
        };
        checkStates(monster->setup, "");
        for (size_t index = 0; index < monster->mapVariants.size(); ++index)
            checkStates(monster->mapVariants[index].setup, "mapVariants[" + std::to_string(index) + "].");
    }
}

void ValidateMonsterTalkTexts(std::span<const MonsterDefinition* const> monsters, const MonsterTalkTexts& talkTexts,
                              std::span<const char* const> uiLocales, const std::string& source,
                              std::vector<ItemDataIssue>& issues)
{
    const auto checkText = [&](const LocalizedString& text, int number, const std::string& field)
    {
        std::string missing;
        for (const char* locale : uiLocales)
        {
            if (!text.HasOwn(locale))
                missing += (missing.empty() ? "" : ", ") + std::string(locale);
        }
        if (!missing.empty())
            issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, number, field,
                              "has no text in " + missing + ": the English one shows there"});
    };
    ForEachSharedTalkText(talkTexts, [&](const LocalizedString& text, const std::string& field)
                          { checkText(text, ItemDataIssue::NoItem, field); });
    // The fields as Monster Translations' Talk texts names them
    // ("mapVariants[0].onTalk[0].message"), so its search finds them.
    for (const MonsterDefinition* monster : monsters)
    {
        ForEachMonsterTalkText(
            *monster, [&](const LocalizedString& text, const std::string& field)
            { checkText(text, monster->number, "monster " + std::to_string(monster->number) + " " + field); });
    }
}

void ValidateMonsterNameLocales(std::span<const MonsterDefinition> monsters, std::span<const char* const> uiLocales,
                                std::vector<ItemDataIssue>& issues)
{
    const auto isShown = [&](const std::string& locale)
    {
        return std::any_of(uiLocales.begin(), uiLocales.end(),
                           [&](const char* uiLocale) { return locale == uiLocale; });
    };
    for (const MonsterDefinition& monster : monsters)
    {
        for (const auto& [locale, text] : monster.names.GetTranslations())
        {
            if (!isShown(locale))
            {
                AddIssue(issues, ItemDataIssueSeverity::Warning, monster,
                         "the " + locale + " name is never shown: the game has no texts in that language");
            }
        }
    }
}
namespace
{
const char* const NoneLook = "none";

void CheckLookName(const std::optional<std::string>& name, const char* field, const char* kind, const char* file,
                   const LookExists& exists, const std::string& place, const std::string& source, int number,
                   std::vector<ItemDataIssue>& issues)
{
    if (!name || *name == NoneLook || !exists || exists(*name))
        return;
    issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, number, place + " " + field,
                      *name + " is no " + kind + " of " + file + ": it is drawn without it"});
}

// An effect a monster names that does something in the update, which only items run.
void CheckEffectParts(const std::optional<std::string>& name, const LookExists& exists, const LookExists& runsInUpdate,
                      const std::string& place, const std::string& source, int number,
                      std::vector<ItemDataIssue>& issues)
{
    if (!name || *name == NoneLook || !runsInUpdate || (exists && !exists(*name)) || !runsInUpdate(*name))
        return;
    issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, number, place + " effect",
                      *name + " has an object light, things on bones or an animation speed, which only items run: "
                              "the monster gets only its values before the drawing"});
}
} // namespace

void ValidateMonsterLooks(std::span<const MonsterDefinition* const> monsters, const LookExists& renderStyleExists,
                          const LookExists& effectExists, const std::string& source, std::vector<ItemDataIssue>& issues,
                          const LookExists& runsInUpdate)
{
    for (const MonsterDefinition* monster : monsters)
    {
        const auto check = [&](const MonsterSetup& setup, const std::string& where)
        {
            const std::string place = "monster " + std::to_string(monster->number) + where;
            CheckLookName(setup.renderStyle, "renderStyle", "render style", "Data/Effects/RenderStyles.json",
                          renderStyleExists, place, source, monster->number, issues);
            CheckLookName(setup.effect, "effect", "effect", "Data/Effects/Effects.json", effectExists, place, source,
                          monster->number, issues);
            CheckEffectParts(setup.effect, effectExists, runsInUpdate, place, source, monster->number, issues);
        };
        const auto checkStates = [&](const MonsterSetup& setup, const std::string& where)
        {
            check(setup, where);
            for (size_t index = 0; index < setup.stateVariants.size(); ++index)
                check(setup.stateVariants[index].setup, where + " stateVariants[" + std::to_string(index) + "]");
        };
        checkStates(monster->setup, "");
        for (size_t index = 0; index < monster->mapVariants.size(); ++index)
            checkStates(monster->mapVariants[index].setup, " mapVariants[" + std::to_string(index) + "]");
    }
}

std::map<std::string, int, std::less<>> CountMonsterLookUses(std::span<const MonsterDefinition* const> monsters,
                                                             std::span<const MonsterModelDefinition* const> models,
                                                             bool effects)
{
    std::map<std::string, int, std::less<>> uses;
    std::set<std::string, std::less<>> named;
    const auto add = [&](const std::optional<std::string>& name)
    {
        if (name && *name != NoneLook)
            named.insert(*name);
    };
    const auto addStates = [&](const MonsterSetup& setup)
    {
        add(effects ? setup.effect : setup.renderStyle);
        for (const MonsterStateVariant& state : setup.stateVariants)
            add(effects ? state.setup.effect : state.setup.renderStyle);
    };
    for (const MonsterDefinition* monster : monsters)
    {
        named.clear();
        addStates(monster->setup);
        for (const MonsterMapVariant& variant : monster->mapVariants)
            addStates(variant.setup);
        for (const std::string& name : named)
            ++uses[name];
    }
    for (const MonsterModelDefinition* model : models)
    {
        const std::string& name = effects ? model->effect : model->renderStyle;
        if (!name.empty())
            ++uses[name];
    }
    return uses;
}

void ValidateMonsterModelLooks(std::span<const MonsterModelDefinition* const> models,
                               const LookExists& renderStyleExists, const LookExists& effectExists,
                               const std::string& source, std::vector<ItemDataIssue>& issues,
                               const LookExists& runsInUpdate)
{
    const auto named = [](const std::string& name) { return name.empty() ? std::nullopt : std::optional(name); };
    for (const MonsterModelDefinition* model : models)
    {
        const std::string place = "model " + model->slot;
        CheckLookName(named(model->renderStyle), "renderStyle", "render style", "Data/Effects/RenderStyles.json",
                      renderStyleExists, place, source, ItemDataIssue::NoItem, issues);
        CheckLookName(named(model->effect), "effect", "effect", "Data/Effects/Effects.json", effectExists, place,
                      source, ItemDataIssue::NoItem, issues);
        CheckEffectParts(named(model->effect), effectExists, runsInUpdate, place, source, ItemDataIssue::NoItem,
                         issues);
    }
}
} // namespace Data::Monsters
