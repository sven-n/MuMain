#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Core/Platform/ProcessLocale.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "I18N/All.h"
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
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#ifdef MU_ENABLE_CONTROL_SOCKET
#include "App/Control/ControlCommands.h"
#include "App/Control/ControlProtocol.h"
#endif

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

// The UI's language for the scope; the language before it comes back also when the test stops early.
class UiLocaleScope
{
public:
    explicit UiLocaleScope(const char* locale) : m_previous(I18N::GetCurrentLocale())
    {
        I18N::SetLocale(locale);
    }
    ~UiLocaleScope()
    {
        I18N::SetLocale(m_previous.c_str());
    }
    UiLocaleScope(const UiLocaleScope&) = delete;
    UiLocaleScope& operator=(const UiLocaleScope&) = delete;

private:
    std::string m_previous;
};

// A test's measure of description text for the scope; afterwards the game's font measures it again.
class TextMeasureScope
{
public:
    explicit TextMeasureScope(ItemTooltipTextMeasure measure)
    {
        SetItemTooltipTextMeasure(measure);
    }
    ~TextMeasureScope()
    {
        SetItemTooltipTextMeasure(nullptr);
    }
    TextMeasureScope(const TextMeasureScope&) = delete;
    TextMeasureScope& operator=(const TextMeasureScope&) = delete;
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
        const bool number = Engine::Items::TooltipValues::GetItemTooltipValue(values, name).has_value();
        const bool text = Engine::Items::TooltipValues::GetItemTooltipText(values, name).has_value();
        CHECK((number || text) == (name != Data::Items::LevelPlaceholder));
        CHECK_FALSE((number && text));
    }
    CHECK_FALSE(Engine::Items::TooltipValues::GetItemTooltipValue(values, "sparkle").has_value());
    CHECK_FALSE(Engine::Items::TooltipValues::GetItemTooltipText(values, "sparkle").has_value());
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
    const ItemTooltipValues values = Engine::Items::TooltipValues::ComputeItemTooltipValues(kris);
    CHECK(values.damageMin == kris.DamageMin);
    CHECK(values.damageMax == kris.DamageMax);
    CHECK(values.hasDamage);
    CHECK_FALSE(values.damageHarmonyBonus);
    CHECK(values.maxDurability > 0);
    CHECK(values.strength.required == kris.RequireStrength);
    CHECK(values.strength.missing == kris.RequireStrength - 18);
    CHECK_FALSE(values.strength.reduced);
    CHECK(Engine::Items::TooltipValues::GetItemTooltipValue(values, "requiredStrength") == values.strength.required);
    CHECK(Engine::Items::TooltipValues::GetItemTooltipValue(values, "missingStrength") == values.strength.missing);
    CHECK(Engine::Items::TooltipValues::GetItemTooltipValue(values, "maxDurability") == values.maxDurability);
    CHECK_FALSE(values.excellent);

    // A socket count past the item's sockets reads only its sockets: the strength socket's seed in the next field
    // isn't taken for a sixth socket.
    ITEM corrupted = kris;
    corrupted.SocketCount = 255;
    std::fill(std::begin(corrupted.SocketSeedID), std::end(corrupted.SocketSeedID), BYTE{0});
    std::fill(std::begin(corrupted.SocketSphereLv), std::end(corrupted.SocketSphereLv), BYTE{38});
    CHECK_FALSE(Engine::Items::TooltipValues::ComputeItemTooltipValues(corrupted).strength.reduced);

    // The Blade Master lacks nothing of it.
    MakeBladeMaster(hero.Hero());
    CHECK(Engine::Items::TooltipValues::ComputeItemTooltipValues(kris).strength.missing == 0);

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
    const ItemTooltipValues loweredValues = Engine::Items::TooltipValues::ComputeItemTooltipValues(lowered);
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
    CHECK(values.addOptionTime == addOption.m_Time);
    // How long it lasts is written only when a line asks for it.
    const std::optional<std::string> duration =
        Engine::Items::TooltipValues::GetItemTooltipText(values, "addOptionDuration");
    REQUIRE(duration.has_value());
    CHECK_FALSE(duration->empty());
}

