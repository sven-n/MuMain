#include "stdafx.h"

#include "MonsterDataHandler.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelJsonStorage.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/DataHandler/MonsterData/MonsterSoundJsonStorage.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterModelValidation.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundValidation.h"
#include "I18N/All.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <map>

namespace Data::Monsters
{
namespace
{
constexpr std::string_view MonsterDataLabel = "Monster data";
constexpr std::string_view MonsterDataName = "monster data";
constexpr std::string_view MonsterModelsLabel = "Monster models";
constexpr std::string_view MonsterModelsName = "monster model list";
constexpr std::string_view MonsterSoundsLabel = "Monster sounds";
constexpr std::string_view MonsterSoundsName = "monster sound sets";

// What the monster table, the model table and these sound sets name of each
// other: the game loads them one after the other. The tables' entries are
// read in place.
void ValidateLinks(std::span<const MonsterSoundSet> sets, std::vector<Items::ItemDataIssue>& issues)
{
    std::vector<const MonsterDefinition*> monsters;
    for (int number = 0; number < MonsterNumberCount; ++number)
    {
        if (const MonsterDefinition* monster = g_MonsterDatabase.Find(number))
        {
            monsters.push_back(monster);
        }
    }
    ValidateMonsterSoundLinks(monsters, g_MonsterModelDatabase.GetAll(), sets, GetMonstersFile().generic_string(),
                              GetMonsterSoundsFile().generic_string(), issues);
}

#ifdef _EDITOR
// The entries of the table, sorted by number.
std::vector<MonsterDefinition> CollectMonsters()
{
    std::vector<MonsterDefinition> monsters;
    for (int number = 0; number < MonsterNumberCount; ++number)
    {
        if (const MonsterDefinition* monster = g_MonsterDatabase.Find(number))
        {
            monsters.push_back(*monster);
        }
    }
    return monsters;
}

// The sets of the set table, sorted by name.
std::vector<MonsterSoundSet> CollectSoundSets()
{
    std::vector<MonsterSoundSet> sets;
    for (const MonsterSoundSet* set : g_MonsterSoundSetDatabase.GetAll())
    {
        sets.push_back(*set);
    }
    return sets;
}

// The entries of the model table, sorted by slot.
std::vector<MonsterModelDefinition> CollectModels()
{
    std::vector<MonsterModelDefinition> models;
    for (const MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        models.push_back(*model);
    }
    return models;
}

// The entries by slot.
std::map<int, MonsterModelDefinition> BySlot(std::span<const MonsterModelDefinition> models)
{
    std::map<int, MonsterModelDefinition> bySlot;
    for (const MonsterModelDefinition& model : models)
    {
        if (const std::optional<int> slot = FindMonsterModelSlot(model.slot))
        {
            bySlot.emplace(*slot, model);
        }
    }
    return bySlot;
}
#endif

// The languages check, on top of the checks of the file's reader: names in
// languages the game has no texts for, as problems of the monster list file.
void ValidateLocales(std::span<const MonsterDefinition> monsters, std::vector<Items::ItemDataIssue>& issues)
{
    const std::size_t first = issues.size();
    ValidateMonsterNameLocales(monsters, I18N::GetAvailableLocales(), issues);
    NameIssueSource(issues, first, GetMonstersFile());
}
} // namespace

MonsterDataHandler& MonsterDataHandler::GetInstance()
{
    static MonsterDataHandler instance;
    return instance;
}

bool MonsterDataHandler::Load(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    MonsterDataLoadResult result = LoadMonstersFile(GetMonstersFile());
    ValidateLocales(result.monsters, result.issues);
    LogDataIssues(MonsterDataLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(MonsterDataName, GetMonstersFile(), result.issues);
        return false;
    }

    g_MonsterDatabase.Build(result.monsters, I18N::GetCurrentLocale());
    g_MonsterDatabase.SetUnknownLook(result.unknown);
#ifdef _EDITOR
    SetGroups(result.groups);
#endif
    RegisterLocaleObserver();
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} monsters from {} in {:.1f} ms", result.monsters.size(),
                GetMonstersFile().generic_string(), elapsed.count());
    return true;
}

bool MonsterDataHandler::LoadModels(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    const MonsterModelsLoadResult result = LoadMonsterModelsFile(GetMonsterModelsFile());
    LogDataIssues(MonsterModelsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(MonsterModelsName, GetMonsterModelsFile(), result.issues);
        return false;
    }

    g_MonsterModelDatabase.Build(result.models);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} monster models from {} in {:.1f} ms", result.models.size(),
                GetMonsterModelsFile().generic_string(), elapsed.count());
    return true;
}

