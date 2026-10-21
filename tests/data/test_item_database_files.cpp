#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "ItemStatProfilesScope.h"
#include "TestFiles.h"

#include "Core/Globals/_crypt.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

extern ITEM_ATTRIBUTE* ItemAttribute;

using namespace Data::Items;

// These tests read the data shipped in src/bin/Data, so they also catch
// broken item data in the repo before it is merged.
namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ShippedClientDirectory = DataDirectory.parent_path();

constexpr int KrisType = MakeItemType(0, 0);
constexpr int BladeType = MakeItemType(0, 5);
constexpr int ChaosCastleTicketType = MakeItemType(13, 121);
constexpr int GaionsOrderType = MakeItemType(14, 102);
constexpr BYTE NoSlot = 255;
constexpr WORD ItemFileChecksumKey = 0xE2F1;

// A client folder whose item data is written by the test.
class TemporaryClientFolder
{
public:
    TemporaryClientFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_item_data_" + std::to_string(std::random_device{}())))
    {
        std::filesystem::remove_all(m_directory);
        std::filesystem::create_directories(m_directory / GetItemDataDirectory());
    }

    ~TemporaryClientFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    void WriteItemFile(const std::string& fileName, const std::string& text) const
    {
        std::ofstream(m_directory / GetItemDataDirectory() / fileName, std::ios::binary) << text;
    }

    const std::filesystem::path& Directory() const { return m_directory; }

    // Data/Local/<language>/Item_<language>.bmd in this folder; creates the
    // language folder. CItemDataHandler::GetItemFilePath uses backslashes,
    // which only the game's file functions turn into folders on Linux and
    // macOS, so the path is built from its parts here.
    std::filesystem::path LegacyItemFilePath(const std::wstring& language) const
    {
        const std::filesystem::path folder = m_directory / "Data" / "Local" / language;
        std::filesystem::create_directories(folder);
        return folder / (L"Item_" + language + L".bmd");
    }

    // The shipped Data/Items, and empty folders for the bmd export.
    void CopyShippedItems() const
    {
        std::filesystem::copy(DataDirectory / "Items", m_directory / GetItemDataDirectory(),
                              std::filesystem::copy_options::recursive |
                                  std::filesystem::copy_options::overwrite_existing);
        for (const ItemBmdLanguage& language : GetItemBmdLanguages())
        {
            std::filesystem::create_directories(m_directory / "Data" / "Local" / language.folder);
        }
    }

    // The shipped glow color list, which the item models name their colors
    // from.
    void CopyShippedGlowColors() const
    {
        const std::filesystem::path file = m_directory / GetGlowColorsFile();
        std::filesystem::create_directories(file.parent_path());
        std::filesystem::copy_file(ShippedClientDirectory / GetGlowColorsFile(), file);
    }

private:
    std::filesystem::path m_directory;
};

// Empties the item model database when the test ends, also when a check
// stops it, so the next test starts without models.
class ItemModelDatabaseScope
{
public:
    ItemModelDatabaseScope() = default;
    ~ItemModelDatabaseScope()
    {
        g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
    }

    ItemModelDatabaseScope(const ItemModelDatabaseScope&) = delete;
    ItemModelDatabaseScope& operator=(const ItemModelDatabaseScope&) = delete;
};

// A legacy Item_<language>.bmd with 30-byte names, as the original client
// data has it. The repo does not ship these files, so tests build them.
class LegacyItemFile
{
public:
    LegacyItemFile() : m_records(MAX_ITEM) {}

    ITEM_ATTRIBUTE_FILE_LEGACY& Item(int itemType)
    {
        return m_records[itemType];
    }

    // Like in the original files, a name longer than the name field runs on
    // into the fields after it. Set the fields first.
    void SetName(int itemType, std::string_view name)
    {
        auto* bytes = reinterpret_cast<char*>(&m_records[itemType]);
        std::memcpy(bytes, name.data(), name.size());
        bytes[name.size()] = '\0';
    }

    void Write(const std::filesystem::path& path) const
    {
        std::vector<ITEM_ATTRIBUTE_FILE_LEGACY> encrypted = m_records;
        for (ITEM_ATTRIBUTE_FILE_LEGACY& record : encrypted)
        {
            BuxConvert(reinterpret_cast<BYTE*>(&record), sizeof(record));
        }
        const auto* bytes = reinterpret_cast<const BYTE*>(encrypted.data());
        const DWORD size = static_cast<DWORD>(encrypted.size() * sizeof(ITEM_ATTRIBUTE_FILE_LEGACY));
        const DWORD checksum = GenerateCheckSum2(bytes, size, ItemFileChecksumKey);

        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes), size);
        file.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
    }

private:
    std::vector<ITEM_ATTRIBUTE_FILE_LEGACY> m_records;
};

const ItemDefinition* FindItem(const std::vector<ItemDefinition>& items, int itemType)
{
    const auto found = std::find_if(items.begin(), items.end(), [&](const ItemDefinition& item) {
        return MakeItemType(item.group, item.number) == itemType;
    });
    return found != items.end() ? &*found : nullptr;
}
} // namespace

TEST_CASE("Shipped item data loads without problems [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");

    for (const ItemDataIssue& issue : result.issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
    CHECK(result.items.size() > 900);
}

TEST_CASE("The shipped item names are in languages the game shows [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    std::vector<ItemDataIssue> issues;
    ValidateNameLocales(result.items, I18N::GetAvailableLocales(), issues);
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
}

TEST_CASE("The shipped descriptions' placeholders and shared texts are known [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    const ItemStatsLoadResult stats = LoadItemStatsFile(DataDirectory / "Items" / "Stats" / ItemStatsFileName);
    REQUIRE_FALSE(HasErrors(stats.issues));
    const ItemTooltipLoadResult tooltips =
        LoadItemTooltipFile(DataDirectory / "Items" / "Tooltips" / ItemTooltipFileName);
    REQUIRE_FALSE(HasErrors(tooltips.issues));
    std::vector<ItemDataIssue> issues;
    ValidateDescriptionPlaceholders(result.items, ItemStatProfiles(stats.file), tooltips.file, ItemStatsFileName,
                                    issues);
    ValidateDescriptionTexts(result.items, tooltips.file, ItemTooltipFileName, issues);
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
}

// The shipped tooltip file, read once.
const ItemTooltipFile& ShippedTooltips()
{
    static const ItemTooltipFile file =
        LoadItemTooltipFile(DataDirectory / "Items" / "Tooltips" / ItemTooltipFileName).file;
    return file;
}

// A description line's text: its shared text of the tooltip file, or the text written in place.
const Data::LocalizedString& LineText(const ItemDescriptionLine& line)
{
    static const Data::LocalizedString none;
    if (line.textName.empty())
    {
        return line.text;
    }
    const Data::LocalizedString* text = ShippedTooltips().FindText(line.textName);
    return text != nullptr ? *text : none;
}

