#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Core/Platform/ProcessLocale.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/SkillData/SkillDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ItemTooltipSnapshot.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "Network/Server/SocketSystem.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/UIJewelHarmony.h"
#include "UI/Inventory/ItemMng.h"

#include <algorithm>
#include <iterator>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// Points the tooltip at a test-owned hero, as ItemHelpScope does for the item help window.
class TooltipHeroScope
{
public:
    TooltipHeroScope() : m_previousHero(CharacterAttribute), m_previousHeroCharacter(::Hero)
    {
        CharacterAttribute = &m_hero;
        ::Hero = &m_heroCharacter;
    }

    ~TooltipHeroScope()
    {
        CharacterAttribute = m_previousHero;
        ::Hero = m_previousHeroCharacter;
    }

    CHARACTER_ATTRIBUTE& Hero()
    {
        return m_hero;
    }

private:
    CHARACTER_ATTRIBUTE m_hero{};
    CHARACTER m_heroCharacter;
    CHARACTER_ATTRIBUTE* m_previousHero;
    CHARACTER* m_previousHeroCharacter;
};

// The game data a tooltip reads besides the items, loaded as the game loads it (ZzzOpenData.cpp, CUIManager): the
// skills (the orbs' and scrolls' names and damage), the socket items, the item sets, the monster names (the
// transformation rings), the Jewel of Harmony and item add
// option tables, and the buff system's time texts. Run in a ClientDataScope, which makes the paths relative to the
// client.
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
        // The transformation rings' names are monster names.
        mu_swprintf(path, L"Data\\Local\\%ls\\NpcName_%ls.txt", g_strSelectedML.c_str(), g_strSelectedML.c_str());
        OpenMonsterScript(path);
        m_previousHarmony = g_pUIJewelHarmonyinfo;
        m_previousAddOptions = g_pItemAddOptioninfo;
        g_pUIJewelHarmonyinfo = JewelHarmonyInfo::MakeInfo();
        g_pItemAddOptioninfo = ItemAddOptioninfo::MakeInfo();
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
    bool m_madeBuffSystem = false;
};
} // namespace

namespace
{
// A fixed width per character instead of the game's font, so the descriptions wrap the same on every platform.
int FixedWidthMeasure(const wchar_t*, size_t length)
{
    return static_cast<int>(length) * 4;
}

// The heroes the snapshot is recorded for: a new Dark Wizard, who lacks every requirement, and a level 400 Blade
// Master (a third class), who meets most of them.
void MakeNewDarkWizard(CHARACTER_ATTRIBUTE& hero)
{
    hero.Class = CLASS_WIZARD;
    hero.Level = 1;
    hero.Strength = 18;
    hero.Dexterity = 18;
    hero.Vitality = 15;
    hero.Energy = 30;
    // The seed spheres' life and mana per monster kill are shares of these.
    hero.LifeMax = 61;
    hero.ManaMax = 62;
}

void MakeBladeMaster(CHARACTER_ATTRIBUTE& hero)
{
    hero.Class = CLASS_BLADEMASTER;
    hero.Level = 400;
    hero.Strength = 2000;
    hero.Dexterity = 1000;
    hero.Vitality = 800;
    hero.Energy = 300;
    hero.LifeMax = 6000;
    hero.ManaMax = 1500;
}

// FNV-1a: a hash that is the same on every platform.
uint64_t StableHash(const std::string& text)
{
    uint64_t hash = 14695981039346656037ull;
    for (const unsigned char character : text)
    {
        hash = (hash ^ character) * 1099511628211ull;
    }
    return hash;
}

// "<hero> <group>/<number> <hash>" for each item of a snapshot: the hash of all its tooltips.
std::map<std::string, std::string> HashesByItem(const std::string& hero, const std::string& snapshot,
                                                std::map<std::string, std::string>& tooltips)
{
    std::map<std::string, std::string> hashes;
    std::istringstream lines(snapshot);
    std::string line;
    std::string item;
    while (std::getline(lines, line))
    {
        if (!line.starts_with("  "))
        {
            item = hero + " " + line.substr(0, line.find(' '));
        }
        tooltips[item] += line + "\n";
    }
    for (const auto& [key, text] : tooltips)
    {
        char hash[17];
        std::snprintf(hash, sizeof(hash), "%016llx", static_cast<unsigned long long>(StableHash(text)));
        hashes[key] = hash;
    }
    return hashes;
}

// The game's process locale for the scope: macOS writes wide text (mu_swprintf) through LC_CTYPE, and in the test's
// "C" locale it leaves any text with a character above U+00FF unwritten, such as an ancient name with the English set
// names' typographic apostrophe (U+2019).
class ProcessLocaleScope
{
public:
    ProcessLocaleScope() : m_previous(std::setlocale(LC_ALL, nullptr))
    {
        Core::Platform::ApplyProcessLocale();
    }
    ~ProcessLocaleScope()
    {
        std::setlocale(LC_ALL, m_previous.c_str());
    }
    ProcessLocaleScope(const ProcessLocaleScope&) = delete;
    ProcessLocaleScope& operator=(const ProcessLocaleScope&) = delete;

private:
    std::string m_previous;
};
} // namespace