bool MonsterDataHandler::LoadSounds(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    MonsterSoundsLoadResult result = LoadMonsterSoundsFile(GetMonsterSoundsFile());
    if (!Items::HasFatal(result.issues))
    {
        ValidateLinks(result.sets, result.issues);
    }
    LogDataIssues(MonsterSoundsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(MonsterSoundsName, GetMonsterSoundsFile(), result.issues);
        return false;
    }

    g_MonsterSoundSetDatabase.Build(result.sets);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} monster sound sets from {} in {:.1f} ms", result.sets.size(),
                GetMonsterSoundsFile().generic_string(), elapsed.count());
    return true;
}

void MonsterDataHandler::RegisterLocaleObserver()
{
    if (m_localeObserverRegistered)
    {
        return;
    }
    I18N::RegisterLocaleObserver(&MonsterDataHandler::OnLocaleChanged, this);
    m_localeObserverRegistered = true;
}

void MonsterDataHandler::OnLocaleChanged(void* context) noexcept
{
    auto* handler = static_cast<MonsterDataHandler*>(context);
    g_MonsterDatabase.SetDisplayLocale(I18N::GetCurrentLocale());
    if (handler->m_namesChanged != nullptr)
    {
        handler->m_namesChanged(AllNumbers);
    }
}

#ifdef _EDITOR
std::optional<MonsterDefinition> MonsterDataHandler::ReadMonster(int number) const
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    return monster != nullptr ? std::optional<MonsterDefinition>(*monster) : std::nullopt;
}

void MonsterDataHandler::RestoreMonsters(std::span<const MonsterSnapshot> snapshots)
{
    // The callback is told once: the number restored, or AllNumbers when more
    // were. The game walks every character for each call.
    std::optional<int> changed;
    for (const MonsterSnapshot& snapshot : snapshots)
    {
        if (snapshot.number < 0 || snapshot.number >= MonsterNumberCount)
        {
            continue;
        }
        if (snapshot.monster)
        {
            MonsterDefinition monster = *snapshot.monster;
            monster.number = snapshot.number;
            g_MonsterDatabase.Set(monster);
        }
        else
        {
            g_MonsterDatabase.Remove(snapshot.number);
        }
        const bool sameNumber = !changed || *changed == snapshot.number;
        changed = sameNumber ? snapshot.number : AllNumbers;
    }
    if (changed)
    {
        NotifyNamesChanged(*changed);
        if (m_monstersChanged != nullptr)
        {
            m_monstersChanged(*changed);
        }
        // The monster may name another sound set.
        m_soundsToLoad = true;
    }
}

std::optional<MonsterSoundSet> MonsterDataHandler::ReadSoundSet(const std::string& name) const
{
    const MonsterSoundSet* set = g_MonsterSoundSetDatabase.Find(name);
    return set != nullptr ? std::optional<MonsterSoundSet>(*set) : std::nullopt;
}

void MonsterDataHandler::RestoreSoundSets(std::span<const MonsterSoundSetSnapshot> snapshots)
{
    for (const MonsterSoundSetSnapshot& snapshot : snapshots)
    {
        if (snapshot.set)
        {
            MonsterSoundSet set = *snapshot.set;
            set.name = snapshot.name;
            g_MonsterSoundSetDatabase.Set(set);
        }
        else
        {
            g_MonsterSoundSetDatabase.Remove(snapshot.name);
        }
    }
    m_soundsToLoad = true;
}

void MonsterDataHandler::ValidateSounds(std::vector<Items::ItemDataIssue>& issues) const
{
    const std::vector<MonsterSoundSet> sets = CollectSoundSets();
    ValidateMonsterSoundSetNames(sets, GetMonsterSoundsFile().generic_string(), issues);
    ValidateLinks(sets, issues);
}

void MonsterDataHandler::Validate(std::vector<Items::ItemDataIssue>& issues) const
{
    const std::vector<MonsterDefinition> monsters = CollectMonsters();
    const std::size_t first = issues.size();
    ValidateMonsterNames(monsters, issues);
    NameIssueSource(issues, first, GetMonstersFile());
    ValidateLocales(monsters, issues);
}

void MonsterDataHandler::ValidateModels(std::vector<Items::ItemDataIssue>& issues) const
{
    const std::vector<MonsterModelDefinition> models = CollectModels();
    const std::string source = GetMonsterModelsFile().generic_string();
    ValidateMonsterModelNames(models, source, issues);
    ValidateMonsterModelFiles(models, source, issues);
}