// A description line in English as the tooltip fills it for the item at `level`: the values the line binds, the
// item's level values, {level} and the stat file's tables.
std::string FilledLine(const ItemDefinition& item, const ItemDescriptionLine& line, int level,
                       const ItemStatProfiles& profiles)
{
    return FillPlaceholders(LineText(line).GetNeutral(),
                            [&](std::string_view name) -> std::optional<std::string>
                            {
                                for (const auto& [placeholder, value] : line.values)
                                {
                                    if (placeholder == name)
                                    {
                                        name = value;
                                    }
                                }
                                if (const ItemLevelValues* values = item.FindLevelValues(name))
                                {
                                    return level < static_cast<int>(values->values.size())
                                               ? std::optional<std::string>(values->values[level].text.GetNeutral())
                                               : std::nullopt;
                                }
                                if (const std::optional<int> value = GetPlaceholderValue(name, level, profiles))
                                {
                                    return std::to_string(*value);
                                }
                                return std::nullopt;
                            });
}

// The durability lines of the descriptions (step 5b-3b), which came from another part of the old code.
bool IsDurabilityLine(const ItemDescriptionLine& line)
{
    return !line.when.empty() || line.textName == "durability" || line.textName == "numberOfItems" ||
           line.textName == "life" || line.textName == "usableTimes";
}

// The lines the tooltip code wrote for the wings, capes and pets before they were their descriptions, as it printed
// them in English: the old formulas, to compare the data with.
std::vector<std::string> OldWingAndPetLines(int group, int number, int level)
{
    const auto increase = [](int value) { return "Increase " + std::to_string(value) + "% of Damage"; };
    const auto absorb = [](int value) { return "Absorb " + std::to_string(value) + "% of Damage"; };
    const std::string speed = "Increase speed";
    const auto in = [&](std::initializer_list<int> numbers)
    { return std::find(numbers.begin(), numbers.end(), number) != numbers.end(); };
    if (group == 12 && in({0, 1, 2, 41, 131, 132, 133, 134}))
    {
        return {increase(12 + level * 2), absorb(12 + level * 2), speed};
    }
    if (group == 12 && in({3, 4, 5, 6, 42}))
    {
        return {increase(32 + level), absorb(25 + level * 2), speed};
    }
    if (group == 12 && in({36, 37, 38, 39, 43}))
    {
        return {increase(39 + level * 2), absorb(39 + level * 2), speed};
    }
    if (group == 12 && in({40, 50}))
    {
        return {increase(39 + level * 2), absorb(24 + level * 2), speed};
    }
    if (group == 12 && in({130, 135}))
    {
        return {increase(20 + level * 2), absorb(20 + level * 2), speed};
    }
    if (group == 12 && number == 49)
    {
        return {increase(20 + level * 2), absorb(10 + level * 2)};
    }
    if (group == 13 && number == 30)
    {
        return {increase(20 + level * 2), absorb(10 + level)};
    }
    if (group == 13 && number == 3)
    {
        return {increase(15), absorb(10)};
    }
    if (group == 13 && number == 0)
    {
        return {absorb(20), "Max HP +50 increased"};
    }
    if (group == 13 && number == 1)
    {
        return {"Increase 30% of attacking & Wizardry Dmg"};
    }
    return {};
}

TEST_CASE("The wings', capes' and pets' tooltip values come from their data as from the old formulas [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    const ItemStatsLoadResult stats = LoadItemStatsFile(DataDirectory / "Items" / "Stats" / ItemStatsFileName);
    REQUIRE_FALSE(HasErrors(stats.issues));
    const ItemStatProfiles profiles(stats.file);
    int compared = 0;
    for (const ItemDefinition& item : result.items)
    {
        if (OldWingAndPetLines(item.group, item.number, 0).empty())
        {
            continue;
        }
        for (int level = 0; level < ItemLevelCount; ++level)
        {
            INFO(item.group << "/" << item.number << " +" << level);
            std::vector<std::string> lines;
            for (const ItemDescriptionLine& line : item.description)
            {
                if (!IsDurabilityLine(line))
                {
                    lines.push_back(FilledLine(item, line, level, profiles));
                }
            }
            CHECK(lines == OldWingAndPetLines(item.group, item.number, level));
            ++compared;
        }
    }
    // 24 wings and capes, Horn of Dinorant, Guardian Angel and Imp, at each level.
    CHECK(compared == 27 * ItemLevelCount);
}

// The lines the tooltip code wrote by item level before they were descriptions, in English with their colors.
struct OldLine
{
    std::string text;
    ItemTextColor color = ItemTextColor::White;
    bool operator==(const OldLine& other) const = default;
};

std::vector<OldLine> OldLevelLines(int group, int number, int level)
{
    using enum ItemTextColor;
    const auto one = [&](std::initializer_list<std::pair<int, OldLine>> byLevel) -> std::vector<OldLine>
    {
        for (const auto& [lineLevel, line] : byLevel)
        {
            if (lineLevel == level)
            {
                return {line};
            }
        }
        return {};
    };
    if (group == 13 && number == 7)
    {
        return one({{0, {"Archer", Yellow}}, {1, {"Spearman", Yellow}}});
    }
    if (group == 13 && number == 11)
    {
        return one({{0, {"Scroll of Guardian", Yellow}}, {1, {"Place Life Stone", Yellow}}});
    }
    if (group == 13 && number == 14)
    {
        return one({{0, {"Used to upgrade wings"}}, {1, {"Used in combining Cape of Lord & Warrior's Cloak"}}});
    }
    if (group == 13 && number == 15 && level <= 4)
    {
        constexpr const char* stats[] = {"ENG", "STA", "AGI", "STR", "Command"};
        std::vector<OldLine> lines = {{std::string(stats[level]) + " Increases 1~3 stat points"},
                                      {std::string(stats[level]) + " Possible to decrease stat 1~9 point"},
                                      {"It can be used with item removed.", DarkRed}};
        if (level == 4)
        {
            lines.push_back({"Can be equipped by Dark Lord"});
        }
        return lines;
    }
    if (group == 13 && (number == 16 || number == 17))
    {
        return {{"Used when creating a Cloak of Invisibility"}};
    }
    if (group == 13 && number == 18)
    {
        return {{"Used when entering Blood Castle"},
                {""},
                {"The remaining time is shown"},
                {"when you right click on your mouse."}};
    }
    if (group == 13 && number == 31)
    {
        return one({{0, {"Used in Dark Horse resurrection"}}, {1, {"Used in Dark Raven resurrection"}}});
    }
    if (group == 14 && number == 7 && level == 0)
    {
        return {{"Damage +20% increase effect", DarkRed},
                {"Duration 60 seconds", DarkRed},
                {"Only applicable for castle gate and statue", DarkRed}};
    }
    if (group == 14 && number == 7 && level == 1)
    {
        return {{"Increase +8 AG recovery speed", DarkRed},
                {"Increase resistance of Lightning and Ice", DarkRed},
                {"Attacking speed will increase +20", DarkRed}};
    }
    if (group == 14 && number == 10 && level >= 1 && level <= 8)
    {
        return {{"Warp to the corresponding area after 3 seconds"}};
    }
    if (group == 14 && number == 21)
    {
        return one({{1, {"Used in the online event"}},
                    {2, {"Used in the My Friend event."}},
                    {3, {"Use in siege registration"}}});
    }
    return {};
}