// A bundle holds ten jewels per level step; every other item holds none, a single jewel and any equipment too.
TEST_CASE("Only a bundle has a bundle count [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeNewDarkWizard(hero.Hero());

    const auto bundleCount = [](int group, int number, int level)
    {
        ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(group, number);
        params.Level = static_cast<BYTE>(level);
        ITEM item;
        FillItemFromParameters(item, params);
        return Engine::Items::TooltipValues::ComputeItemTooltipValues(item).bundleCount;
    };
    CHECK(bundleCount(12, 30, 2) == 30); // Bundle of Jewel of Bless +2
    CHECK(bundleCount(14, 13, 0) == 0);  // Jewel of Bless
    CHECK(bundleCount(0, 0, 3) == 0);    // Kris +3
}

TEST_CASE("An item's duration text follows the UI's language [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    using Data::Items::TooltipValue;
    using Engine::Items::TooltipValues::GetItemTooltipText;

    // An hour and a second: the words for hours and seconds differ between English and German.
    ItemTooltipValues values;
    values.addOptionTime = 3601;
    const auto written = [&]
    {
        std::wstring text;
        g_StringTime(values.addOptionTime, text, true);
        return text;
    };
    I18N::SetLocale("en");
    const std::wstring english = GetItemTooltipText(values, TooltipValue::AddOptionDuration);
    CHECK(english == written());
    // The same time after a language change is written again in the new language.
    I18N::SetLocale("de");
    const std::wstring german = GetItemTooltipText(values, TooltipValue::AddOptionDuration);
    CHECK(german == written());
    CHECK(german != english);
    I18N::SetLocale("en");
    CHECK(GetItemTooltipText(values, TooltipValue::AddOptionDuration) == english);
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

TEST_CASE("An item's lines drawn after its options follow them [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);
    const auto indexOf = [](const std::vector<ItemTooltipLine>& lines, std::wstring_view start)
    {
        const auto found = std::find_if(lines.begin(), lines.end(),
                                        [&](const ItemTooltipLine& line) { return line.text.starts_with(start); });
        return found != lines.end() ? static_cast<int>(found - lines.begin()) : -1;
    };

    // The Elite Transfer Skeleton Ring's vitality is the hero's level.
    const std::vector<ItemTooltipLine> ring = Engine::Items::TooltipSnapshot::TooltipOfParameters(Engine::Items::TooltipSnapshot::FullItemParameters(13, 39));
    CHECK(indexOf(ring, L"vitality +" + std::to_wstring(hero.Hero().Level)) >= 0);

    // The Pet Skeleton leaves out the blank line after the options (noBlankAfterOptions): one blank line, its options',
    // comes before its lines, not two.
    const std::vector<ItemTooltipLine> pet = Engine::Items::TooltipSnapshot::TooltipOfParameters(Engine::Items::TooltipSnapshot::FullItemParameters(13, 123));
    const int equipping = indexOf(pet, L"Equipping along with a Skeleton");
    REQUIRE(equipping > 1);
    CHECK(pet[equipping - 1].text == L"\n");
    CHECK(pet[equipping - 2].text != L"\n");
    // The Panda Ring's blank line is there.
    const std::vector<ItemTooltipLine> panda = Engine::Items::TooltipSnapshot::TooltipOfParameters(Engine::Items::TooltipSnapshot::FullItemParameters(13, 76));
    const int transform = indexOf(panda, L"Transform");
    REQUIRE(transform > 0);
    CHECK(panda[transform - 1].text == L"\n");
    SetItemTooltipTextMeasure(nullptr);
}

TEST_CASE("The item names of windows and ground labels come from the name lines [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());

    // Without name lines: the name, " +level" above +0, and no "Excellent" or set name.
    CHECK(GetItemNameText(ITEM_SWORD, 0) == L"Kris");
    CHECK(GetItemNameText(ITEM_SWORD, 5) == L"Kris +5");
    ITEM kris;
    FillItemFromParameters(kris, Engine::Items::TooltipSnapshot::FullItemParameters(0, 0));
    kris.ExcellentFlags = 1;
    CHECK(GetItemNameText(kris.Type, kris.Level, &kris) == L"Kris");

    // A name line's text at its levels; the standard name where none holds.
    CHECK(GetItemNameText(ITEM_BOX_OF_LUCK, 4) == L"Olive of love");
    CHECK(GetItemNameText(ITEM_LOST_MAP, 0) == L"Lost Map");
    CHECK(GetItemNameText(ITEM_LOST_MAP, 3) == L"Lost Map +3");
    CHECK(GetItemNameText(ITEM_SPIRIT, 0) == L"Spirit of Dark horse");
    // The Weapon of Archangel's level names the weapon; a dropped one keeps it (CreateItemDrop), so its ground label
    // names the right one.
    CHECK(GetItemNameText(ITEM_WEAPON_OF_ARCHANGEL, 0) == L"Absolute Staff of Archangel");
    CHECK(GetItemNameText(ITEM_WEAPON_OF_ARCHANGEL, 1) == L"Absolute Sword of Archangel");
    CHECK(GetItemNameText(ITEM_WEAPON_OF_ARCHANGEL, 2) == L"Absolute Crossbow of Archangel");

    // A line's condition on the item: without the item, its options read 0.
    CHECK(GetItemNameText(ITEM_HORN_OF_FENRIR, 0) == L"Horn of Fenrir");
    ITEM horn;
    FillItemFromParameters(horn, Engine::Items::TooltipSnapshot::FullItemParameters(13, 37));
    horn.ExcellentFlags = 1;
    CHECK(GetItemNameText(horn.Type, 0, &horn) == L"Horn of Fenrir +Destroy");

    wchar_t name[64] = L"";
    GetItemName(ITEM_BOX_OF_LUCK, 4, name);
    CHECK(std::wstring(name) == L"Olive of love");
    CHECK(GetItemNameText(-1, 0).empty());
}

// The item cases moved into the data with their translations: a lost language or a broken placeholder shows here, as
// the tooltip snapshot is English.
TEST_CASE("The item cases' lines are in the UI's language [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    TextMeasureScope measure(&FixedWidthMeasure);
    UiLocaleScope german("de");

    const auto lines = [](int group, int number, int level)
    {
        ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(group, number);
        params.Level = static_cast<BYTE>(level);
        std::vector<std::wstring> texts;
        for (const ItemTooltipLine& line : Engine::Items::TooltipSnapshot::TooltipOfParameters(params))
        {
            texts.push_back(line.text);
        }
        return texts;
    };
    const auto has = [](const std::vector<std::wstring>& texts, const std::wstring& text)
    { return std::find(texts.begin(), texts.end(), text) != texts.end(); };

    const std::vector<std::wstring> bundle = lines(12, 30, 0);
    const std::vector<std::wstring> arrows = lines(4, 15, 1);
    const std::vector<std::wstring> boxOfLuck = lines(14, 11, 14);
    const std::vector<std::wstring> jackOLantern = lines(14, 46, 0);

    CHECK(has(bundle, L"10 Juwel des Segens kombiniert"));
    CHECK(has(bundle, L"Kann nach dem Zerlegen verwendet werden"));
    CHECK(has(arrows, L"Erhöht den Schaden um 3%"));
    CHECK(has(arrows, L"Zusätzlicher Schaden +1"));
    CHECK(has(boxOfLuck, L"Freier Eintritt nach Kalima"));
    CHECK(has(boxOfLuck, L"Kann nicht verkauft werden."));
    CHECK(std::any_of(jackOLantern.begin(), jackOLantern.end(),
                      [](const std::wstring& text)
                      {
                          return text.starts_with(L"Angriffsgeschwindigkeit +") &&
                                 text.find(L'{') == std::wstring::npos;
                      }));
}

// The option lines are the lines the full item has and the plain one doesn't (luck, skill, the option, the excellent
// options); an item's restriction drawn after its options comes after the last of them.
TEST_CASE("A transformation ring's restriction comes after its option lines [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    // Its start: the line wraps at the tooltip's width.
    const std::wstring restriction = L"Unable to Equip with a Different";
    for (const int number : {10, 39, 41, 68, 76, 122})
    {
        INFO("13/" << number);
        const std::vector<ItemTooltipLine> plain = Engine::Items::TooltipSnapshot::TooltipOfParameters(Engine::Items::TooltipSnapshot::FullItemParameters(13, number));
        ItemCreationParams full = Engine::Items::TooltipSnapshot::FullItemParameters(13, number);
        full.WithLuck = true;
        full.WithSkill = true;
        full.WithOption = true;
        full.OptionLevel = 4;
        full.HasExcellentOption = true;
        full.ExcellentFlags = 63;
        const std::vector<ItemTooltipLine> lines = Engine::Items::TooltipSnapshot::TooltipOfParameters(full);

        const auto at = std::find_if(lines.begin(), lines.end(),
                                     [&](const ItemTooltipLine& line) { return line.text.starts_with(restriction); });
        std::string shown;
        for (const ItemTooltipLine& line : lines)
        {
            shown += Core::Text::ToUtf8(line.text.c_str()) + " | ";
        }
        INFO(shown);
        if (at == lines.end())
        {
            CHECK(at != lines.end());
            continue;
        }
        int lastOption = -1;
        for (int index = 0; index < static_cast<int>(lines.size()); ++index)
        {
            const bool inPlain = std::any_of(plain.begin(), plain.end(), [&](const ItemTooltipLine& line)
                                             { return line.text == lines[index].text; });
            if (!inPlain && lines[index].text != L"\n")
            {
                lastOption = index;
            }
        }
        CHECK(lastOption >= 0);
        CHECK(at - lines.begin() > lastOption);
    }

    SetItemTooltipTextMeasure(nullptr);
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

// The standard name's color, by precedence: ancient, socket, excellent with an option, +7 and above, an option, else
// white. The name is the tooltip's first line that isn't blank.
TEST_CASE("A tooltip's name color goes by the item's kind, level and options in their order [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);
    const auto name = [](int level, bool luck, bool excellent, bool ancient)
    {
        ItemCreationParams kris = Engine::Items::TooltipSnapshot::FullItemParameters(0, 0);
        kris.Level = static_cast<BYTE>(level);
        kris.WithLuck = luck;
        kris.HasExcellentOption = excellent;
        kris.ExcellentFlags = excellent ? 1 : 0;
        kris.IsAncient = ancient;
        kris.AncientDiscriminator = ancient ? 1 : 0;
        const std::vector<ItemTooltipLine> lines = Engine::Items::TooltipSnapshot::TooltipOfParameters(kris);
        const auto name = std::find_if(lines.begin(), lines.end(),
                                       [](const ItemTooltipLine& line) { return line.text != L"\n"; });
        REQUIRE(name != lines.end());
        return *name;
    };

    const ItemTooltipLine plain = name(0, false, false, false);
    CHECK(plain.text == L"Kris");
    CHECK(plain.color == TEXT_COLOR_WHITE);
    CHECK(name(0, true, false, false).color == TEXT_COLOR_BLUE);
    CHECK(name(7, false, false, false).color == TEXT_COLOR_YELLOW);
    // +7 and above wins over an option, excellent with an option over the level, ancient over all.
    CHECK(name(9, true, false, false).color == TEXT_COLOR_YELLOW);
    CHECK(name(9, true, true, false).color == TEXT_COLOR_GREEN);
    CHECK(name(9, true, true, true).color == TEXT_COLOR_GREEN_BLUE);

    // In the item names' language, with translated item names on.
    const std::string uiLocale = I18N::GetCurrentLocale();
    const bool translateItemNames = g_ItemDataHandler.GetTranslateItemNames();
    g_ItemDataHandler.SetTranslateItemNames(true);
    I18N::SetLocale("pt");
    const std::wstring portuguese = name(9, false, false, false).text;
    I18N::SetLocale(uiLocale.c_str());
    g_ItemDataHandler.SetTranslateItemNames(translateItemNames);
    CHECK(portuguese == L"Cris +9");

    SetItemTooltipTextMeasure(nullptr);
}

// The quests, the mixes, the server's messages and the ground labels name an item with GetItemName: for every item at
// every level, the name is the tooltip's first line that isn't blank, which the recorded snapshot covers.
TEST_CASE("Every item's name at every level is its tooltip's name [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    int names = 0;
    int different = 0;
    for (const Data::Items::ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
        {
            ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(definition.group, definition.number);
            params.Level = static_cast<BYTE>(level);
            const std::vector<ItemTooltipLine> lines = Engine::Items::TooltipSnapshot::TooltipOfParameters(params);
            // The name is the first line that isn't blank.
            const auto first = std::find_if(lines.begin(), lines.end(),
                                            [](const ItemTooltipLine& line) { return line.text != L"\n"; });
            if (first == lines.end())
            {
                continue;
            }
            const int type = definition.group * MAX_ITEM_INDEX + definition.number;
            const std::wstring name = GetItemNameText(type, level);
            wchar_t buffer[128] = L"";
            GetItemName(type, level, buffer);
            ++names;
            if (name != first->text || std::wstring(buffer) != name)
            {
                if (++different <= 5)
                {
                    INFO(definition.group << "/" << definition.number << " +" << level);
                    CHECK(Core::Text::ToUtf8(name.c_str()) == Core::Text::ToUtf8(first->text.c_str()));
                    CHECK(Core::Text::ToUtf8(buffer) == Core::Text::ToUtf8(name.c_str()));
                }
            }
        }
    }
    SetItemTooltipTextMeasure(nullptr);
    CHECK(names > 911 * 10);
    CHECK(different == 0);
}

// The ten jewel and stone bundles name their count, 10 per level from 10 at +0, in the item names' language.
TEST_CASE("A bundle's name has its count at every level [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;

    const std::pair<int, std::wstring> bundles[] = {
        {30, L"Bundle of Jewel of Bless"},        {31, L"Bundle of Jewel of Soul"},
        {136, L"Bundle of Jewel of Life"},        {137, L"Bundle of Jewel of Creation"},
        {138, L"Bundle of Jewel of Guardian"},    {139, L"Bundle of Gemstone"},
        {140, L"Bundle of Jewel of Harmony"},     {141, L"Bundle of Jewel of Chaos"},
        {142, L"Bundle of Lower Refining Stone"}, {143, L"Bundle of Higher Refining Stone"},
    };
    for (const auto& [number, name] : bundles)
    {
        for (int level = 0; level <= 3; ++level)
        {
            INFO("12/" << number << " +" << level);
            CHECK(GetItemNameText(ITEM_WING + number, level) == name + L" x" + std::to_wstring(10 * (level + 1)));
        }
    }

    // In another language, with translated item names on: the translated name, the same count.
    const std::string uiLocale = I18N::GetCurrentLocale();
    const bool translateItemNames = g_ItemDataHandler.GetTranslateItemNames();
    g_ItemDataHandler.SetTranslateItemNames(true);
    I18N::SetLocale("es");
    const std::wstring spanish = GetItemNameText(ITEM_WING + 142, 3);
    I18N::SetLocale(uiLocale.c_str());
    g_ItemDataHandler.SetTranslateItemNames(translateItemNames);
    CHECK(spanish == L"Fardo de Piedras de Más Baja Refinación x40");
}

TEST_CASE("With translated item names off, the names are English and the rest of the tooltip is translated "
          "[data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);
    const std::string uiLocale = I18N::GetCurrentLocale();
    const bool translateItemNames = g_ItemDataHandler.GetTranslateItemNames();
    g_ItemDataHandler.SetTranslateItemNames(false);
    I18N::SetLocale("de");
    const auto has = [](const std::vector<ItemTooltipLine>& lines, std::wstring_view start)
    {
        return std::any_of(lines.begin(), lines.end(),
                           [&](const ItemTooltipLine& line) { return line.text.starts_with(start); });
    };

    // The Wings of Elf: the standard name, the stat lines of the tooltip file.
    const std::vector<ItemTooltipLine> wings = Engine::Items::TooltipSnapshot::TooltipOfParameters(Engine::Items::TooltipSnapshot::FullItemParameters(12, 0));
    // A name line's text, here the +0 Transformation Ring's.
    const std::vector<ItemTooltipLine> ring = Engine::Items::TooltipSnapshot::TooltipOfParameters(Engine::Items::TooltipSnapshot::FullItemParameters(13, 10));

    I18N::SetLocale(uiLocale.c_str());
    g_ItemDataHandler.SetTranslateItemNames(translateItemNames);
    SetItemTooltipTextMeasure(nullptr);

    CHECK(has(wings, L"Wings of Elf"));
    CHECK(has(wings, L"Verteidigung:"));
    CHECK(has(wings, L"Haltbarkeit:"));
    CHECK_FALSE(has(wings, L"Defense:"));
    CHECK(std::any_of(ring.begin(), ring.end(),
                      [](const ItemTooltipLine& line) { return line.text.ends_with(L"Transformation Ring"); }));
}

// A full item's durability is the one the repair window counts as full (CalcMaxDurability): it grows with the level,
// an excellent or ancient option raises it, a staff or stick starts from its magic durability, and the item's byte
// holds at most 255.
TEST_CASE("An item's full durability follows its level, options and magic durability [data][items][tooltip]")
{
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());

    const auto full = [](int group, int number, int level, BYTE excellent, BYTE ancient)
    {
        ItemCreationParams params = Engine::Items::TooltipSnapshot::FullItemParameters(group, number);
        params.Level = static_cast<BYTE>(level);
        params.HasExcellentOption = excellent > 0;
        params.ExcellentFlags = excellent;
        params.IsAncient = ancient > 0;
        params.AncientDiscriminator = ancient;
        return static_cast<int>(Engine::Items::TooltipSnapshot::FullDurability(params));
    };
    // The Kris (0/0): 20 at +0, 34 at +9, 15 more with an excellent option, 20 more as an ancient item.
    CHECK(full(0, 0, 0, 0, 0) == 20);
    CHECK(full(0, 0, 9, 0, 0) == 34);
    CHECK(full(0, 0, 0, 1, 0) == 35);
    CHECK(full(0, 0, 0, 0, 1) == 40);
    // The Skull Staff (5/0) and the Storm Blitz Stick (5/19) have no durability of their own, only a magic one.
    CHECK(full(5, 0, 0, 0, 0) == 20);
    CHECK(full(5, 19, 0, 0, 0) == 95);
    // The Dark Horse (13/4) is at 255 already; its excellent option's 15 more don't fit the byte.
    CHECK(full(13, 4, 0, 1, 0) == 255);
}

#ifdef MU_ENABLE_CONTROL_SOCKET
namespace
{
std::string RunCommand(std::string (*command)(const App::Control::Request&, std::unique_ptr<App::Control::Act>&),
                       const std::string& raw)
{
    std::unique_ptr<App::Control::Act> act;
    return command(App::Control::Request::Parse(raw), act);
}
} // namespace

// Without `durability`, `tooltip` and `repair-tooltip` make a full item, at the durability the repair window counts as
// full, so its repair costs nothing; the item's level and options count. The drawn repair tooltip's RepairEnable stays
// as it is.
TEST_CASE("The item commands make a full item when the durability is left out [data][items][tooltip][control]")
{
    ProcessLocaleScope locale;
    ClientDataScope client(ClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    TooltipDataScope data;
    TooltipHeroScope hero;
    MakeBladeMaster(hero.Hero());
    SetItemTooltipTextMeasure(&FixedWidthMeasure);

    const std::pair<const char*, int> items[] = {
        {R"("group":0,"number":0,"level":9)", 34}, // the Kris +9
        {R"("group":0,"number":0,"excellent":1)", 35},
        {R"("group":0,"number":0,"ancient":1)", 40},
        {R"("group":5,"number":0)", 20},  // the Skull Staff
        {R"("group":5,"number":19)", 95}, // the Storm Blitz Stick
    };
    const int repairEnable = RepairEnable;
    RepairEnable = 7;
    for (const auto& [fields, full] : items)
    {
        INFO(fields);
        const std::string durability = Core::Text::ToUtf8(Formatted(L"Durability: [%d/%d]", full, full).c_str());
        const std::string tooltip =
            RunCommand(&App::Control::Commands::ItemTooltip, std::string(R"({"cmd":"tooltip",)") + fields + "}");
        CHECK(tooltip.find(durability) != std::string::npos);
        const std::string repair = RunCommand(&App::Control::Commands::RepairTooltip,
                                              std::string(R"({"cmd":"repair-tooltip",)") + fields + "}");
        CHECK(repair.find(durability) != std::string::npos);
        CHECK(repair.find("Repairing cost: 0") != std::string::npos);
    }
    CHECK(RepairEnable == 7);
    RepairEnable = repairEnable;

    // A durability given is kept.
    const std::string worn = RunCommand(&App::Control::Commands::ItemTooltip,
                                        R"({"cmd":"tooltip","group":0,"number":0,"level":9,"durability":20})");
    CHECK(worn.find("Durability: [20/34]") != std::string::npos);
    SetItemTooltipTextMeasure(nullptr);
}
#endif