bool MonsterDataHandler::Save(std::vector<Items::ItemDataIssue>& issues)
{
    ValidateLocales(CollectMonsters(), issues);
    std::vector<MonsterGroup> groups = CollectGroups();
    return SaveMonstersFile(GetMonstersFile(), groups, g_MonsterDatabase.GetUnknownLook(), issues);
}

bool MonsterDataHandler::Revert(std::vector<Items::ItemDataIssue>& issues)
{
    MonsterDataLoadResult result = LoadMonstersFile(GetMonstersFile());
    ValidateLocales(result.monsters, result.issues);
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasFatal(result.issues))
    {
        return false;
    }
    g_MonsterDatabase.SetUnknownLook(result.unknown);
    UseMonsters(result.groups);
    return true;
}

void MonsterDataHandler::UseMonsters(std::span<const MonsterGroup> monsters)
{
    // The data changes, not the language the names are shown in.
    const std::string displayLocale = g_MonsterDatabase.GetDisplayLocale();
    g_MonsterDatabase.Build(CollectNumberEntries(monsters), displayLocale);
    SetGroups(monsters);
    RegisterLocaleObserver();
    NotifyNamesChanged(AllNumbers);
    if (m_monstersChanged != nullptr)
    {
        m_monstersChanged(AllNumbers);
    }
}

std::vector<MonsterGroup> MonsterDataHandler::CollectGroups() const
{
    std::vector<MonsterGroup> groups;
    std::bitset<MonsterNumberCount> grouped;
    if (HasCurrentGroups())
    {
        for (const auto& [id, layout] : m_groups)
        {
            MonsterGroup group{layout.shared, {}};
            for (const int number : layout.numbers)
            {
                const MonsterDefinition* now = g_MonsterDatabase.Find(number);
                if (now != nullptr && !grouped[static_cast<std::size_t>(number)])
                {
                    group.numbers.push_back(*now);
                    grouped.set(static_cast<std::size_t>(number));
                }
            }
            if (!group.numbers.empty())
            {
                groups.push_back(std::move(group));
            }
        }
    }
    for (int number = 0; number < MonsterNumberCount; ++number)
    {
        const MonsterDefinition* now = g_MonsterDatabase.Find(number);
        if (now != nullptr && !grouped[static_cast<std::size_t>(number)])
        {
            groups.push_back({*now, {*now}});
        }
    }
    return groups;
}

void MonsterDataHandler::SetGroups(std::span<const MonsterGroup> monsters)
{
    m_groups.clear();
    for (const MonsterGroup& monster : monsters)
    {
        // A monster with one number is a number of no monster here.
        if (monster.numbers.size() < 2)
        {
            continue;
        }
        MonsterGroupLayout layout{MakeSharedValues(monster.shared), {}};
        for (const MonsterDefinition& entry : monster.numbers)
        {
            layout.numbers.push_back(entry.number);
        }
        std::ranges::sort(layout.numbers);
        m_groups.emplace(m_nextGroupId++, std::move(layout));
    }
    m_groupsBuild = g_MonsterDatabase.GetBuildCount();
    IndexGroups();
}

bool MonsterDataHandler::HasCurrentGroups() const
{
    return m_groupsBuild == g_MonsterDatabase.GetBuildCount();
}

void MonsterDataHandler::IndexGroups()
{
    m_groupOfNumber.fill(-1);
    for (const auto& [id, layout] : m_groups)
    {
        for (const int number : layout.numbers)
        {
            if (number >= 0 && number < MonsterNumberCount)
            {
                m_groupOfNumber[static_cast<std::size_t>(number)] = id;
            }
        }
    }
    ++m_groupsVersion;
}

std::optional<int> MonsterDataHandler::FindGroupOf(int number) const
{
    if (number < 0 || number >= MonsterNumberCount || !HasCurrentGroups())
    {
        return std::nullopt;
    }
    const int id = m_groupOfNumber[static_cast<std::size_t>(number)];
    return id >= 0 ? std::optional<int>(id) : std::nullopt;
}

std::optional<MonsterGroupLayout> MonsterDataHandler::ReadGroup(int id) const
{
    const MonsterGroupLayout* group = FindGroup(id);
    return group != nullptr ? std::optional<MonsterGroupLayout>(*group) : std::nullopt;
}

const MonsterGroupLayout* MonsterDataHandler::FindGroup(int id) const
{
    if (!HasCurrentGroups())
    {
        return nullptr;
    }
    const auto found = m_groups.find(id);
    return found != m_groups.end() ? &found->second : nullptr;
}