TEST_CASE("The tooltip texts by item level come from the descriptions as from the old code [data][items]")
{
    constexpr std::pair<int, int> items[] = {{13, 7},  {13, 11}, {13, 14}, {13, 15}, {13, 16}, {13, 17},
                                             {13, 18}, {13, 31}, {14, 7},  {14, 10}, {14, 21}};
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    const ItemStatsLoadResult stats = LoadItemStatsFile(DataDirectory / "Items" / "Stats" / ItemStatsFileName);
    REQUIRE_FALSE(HasErrors(stats.issues));
    const ItemStatProfiles profiles(stats.file);
    for (const auto& [group, number] : items)
    {
        const auto found = std::find_if(result.items.begin(), result.items.end(), [&](const ItemDefinition& item)
                                        { return item.group == group && item.number == number; });
        REQUIRE(found != result.items.end());
        // Every level the items have. The old code also wrote an empty line at the Contract Summon's, Life Stone's and
        // Rena's levels without a text (a stale line of the text list); the data writes none.
        for (int level = 0; level < ItemLevelCount; ++level)
        {
            INFO(group << "/" << number << " +" << level);
            std::vector<OldLine> lines;
            for (const ItemDescriptionLine& line : found->description)
            {
                if (line.IsShownAt(level) && !IsDurabilityLine(line))
                {
                    lines.push_back({FilledLine(*found, line, level, profiles), line.color});
                }
            }
            CHECK(lines == OldLevelLines(group, number, level));
        }
    }
}

// The lines the second item chain of the tooltip code (seals, talismans, tickets, rings, ...) wrote, in English with
// their colors: the item data's description starts with them.
struct OldChainItem
{
    int group;
    int number;
    std::vector<OldLine> lines;
};

const std::vector<OldChainItem>& OldChainLines()
{
    static const std::vector<OldChainItem> items = {
        {14, 54, {{"You can achieve special items with combinations.", ItemTextColor::Blue}}},
        {14, 58, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 59, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 60, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 61, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 62, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 83, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 145, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 146, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 147, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 148, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 149, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 150, {{"Congratulations. Please contact CS team and change it to item.", ItemTextColor::Blue}}},
        {14, 53, {{"Increases the combination rate,but only up to the maximum rate.", ItemTextColor::Blue}}},
        {13,
         43,
         {{"Increases experience gained.", ItemTextColor::Blue},
          {"Warp Command Window available.", ItemTextColor::Blue},
          {"Not applicable to", ItemTextColor::Yellow},
          {"Master Level Characters.", ItemTextColor::Yellow}}},
        {13,
         44,
         {{"Increases experience gained and item drop rate.", ItemTextColor::Blue},
          {"Warp Command Window available.", ItemTextColor::Blue},
          {"Not applicable to", ItemTextColor::Yellow},
          {"Master Level Characters.", ItemTextColor::Yellow}}},
        {13,
         45,
         {{"Prevents experiences to be gained.", ItemTextColor::Blue},
          {"Warp Command Window available.", ItemTextColor::Blue},
          {"Master Level EXP cannot be achieved during the item usage.", ItemTextColor::Yellow}}},
        {13, 59, {{"You may freely move onward.", ItemTextColor::Blue}}},
        {13, 60, {{"It reduces the killing rate.", ItemTextColor::Blue}}},
        {14, 140, {{"Automatic HP recovery 3%", ItemTextColor::Blue}, {"Max. HP increase +100", ItemTextColor::Blue}}},
        {14,
         96,
         {{"Minimum +10 - +15 level item upgrade", ItemTextColor::Blue},
          {"blocks the item dissipation.", ItemTextColor::Purple},
          {"+10~+15 When upgrading level item please put it in combination window.", ItemTextColor::Purple}}},
        {13,
         64,
         {{"Increases Attack Power and Wizardry by 40%", ItemTextColor::Blue},
          {"Increases Attack Speed by 10", ItemTextColor::Purple}}},
        {13,
         65,
         {{"Alleviates monster's damage by 30%", ItemTextColor::Blue},
          {"Increases Maximum Life by 50", ItemTextColor::Purple}}},
        {13, 67, {{"Surrounding Zens are automatically collected.", ItemTextColor::Blue}}},
        {13,
         123,
         {{"Surrounding Zens are automatically collected.", ItemTextColor::Blue},
          {"increases Damage, Wizardry and Curse by 20%", ItemTextColor::Blue},
          {"Increases Attack Speed by 10", ItemTextColor::Blue},
          {"and EXP by 30%.", ItemTextColor::Blue}}},
        {13,
         80,
         {{"Auto-collects zen around you.", ItemTextColor::Blue},
          {"EXP rate 50% increase", ItemTextColor::Blue},
          {"Increase Defensive Skill +50", ItemTextColor::Blue}}},
        {13,
         106,
         {{"Auto-collects zen around you.", ItemTextColor::Blue},
          {"Zen increase 50%", ItemTextColor::Blue},
          {"Increase Defensive Skill +50", ItemTextColor::Blue}}},
        {13, 107, {{"Cannot Repair", ItemTextColor::Red}}},
        {13, 104, {{"Increase Max. AG + Level", ItemTextColor::Blue}}},
        {13, 105, {{"Increase Max. SD + Levelx10", ItemTextColor::Blue}}},
        {13,
         103,
         {{"Up to 170% EXP gain increase, depending on the number of members in your party.", ItemTextColor::Blue}}},
        {13, 70, {{"Save the application location.", ItemTextColor::Blue}}},
        {13,
         81,
         {{"Exp and item will be secured when character dies.", ItemTextColor::Blue},
          {"No penalty for dying.", ItemTextColor::Blue},
          {"Right click to use.", ItemTextColor::Blue}}},
        {13,
         82,
         {{"Item durability will not be decrease for a certain period.", ItemTextColor::Blue},
          {"Keeps item durable", ItemTextColor::Blue},
          {"Applicable to mountable items only besides pet.", ItemTextColor::Blue}}},
        {13,
         93,
         {{"Increases experience gained.", ItemTextColor::Blue},
          {"Warp Command Window available.", ItemTextColor::Blue}}},
        {13,
         94,
         {{"Increases experience gained and item drop rate.", ItemTextColor::Blue},
          {"Warp Command Window available.", ItemTextColor::Blue}}},
        {13,
         61,
         {{"Enables entrance into Illusion Temple.", ItemTextColor::Blue},
          {"You will be assigned to a stage according to your level.", ItemTextColor::Blue},
          {"", ItemTextColor::White}}},
        {14, 84, {{"Drop it to receive the gift.", ItemTextColor::Blue}}},
        {13, 49, {{"Assemble the Scroll of Blood with the contract from the Illusion Sorcery.", ItemTextColor::Blue}}},
        {13, 50, {{"Assemble the Scroll of Blood with the old scrolls.", ItemTextColor::Blue}}},
        {13, 51, {{"This is a mark of Illusion Sorcery; this is required to enter the temple.", ItemTextColor::Blue}}},
        {14, 64, {{"Mobility speed reduces upon achievement.", ItemTextColor::Blue}}},
        {13, 52, {{"Ingredients for the 3rd wing assembly.", ItemTextColor::White}}},
        {13, 53, {{"Ingredients for the 3rd wing assembly.", ItemTextColor::White}}},
        {13, 38, {{"Cannot Repair", ItemTextColor::Red}}},
        {14,
         111,
         {{"That's the only thing that will get Lugard to help you", ItemTextColor::DarkBlue},
          {"enter the Doppelganger area.", ItemTextColor::DarkBlue}}},
        {13,
         109,
         {{"Automatic HP recovery 3%", ItemTextColor::White}, {"Increases Max Mana 4%", ItemTextColor::White}}},
        {13, 110, {{"Automatic HP recovery 3%", ItemTextColor::White}, {"Increase Max HP +4%", ItemTextColor::White}}},
        {13,
         111,
         {{"Automatic HP recovery 3%", ItemTextColor::White},
          {"Increases acquisition rate of Zen after hunting monsters +30%", ItemTextColor::White}}},
        {13, 112, {{"Automatic HP recovery 3%", ItemTextColor::White}, {"Damage Decrease +4%", ItemTextColor::White}}},
        {13,
         113,
         {{"Automatic HP recovery 3%", ItemTextColor::White}, {"Excellent Damage rate +10%", ItemTextColor::White}}},
        {13,
         114,
         {{"Automatic HP recovery 3%", ItemTextColor::White}, {"Attack Speed increase +7", ItemTextColor::White}}},
        {13,
         115,
         {{"Automatic HP recovery 3%", ItemTextColor::White},
          {"Increases acquisition rate of Mana after hunting monsters +Mana/8", ItemTextColor::White}}},
        {14,
         112,
         {{"You can do a Goblin combination with a Sealed Silver Box to create a Silver Box.", ItemTextColor::White}}},
        {14,
         113,
         {{"You can do a Goblin combination with a Sealed Golden Box to create a Golden Box.", ItemTextColor::White}}},
        {14, 120, {{"You can acquire Goblin Points by using the MU Item Shop's storage.", ItemTextColor::White}}},
        {14, 121, {{"You can do a Goblin combination with a Gold Key to create a Golden Box.", ItemTextColor::White}}},
        {14,
         122,
         {{"You can do a Goblin combination with a Silver Key to create a Silver Box.", ItemTextColor::White}}},
        {14, 123, {{"You can drop it with a fixed probability of it turning into a rare item.", ItemTextColor::White}}},
        {14, 124, {{"You can drop it with a fixed probability of it turning into a rare item.", ItemTextColor::White}}},
        {14, 134, {{"It's a box containing various items.", ItemTextColor::White}}},
        {14, 135, {{"It's a box containing various items.", ItemTextColor::White}}},
        {14, 136, {{"It's a box containing various items.", ItemTextColor::White}}},
        {14, 137, {{"It's a box containing various items.", ItemTextColor::White}}},
        {14, 138, {{"It's a box containing various items.", ItemTextColor::White}}},
        {14, 139, {{"It's a box containing various items.", ItemTextColor::White}}},
        {13, 116, {{"Boosts the item drop rate.", ItemTextColor::Blue}}},
        {14, 141, {{"Throw it and you may receive some Zen or items", ItemTextColor::White}}},
        {14, 142, {{"Throw it and you may receive some Zen or items", ItemTextColor::White}}},
        {14, 143, {{"Throw it and you may receive some Zen or items", ItemTextColor::White}}},
        {14, 144, {{"Throw it and you may receive some Zen or items", ItemTextColor::White}}},
        {14,
         19,
         {{"The remaining time is shown", ItemTextColor::White},
          {"when you right click on your mouse.", ItemTextColor::White}}},
    };
    return items;
}

