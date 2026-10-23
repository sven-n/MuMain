#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Core/Platform/ProcessLocale.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "I18N/All.h"
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
        CHECK(WriteItemTooltipSnapshot(snapshot) == 911 * 11 * 16);
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
    ItemCreationParams kris = FullItemParameters(0, 0);
    const int maxDurability = kris.Durability;
    REQUIRE(maxDurability > 1);
    for (const int durability : {maxDurability, 1, 0})
    {
        INFO(durability);
        kris.Durability = static_cast<BYTE>(durability);
        const std::vector<ItemTooltipLine> lines = TooltipOfParameters(kris);
        CHECK(HasLine(lines, Formatted(L"Durability: [%d/%d]", durability, maxDurability)));
    }

    // A stack's count.
    ItemCreationParams apples = FullItemParameters(14, 0);
    apples.Durability = 3;
    CHECK(HasLine(TooltipOfParameters(apples), Formatted(L"Number of items: %d", 3)));
    apples.Durability = 1;
    CHECK(HasLine(TooltipOfParameters(apples), Formatted(L"Number of items: %d", 1)));

    // An expired item says so, in red, and shows no durability.
    kris.Durability = static_cast<BYTE>(maxDurability);
    kris.WithExpiration = true;
    kris.IsExpired = true;
    const std::vector<ItemTooltipLine> expired = TooltipOfParameters(kris);
    CHECK(HasLine(expired, L"Expired Item", TEXT_COLOR_RED));
    CHECK_FALSE(HasLine(expired, Formatted(L"Durability: [%d/%d]", maxDurability, maxDurability)));

    // A timed item shows when it ends, in the local time, and no durability either.
    kris.IsExpired = false;
    ITEM timed;
    FillItemFromParameters(timed, kris);
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

    SetItemTooltipTextMeasure(nullptr);
}

