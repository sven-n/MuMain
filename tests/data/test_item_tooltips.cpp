#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Core/Platform/ProcessLocale.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
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
#include "GameLogic/Skills/SkillManager.h"
#include "Network/Server/SocketSystem.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/UIJewelHarmony.h"
#include "UI/Inventory/ItemMng.h"

#include <algorithm>
#include <iterator>
#include <clocale>
#include <ctime>
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
// skills (the orbs' and scrolls' names and damage), the socket items, the item sets, the monster list (the
// transformation rings), the Jewel of Harmony and item add
// option tables, and the buff system's time texts. Run in a ClientDataScope, which makes the paths relative to the
// client. It puts back what it replaces (the language, the two tables, the buff system and the skill table it made),
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

// The hash of an item's tooltips as recorded in item_tooltip_snapshot.txt.
std::string HashText(const std::string& text)
{
    char hash[17];
    std::snprintf(hash, sizeof(hash), "%016llx", static_cast<unsigned long long>(StableHash(text)));
    return hash;
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

// Every item's tooltip at every level 0 to 15, in the eleven variants of WriteItemTooltipSnapshot (plain, full, ancient,
// harmony, each option alone, worn, broken and expired),
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

    const std::filesystem::path recordedFile = MU_TEST_TOOLTIP_SNAPSHOT;
    const bool update = std::getenv("MU_TOOLTIP_SNAPSHOT_UPDATE") != nullptr;
    std::map<std::string, std::string> recorded;
    if (!update)
    {
        std::ifstream in(recordedFile);
        REQUIRE(in);
        std::string hero;
        std::string item;
        std::string hash;
        while (in >> hero >> item >> hash)
        {
            recorded[hero + " " + item] = hash;
        }
    }

    // Each item is hashed as it is written and compared at once; only a changed item's text is kept, for the first
    // five, so the test holds one item's tooltips at a time.
    std::map<std::string, std::string> hashes;
    std::vector<std::pair<std::string, std::string>> changedTooltips;
    int changed = 0;
    const std::pair<const char*, void (*)(CHARACTER_ATTRIBUTE&)> heroes[] = {{"newDarkWizard", &MakeNewDarkWizard},
                                                                             {"bladeMaster", &MakeBladeMaster}};
    for (const auto& [name, make] : heroes)
    {
        TooltipHeroScope hero;
        make(hero.Hero());
        ::Hero->Class = hero.Hero().Class;
        ::Hero->Level = hero.Hero().Level;
        const int tooltips = Engine::Items::TooltipSnapshot::VisitItemTooltipSnapshots(
            [&, heroName = std::string(name)](int group, int number, const std::string& text)
            {
                const std::string key = heroName + " " + std::to_string(group) + "/" + std::to_string(number);
                const std::string hash = HashText(text);
                hashes[key] = hash;
                const auto found = recorded.find(key);
                if (!update && (found == recorded.end() || found->second != hash) && ++changed <= 5)
                {
                    changedTooltips.emplace_back(key, text);
                }
            });
        CHECK(tooltips == 911 * 11 * 16);
    }
    SetItemTooltipTextMeasure(nullptr);

    if (update)
    {
        std::ofstream out(recordedFile, std::ios::binary);
        for (const auto& [item, hash] : hashes)
        {
            out << item << ' ' << hash << '\n';
        }
        MESSAGE("recorded " << hashes.size() << " items in " << recordedFile.string());
        return;
    }
    for (const auto& [key, text] : changedTooltips)
    {
        INFO(key << " now:\n" << text);
        CHECK(false);
    }
    CHECK(changed == 0);
    CHECK(recorded.size() == hashes.size());
}

namespace
{
// The tooltip's line formatted as the game formats it. The tests write the English texts out, so they hold wherever
// the game keeps them (the .resx or the item data).
template <typename... Args> std::wstring Formatted(const wchar_t* format, Args... args)
{
    wchar_t text[256];
    mu_swprintf(text, format, args...);
    return text;
}

bool HasLine(const std::vector<ItemTooltipLine>& lines, const std::wstring& text, int color = -1)
{
    return std::any_of(lines.begin(), lines.end(), [&](const ItemTooltipLine& line)
                       { return line.text == text && (color < 0 || line.color == color); });
}
} // namespace