TEST_CASE("The second item chain's tooltip texts come from the descriptions as from the old code [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    for (const OldChainItem& old : OldChainLines())
    {
        INFO(old.group << "/" << old.number);
        const auto found = std::find_if(result.items.begin(), result.items.end(), [&](const ItemDefinition& item)
                                        { return item.group == old.group && item.number == old.number; });
        REQUIRE(found != result.items.end());
        REQUIRE(found->description.size() >= old.lines.size());
        for (size_t index = 0; index < old.lines.size(); ++index)
        {
            const ItemDescriptionLine& line = found->description[index];
            CHECK(OldLine{LineText(line).GetNeutral(), line.color} == old.lines[index]);
            CHECK_FALSE(line.HasLevels());
        }
    }
    CHECK(OldChainLines().size() == 72);
}

// The texts the tooltip code wrote for these items are their descriptions now, in every UI language.
TEST_CASE("The original items' tooltip texts are in their descriptions [data][items]")
{
    struct DescribedItem
    {
        int group;
        int number;
        std::string_view text;
    };
    constexpr DescribedItem described[] = {
        {12, 15, "It is used to combine Chaos items"},
        {13, 38, "I.D. of Kantur Chief Scientist. You can enter the Refinery Tower."},
        {14, 13, "It is used to increase your item level up to 6"},
        {14, 14, "It is used to increase your item level up to 7,8,9"},
        {14, 16, "Increases item option by 1 level"},
        {14, 17, "It is used to combine items for a Devil Square Invitation"},
        {14, 18, "It is used to combine items for a Devil Square Invitation"},
        {14, 22, "Used to create fruits that increase stats"},
        {14, 31, "Create and improve items for siege"},
        {14, 41, "Jewel with impurities"},
        {14, 42, "Jewel for item reinforcement"},
        {14, 43, "Grant actual power to reinforced item."},
        {14, 44, "Grant actual power to reinforced item."},
        {14, 160, "Jewel used for repairing a Lucky Item."},
        {14, 161, "Jewel for item reinforcement"},
    };
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    for (const DescribedItem& item : described)
    {
        INFO(item.group << "/" << item.number);
        const auto found = std::find_if(result.items.begin(), result.items.end(), [&](const ItemDefinition& definition)
                                        { return definition.group == item.group && definition.number == item.number; });
        REQUIRE(found != result.items.end());
        // The last line: the Moonstone Pendant's "Cannot Repair" of the second item chain comes before it.
        REQUIRE_FALSE(found->description.empty());
        const Data::LocalizedString& text = LineText(found->description.back());
        CHECK(text.GetNeutral() == item.text);
        CHECK(found->description.back().color == ItemTextColor::White);
        for (const char* locale : I18N::GetAvailableLocales())
        {
            INFO(locale);
            CHECK_FALSE(text.Get(locale).empty());
            if (std::string_view(locale) != Data::LocalizedString::NeutralLocale)
            {
                CHECK(text.GetTranslations().contains(locale));
            }
        }
    }
}