void MonsterDataHandler::RestoreGroups(std::span<const MonsterGroupSnapshot> snapshots)
{
    // The monsters of a table built elsewhere are gone: its numbers are
    // monsters of their own until a change lists them.
    if (!HasCurrentGroups())
    {
        m_groups.clear();
        m_groupsBuild = g_MonsterDatabase.GetBuildCount();
    }
    for (const MonsterGroupSnapshot& snapshot : snapshots)
    {
        if (snapshot.group)
        {
            MonsterGroupLayout layout = *snapshot.group;
            std::ranges::sort(layout.numbers);
            m_groups.insert_or_assign(snapshot.id, std::move(layout));
            m_nextGroupId = std::max(m_nextGroupId, snapshot.id + 1);
        }
        else
        {
            m_groups.erase(snapshot.id);
        }
    }
    IndexGroups();
}

int MonsterDataHandler::MakeGroupId()
{
    return m_nextGroupId++;
}

std::optional<MonsterModelDefinition> MonsterDataHandler::ReadModel(int slot) const
{
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
    return model != nullptr ? std::optional<MonsterModelDefinition>(*model) : std::nullopt;
}

void MonsterDataHandler::RestoreModels(std::span<const MonsterModelSnapshot> snapshots)
{
    for (const MonsterModelSnapshot& snapshot : snapshots)
    {
        const std::string_view symbol = GetMonsterModelSymbol(snapshot.slot);
        if (symbol.empty())
        {
            continue;
        }
        if (snapshot.model)
        {
            MonsterModelDefinition model = *snapshot.model;
            model.slot = std::string(symbol);
            g_MonsterModelDatabase.Set(model);
        }
        else
        {
            g_MonsterModelDatabase.Remove(snapshot.slot);
        }
        m_modelsToReopen.push_back(snapshot.slot);
    }
}

bool MonsterDataHandler::SaveModels(std::vector<Items::ItemDataIssue>& issues)
{
    return SaveMonsterModelsFile(GetMonsterModelsFile(), CollectModels(), issues);
}

bool MonsterDataHandler::RevertModels(std::vector<Items::ItemDataIssue>& issues)
{
    const MonsterModelsLoadResult result = LoadMonsterModelsFile(GetMonsterModelsFile());
    issues.insert(issues.end(), result.issues.begin(), result.issues.end());
    if (Items::HasFatal(result.issues))
    {
        return false;
    }
    UseModels(result.models);
    return true;
}