// Every item's tooltip at every level 0 to 15, as plain, full, ancient and harmony items (WriteItemTooltipSnapshot),
// for the two heroes, against the hashes recorded in item_tooltip_snapshot.txt: a tooltip that changes shows here.
// After a change that is meant, run the test with MU_TOOLTIP_SNAPSHOT_UPDATE=1 to record the new hashes; the control
// socket's `tooltip-snapshot` writes the full lines in the game to compare with.
TEST_CASE("Every item's tooltip is as recorded [data][items][tooltip]")
{
    ProcessLocaleScope locale;
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    std::map<std::string, std::string> hashes;
    std::map<std::string, std::string> tooltips;
    const std::pair<const char*, void (*)(CHARACTER_ATTRIBUTE&)> heroes[] = {{"newDarkWizard", &MakeNewDarkWizard},
                                                                             {"bladeMaster", &MakeBladeMaster}};
    for (const auto& [name, make] : heroes)
    {
        TooltipHeroScope hero;
        make(hero.Hero());
        ::Hero->Class = hero.Hero().Class;
        ::Hero->Level = hero.Hero().Level;
        std::ostringstream snapshot;
        CHECK(WriteItemTooltipSnapshot(snapshot) == 911 * 4 * 16);
        const std::map<std::string, std::string> heroHashes = HashesByItem(name, snapshot.str(), tooltips);
        hashes.insert(heroHashes.begin(), heroHashes.end());
    }
    SetItemTooltipTextMeasure(nullptr);

    const std::filesystem::path recordedFile = MU_TEST_TOOLTIP_SNAPSHOT;
    if (std::getenv("MU_TOOLTIP_SNAPSHOT_UPDATE") != nullptr)
    {
        std::ofstream out(recordedFile, std::ios::binary);
        for (const auto& [item, hash] : hashes)
        {
            out << item << ' ' << hash << '\n';
        }
        MESSAGE("recorded " << hashes.size() << " items in " << recordedFile.string());
        return;
    }

    std::map<std::string, std::string> recorded;
    std::ifstream in(recordedFile);
    REQUIRE(in);
    std::string hero;
    std::string item;
    std::string hash;
    while (in >> hero >> item >> hash)
    {
        recorded[hero + " " + item] = hash;
    }
    int changed = 0;
    for (const auto& [key, current] : hashes)
    {
        const auto found = recorded.find(key);
        if (found == recorded.end() || found->second != current)
        {
            if (++changed <= 5)
            {
                INFO(key << " now:\n" << tooltips[key]);
                CHECK(false);
            }
        }
    }
    CHECK(changed == 0);
    CHECK(recorded.size() == hashes.size());
}

TEST_CASE("Every built-in placeholder but {level} is a value of the tooltip's stat lines [data][items][tooltip]")
{
    const ItemTooltipValues values;
    for (const std::string_view name : Data::Items::GetBuiltInPlaceholders())
    {
        INFO(std::string(name));
        CHECK(GetItemTooltipValue(values, name).has_value() == (name != Data::Items::LevelPlaceholder));
    }
    CHECK_FALSE(GetItemTooltipValue(values, "sparkle").has_value());
}

TEST_CASE("The tooltip's values of an item follow the item and the hero [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeNewDarkWizard(hero.Hero());

    ItemCreationParams params = FullItemParameters(0, 0);
    params.Level = 7;
    ITEM kris;
    FillItemFromParameters(kris, params);
    const ItemTooltipValues values = ComputeItemTooltipValues(kris);
    CHECK(values.damageMin == kris.DamageMin);
    CHECK(values.damageMax == kris.DamageMax);
    CHECK(values.hasDamage);
    CHECK_FALSE(values.damageHarmonyBonus);
    CHECK(values.maxDurability > 0);
    CHECK(values.strength.required == kris.RequireStrength);
    CHECK(values.strength.missing == kris.RequireStrength - 18);
    CHECK_FALSE(values.strength.reduced);
    CHECK(GetItemTooltipValue(values, "requiredStrength") == values.strength.required);
    CHECK(GetItemTooltipValue(values, "missingStrength") == values.strength.missing);
    CHECK(GetItemTooltipValue(values, "maxDurability") == values.maxDurability);
    CHECK_FALSE(values.excellent);

    // A socket count past the item's sockets reads only its sockets: the strength socket's seed in the next field
    // isn't taken for a sixth socket.
    ITEM corrupted = kris;
    corrupted.SocketCount = 255;
    std::fill(std::begin(corrupted.SocketSeedID), std::end(corrupted.SocketSeedID), BYTE{0});
    std::fill(std::begin(corrupted.SocketSphereLv), std::end(corrupted.SocketSphereLv), BYTE{38});
    CHECK_FALSE(ComputeItemTooltipValues(corrupted).strength.reduced);

    // The Blade Master lacks nothing of it.
    MakeBladeMaster(hero.Hero());
    CHECK(ComputeItemTooltipValues(kris).strength.missing == 0);
}