// The lines that depend on the durability or the expiry, which the snapshot's full items don't show apart, in English.
TEST_CASE("A tooltip's durability, count and expiry lines follow the item [data][items][tooltip]")
{
    ProcessLocaleScope locale;
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    // A weapon's durability, full, worn and broken, out of the same maximum.
    ItemCreationParams kris = Engine::Items::TooltipSnapshot::FullItemParameters(0, 0);
    const int maxDurability = kris.Durability;
    REQUIRE(maxDurability > 1);
    for (const int durability : {maxDurability, 1, 0})
    {
        INFO(durability);
        kris.Durability = static_cast<BYTE>(durability);
        const std::vector<ItemTooltipLine> lines = Engine::Items::TooltipSnapshot::TooltipOfParameters(kris);
        CHECK(HasLine(lines, Formatted(L"Durability: [%d/%d]", durability, maxDurability)));
    }

    // A stack's count.
    ItemCreationParams apples = Engine::Items::TooltipSnapshot::FullItemParameters(14, 0);
    apples.Durability = 3;
    CHECK(HasLine(Engine::Items::TooltipSnapshot::TooltipOfParameters(apples), Formatted(L"Number of items: %d", 3)));
    apples.Durability = 1;
    CHECK(HasLine(Engine::Items::TooltipSnapshot::TooltipOfParameters(apples), Formatted(L"Number of items: %d", 1)));

    // An expired item says so, in red, and shows no durability.
    kris.Durability = static_cast<BYTE>(maxDurability);
    kris.WithExpiration = true;
    kris.IsExpired = true;
    const std::vector<ItemTooltipLine> expired = Engine::Items::TooltipSnapshot::TooltipOfParameters(kris);
    CHECK(HasLine(expired, L"Expired Item", TEXT_COLOR_RED));
    CHECK_FALSE(HasLine(expired, Formatted(L"Durability: [%d/%d]", maxDurability, maxDurability)));

    // A timed item shows when it ends, in the local time, and no durability either.
    kris.IsExpired = false;
    // Made by the item manager, as the game makes it: its reference count (1) comes right after the expiry time, so a
    // read of more than the expiry time's own bytes would take it as the time's high half.
    mu::ui::window::CItemMng items;
    ITEM& timed = *items.CreateItemByParameters(&kris);
    timed.lExpireTime = 1767225600; // 2026-01-01 00:00 UTC
    const std::vector<ItemTooltipLine> timedLines = BuildItemTooltipLines(&timed, false);
    time_t expireTime = timed.lExpireTime;
    const tm* local = localtime(&expireTime);
    REQUIRE(local != nullptr);
    CHECK(HasLine(timedLines, L"Expiration Day", TEXT_COLOR_ORANGE));
    CHECK(HasLine(timedLines,
                  Formatted(L"%d-%02d-%02d  %02d:%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                            local->tm_hour, local->tm_min),
                  TEXT_COLOR_BLUE));
    CHECK_FALSE(HasLine(timedLines, Formatted(L"Durability: [%d/%d]", maxDurability, maxDurability)));
    // Without its end date (the server sends it apart), it shows no tooltip.
    timed.lExpireTime = 0;
    CHECK(BuildItemTooltipLines(&timed, false).empty());
    items.DeleteItem(&timed);

    SetItemTooltipTextMeasure(nullptr);
}

TEST_CASE("Every built-in placeholder but {level} is a value the game computes for the item [data][items][tooltip]")
{
    using namespace Engine::Items::TooltipValues;
    const ItemTooltipValues values;
    const auto builtIns = Data::Items::GetBuiltInPlaceholders();
    for (const std::string_view name : builtIns)
    {
        INFO(std::string(name));
        // A number, or a text ({addOptionDuration}), and only one of them.
        const bool number = GetItemTooltipValue(values, name).has_value();
        const bool text = GetItemTooltipText(values, name).has_value();
        CHECK((number || text) == (name != Data::Items::LevelPlaceholder));
        CHECK_FALSE((number && text));
    }
    CHECK_FALSE(GetItemTooltipValue(values, "sparkle").has_value());
    CHECK_FALSE(GetItemTooltipText(values, "sparkle").has_value());
    // And the other way round: a value the game can fill that the placeholder checks don't know would make every
    // description using it fail to load.
    for (const std::string_view name : GetItemTooltipValueNames())
    {
        INFO(std::string(name));
        CHECK(std::find(builtIns.begin(), builtIns.end(), name) != builtIns.end());
    }
}

TEST_CASE("The tooltip's values of an item follow the item and the hero [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeNewDarkWizard(hero.Hero());

    using namespace Engine::Items::TooltipValues;
    ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(0, 0);
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

    // A Jewel of Harmony option that lowers the strength requirement (a weapon's option 3, at level 13, on a +13
    // item) shows the lowered requirement, marked reduced (yellow); agility is untouched.
    ItemCreationParams refined = Engine::Items::TooltipSnapshot::FullItemParameters(0, 0);
    refined.Level = 13;
    refined.HasHarmonyOption = true;
    refined.HarmonyOptionType = 3;
    refined.HarmonyOptionLevel = 13;
    ITEM lowered;
    FillItemFromParameters(lowered, refined);
    StrengthenCapability capability{};
    g_pUIJewelHarmonyinfo->GetStrengthenCapability(&capability, &lowered, 0);
    REQUIRE(capability.SI_isNB);
    REQUIRE(capability.SI_NB.SI_force > 0);
    const ItemTooltipValues loweredValues = ComputeItemTooltipValues(lowered);
    CHECK(loweredValues.strength.required == lowered.RequireStrength - capability.SI_NB.SI_force);
    CHECK(loweredValues.strength.reduced);
    CHECK(loweredValues.dexterity.required == lowered.RequireDexterity);
    CHECK_FALSE(loweredValues.dexterity.reduced);
}

// Without a hero (an editor preview, before entering the world), the requirements are still shown and nothing is
// missing, instead of reading through a null hero.
TEST_CASE("The tooltip's values need no hero [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    CHARACTER_ATTRIBUTE* const previous = CharacterAttribute;
    CharacterAttribute = nullptr;
    ITEM kris;
    FillItemFromParameters(kris, Engine::Items::TooltipSnapshot::FullItemParameters(0, 0));
    const ItemTooltipValues values = Engine::Items::TooltipValues::ComputeItemTooltipValues(kris);
    CharacterAttribute = previous;
    CHECK(values.strength.required == kris.RequireStrength);
    CHECK(values.strength.missing == 0);
    CHECK(values.level.missing == 0);
}

// A Jewel of Harmony option that brings a requirement to exactly 0 still shows the line, at 0 and in yellow, as the
// old code did: the Grand Viper Staff (5/10) asks for 37 agility, which its option 3 at level 12 takes off.
TEST_CASE("A requirement reduced to 0 still shows [data][items][tooltip]")
{
    ProcessLocaleScope locale;
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeNewDarkWizard(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(5, 10);
    params.Level = 13;
    params.HasHarmonyOption = true;
    params.HarmonyOptionType = 3;
    params.HarmonyOptionLevel = 12;
    ITEM staff;
    FillItemFromParameters(staff, params);
    const ItemTooltipValues values = Engine::Items::TooltipValues::ComputeItemTooltipValues(staff);
    REQUIRE(values.dexterity.reduced);
    REQUIRE(values.dexterity.required == 0);
    CHECK(HasLine(BuildItemTooltipLines(&staff, false), L"Agility Requirement: 0", TEXT_COLOR_YELLOW));

    SetItemTooltipTextMeasure(nullptr);
}

TEST_CASE("An item's add option values come from its add option [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeNewDarkWizard(hero.Hero());

    // The Seal of Healing (13/62): its experience rate and life recovery, and how long it lasts.
    ITEM seal;
    FillItemFromParameters(seal, Engine::Items::TooltipSnapshot::FullItemParameters(13, 62));
    const ITEM_ADD_OPTION& addOption = g_pItemAddOptioninfo->GetItemAddOtioninfo(seal.Type);
    REQUIRE(addOption.m_byValue1 != 0);
    const ItemTooltipValues values = Engine::Items::TooltipValues::ComputeItemTooltipValues(seal);
    CHECK(values.addOptionValue1 == addOption.m_byValue1);
    CHECK(values.addOptionValue2 == addOption.m_byValue2);
    CHECK(Engine::Items::TooltipValues::GetItemTooltipValue(values, "addOptionValue1") == static_cast<int>(addOption.m_byValue1));
    CHECK_FALSE(values.addOptionDuration.empty());
    CHECK(Engine::Items::TooltipValues::GetItemTooltipText(values, "addOptionDuration") == values.addOptionDuration);
}

TEST_CASE("The Weapon of Archangel shows its reward weapon's skill mana from the skill data [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeNewDarkWizard(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    int mana = 0;
    gSkillManager.GetSkillInformation(AT_SKILL_CYCLONE, 1, nullptr, &mana, nullptr);
    REQUIRE(mana > 0);
    ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(13, 19);
    params.Level = 1;
    const std::vector<ItemTooltipLine> lines = Engine::Items::TooltipSnapshot::TooltipOfParameters(params);
    SetItemTooltipTextMeasure(nullptr);
    const std::wstring expected = L"Cyclone Cutting skill (Mana:" + std::to_wstring(mana) + L")";
    CHECK(std::any_of(lines.begin(), lines.end(), [&](const ItemTooltipLine& line) { return line.text == expected; }));
}

TEST_CASE("A class condition holds for a hero of that class [data][items][tooltip]")
{
    TooltipHeroScope hero;
    Data::Items::TooltipCondition darkLord;
    std::string error;
    REQUIRE(Data::Items::ReadTooltipCondition("class:darkLord", darkLord, error));
    CHECK(Data::Items::WriteTooltipCondition(darkLord) == "class:darkLord");
    CHECK_FALSE(Data::Items::ReadTooltipCondition("class:darkLrod", darkLord, error));
    CHECK(error.find("names no class") != std::string::npos);
    REQUIRE(Data::Items::ReadTooltipCondition("class:darkLord", darkLord, error));

    const ItemTooltipValues values;
    const std::vector<Data::Items::TooltipCondition> conditions = {darkLord};
    ::Hero->Class = CLASS_DARK_LORD;
    CHECK(Engine::Items::TooltipValues::AreTooltipConditionsMet(conditions, 0, 0, values));
    ::Hero->Class = CLASS_WIZARD;
    CHECK_FALSE(Engine::Items::TooltipValues::AreTooltipConditionsMet(conditions, 0, 0, values));
    // A Lord Emperor is a Dark Lord too.
    ::Hero->Class = CLASS_LORDEMPEROR;
    CHECK(Engine::Items::TooltipValues::AreTooltipConditionsMet(conditions, 0, 0, values));
}

// The second chain's cases with conditions, as the tooltip shows them: the Command Fruit's class line, white for a
// Dark Lord and dark red for the other classes, and the Wizard's Ring's lines by its level.
TEST_CASE("The Command Fruit's class line and the Wizard's Ring's lines follow the hero and the level "
          "[data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    ItemCreationParams fruit = Engine::Items::TooltipSnapshot::FullItemParameters(13, 15);
    fruit.Level = 4;
    ::Hero->Class = CLASS_DARK_LORD;
    CHECK(HasLine(Engine::Items::TooltipSnapshot::TooltipOfParameters(fruit), L"Can be equipped by Dark Lord", TEXT_COLOR_WHITE));
    ::Hero->Class = CLASS_LORDEMPEROR;
    CHECK(HasLine(Engine::Items::TooltipSnapshot::TooltipOfParameters(fruit), L"Can be equipped by Dark Lord", TEXT_COLOR_WHITE));
    ::Hero->Class = CLASS_WIZARD;
    CHECK(HasLine(Engine::Items::TooltipSnapshot::TooltipOfParameters(fruit), L"Can be equipped by Dark Lord", TEXT_COLOR_DARKRED));

    // The Wizard's Ring: +0 and +3 can't be repaired and show their durability; +1 and +2 are the event rings, which
    // say instead when they can be dropped and that they can't be stored, traded or sold.
    for (const int level : {0, 1, 2, 3})
    {
        INFO("+" << level);
        ItemCreationParams ring = Engine::Items::TooltipSnapshot::FullItemParameters(13, 20);
        ring.Level = static_cast<BYTE>(level);
        const std::vector<ItemTooltipLine> lines = Engine::Items::TooltipSnapshot::TooltipOfParameters(ring);
        const bool eventRing = level == 1 || level == 2;
        CHECK(HasLine(lines, L"Cannot Repair", TEXT_COLOR_RED) == !eventRing);
        CHECK(std::any_of(lines.begin(), lines.end(),
                          [](const ItemTooltipLine& line) { return line.text.starts_with(L"Durability:"); }) ==
              !eventRing);
        CHECK(HasLine(lines, L"Can be dropped after level 40") == (level == 1));
        CHECK(HasLine(lines, L"Can be dropped after level 80") == (level == 2));
        CHECK(HasLine(lines, L"Cannot be sold.", TEXT_COLOR_RED) == eventRing);
    }

    SetItemTooltipTextMeasure(nullptr);
}
