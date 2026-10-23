#include "stdafx.h"

#include "MonsterDataHandler.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/MonsterData/MonsterJsonStorage.h"
#include "Data/GameData/MonsterData/MonsterDataValidation.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"

#include <chrono>

namespace Data::Monsters
{
namespace
{
constexpr std::string_view MonsterDataLabel = "Monster data";
constexpr std::string_view MonsterDataName = "monster data";
} // namespace

MonsterDataHandler& MonsterDataHandler::GetInstance()
{
    static MonsterDataHandler instance;
    return instance;
}

bool MonsterDataHandler::Load(std::string_view nameLocale, std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    MonsterDataLoadResult result = LoadMonstersFile(GetMonstersFile());
    ValidateMonsterNameLocales(result.monsters, I18N::GetAvailableLocales(), result.issues);
    LogDataIssues(MonsterDataLabel, result.issues);
    if (Items::HasErrors(result.issues))
    {
        errorMessage = DescribeDataErrors(MonsterDataName, GetMonstersFile(), result.issues);
        return false;
    }

    g_MonsterDatabase.Build(result.monsters, nameLocale);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} monsters from {} in {:.1f} ms", result.monsters.size(),
                GetMonstersFile().generic_string(), elapsed.count());
    return true;
}
} // namespace Data::Monsters