TEST_CASE("Loading item data fills ItemAttribute in the UI locale [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");

    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    CHECK(g_ItemDatabase.GetLogName(KrisType) == "Kris (0,0)");
    CHECK(client.Name(BladeType) == L"Blade");

    I18N::SetLocale("pt");
    CHECK(client.Name(BladeType) == L"Lâmina");
    CHECK(g_ItemDatabase.GetLogName(BladeType) == "Blade (0,5)");

    I18N::SetLocale("de");
    CHECK(client.Name(BladeType) == L"Klinge");
    // The German name is the English one.
    CHECK(client.Name(KrisType) == L"Kris");
}

TEST_CASE("Item data errors stop loading [data][items]")
{
    TemporaryClientFolder folder;
    folder.WriteItemFile("Group00_Sword.json", R"({"formatVersion": 1, "group": 0, "items": [{"number": 0}]})");
    ClientDataScope client(folder.Directory());

    std::string errorMessage;
    CHECK_FALSE(g_ItemDataHandler.Load(errorMessage));
    CHECK(errorMessage.find("Group00_Sword.json") != std::string::npos);
}

// The bmd import and the item editor keep a weapon kind that its group gives
// following the group and slot; every shipped item has that kind.
TEST_CASE("The shipped weapon kinds are the ones their groups give [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        CAPTURE(definition.group);
        CAPTURE(definition.number);
        CHECK(definition.weaponKind == GetGroupWeaponKind(definition));
    }
}

TEST_CASE("Saved item data loads back the same [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    TemporaryClientFolder folder;
    const std::filesystem::path savedDirectory = folder.Directory() / GetItemDataDirectory();

    std::vector<ItemDataIssue> issues;
    REQUIRE(SaveItemDataDirectory(savedDirectory, shipped.items, issues) == ItemDataSaveResult::Saved);

    const ItemDataLoadResult saved = LoadItemDataDirectory(savedDirectory);
    CHECK(saved.issues.empty());
    CHECK(saved.items.size() == shipped.items.size());
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        const std::string fileName = GetItemGroupFileName(group);
        std::ifstream shippedFile(DataDirectory / "Items" / fileName, std::ios::binary);
        std::ifstream savedFile(savedDirectory / fileName, std::ios::binary);
        const std::string shippedText((std::istreambuf_iterator<char>(shippedFile)), std::istreambuf_iterator<char>());
        const std::string savedText((std::istreambuf_iterator<char>(savedFile)), std::istreambuf_iterator<char>());
        INFO(fileName);
        CHECK(savedText == shippedText);
    }
}

TEST_CASE("Bmd import recovers long names and repairs the fields they overwrote [data][items]")
{
    TemporaryClientFolder folder;
    LegacyItemFile english;
    english.Item(ChaosCastleTicketType).m_byItemSlot = NoSlot;
    // The name runs on into TwoHand, Level and, with its null byte, the slot.
    english.SetName(ChaosCastleTicketType, "Open Access Ticket to Chaos Castle");
    english.Write(folder.LegacyItemFilePath(L"Eng"));
    LegacyItemFile portuguese;
    portuguese.Item(ChaosCastleTicketType).m_byItemSlot = NoSlot;
    portuguese.SetName(ChaosCastleTicketType, "Ingresso Castelo");
    portuguese.Write(folder.LegacyItemFilePath(L"Por"));
    ClientDataScope client(folder.Directory());

    const ItemBmdImportResult result = ImportItemBmdFiles();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK(result.importedLocales == std::vector<std::string>{"en", "pt"});
    CHECK(result.recoveredNameCount == 1);

    const ItemDefinition* ticket = FindItem(result.items, ChaosCastleTicketType);
    REQUIRE(ticket != nullptr);
    CHECK(ticket->names.GetNeutral() == "Open Access Ticket to Chaos Castle");
    CHECK_FALSE(ticket->twoHanded);
    CHECK(ticket->level == 0);
    CHECK(ticket->slot == ItemSlot::None);
    REQUIRE_FALSE(result.repairs.empty());
    CHECK(result.repairs.front().fromLocale == "pt");
}

TEST_CASE("Bmd import reads Portuguese and Spanish names as Windows-1252 [data][items]")
{
    TemporaryClientFolder folder;
    LegacyItemFile english;
    english.SetName(BladeType, "Blade");
    english.SetName(GaionsOrderType, "Gaion\xA1\xAFs Order"); // A1 AF: a right quote on Korean systems
    english.Write(folder.LegacyItemFilePath(L"Eng"));
    LegacyItemFile portuguese;
    portuguese.SetName(BladeType, "L\xE2mina");
    portuguese.Write(folder.LegacyItemFilePath(L"Por"));
    ClientDataScope client(folder.Directory());

    const ItemBmdImportResult result = ImportItemBmdFiles();
    REQUIRE_FALSE(HasErrors(result.issues));

    const ItemDefinition* blade = FindItem(result.items, BladeType);
    REQUIRE(blade != nullptr);
    CHECK(blade->names.Get("pt") == "L\xC3\xA2mina");

    const ItemDefinition* gaionsOrder = FindItem(result.items, GaionsOrderType);
    REQUIRE(gaionsOrder != nullptr);
    CHECK(gaionsOrder->names.GetNeutral() == "Gaion's Order");
}

TEST_CASE("A folder that cannot be written is a write failure, not a data error [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    TemporaryClientFolder folder;
    // A file where the item folder should be: nothing can be written there.
    const std::filesystem::path blocked = folder.Directory() / "blocked";
    std::ofstream(blocked) << "not a folder";

    std::vector<ItemDataIssue> issues;
    CHECK(SaveItemDataDirectory(blocked, shipped.items, issues) == ItemDataSaveResult::WriteFailed);
    CHECK_FALSE(issues.empty());
}

// The items and their models are saved together: when one of the files
// cannot be written, none is replaced, so a moved item never loses its model
// on disk.
TEST_CASE("A save that cannot write all its files replaces none of them [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    REQUIRE_FALSE(HasErrors(shipped.issues));
    const ItemModelDataLoadResult shippedModels = LoadItemModelDataDirectory(DataDirectory / "Items" / "Models");
    REQUIRE_FALSE(HasErrors(shippedModels.issues));
    ItemModelDatabase models;
    models.Build(shippedModels.models, Data::Effects::GlowColorList{});
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    const std::filesystem::path itemDirectory = folder.Directory() / GetItemDataDirectory();
    // Kris moves to a free number, so its group file changes.
    std::vector<ItemDefinition> items = shipped.items;
    const auto kris = std::find_if(items.begin(), items.end(), [](const ItemDefinition& item)
                                   { return MakeItemType(item.group, item.number) == KrisType; });
    REQUIRE(kris != items.end());
    kris->number = 100;
    // A file where the model folder should be: no model file can be written.
    const std::filesystem::path blockedModels = folder.Directory() / "blocked";
    std::ofstream(blockedModels) << "not a folder";
    ItemGroupSet modelGroups;
    modelGroups.set(0);

    std::vector<ItemDataIssue> issues;
    CHECK(SaveItemAndModelDataDirectories(itemDirectory, items, blockedModels, models.GetAllSlots(), modelGroups,
                                          issues) == ItemDataSaveResult::WriteFailed);
    CHECK_FALSE(issues.empty());
    const std::string swordFile = GetItemGroupFileName(0);
    CHECK(TestFiles::ReadWholeFile(itemDirectory / swordFile) ==
          TestFiles::ReadWholeFile(DataDirectory / "Items" / swordFile));
    CHECK_FALSE(std::filesystem::exists(itemDirectory / (swordFile + ".tmp")));
}