bool MonsterDataHandler::SaveAll(std::vector<Items::ItemDataIssue>& issues)
{
    ValidateLocales(CollectMonsters(), issues);
    std::vector<MonsterGroup> groups = CollectGroups();
    // All are checked, so that the problems of all are listed.
    const std::optional<std::string> monstersText =
        MakeMonstersFileText(GetMonstersFile(), groups, g_MonsterDatabase.GetUnknownLook(), issues);
    const std::optional<std::string> modelsText =
        MakeMonsterModelsFileText(GetMonsterModelsFile(), CollectModels(), issues);
    const std::vector<MonsterSoundSet> sets = CollectSoundSets();
    const std::optional<std::string> soundsText = MakeMonsterSoundsFileText(GetMonsterSoundsFile(), sets, issues);
    ValidateLinks(sets, issues);
    if (!monstersText || !modelsText || !soundsText)
    {
        return false;
    }
    // All files or none: the new monster list next to the old model list or
    // sound sets may name a model or a set they lack.
    const std::array<TextFileToWrite, 3> files{{{GetMonstersFile(), *monstersText},
                                                {GetMonsterModelsFile(), *modelsText},
                                                {GetMonsterSoundsFile(), *soundsText}}};
    std::filesystem::path failed;
    std::string error;
    if (!WriteTextFiles(files, failed, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, failed.generic_string(), Items::ItemDataIssue::NoItem,
                          Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    ++m_filesVersion;
    return true;
}

bool MonsterDataHandler::SaveSoundSets(const std::function<bool(std::vector<MonsterSoundSet>& sets)>& change,
                                       std::vector<Items::ItemDataIssue>& issues)
{
    MonsterSoundsLoadResult file = LoadMonsterSoundsFile(GetMonsterSoundsFile());
    if (Items::HasFatal(file.issues))
    {
        issues.insert(issues.end(), file.issues.begin(), file.issues.end());
        return false;
    }
    if (!change(file.sets))
    {
        return false;
    }
    const std::optional<std::string> text = MakeMonsterSoundsFileText(GetMonsterSoundsFile(), file.sets, issues);
    if (!text)
    {
        return false;
    }
    std::string error;
    if (!WriteTextFile(GetMonsterSoundsFile(), *text, error))
    {
        issues.push_back({Items::ItemDataIssueSeverity::Fatal, GetMonsterSoundsFile().generic_string(),
                          Items::ItemDataIssue::NoItem, Items::ItemDataIssue::NoItem, "", error});
        return false;
    }
    return true;
}

std::optional<std::string>
MonsterDataHandler::MakeMonstersFileWith(const std::function<bool(std::vector<MonsterGroup>& groups)>& change,
                                         std::vector<Items::ItemDataIssue>& issues) const
{
    MonsterDataLoadResult file = LoadMonstersFile(GetMonstersFile());
    if (Items::HasFatal(file.issues))
    {
        issues.insert(issues.end(), file.issues.begin(), file.issues.end());
        return std::nullopt;
    }
    if (!change(file.groups))
    {
        return std::nullopt;
    }
    return MakeMonstersFileText(GetMonstersFile(), file.groups, file.unknown, issues);
}

bool MonsterDataHandler::RevertAll(std::vector<Items::ItemDataIssue>& issues)
{
    MonsterDataLoadResult monsters = LoadMonstersFile(GetMonstersFile());
    ValidateLocales(monsters.monsters, monsters.issues);
    const MonsterModelsLoadResult models = LoadMonsterModelsFile(GetMonsterModelsFile());
    const MonsterSoundsLoadResult sounds = LoadMonsterSoundsFile(GetMonsterSoundsFile());
    issues.insert(issues.end(), monsters.issues.begin(), monsters.issues.end());
    issues.insert(issues.end(), models.issues.begin(), models.issues.end());
    issues.insert(issues.end(), sounds.issues.begin(), sounds.issues.end());
    if (Items::HasFatal(monsters.issues) || Items::HasFatal(models.issues) || Items::HasFatal(sounds.issues))
    {
        return false;
    }
    g_MonsterDatabase.SetUnknownLook(monsters.unknown);
    UseMonsters(monsters.groups);
    UseModels(models.models);
    UseSoundSets(sounds.sets);
    ++m_filesVersion;
    return true;
}

void MonsterDataHandler::UseSoundSets(std::span<const MonsterSoundSet> sets)
{
    g_MonsterSoundSetDatabase.Build(sets);
    m_soundsToLoad = true;
}

void MonsterDataHandler::UseModels(std::span<const MonsterModelDefinition> models)
{
    // The slots whose entry the file changes, also those it removes.
    const std::map<int, MonsterModelDefinition> before = BySlot(CollectModels());
    const std::map<int, MonsterModelDefinition> after = BySlot(models);
    for (const auto& [slot, model] : before)
    {
        const auto read = after.find(slot);
        if (read == after.end() || !(read->second == model))
        {
            m_modelsToReopen.push_back(slot);
        }
    }
    for (const auto& [slot, model] : after)
    {
        if (!before.contains(slot))
        {
            m_modelsToReopen.push_back(slot);
        }
    }
    g_MonsterModelDatabase.Build(models);
}

void MonsterDataHandler::ReopenChangedModels()
{
    std::vector<int> slots;
    slots.swap(m_modelsToReopen);
    std::ranges::sort(slots);
    slots.erase(std::unique(slots.begin(), slots.end()), slots.end());
    std::vector<int> reopened;
    for (const int slot : slots)
    {
        const std::vector<int> opened = ModelLoader::Reopen(slot);
        reopened.insert(reopened.end(), opened.begin(), opened.end());
    }
    if (!reopened.empty() && m_modelsReopened != nullptr)
    {
        m_modelsReopened(reopened);
    }

    if (m_soundsToLoad)
    {
        m_soundsToLoad = false;
        ModelLoader::LoadSoundsOfOpenModels();
    }

    std::vector<int> toOpen;
    toOpen.swap(m_modelsToOpen);
    for (const int slot : toOpen)
    {
        if (!ModelLoader::IsOpen(slot))
        {
            ModelLoader::Open(slot);
        }
    }
}

void MonsterDataHandler::OpenModelForTools(int slot)
{
    m_modelsToOpen.push_back(slot);
}

void MonsterDataHandler::NotifyNamesChanged(int number) const
{
    if (m_namesChanged != nullptr)
    {
        m_namesChanged(number);
    }
}
#endif
} // namespace Data::Monsters
