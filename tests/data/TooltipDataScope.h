#pragma once

#include "Data/DataHandler/BuffData/BuffStorage.h"
#include "Data/DataHandler/SkillData/MasterSkillStorage.h"
#include "Data/GameData/BuffData/BuffLineTexts.h"
#include "Data/GameData/SkillData/MasterSkillDatabase.h"
#include "Data/GameData/SkillData/MasterSkillsJson.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "doctest.h"
#include "Data/DataHandler/SkillData/SkillDataHandler.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/Buffs/w_BuffStateSystem.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "Network/Server/SocketSystem.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/UIJewelHarmony.h"

#include <string>
#include <vector>

// The game data a tooltip reads besides the items, loaded as the game loads it (ZzzOpenData.cpp, CUIManager): the
// skills (the orbs' and scrolls' names and damage), the socket items, the item sets, the monster list (the
// transformation rings), the Jewel of Harmony and item add
// option tables, the buffs (the values and durations of the items that give one) and the buff system's time texts. Run
// in a ClientDataScope, which makes the paths relative to the client. It puts back what it replaces (the language, the
// three tables, the buff system and the skill table it made),
// but the item sets, the socket items and the monster list stay loaded for the tests after it: the game has no
// way to unload them, and every one of these loads replaces what was there.
class TooltipDataScope
{
public:
    TooltipDataScope() : m_previousLanguage(g_strSelectedML)
    {
        g_strSelectedML = L"Eng";
        if (SkillAttribute == nullptr)
        {
            m_skills = new SKILL_ATTRIBUTE[MAX_SKILLS]{};
            SkillAttribute = m_skills;
        }
        wchar_t path[MAX_PATH];
        mu_swprintf(path, L"Data\\Local\\%ls\\Skill_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
        g_SkillDataHandler.Load(path);
        mu_swprintf(path, L"Data\\Local\\%ls\\SocketItem_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
        g_SocketItemMgr.OpenSocketItemScript(path);
        g_csItemOption.OpenItemSetScript();
        // The transformation rings' names are monster names (Data/Monsters/Monsters.json).
        std::string monsterError;
        g_MonsterDataHandler.Load(monsterError);
        m_previousHarmony = g_pUIJewelHarmonyinfo;
        m_previousAddOptions = g_pItemAddOptioninfo;
        g_pUIJewelHarmonyinfo = JewelHarmonyInfo::MakeInfo();
        g_pItemAddOptioninfo = ItemAddOptioninfo::MakeInfo();
        m_previousBuffs = g_BuffDatabase.GetAll();
        m_previousBuffLocale = g_BuffDatabase.GetDisplayLocale();
        const Data::Buffs::BuffsLoadResult buffs = Data::Buffs::LoadBuffsFile(Data::Buffs::GetBuffsFile());
        // A broken Buffs.json shows here, not as wrong values in the tests after it.
        for (const Data::Items::ItemDataIssue& issue : buffs.issues)
        {
            CHECK_MESSAGE(issue.severity == Data::Items::ItemDataIssueSeverity::Warning, issue.ToString());
        }
        g_BuffDatabase.Build(buffs.buffs, "en");
        // The master skills' buff parts and the skill tooltip's buff texts (buffs design B9, B18).
        m_previousMasterSkills = g_MasterSkillDatabase.GetAll();
        Data::Skills::MasterSkillsLoadResult masters =
            Data::Skills::LoadMasterSkillsFile(Data::Skills::GetMasterSkillsFile());
        Data::Skills::ValidateMasterSkillBuffs(
            masters.skills, {}, buffs.buffs, [&](int skill)
            { return Data::Skills::FindMasterSkillBuff(skill, masters.skills); }, "MasterSkills.json", masters.issues);
        Data::Buffs::BuffTextsLoadResult texts = Data::Buffs::LoadBuffTextsFile(Data::Buffs::GetBuffTextsFile());
        for (const auto* issues : {&masters.issues, &texts.issues})
        {
            for (const Data::Items::ItemDataIssue& issue : *issues)
            {
                CHECK_MESSAGE(issue.severity == Data::Items::ItemDataIssueSeverity::Warning, issue.ToString());
            }
        }
        g_MasterSkillDatabase.Build(masters.skills);
        m_previousBuffTexts = g_BuffLineTexts.GetTexts();
        m_previousBuffTextLocale = g_BuffLineTexts.GetDisplayLocale();
        g_BuffLineTexts.SetTexts(std::move(texts.texts), "en");
        // The time texts of the elixirs and seals (g_StringTime).
        if (!g_BuffSystem)
        {
            g_BuffSystem = BuffStateSystem::Make();
            m_madeBuffSystem = true;
        }
    }

    ~TooltipDataScope()
    {
        if (m_madeBuffSystem)
        {
            g_BuffSystem.reset();
        }
        delete g_pUIJewelHarmonyinfo;
        delete g_pItemAddOptioninfo;
        g_pUIJewelHarmonyinfo = m_previousHarmony;
        g_pItemAddOptioninfo = m_previousAddOptions;
        g_BuffDatabase.Build(m_previousBuffs, m_previousBuffLocale);
        g_MasterSkillDatabase.Build(m_previousMasterSkills);
        g_BuffLineTexts.SetTexts(std::move(m_previousBuffTexts), m_previousBuffTextLocale);
        if (m_skills != nullptr)
        {
            SkillAttribute = nullptr;
            delete[] m_skills;
        }
        g_strSelectedML = m_previousLanguage;
    }

private:
    std::wstring m_previousLanguage;
    SKILL_ATTRIBUTE* m_skills = nullptr;
    JewelHarmonyInfo* m_previousHarmony = nullptr;
    ItemAddOptioninfo* m_previousAddOptions = nullptr;
    std::vector<Data::Buffs::BuffDefinition> m_previousBuffs;
    std::string m_previousBuffLocale;
    std::vector<Data::Skills::MasterSkillDefinition> m_previousMasterSkills;
    Data::Items::ItemOptionTexts m_previousBuffTexts;
    std::string m_previousBuffTextLocale;
    bool m_madeBuffSystem = false;
};