#ifdef _EDITOR
TEST_CASE("Item editor changes go into the item database [data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    SUBCASE("a changed value is copied")
    {
        ItemAttribute[BladeType].Width = 2;
        g_ItemDataHandler.OnItemEdited(BladeType);

        CHECK(g_ItemDatabase.Find(BladeType)->width == 2);
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina");
    }

    SUBCASE("a changed name is stored for the current UI locale")
    {
        I18N::SetLocale("pt");
        const std::wstring newName = L"Lâmina Nova";
        std::fill(std::begin(ItemAttribute[BladeType].Name), std::end(ItemAttribute[BladeType].Name), L'\0');
        newName.copy(ItemAttribute[BladeType].Name, newName.size());
        g_ItemDataHandler.OnItemEdited(BladeType);

        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina Nova");
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == "Blade");
    }

    SUBCASE("a name set for another language goes into the database, not into the shown name")
    {
        REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "pl", "Ostrze"));
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pl") == "Ostrze");
        CHECK(std::wstring(ItemAttribute[BladeType].Name) == L"Blade");
    }

    SUBCASE("a name set for the shown language is shown, also when it is longer than the game shows")
    {
        const std::string longName(60, 'B');
        REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "en", longName));
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == longName);
        CHECK(std::wstring(ItemAttribute[BladeType].Name) == std::wstring(MAX_ITEM_NAME - 1, L'B'));
    }

    SUBCASE("an empty name removes a translation, but not the English name")
    {
        REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "pt", ""));
        CHECK_FALSE(g_ItemDatabase.Find(BladeType)->names.GetTranslations().contains("pt"));
        CHECK_FALSE(g_ItemDataHandler.SetItemName(BladeType, "en", ""));
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == "Blade");
        CHECK_FALSE(g_ItemDataHandler.SetItemName(MakeItemType(0, 500), "en", "Nothing"));
    }

    SUBCASE("a name with the separator is refused, since the save would refuse it")
    {
        CHECK_FALSE(g_ItemDataHandler.SetItemName(BladeType, "pt", "L\xC3\xA2mina||de=Klinge"));
        CHECK_FALSE(g_ItemDataHandler.SetItemName(BladeType, "en", "Bl||ade"));
        CHECK(g_ItemDatabase.Find(BladeType)->names.GetNeutral() == "Blade");
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina");
    }

    SUBCASE("an item without an English name gets that first, so it cannot lose its last name")
    {
        ItemDefinition untranslated = *g_ItemDatabase.Find(KrisType);
        untranslated.names = Data::LocalizedString{};
        untranslated.names.Set("pt", "Cris");
        g_ItemDatabase.Set(untranslated);

        CHECK_FALSE(g_ItemDataHandler.SetItemName(KrisType, "pt", ""));
        CHECK_FALSE(g_ItemDataHandler.SetItemName(KrisType, "es", "Daga"));
        REQUIRE(g_ItemDatabase.Find(KrisType) != nullptr);
        CHECK(g_ItemDatabase.Find(KrisType)->names.Get("pt") == "Cris");
        CHECK(g_ItemDataHandler.SetItemName(KrisType, "en", "Kris"));
    }

    SUBCASE("a value change does not turn the English fallback into a translation")
    {
        // No item has a Polish name: the English one is shown.
        I18N::SetLocale("pl");
        REQUIRE(client.Name(KrisType) == L"Kris");
        ItemAttribute[KrisType].Width = 2;
        g_ItemDataHandler.OnItemEdited(KrisType);

        CHECK_FALSE(g_ItemDatabase.Find(KrisType)->names.GetTranslations().contains("pl"));
    }

    SUBCASE("a value change keeps a name longer than ITEM_ATTRIBUTE can hold")
    {
        const std::string longName(MAX_ITEM_NAME + 10, 'x');
        ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        kris.names.Set("en", longName);
        g_ItemDatabase.Set(kris);
        ToItemAttribute(*g_ItemDatabase.Find(KrisType), ItemAttribute[KrisType]);

        ItemAttribute[KrisType].Width = 2;
        g_ItemDataHandler.OnItemEdited(KrisType);

        CHECK(g_ItemDatabase.Find(KrisType)->names.GetNeutral() == longName);
        CHECK(g_ItemDatabase.Find(KrisType)->width == 2);
    }

    SUBCASE("a new weapon gets its group's weapon kind")
    {
        const int newSpearType = MakeItemType(3, 100);
        REQUIRE(g_ItemDatabase.Find(newSpearType) == nullptr);
        const std::wstring name = L"New Spear";
        std::fill(std::begin(ItemAttribute[newSpearType].Name), std::end(ItemAttribute[newSpearType].Name), L'\0');
        name.copy(ItemAttribute[newSpearType].Name, name.size());
        ItemAttribute[newSpearType].m_byItemSlot = static_cast<BYTE>(ItemSlot::MainHand);
        g_ItemDataHandler.OnItemEdited(newSpearType);

        REQUIRE(g_ItemDatabase.Find(newSpearType) != nullptr);
        CHECK(g_ItemDatabase.GetWeaponKind(newSpearType) == WeaponKind::Spear);
    }

    SUBCASE("a new bow is a bow once it is two-handed and on offHand, in any order")
    {
        const int newBowType = MakeItemType(4, 100);
        REQUIRE(g_ItemDatabase.Find(newBowType) == nullptr);
        const std::wstring name = L"New Bow";
        std::fill(std::begin(ItemAttribute[newBowType].Name), std::end(ItemAttribute[newBowType].Name), L'\0');
        name.copy(ItemAttribute[newBowType].Name, name.size());
        ItemAttribute[newBowType].m_byItemSlot = static_cast<BYTE>(ItemSlot::MainHand);
        g_ItemDataHandler.OnItemEdited(newBowType);
        ItemAttribute[newBowType].TwoHand = true;
        g_ItemDataHandler.OnItemEdited(newBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(newBowType) == WeaponKind::Crossbow);

        ItemAttribute[newBowType].m_byItemSlot = static_cast<BYTE>(ItemSlot::OffHand);
        g_ItemDataHandler.OnItemEdited(newBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(newBowType) == WeaponKind::Bow);

        // The other order: on offHand first, then two-handed.
        const int otherBowType = MakeItemType(4, 101);
        REQUIRE(g_ItemDatabase.Find(otherBowType) == nullptr);
        std::fill(std::begin(ItemAttribute[otherBowType].Name), std::end(ItemAttribute[otherBowType].Name), L'\0');
        name.copy(ItemAttribute[otherBowType].Name, name.size());
        ItemAttribute[otherBowType].m_byItemSlot = static_cast<BYTE>(ItemSlot::OffHand);
        g_ItemDataHandler.OnItemEdited(otherBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(otherBowType) == WeaponKind::None);
        ItemAttribute[otherBowType].TwoHand = true;
        g_ItemDataHandler.OnItemEdited(otherBowType);
        CHECK(g_ItemDatabase.GetWeaponKind(otherBowType) == WeaponKind::Bow);
    }

    SUBCASE("a moved weapon's kind follows its new group")
    {
        const int emptySpearType = MakeItemType(3, 100);
        const int emptySwordType = MakeItemType(0, 100);
        REQUIRE(g_ItemDatabase.Find(emptySpearType) == nullptr);
        REQUIRE(g_ItemDatabase.Find(emptySwordType) == nullptr);

        std::swap(ItemAttribute[KrisType], ItemAttribute[emptySpearType]);
        g_ItemDataHandler.OnItemsSwapped(KrisType, emptySpearType);
        CHECK(g_ItemDatabase.GetWeaponKind(emptySpearType) == WeaponKind::Spear);

        std::swap(ItemAttribute[ITEM_SPEAR], ItemAttribute[emptySwordType]);
        g_ItemDataHandler.OnItemsSwapped(ITEM_SPEAR, emptySwordType);
        CHECK(g_ItemDatabase.GetWeaponKind(emptySwordType) == WeaponKind::Sword);
    }

    SUBCASE("a sword is a glove while only the Rage Fighter may use it")
    {
        REQUIRE(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Sword);
        const ITEM_ATTRIBUTE kris = ItemAttribute[KrisType];
        for (int characterClass = 0; characterClass < MAX_CLASS; ++characterClass)
        {
            ItemAttribute[KrisType].RequireClass[characterClass] = characterClass == CLASS_RAGEFIGHTER ? 1 : 0;
        }
        g_ItemDataHandler.OnItemEdited(KrisType);
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Glove);

        ItemAttribute[KrisType] = kris;
        g_ItemDataHandler.OnItemEdited(KrisType);
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Sword);
    }

    SUBCASE("a staff is a book while it is on offHand")
    {
        REQUIRE(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Staff);
        ItemAttribute[ITEM_STAFF].m_byItemSlot = static_cast<BYTE>(ItemSlot::OffHand);
        g_ItemDataHandler.OnItemEdited(ITEM_STAFF);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Book);

        ItemAttribute[ITEM_STAFF].m_byItemSlot = static_cast<BYTE>(ItemSlot::MainHand);
        g_ItemDataHandler.OnItemEdited(ITEM_STAFF);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Staff);
    }

    SUBCASE("an undone edit undoes the weapon kind")
    {
        ItemAttribute[ITEM_BOLT].TwoHand = true;
        g_ItemDataHandler.OnItemEdited(ITEM_BOLT);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_BOLT) == WeaponKind::Bow);

        ItemAttribute[ITEM_BOLT].TwoHand = false;
        g_ItemDataHandler.OnItemEdited(ITEM_BOLT);
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_BOLT) == WeaponKind::None);
    }

    SUBCASE("a moved shield's stat profile follows its weapon kind")
    {
        const int emptySwordType = MakeItemType(0, 100);
        REQUIRE(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile == "shield");
        std::swap(ItemAttribute[ITEM_SMALL_SHIELD], ItemAttribute[emptySwordType]);
        g_ItemDataHandler.OnItemsSwapped(ITEM_SMALL_SHIELD, emptySwordType);
        CHECK(g_ItemDatabase.GetWeaponKind(emptySwordType) == WeaponKind::None);
        CHECK(g_ItemDatabase.Find(emptySwordType)->statProfile.empty());
    }

    SUBCASE("the Horn of Fenrir keeps the default its file names")
    {
        REQUIRE(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->statProfile == "default");
        ItemAttribute[ITEM_HORN_OF_FENRIR].RequireLevel = 0;
        g_ItemDataHandler.OnItemEdited(ITEM_HORN_OF_FENRIR);
        ItemAttribute[ITEM_HORN_OF_FENRIR].RequireLevel = 300;
        g_ItemDataHandler.OnItemEdited(ITEM_HORN_OF_FENRIR);
        CHECK(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->statProfile == "default");
    }

    SUBCASE("an item without a stat profile takes the one its data gives when edited")
    {
        ItemDefinition shield = *g_ItemDatabase.Find(ITEM_SMALL_SHIELD);
        shield.statProfile.clear();
        g_ItemDatabase.Set(shield);
        g_ItemDataHandler.OnItemEdited(ITEM_SMALL_SHIELD);
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile == "shield");
    }

    SUBCASE("a wing's stat profile follows its slot")
    {
        REQUIRE(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "firstTierWing");
        ItemAttribute[ITEM_WINGS_OF_ELF].m_byItemSlot = static_cast<BYTE>(ItemSlot::Helm);
        g_ItemDataHandler.OnItemEdited(ITEM_WINGS_OF_ELF);
        CHECK(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "armor");

        ItemAttribute[ITEM_WINGS_OF_ELF].m_byItemSlot = static_cast<BYTE>(ItemSlot::Wings);
        g_ItemDataHandler.OnItemEdited(ITEM_WINGS_OF_ELF);
        CHECK(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "firstTierWing");
    }

    SUBCASE("a moved item keeps all its names")
    {
        const int emptyType = MakeItemType(0, 100);
        REQUIRE(g_ItemDatabase.Find(emptyType) == nullptr);
        std::swap(ItemAttribute[BladeType], ItemAttribute[emptyType]);
        g_ItemDataHandler.OnItemsSwapped(BladeType, emptyType);

        const ItemDefinition* moved = g_ItemDatabase.Find(emptyType);
        REQUIRE(moved != nullptr);
        CHECK(moved->names.Get("pt") == "L\xC3\xA2mina");
        CHECK(g_ItemDatabase.Find(BladeType) == nullptr);
    }
}

