#include "stdafx.h"

#include "MonsterModelValidation.h"

#include "Core/Platform/PathResolve.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterModelsJson.h"

#include <algorithm>
#include <filesystem>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

class ModelProblems
{
public:
    ModelProblems(const std::string& source, const std::string& slot, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_slot(slot), m_issues(issues)
    {
    }

    void Add(ItemDataIssueSeverity severity, const std::string& key, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                            MonsterModelField(m_slot, key), message});
    }

private:
    const std::string& m_source;
    const std::string& m_slot;
    std::vector<ItemDataIssue>& m_issues;
};

void ValidateParts(const MonsterModelDefinition& model, ModelProblems& problems)
{
    for (size_t i = 0; i < model.parts.size(); ++i)
    {
        const MonsterModelPart& part = model.parts[i];
        const std::string key = "parts[" + std::to_string(i) + "]";
        if (!part.slot.empty())
        {
            const std::optional<int> slot = FindMonsterModelSlot(part.slot);
            if (!slot || !IsNpcRangeSlot(*slot))
            {
                problems.Add(ItemDataIssueSeverity::Fatal, key + ".slot", "\"" + part.slot + "\" is not an NPC slot");
            }
        }
        else if (!g_EffectTypeCatalogue.FindType(Effects::EffectKind::Effect, part.effect))
        {
            problems.Add(ItemDataIssueSeverity::Fatal, key + ".effect",
                         "\"" + part.effect + "\" is not an effect type of Data/Effects/EffectTypes.json");
        }
    }
}

} // namespace

bool IsInClientFolder(const std::string& path, bool folder)
{
#ifdef _WIN32
    const std::filesystem::path resolved = std::filesystem::u8path(path);
#else
    // The data spells files as they are on disk, but checks the way the game finds them.
    const std::filesystem::path resolved = MuResolvePath(path.c_str());
#endif
    std::error_code ignored;
    return folder ? std::filesystem::is_directory(resolved, ignored)
                  : std::filesystem::is_regular_file(resolved, ignored);
}

void ValidateMonsterModelNames(std::span<const MonsterModelDefinition> models, const std::string& source,
                               std::vector<ItemDataIssue>& issues)
{
    for (const MonsterModelDefinition& model : models)
    {
        ModelProblems problems(source, model.slot, issues);
        for (size_t i = 0; i < model.bitmaps.size(); ++i)
        {
            if (!FindMonsterModelBitmapSlot(model.bitmaps[i].slot))
            {
                problems.Add(ItemDataIssueSeverity::Fatal, "bitmaps[" + std::to_string(i) + "].slot",
                             "\"" + model.bitmaps[i].slot + "\" is not a bitmap slot of monster models");
            }
        }
        ValidateParts(model, problems);
    }
}

void ValidateMonsterModelFiles(std::span<const MonsterModelDefinition> models, const std::string& source,
                               std::vector<ItemDataIssue>& issues)
{
    for (const MonsterModelDefinition& model : models)
    {
        ModelProblems problems(source, model.slot, issues);
        if (!IsInClientFolder(model.load.file))
        {
            problems.Add(ItemDataIssueSeverity::Warning, "file",
                         model.load.file + " is not in the client folder; the model has no body");
        }
        for (size_t i = 0; i < model.parts.size(); ++i)
        {
            if (!IsInClientFolder(model.parts[i].load.file))
            {
                problems.Add(ItemDataIssueSeverity::Warning, "parts[" + std::to_string(i) + "].file",
                             model.parts[i].load.file + " is not in the client folder");
            }
        }
    }
}
} // namespace Data::Monsters