TEST_CASE("Every built-in placeholder but {level} is a value the game computes for the item [data][items][tooltip]")
{
    const ItemTooltipValues values;
    for (const std::string_view name : Data::Items::GetBuiltInPlaceholders())
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
    FillItemFromParameters(seal, FullItemParameters(13, 62));
    const ITEM_ADD_OPTION& addOption = g_pItemAddOptioninfo->GetItemAddOtioninfo(seal.Type);
    REQUIRE(addOption.m_byValue1 != 0);
    const ItemTooltipValues values = ComputeItemTooltipValues(seal);
    CHECK(values.addOptionValue1 == addOption.m_byValue1);
    CHECK(values.addOptionValue2 == addOption.m_byValue2);
    CHECK(GetItemTooltipValue(values, "addOptionValue1") == static_cast<int>(addOption.m_byValue1));
    CHECK_FALSE(values.addOptionDuration.empty());
    CHECK(GetItemTooltipText(values, "addOptionDuration") == values.addOptionDuration);
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
    ItemCreationParams params = FullItemParameters(13, 19);
    params.Level = 1;
    const std::vector<ItemTooltipLine> lines = TooltipOfParameters(params);
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
    const std::vector<ItemTooltipLine> ring = TooltipOfParameters(FullItemParameters(13, 39));
    CHECK(indexOf(ring, L"vitality +" + std::to_wstring(hero.Hero().Level)) >= 0);

    // The Pet Skeleton leaves out the blank line after the options (noBlankAfterOptions): one blank line, its options',
    // comes before its lines, not two.
    const std::vector<ItemTooltipLine> pet = TooltipOfParameters(FullItemParameters(13, 123));
    const int equipping = indexOf(pet, L"Equipping along with a Skeleton");
    REQUIRE(equipping > 1);
    CHECK(pet[equipping - 1].text == L"\n");
    CHECK(pet[equipping - 2].text != L"\n");
    // The Panda Ring's blank line is there.
    const std::vector<ItemTooltipLine> panda = TooltipOfParameters(FullItemParameters(13, 76));
    const int transform = indexOf(panda, L"Transform");
    REQUIRE(transform > 0);
    CHECK(panda[transform - 1].text == L"\n");
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
    CHECK(AreTooltipConditionsMet(conditions, 0, 0, values));
    ::Hero->Class = CLASS_WIZARD;
    CHECK_FALSE(AreTooltipConditionsMet(conditions, 0, 0, values));
    // A Lord Emperor is a Dark Lord too.
    ::Hero->Class = CLASS_LORDEMPEROR;
    CHECK(AreTooltipConditionsMet(conditions, 0, 0, values));
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
    FillItemFromParameters(kris, FullItemParameters(0, 0));
    kris.ExcellentFlags = 1;
    CHECK(GetItemNameText(kris.Type, kris.Level, &kris) == L"Kris");

    // A name line's text at its levels; the standard name where none holds.
    CHECK(GetItemNameText(ITEM_BOX_OF_LUCK, 4) == L"Olive of love");
    CHECK(GetItemNameText(ITEM_LOST_MAP, 0) == L"Lost Map");
    CHECK(GetItemNameText(ITEM_LOST_MAP, 3) == L"Lost Map +3");
    CHECK(GetItemNameText(ITEM_SPIRIT, 0) == L"Spirit of Dark horse");

    // A line's condition on the item: without the item, its options read 0.
    CHECK(GetItemNameText(ITEM_HORN_OF_FENRIR, 0) == L"Horn of Fenrir");
    ITEM horn;
    FillItemFromParameters(horn, FullItemParameters(13, 37));
    horn.ExcellentFlags = 1;
    CHECK(GetItemNameText(horn.Type, 0, &horn) == L"Horn of Fenrir +Destroy");

    wchar_t name[64] = L"";
    GetItemName(ITEM_BOX_OF_LUCK, 4, name);
    CHECK(std::wstring(name) == L"Olive of love");
    CHECK(GetItemNameText(-1, 0).empty());
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

    const std::wstring restriction = L"Unable to Equip with a Different Transformation Ring";
    for (const int number : {10, 39, 41, 68, 76, 122})
    {
        INFO("13/" << number);
        const std::vector<ItemTooltipLine> plain = TooltipOfParameters(FullItemParameters(13, number));
        ItemCreationParams full = FullItemParameters(13, number);
        full.WithLuck = true;
        full.WithSkill = true;
        full.WithOption = true;
        full.OptionLevel = 4;
        full.HasExcellentOption = true;
        full.ExcellentFlags = 63;
        const std::vector<ItemTooltipLine> lines = TooltipOfParameters(full);

        const auto at = std::find_if(lines.begin(), lines.end(),
                                     [&](const ItemTooltipLine& line) { return line.text == restriction; });
        std::string shown;
        for (const ItemTooltipLine& line : lines)
        {
            shown += Core::Text::ToUtf8(line.text.c_str()) + " | ";
        }
        INFO(shown);
        REQUIRE(at != lines.end());
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

    ItemCreationParams fruit = FullItemParameters(13, 15);
    fruit.Level = 4;
    ::Hero->Class = CLASS_DARK_LORD;
    CHECK(HasLine(TooltipOfParameters(fruit), L"Can be equipped by Dark Lord", TEXT_COLOR_WHITE));
    ::Hero->Class = CLASS_LORDEMPEROR;
    CHECK(HasLine(TooltipOfParameters(fruit), L"Can be equipped by Dark Lord", TEXT_COLOR_WHITE));
    ::Hero->Class = CLASS_WIZARD;
    CHECK(HasLine(TooltipOfParameters(fruit), L"Can be equipped by Dark Lord", TEXT_COLOR_DARKRED));

    // The Wizard's Ring: +0 and +3 can't be repaired and show their durability; +1 and +2 are the event rings, which
    // say instead when they can be dropped and that they can't be stored, traded or sold.
    for (const int level : {0, 1, 2, 3})
    {
        INFO("+" << level);
        ItemCreationParams ring = FullItemParameters(13, 20);
        ring.Level = static_cast<BYTE>(level);
        const std::vector<ItemTooltipLine> lines = TooltipOfParameters(ring);
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
        ItemCreationParams kris = FullItemParameters(0, 0);
        kris.Level = static_cast<BYTE>(level);
        kris.WithLuck = luck;
        kris.HasExcellentOption = excellent;
        kris.ExcellentFlags = excellent ? 1 : 0;
        kris.IsAncient = ancient;
        kris.AncientDiscriminator = ancient ? 1 : 0;
        const std::vector<ItemTooltipLine> lines = TooltipOfParameters(kris);
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
            ItemCreationParams params = FullItemParameters(definition.group, definition.number);
            params.Level = static_cast<BYTE>(level);
            const std::vector<ItemTooltipLine> lines = TooltipOfParameters(params);
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
    const std::vector<ItemTooltipLine> wings = TooltipOfParameters(FullItemParameters(12, 0));
    // A name line's text, here the +0 Transformation Ring's.
    const std::vector<ItemTooltipLine> ring = TooltipOfParameters(FullItemParameters(13, 10));

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