// The item editor moves an item's model with the item, and saving writes the
// model files of the moved models.
TEST_CASE("A moved item takes its model along, and saving writes it [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    // A model file the save must not touch: no model of its group moves.
    const std::filesystem::path axeModels = folder.Directory() / GetItemModelDataDirectory() / GetItemGroupFileName(1);
    std::ofstream(axeModels, std::ios::binary | std::ios::app) << "\n";
    const std::string axeModelsText = TestFiles::ReadWholeFile(axeModels);
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope models;
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE_MESSAGE(g_ItemDataHandler.LoadModels(errorMessage), errorMessage);
    const int emptyType = MakeItemType(0, 100);
    REQUIRE(g_ItemDatabase.Find(emptyType) == nullptr);
    REQUIRE(g_ItemModelDatabase.Find(emptyType) == nullptr);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    const std::string krisModelFile = g_ItemModelDatabase.Find(KrisType)->file;

    std::swap(ItemAttribute[KrisType], ItemAttribute[emptyType]);
    g_ItemDataHandler.OnItemsSwapped(KrisType, emptyType);
    REQUIRE(g_ItemModelDatabase.Find(emptyType) != nullptr);
    CHECK(g_ItemModelDatabase.Find(emptyType)->file == krisModelFile);
    CHECK(g_ItemModelDatabase.Find(KrisType) == nullptr);

    std::vector<ItemDataIssue> issues;
    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    const ItemModelDataLoadResult saved = LoadItemModelDataDirectory(folder.Directory() / GetItemModelDataDirectory());
    CHECK_FALSE(HasErrors(saved.issues));
    const auto findModel = [&saved](int group, int number) -> const ItemModelDefinition*
    {
        const auto model =
            std::find_if(saved.models.begin(), saved.models.end(), [&](const ItemModelDefinition& candidate)
                         { return candidate.group == group && candidate.number == number; });
        return model != saved.models.end() ? &*model : nullptr;
    };
    REQUIRE(findModel(0, 100) != nullptr);
    CHECK(findModel(0, 100)->file == krisModelFile);
    CHECK(findModel(0, 0) == nullptr);
    CHECK(TestFiles::ReadWholeFile(axeModels) == axeModelsText);
}

// The bmd files have the items at their old numbers, so the import moves the
// models of moved items back as well, also after a save.
TEST_CASE("A bmd import moves the models of moved items back [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope models;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE_MESSAGE(g_ItemDataHandler.LoadModels(errorMessage), errorMessage);
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    const std::string krisModelFile = g_ItemModelDatabase.Find(KrisType)->file;
    const int emptyType = MakeItemType(0, 100);
    std::swap(ItemAttribute[KrisType], ItemAttribute[emptyType]);
    g_ItemDataHandler.OnItemsSwapped(KrisType, emptyType);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) == nullptr);
    std::vector<ItemDataIssue> issues;

    SUBCASE("before a save") {}
    SUBCASE("after a save")
    {
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    }

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    REQUIRE(g_ItemDatabase.Find(KrisType) != nullptr);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    CHECK(g_ItemModelDatabase.Find(KrisType)->file == krisModelFile);
    CHECK(g_ItemModelDatabase.Find(emptyType) == nullptr);

    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    const std::string swordFile = GetItemGroupFileName(0);
    CHECK(TestFiles::ReadWholeFile(folder.Directory() / GetItemModelDataDirectory() / swordFile) ==
          TestFiles::ReadWholeFile(DataDirectory / "Items" / "Models" / swordFile));
}

// The names of a language whose item.bmd is missing stay as they are, like
// the names of the languages that have none (German).
TEST_CASE("A bmd import keeps the names of a language whose item.bmd is missing [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    REQUIRE(std::filesystem::remove(folder.LegacyItemFilePath(L"Spn")));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK(result.importedLocales == std::vector<std::string>{"en", "pt"});
    const ItemDefinition* shortSword = g_ItemDatabase.Find(0, 1);
    REQUIRE(shortSword != nullptr);
    CHECK(shortSword->names.Get("es") == "Espada Corta");
    CHECK(shortSword->names.Get("de") == "Kurzschwert");
    CHECK(shortSword->names.Get("pt") == "Espada curta");
}

TEST_CASE("Exporting the items as bmd and importing them again gives the same data [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK(result.validationIssues.empty());
    // Tags, wing tiers, weapon kinds, potion priorities and rule flags are not in the bmd files;
    // the items keep the ones they have, so the files below match.
    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
    {
        INFO(GetItemGroupFileName(group));
        CHECK(WriteItemGroupJson(group, g_ItemDatabase.GetAllSlots()) ==
              TestFiles::ReadWholeFile(DataDirectory / "Items" / GetItemGroupFileName(group)));
    }
}

// Export as bmd writes a missing name as the English one; the import must not
// turn it into a name that is the same in that language.
TEST_CASE("A bmd round trip keeps missing names missing and names that are the same [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "pt", ""));
    REQUIRE(g_ItemDataHandler.SetItemName(KrisType, "es", "Kris"));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasErrors(result.issues));
    CHECK_FALSE(HasNameIn(*g_ItemDatabase.Find(BladeType), "pt"));
    CHECK(GetOwnName(*g_ItemDatabase.Find(KrisType), "es") == "Kris");
    CHECK(HasNameIn(*g_ItemDatabase.Find(BladeType), "es"));
}

TEST_CASE("The save warns about an item name in a language the game has no texts for [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.SetItemName(BladeType, "fr", "Lame"));

    std::vector<ItemDataIssue> issues;
    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    CHECK(std::ranges::any_of(issues,
                              [](const ItemDataIssue& issue)
                              {
                                  return issue.severity == ItemDataIssueSeverity::Warning && issue.group == 0 &&
                                         issue.number == 5 && issue.message.find("the fr name") != std::string::npos;
                              }));
}

// The bmd import and the editor give items the profile their data gives; a stat file can lack it
// (renamed or removed). The item then keeps the default with a warning; an item that still names
// the profile is an error on save.
TEST_CASE("A derived stat profile the item stat file does not have keeps the default [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));

    ItemStatsLoadResult stats = LoadItemStatsFile(folder.Directory() / GetItemStatsFile());
    REQUIRE_FALSE(HasErrors(stats.issues));
    std::erase_if(stats.file.profiles, [](const WrittenStatProfile& profile) { return profile.name == "shield"; });
    ItemStatProfilesScope restoreProfiles;
    g_ItemDatabase.SetStatProfiles(ItemStatProfiles(stats.file));
    const auto namesShield = [](ItemDataIssueSeverity severity)
    {
        return [severity](const ItemDataIssue& issue)
        {
            return issue.severity == severity && issue.field == "statProfile" &&
                   issue.message.find("\"shield\"") != std::string::npos;
        };
    };

    SUBCASE("the import")
    {
        const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
        REQUIRE_FALSE(HasErrors(result.issues));
        CHECK_FALSE(HasErrors(result.validationIssues));
        CHECK(std::ranges::any_of(result.validationIssues, namesShield(ItemDataIssueSeverity::Warning)));
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile.empty());
    }

    SUBCASE("an edit")
    {
        g_ItemDataHandler.OnItemEdited(ITEM_SMALL_SHIELD);
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile.empty());
    }

    SUBCASE("the save of items that still name it, with the item checks")
    {
        // An item without an English name is an error of the item checks.
        ItemDefinition untranslated = *g_ItemDatabase.Find(KrisType);
        untranslated.names = Data::LocalizedString{};
        untranslated.names.Set("pt", "Cris");
        g_ItemDatabase.Set(untranslated);

        std::vector<ItemDataIssue> issues;
        CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::InvalidData);
        CHECK(std::ranges::any_of(issues, namesShield(ItemDataIssueSeverity::Error)));
        CHECK(std::ranges::any_of(issues,
                                  [](const ItemDataIssue& issue)
                                  {
                                      return issue.severity == ItemDataIssueSeverity::Error && issue.group == 0 &&
                                             issue.number == 0 && issue.field != "statProfile";
                                  }));
    }
}

TEST_CASE("Exporting unchanged bmd files is not a failure [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    std::string firstChangeLog;
    CHECK(g_ItemDataHandler.ExportAsBmd(firstChangeLog));

    std::string secondChangeLog;
    CHECK(g_ItemDataHandler.ExportAsBmd(secondChangeLog));
}
#endif
