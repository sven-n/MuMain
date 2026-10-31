#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "ItemModelDatabaseScope.h"
#include "ItemStatProfilesScope.h"
#include "TestFiles.h"
#include "TestImGuiContext.h"

#include "Core/Globals/_crypt.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemDataLoader.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Physics/PhysicsManager.h"
#include "GameLogic/Items/ItemStats.h"
#include "Core/Text/Utf8.h"
#include "I18N/All.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"
#ifdef _EDITOR
#include "Core/MuEditorCore.h"
#include "UI/Common/MuEditorLayout.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ItemEditor/ItemEditorActions.h"
#include "UI/ItemEditor/ItemListModel.h"
#include "UI/ItemTools/GlowColorDialog.h"
#include "UI/ItemTools/ItemAddDialog.h"
#include "UI/ItemTools/ItemEdits.h"
#include "UI/ItemTools/ItemFieldWidgets.h"
#include "UI/ItemTools/ItemHeldByCharacter.h"
#include "UI/ItemTools/ItemNamesakes.h"
#include "UI/ItemTools/ItemRequirementTable.h"
#include "UI/ItemTools/ItemRulesTable.h"
#include "UI/ItemTools/ItemSelection.h"
#include "UI/ItemTools/ItemStatFileChange.h"
#include "UI/ItemTools/ItemUsage.h"
#include "UI/ItemTools/ItemUsers.h"
#include "UI/ItemTools/MuItemBasicsUI.h"
#include "UI/ItemTools/MuItemBrowserUI.h"
#include "UI/ItemTools/MuItemCategoriesUI.h"
#include "UI/DataChecks/MuDataChecksUI.h"
#include "UI/ItemTools/MuItemFormulasUI.h"
#include "UI/ItemTools/MuItemGlowColorsUI.h"
#include "UI/ItemTools/MuItemModelUI.h"
#include "UI/ItemTools/MuItemRequirementsUI.h"
#include "UI/ItemTools/MuItemRulesUI.h"
#include "UI/ItemTools/MuItemSyncUI.h"
#include "UI/ItemTranslations/MuItemTranslationsUI.h"
#include "UI/MapEditor/MapEditorUI.h"
#include "TestModelSlots.h"
#include "imgui.h"
#include "imgui_internal.h"
#endif

#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <memory>
#include <optional>
#include <sstream>
#include <random>
#include <span>
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
    REQUIRE_FALSE(HasFatal(stats.issues));
    const ItemTooltipLoadResult tooltips =
        LoadItemTooltipFile(DataDirectory / "Items" / "Tooltips" / ItemTooltipFileName);
    REQUIRE_FALSE(HasFatal(tooltips.issues));
    std::vector<ItemDataIssue> issues;
    ValidateDescriptionPlaceholders(result.items, ItemStatProfiles(stats.file), tooltips.file, ItemStatsFileName,
                                    issues);
    ValidateDescriptionTexts(result.items, tooltips.file, ItemTooltipFileName, issues);
    ValidateTableUses(result.items, ItemStatProfiles(stats.file), tooltips.file, ItemStatsFileName, issues);
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

// A description line in `locale` as the tooltip fills it for the item at `level`: the values the line binds, the
// item's level values, {level} and the stat file's tables.
std::string FilledLine(const ItemDefinition& item, const ItemDescriptionLine& line, int level,
                       const ItemStatProfiles& profiles, std::string_view locale = Data::LocalizedString::NeutralLocale)
{
    return FillPlaceholders(LineText(line).Get(locale),
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
                                               ? std::optional<std::string>(values->values[level].text.Get(locale))
                                               : std::nullopt;
                                }
                                if (const std::optional<int> value = GetPlaceholderValue(name, level, profiles))
                                {
                                    return std::to_string(*value);
                                }
                                return std::nullopt;
                            });
}

// The durability lines of the descriptions (step 5b-3b), which came from another part of the old code: the durability
// shared texts, or a line shown by the item's durability ("durability", "durability=2"). Another shared text, or a line
// with another condition, is no durability line, so the old-code comparisons keep it.
bool IsDurabilityLine(const ItemDescriptionLine& line)
{
    return line.textName == "durability" || line.textName == "numberOfItems" || line.textName == "life" ||
           line.textName == "usableTimes" ||
           std::any_of(line.when.begin(), line.when.end(),
                       [](const TooltipCondition& condition) { return condition.value == TooltipValue::Durability; });
}

// A line the tooltip code wrote for the wings, capes and pets before it was their descriptions: one of the game's
// texts that other code still prints ("Increase %d%% of Damage", "Absorb %d%% of Damage", "Increase speed") with its
// value, or another text, in English.
struct OldWingLine
{
    enum class Kind
    {
        Increase,
        Absorb,
        Speed,
        Other,
    };
    Kind kind;
    int value = 0;
    std::string english;

    bool operator==(const OldWingLine&) const = default;
};

// The old formulas, to compare the data with.
std::vector<OldWingLine> OldWingAndPetLines(int group, int number, int level)
{
    const auto increase = [](int value) { return OldWingLine{OldWingLine::Kind::Increase, value}; };
    const auto absorb = [](int value) { return OldWingLine{OldWingLine::Kind::Absorb, value}; };
    const OldWingLine speed{OldWingLine::Kind::Speed};
    const auto other = [](const char* english) { return OldWingLine{OldWingLine::Kind::Other, 0, english}; };
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
        return {absorb(20), other("Max HP +50 increased")};
    }
    if (group == 13 && number == 1)
    {
        return {other("Increase 30% of attacking & Wizardry Dmg")};
    }
    return {};
}

// What the old code printed for the line in the UI's language: the game's text with its value, as printf fills
// "%d" and "%%" (the only conversions these texts have); another text only in English.
std::optional<std::string> PrintOldWingLine(const OldWingLine& line)
{
    const wchar_t* format = nullptr;
    switch (line.kind)
    {
    case OldWingLine::Kind::Increase:
        format = I18N::Game::IncreaseDOfDamage;
        break;
    case OldWingLine::Kind::Absorb:
        format = I18N::Game::AbsorbDOfDamage;
        break;
    case OldWingLine::Kind::Speed:
        format = I18N::Game::IncreaseSpeed;
        break;
    case OldWingLine::Kind::Other:
        return std::nullopt;
    }
    const std::string text = Core::Text::ToUtf8(format);
    std::string printed;
    for (size_t position = 0; position < text.size(); ++position)
    {
        if (text.compare(position, 2, "%%") == 0)
        {
            printed += '%';
            ++position;
        }
        else if (text.compare(position, 2, "%d") == 0)
        {
            printed += std::to_string(line.value);
            ++position;
        }
        else
        {
            printed += text[position];
        }
    }
    return printed;
}

std::string EnglishOldWingLine(const OldWingLine& line)
{
    switch (line.kind)
    {
    case OldWingLine::Kind::Increase:
        return "Increase " + std::to_string(line.value) + "% of Damage";
    case OldWingLine::Kind::Absorb:
        return "Absorb " + std::to_string(line.value) + "% of Damage";
    case OldWingLine::Kind::Speed:
        return "Increase speed";
    case OldWingLine::Kind::Other:
        break;
    }
    return line.english;
}

TEST_CASE("The wings', capes' and pets' tooltip values come from their data as from the old formulas [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    const ItemStatsLoadResult stats = LoadItemStatsFile(DataDirectory / "Items" / "Stats" / ItemStatsFileName);
    REQUIRE_FALSE(HasFatal(stats.issues));
    const ItemStatProfiles profiles(stats.file);
    // The test switches the UI language, and once a test has loaded the item data every switch refills
    // ItemAttribute: it needs an item table of its own.
    ClientDataScope client(ShippedClientDirectory);
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
            const std::vector<OldWingLine> old = OldWingAndPetLines(item.group, item.number, level);
            // Not the durability lines, which came from another part of the old code.
            std::vector<const ItemDescriptionLine*> wingLines;
            for (const ItemDescriptionLine& line : item.description)
            {
                if (!IsDurabilityLine(line))
                {
                    wingLines.push_back(&line);
                }
            }
            REQUIRE(wingLines.size() == old.size());
            for (size_t index = 0; index < old.size(); ++index)
            {
                const ItemDescriptionLine& line = *wingLines[index];
                CHECK(FilledLine(item, line, level, profiles) == EnglishOldWingLine(old[index]));
                // In every language, as the game's texts that other code still prints have it.
                for (const char* locale : I18N::GetAvailableLocales())
                {
                    INFO(locale);
                    I18N::SetLocale(locale);
                    if (const std::optional<std::string> printed = PrintOldWingLine(old[index]))
                    {
                        CHECK(FilledLine(item, line, level, profiles, locale) == *printed);
                    }
                }
                I18N::SetLocale("en");
            }
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
            // White for a Dark Lord only, as every class line (step 5c-1); the old code had it white for everyone.
            lines.push_back({"Can be equipped by Dark Lord", DarkRed});
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
    REQUIRE_FALSE(HasFatal(stats.issues));
    const ItemStatProfiles profiles(stats.file);
    for (const auto& [group, number] : items)
    {
        const auto found = std::find_if(result.items.begin(), result.items.end(), [&](const ItemDefinition& item)
                                        { return item.group == group && item.number == number; });
        REQUIRE(found != result.items.end());
        // Every level the items have. The old code also wrote an empty line at the Contract Summon's, Life Stone's and
        // Rena's levels without a text, and two at the Fruits' +5 to +15 before "It can be used with item removed." (a
        // stale line of the text list); the data writes none of them. Fruits only exist at +0 to +4.
        for (int level = 0; level < ItemLevelCount; ++level)
        {
            INFO(group << "/" << number << " +" << level);
            std::vector<OldLine> lines;
            for (const ItemDescriptionLine& line : found->description)
            {
                // Not the durability lines, which came from another part of the old code.
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

// The translations of those texts are the ones the old code showed: item_chain_texts_before.json records them from the
// .resx before the texts moved (a format filled in with the item's values), by the English text. A line that names a
// shared text is checked by that text of the tooltip file.
TEST_CASE("The second item chain's tooltip texts keep their translations [data][items]")
{
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    std::ifstream file(std::filesystem::path(MU_TEST_CHAIN_TEXTS_BEFORE));
    REQUIRE(file);
    const nlohmann::json before = nlohmann::json::parse(file).at("texts");
    ItemTooltipFile tooltips;
    {
        std::ifstream tooltipFile(DataDirectory / "Items" / "Tooltips" / "Tooltip.json", std::ios::binary);
        REQUIRE(tooltipFile);
        std::ostringstream tooltipText;
        tooltipText << tooltipFile.rdbuf();
        std::vector<ItemDataIssue> issues;
        ReadItemTooltipJson(tooltipText.str(), "Tooltip.json", tooltips, issues);
    }
    int checked = 0;
    for (const OldChainItem& old : OldChainLines())
    {
        const auto found = std::find_if(result.items.begin(), result.items.end(), [&](const ItemDefinition& item)
                                        { return item.group == old.group && item.number == old.number; });
        REQUIRE(found != result.items.end());
        for (size_t index = 0; index < old.lines.size(); ++index)
        {
            const ItemDescriptionLine& line = found->description[index];
            const Data::LocalizedString* text = &line.text;
            if (!line.textName.empty())
            {
                text = tooltips.FindText(line.textName);
                REQUIRE(text != nullptr);
            }
            const std::string english = text->GetNeutral();
            const auto recorded = before.find(english);
            if (recorded == before.end())
            {
                continue;
            }
            for (const auto& [locale, translation] : recorded->items())
            {
                INFO(old.group << "/" << old.number << " line " << index << " " << locale);
                CHECK(text->Get(locale) == translation.get<std::string>());
                ++checked;
            }
        }
    }
    CHECK(checked >= 1050);
}

// What the old code printed for an item's level lines in the UI's language, where the texts are the game's that other
// code still prints: one entry per line of OldLevelLines, empty for a text only the tooltip had (removed since). A
// Fruit's stat lines started with the stat's name; the rest of them was a removed text.
struct PrintedOldLine
{
    std::string text;
    bool isPrefix = false;
};

std::string PrintWithName(const wchar_t* format, const wchar_t* name)
{
    // The game's wide texts have "%ls" where the .resx has "%s".
    std::string text = Core::Text::ToUtf8(format);
    const size_t wide = text.find("%ls");
    const size_t position = wide != std::string::npos ? wide : text.find("%s");
    REQUIRE(position != std::string::npos);
    return text.replace(position, wide != std::string::npos ? 3 : 2, Core::Text::ToUtf8(name));
}

std::vector<std::optional<PrintedOldLine>> PrintedOldLevelLines(int group, int number, int level)
{
    const auto printed = [](const wchar_t* text) { return std::optional<PrintedOldLine>({Core::Text::ToUtf8(text)}); };
    if (group == 13 && number == 15 && level <= 4)
    {
        const wchar_t* const stats[] = {I18N::Game::ENG, I18N::Game::STA, I18N::Game::AGI, I18N::Game::STR,
                                        I18N::Game::Command};
        const PrintedOldLine stat{Core::Text::ToUtf8(stats[level]) + " ", true};
        std::vector<std::optional<PrintedOldLine>> lines = {stat, stat, printed(I18N::Game::ItCanBeUsedWithItemRemoved)};
        if (level == 4)
        {
            lines.push_back(PrintedOldLine{PrintWithName(I18N::Game::CanBeEquippedByS, I18N::Game::DarkLord)});
        }
        return lines;
    }
    return std::vector<std::optional<PrintedOldLine>>(OldLevelLines(group, number, level).size());
}

// The lines whose texts other code still prints, in every language as the game prints them.
TEST_CASE("The tooltip texts by item level are the game's texts in every language [data][items]")
{
    // Archer, Spearman, Scroll of Guardian and Place Life Stone are item names from the data now, not game texts.
    constexpr std::pair<int, int> items[] = {{13, 15}};
    const ItemDataLoadResult result = LoadItemDataDirectory(DataDirectory / "Items");
    const ItemStatsLoadResult stats = LoadItemStatsFile(DataDirectory / "Items" / "Stats" / ItemStatsFileName);
    REQUIRE_FALSE(HasFatal(stats.issues));
    const ItemStatProfiles profiles(stats.file);
    // The test switches the UI language: it needs an item table of its own.
    ClientDataScope client(ShippedClientDirectory);
    int compared = 0;
    for (const auto& [group, number] : items)
    {
        const ItemDefinition* found = FindItem(result.items, MakeItemType(group, number));
        REQUIRE(found != nullptr);
        for (int level = 0; level < ItemLevelCount; ++level)
        {
            std::vector<const ItemDescriptionLine*> shown;
            for (const ItemDescriptionLine& line : found->description)
            {
                if (line.IsShownAt(level))
                {
                    shown.push_back(&line);
                }
            }
            for (const char* locale : I18N::GetAvailableLocales())
            {
                INFO(group << "/" << number << " +" << level << " " << locale);
                I18N::SetLocale(locale);
                const std::vector<std::optional<PrintedOldLine>> old = PrintedOldLevelLines(group, number, level);
                REQUIRE(shown.size() == old.size());
                for (size_t index = 0; index < old.size(); ++index)
                {
                    if (!old[index])
                    {
                        continue;
                    }
                    const std::string text = FilledLine(*found, *shown[index], level, profiles, locale);
                    if (old[index]->isPrefix)
                    {
                        CHECK(text.starts_with(old[index]->text));
                    }
                    else
                    {
                        CHECK(text == old[index]->text);
                    }
                    ++compared;
                }
            }
            I18N::SetLocale("en");
        }
    }
    // The Fruits' 3 lines at +0 to +3 and 4 at +4, in every language.
    CHECK(compared == (4 * 3 + 4) * static_cast<int>(std::size(I18N::GetAvailableLocales())));
}

// The damage texts without the English "of" and the Fruits' stat words, as #828 fixed them: rendered with 12 in each
// language they had wrong, and no translation of the two texts keeps "% of".
TEST_CASE("The damage texts and the Fruits' stat words read right in every language [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const Data::Items::ItemTooltipFile& tooltips = g_ItemDatabase.GetTooltipFile();
    const Data::LocalizedString* increase = tooltips.FindText("damageIncrease");
    const Data::LocalizedString* absorb = tooltips.FindText("damageAbsorb");
    REQUIRE(increase != nullptr);
    REQUIRE(absorb != nullptr);
    const auto twelve = [](std::string_view text)
    { return Data::Items::FillPlaceholders(text, [](std::string_view) { return std::optional<std::string>("12"); }); };

    struct Damage
    {
        const char* locale;
        const char* increase;
        const char* absorb;
    };
    const Damage damages[] = {
        {"es", "Aumenta el da\u00f1o un 12%", "Absorbe el 12% del da\u00f1o"},
        {"pt", "Aumenta o dano em 12%", "Absorve 12% do dano"},
        {"ru", "\u0423\u0432\u0435\u043b\u0438\u0447\u0435\u043d\u0438\u0435 \u0443\u0440\u043e\u043d\u0430 \u043d\u0430 12%", "\u041f\u043e\u0433\u043b\u043e\u0449\u0435\u043d\u0438\u0435 12% \u0443\u0440\u043e\u043d\u0430"},
        {"uk", "\u0417\u0431\u0456\u043b\u044c\u0448\u0435\u043d\u043d\u044f \u0448\u043a\u043e\u0434\u0438 \u043d\u0430 12%", "\u041f\u043e\u0433\u043b\u0438\u043d\u0430\u043d\u043d\u044f 12% \u0448\u043a\u043e\u0434\u0438"},
        {"id", "Meningkatkan Kerusakan 12%", "Menyerap 12% Kerusakan"},
        {"tl", "Dagdagan ang Pinsala nang 12%", "Isipsip ang 12% ng Pinsala"},
        {"zh-TW", "\u589e\u52a0 12% \u50b7\u5bb3", "\u5438\u653612%\u50b7\u5bb3"},
    };
    for (const Damage& damage : damages)
    {
        INFO(damage.locale);
        CHECK(twelve(increase->Get(damage.locale)) == damage.increase);
        CHECK(twelve(absorb->Get(damage.locale)) == damage.absorb);
    }
    for (const Data::LocalizedString* text : {increase, absorb})
    {
        for (const auto& [locale, translation] : text->GetTranslations())
        {
            INFO(locale);
            CHECK(translation.find("% of") == std::string::npos);
        }
    }

    // The Command Fruit's {stat} at +0 to +4: ENG, STA, AGI, STR and Command.
    const Data::Items::ItemDefinition* fruit = g_ItemDatabase.Find(13 * MAX_ITEM_INDEX + 15);
    REQUIRE(fruit != nullptr);
    REQUIRE(fruit->levelValues.size() == 1);
    const std::vector<Data::Items::ItemLevelValue>& words = fruit->levelValues.front().values;
    REQUIRE(words.size() == 5);
    struct Stats
    {
        const char* locale;
        std::array<const char*, 5> words;
    };
    const Stats stats[] = {
        {"es", {"ENE", "VIT", "AGI", "FUE", "Dominio"}},
        {"pt", {"ENE", "VIT", "AGI", "FOR", "Comando"}},
        {"ru", {"\u042d\u041d\u0415", "\u0412\u042b\u041d", "\u041b\u0412\u041a", "\u0421\u0418\u041b", "\u041a\u043e\u043c\u0430\u043d\u0434\u043e\u0432\u0430\u043d\u0438\u0435"}},
        {"uk", {"\u0415\u041d\u0415", "\u0412\u0418\u0422", "\u0421\u041f\u0420", "\u0421\u0418\u041b", "\u041a\u043e\u043c\u0430\u043d\u0434\u0443\u0432\u0430\u043d\u043d\u044f"}},
        {"zh-TW", {"\u80fd\u91cf", "\u9ad4\u529b", "\u654f\u6377", "\u529b\u91cf", "\u7d71\u7387"}},
        {"id", {"ENG", "STA", "AGI", "STR", "Kepemimpinan"}},
    };
    for (const Stats& stat : stats)
    {
        for (size_t level = 0; level < words.size(); ++level)
        {
            INFO(stat.locale << " +" << level);
            CHECK(words[level].text.Get(stat.locale) == stat.words[level]);
        }
    }
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
        const ItemDefinition* found = FindItem(result.items, MakeItemType(item.group, item.number));
        REQUIRE(found != nullptr);
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

// Turns translated item names back on when the test ends, as the other tests
// expect them.
class TranslatedItemNamesScope
{
public:
    ~TranslatedItemNamesScope()
    {
        g_ItemDataHandler.SetTranslateItemNames(true);
    }
};

TEST_CASE("Item names stay English while translated item names are off, also after a language change [data][items]")
{
    ClientDataScope client(ShippedClientDirectory);
    TranslatedItemNamesScope translatedNames;
    I18N::SetLocale("pt");
    g_ItemDataHandler.SetTranslateItemNames(false);

    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    CHECK(client.Name(BladeType) == L"Blade");

    I18N::SetLocale("de");
    CHECK(client.Name(BladeType) == L"Blade");

    g_ItemDataHandler.SetTranslateItemNames(true);
    CHECK(client.Name(BladeType) == L"Klinge");

    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(client.Name(BladeType) == L"Blade");
    CHECK(g_ItemDatabase.Find(BladeType)->name == L"Blade");
}

TEST_CASE("Item data errors stop loading [data][items]")
{
    TemporaryClientFolder folder;
    // An entry without a name is fine; a name that is not a text is not.
    folder.WriteItemFile("Group00_Sword.json",
                         R"({"formatVersion": 1, "group": 0, "items": [{"number": 0, "name": 5}]})");
    ClientDataScope client(folder.Directory());

    std::string errorMessage;
    CHECK_FALSE(g_ItemDataHandler.Load(errorMessage));
    CHECK(errorMessage.find("Group00_Sword.json") != std::string::npos);
}

TEST_CASE("An empty item name stops loading [data][items]")
{
    // Unlike an entry without a name, which has no name key at all.
    TemporaryClientFolder folder;
    folder.WriteItemFile("Group00_Sword.json",
                         R"({"formatVersion": 1, "group": 0, "items": [{"number": 0, "name": ""}]})");
    ClientDataScope client(folder.Directory());

    std::string errorMessage;
    CHECK_FALSE(g_ItemDataHandler.Load(errorMessage));
    CHECK(errorMessage.find("the English name is missing") != std::string::npos);
}

TEST_CASE("A description placeholder that names no table stops loading [data][items]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    const std::filesystem::path potions = folder.Directory() / GetItemDataDirectory() / "Group14_Potion.json";
    std::string text = TestFiles::ReadWholeFile(potions);
    // The Jewel of Bless's text, with its translations, becomes an English text that names no table.
    const size_t found = text.find("\"en\": \"It is used to increase your item level up to 6\"");
    REQUIRE(found != std::string::npos);
    const size_t start = text.rfind("\"text\": {", found);
    const size_t end = text.find('}', found);
    REQUIRE(start != std::string::npos);
    REQUIRE(end != std::string::npos);
    text.replace(start, end + 1 - start, "\"text\": \"Up to +{table:noSuchTable}\"");
    std::ofstream(potions, std::ios::binary) << text;
    ClientDataScope client(folder.Directory());

    std::string errorMessage;
    CHECK_FALSE(g_ItemDataHandler.Load(errorMessage));
    CHECK(errorMessage.find("has the unknown placeholder {table:noSuchTable}") != std::string::npos);
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
    REQUIRE_FALSE(HasFatal(result.issues));
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

// The orbs, crystals and scrolls keep their requirements as values (their profiles keep the base
// value); the original client's bmd files hold weights for them, which its code grew.
TEST_CASE("The orbs, crystals and scrolls hold their requirements as weights in a bmd file [data][items]")
{
    const ItemStatsLoadResult stats = LoadItemStatsFile(ShippedClientDirectory / GetItemStatsFile());
    REQUIRE(stats.issues.empty());
    const ItemStatProfiles profiles(stats.file);

    // Healing Orb: 20 + weight * drop level * 4 / 100.
    ItemDefinition orb;
    orb.group = 12;
    orb.number = 8;
    orb.level = 8;
    orb.statProfile = "orb";
    orb.requirements.energy = 100;
    REQUIRE(HasRequirementWeightsInBmd(profiles, orb));
    ToRequirementValues(profiles, orb);
    CHECK(orb.requirements.energy == 52);
    ITEM_ATTRIBUTE attribute{};
    CHECK(ToBmdRequirementWeights(profiles, orb, attribute).empty());
    CHECK(attribute.RequireEnergy == 100);
    // Without a profile it names, the rules give it the orb profile.
    ItemDefinition ruledOrb = orb;
    ruledOrb.statProfile.clear();
    CHECK(HasRequirementWeightsInBmd(profiles, ruledOrb));
    // Given the default profile, its requirements grow like any item's: weights.
    ItemDefinition defaultOrb = orb;
    defaultOrb.statProfile = "default";
    CHECK_FALSE(HasRequirementWeightsInBmd(profiles, defaultOrb));

    // Scroll of Wizardry Enhance: the energy from the level requirement, 20 + weight * 220 * 4 / 100.
    ItemDefinition scroll;
    scroll.group = 15;
    scroll.number = 28;
    scroll.level = 100;
    scroll.statProfile = "scroll";
    scroll.requirements.level = 220;
    scroll.requirements.energy = 118;
    ToRequirementValues(profiles, scroll);
    CHECK(scroll.requirements.level == 220);
    CHECK(scroll.requirements.energy == 1058);
    CHECK(ToBmdRequirementWeights(profiles, scroll, attribute).empty());
    CHECK(attribute.RequireEnergy == 118);

    // A value no weight gives (each weight adds 4): the nearest one.
    ItemDefinition steps = orb;
    steps.level = 100;
    steps.requirements.energy = 27;
    const std::vector<ItemRequirement> inexact = ToBmdRequirementWeights(profiles, steps, attribute);
    CHECK(inexact == std::vector<ItemRequirement>{ItemRequirement::Energy});
    CHECK(attribute.RequireEnergy == 2);

    // A weapon keeps its weights in both.
    ItemDefinition kris;
    kris.group = 0;
    kris.slot = ItemSlot::MainHand;
    kris.statProfile = "weapon";
    CHECK_FALSE(HasRequirementWeightsInBmd(profiles, kris));
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
    REQUIRE_FALSE(HasFatal(result.issues));

    const ItemDefinition* blade = FindItem(result.items, BladeType);
    REQUIRE(blade != nullptr);
    CHECK(blade->names.Get("pt") == "L\xC3\xA2mina");

    const ItemDefinition* gaionsOrder = FindItem(result.items, GaionsOrderType);
    REQUIRE(gaionsOrder != nullptr);
    CHECK(gaionsOrder->names.GetNeutral() == "Gaion's Order");
}

TEST_CASE("Bmd import reads a name that only looks like UTF-8 as Windows-1252 [data][items]")
{
    TemporaryClientFolder folder;
    LegacyItemFile english;
    english.SetName(BladeType, "Blade");
    english.Write(folder.LegacyItemFilePath(L"Eng"));
    LegacyItemFile portuguese;
    // C0 AF would be an overlong form of '/' in UTF-8; read as Windows-1252 it is U+00C0 U+00AF.
    portuguese.SetName(BladeType, "Blade \xC0\xAF");
    portuguese.Write(folder.LegacyItemFilePath(L"Por"));
    ClientDataScope client(folder.Directory());

    const ItemBmdImportResult result = ImportItemBmdFiles();
    REQUIRE_FALSE(HasFatal(result.issues));
    CHECK(result.reencodedNameCount == 1);

    const ItemDefinition* blade = FindItem(result.items, BladeType);
    REQUIRE(blade != nullptr);
    CHECK(blade->names.Get("pt") == "Blade \xC3\x80\xC2\xAF");
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
    // The issue names the temporary file that could not be written.
    CHECK(std::any_of(issues.begin(), issues.end(), [](const ItemDataIssue& issue)
                      { return issue.message == "could not be written" && issue.source.ends_with(".json.tmp"); }));
}

TEST_CASE("A file that cannot be replaced is a write failure and leaves no temporary file [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    const std::filesystem::path itemDirectory = folder.Directory() / GetItemDataDirectory();
    // A folder with the name of the sword file: the written file cannot take its place.
    const std::filesystem::path swordFile = itemDirectory / GetItemGroupFileName(0);
    std::filesystem::remove(swordFile);
    std::filesystem::create_directories(swordFile / "inside");

    std::vector<ItemDataIssue> issues;
    CHECK(SaveItemDataDirectory(itemDirectory, shipped.items, issues) == ItemDataSaveResult::WriteFailed);
    CHECK(std::any_of(
        issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
        { return issue.source == swordFile.string() && issue.message.starts_with("could not be replaced: "); }));
    CHECK(std::filesystem::is_directory(swordFile));
    CHECK_FALSE(std::filesystem::exists(swordFile.string() + ".tmp"));
}

TEST_CASE("Every number with a model has an entry in the item files [data][items]")
{
    const ItemDataLoadResult items = LoadItemDataDirectory(DataDirectory / "Items");
    REQUIRE_FALSE(HasFatal(items.issues));
    const ItemModelDataLoadResult models = LoadItemModelDataDirectory(DataDirectory / "Items" / "Models");
    std::vector<ItemDataIssue> issues;
    ValidateModelsHaveItemEntries(items.items, models.models, issues);
    for (const ItemDataIssue& issue : issues)
    {
        INFO(issue.group << "," << issue.number << ": " << issue.message);
        CHECK(false);
    }

    // Sphere 4 and 5: the client draws them and code names them, but has no item there yet.
    for (const int number : {73, 74})
    {
        const auto entry = std::ranges::find_if(items.items, [number](const ItemDefinition& item)
                                                { return item.group == 12 && item.number == number; });
        REQUIRE(entry != items.items.end());
        CHECK(entry->entryWithoutName);
    }
}

// The items and their models are saved together: when one of the files
// cannot be written, none is replaced, so a moved item never loses its model
// on disk.
TEST_CASE("A save that cannot write all its files replaces none of them [data][items]")
{
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    REQUIRE_FALSE(HasFatal(shipped.issues));
    const ItemModelDataLoadResult shippedModels = LoadItemModelDataDirectory(DataDirectory / "Items" / "Models");
    REQUIRE_FALSE(HasFatal(shippedModels.issues));
    ItemModelDatabase models;
    models.Build(shippedModels.models, Data::Effects::GlowColorList{});
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    const std::filesystem::path itemDirectory = folder.Directory() / GetItemDataDirectory();
    // Kris's price changes, so its group file changes.
    std::vector<ItemDefinition> items = shipped.items;
    const auto kris = std::find_if(items.begin(), items.end(), [](const ItemDefinition& item)
                                   { return MakeItemType(item.group, item.number) == KrisType; });
    REQUIRE(kris != items.end());
    kris->buyPrice += 1;
    // A file where the model folder should be: no model file can be written.
    const std::filesystem::path blockedModels = folder.Directory() / "blocked";
    std::ofstream(blockedModels) << "not a folder";
    ItemGroupSet modelGroups;
    modelGroups.set(0);

    std::vector<ItemDataIssue> issues;
    CHECK(SaveItemAndModelDataDirectories(itemDirectory, items, {blockedModels, models.GetAllSlots(), modelGroups},
                                          issues) == ItemDataSaveResult::WriteFailed);
    CHECK_FALSE(issues.empty());
    const std::string swordFile = GetItemGroupFileName(0);
    CHECK(TestFiles::ReadWholeFile(itemDirectory / swordFile) ==
          TestFiles::ReadWholeFile(DataDirectory / "Items" / swordFile));
    CHECK_FALSE(std::filesystem::exists(itemDirectory / (swordFile + ".tmp")));
}

#ifdef _EDITOR
namespace
{
// An item tool's edit: the item as it is, changed, set back (SetItem).
template <typename TChange> void EditItem(int itemType, TChange&& change)
{
    ItemDefinition item = *g_ItemDatabase.Find(itemType);
    change(item);
    g_ItemDataHandler.SetItem(item);
}

// The character wears items of these types for the scope (IsHeldByCharacter).
class WornItemsScope
{
public:
    explicit WornItemsScope(std::initializer_list<int> itemTypes)
        : m_previous(CharacterMachine), m_machine(std::make_unique<CHARACTER_MACHINE>())
    {
        // Type 0 is the Kris: every other place holds nothing.
        for (ITEM& equipped : m_machine->Equipment)
        {
            equipped.Type = -1;
        }
        size_t index = 0;
        for (const int itemType : itemTypes)
        {
            m_machine->Equipment[index++].Type = static_cast<short>(itemType);
        }
        CharacterMachine = m_machine.get();
    }

    ~WornItemsScope()
    {
        CharacterMachine = m_previous;
    }

    WornItemsScope(const WornItemsScope&) = delete;
    WornItemsScope& operator=(const WornItemsScope&) = delete;

private:
    CHARACTER_MACHINE* m_previous;
    std::unique_ptr<CHARACTER_MACHINE> m_machine;
};

// The game's characters for the scope, with the editor's hook that drops what
// they wear of cloth (CMuEditorCore::ConnectGameHooks) and one that keeps the
// item types whose opened model slots took other data.
class WornClothScope
{
public:
    WornClothScope() : m_previous(CharactersClient), m_characters(new CHARACTER[MAX_CHARACTERS_CLIENT + 1]{})
    {
        CharactersClient = m_characters.get();
        GetChangedSlots().clear();
        g_ItemDataHandler.SetOpenedModelHooks({&MuEditor::Items::DropWornCloth, &KeepChangedSlots});
    }

    ~WornClothScope()
    {
        g_ItemDataHandler.SetOpenedModelHooks({});
        MuEditor::Items::DropWornCloth();
        CharactersClient = m_previous;
    }

    // The item types of the last change of opened model slots.
    static std::vector<int>& GetChangedSlots()
    {
        static std::vector<int> itemTypes;
        return itemTypes;
    }

    WornClothScope(const WornClothScope&) = delete;
    WornClothScope& operator=(const WornClothScope&) = delete;

    CHARACTER& GetCharacter(int index)
    {
        return m_characters[index];
    }

    // The character wears the pants with a cloth, as RenderPartObject makes it.
    static const PART_t& Wear(CHARACTER& character, int modelType)
    {
        PART_t& pants = character.BodyPart[BODYPART_PANTS];
        pants.Type = static_cast<short>(modelType);
        pants.m_pCloth[0] = new CPhysicsClothMesh;
        pants.m_byNumCloth = 1;
        return pants;
    }

private:
    static void KeepChangedSlots(std::span<const int> itemTypes)
    {
        GetChangedSlots().assign(itemTypes.begin(), itemTypes.end());
    }

    CHARACTER* m_previous;
    std::unique_ptr<CHARACTER[]> m_characters;
};
} // namespace

// The cloth of worn pants moves a mesh of its model's slot every frame and was
// made from the data the slot had: it goes before the slot opens other data,
// and the game makes it again.
TEST_CASE("Changing an opened item model drops the cloth the characters wear [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    REQUIRE_FALSE(g_ItemDataHandler.HasModelsToReopen());
    ItemModelDefinition divine = *g_ItemModelDatabase.Find(ITEM_DIVINE_PANTS);

    WornClothScope characters;
    // Another player, and the character of the character creation.
    const PART_t& player = WornClothScope::Wear(characters.GetCharacter(7), MODEL_DIVINE_PANTS);
    const PART_t& created = WornClothScope::Wear(CharacterView, MODEL_DIVINE_PANTS);
    const auto clothDropped = [&] { return player.m_pCloth[0] == nullptr && created.m_pCloth[0] == nullptr; };

    SUBCASE("a model opened again")
    {
        divine.material.specular += 0.25;
        g_ItemDataHandler.SetItemModel(divine);
        REQUIRE(g_ItemDataHandler.HasModelsToReopen());
        // The editor opens it again between frames.
        CHECK_FALSE(clothDropped());
        g_ItemDataHandler.ReopenChangedModels({});
        CHECK(clothDropped());
        CHECK(WornClothScope::GetChangedSlots() == std::vector<int>{ITEM_DIVINE_PANTS});
    }
    SUBCASE("a removed model entry")
    {
        ItemModelDefinition none;
        none.group = divine.group;
        none.number = divine.number;
        g_ItemDataHandler.SetItemModel(none);
        g_ItemDataHandler.ReopenChangedModels({});
        CHECK(clothDropped());
    }
    SUBCASE("a moved item, whose opened model moves at once")
    {
        const int freeType = MakeItemType(9, 100);
        REQUIRE(IsFreeItemType(freeType));
        REQUIRE(g_ItemDataHandler.MoveItem(ITEM_DIVINE_PANTS, freeType));
        CHECK(clothDropped());
        // Both slots took other data.
        std::vector<int> changed = WornClothScope::GetChangedSlots();
        std::ranges::sort(changed);
        CHECK(changed == std::vector<int>{ITEM_DIVINE_PANTS, freeType});
    }
    SUBCASE("a display value keeps the cloth")
    {
        divine.inventory.scale += 0.001;
        g_ItemDataHandler.SetItemModel(divine);
        CHECK_FALSE(g_ItemDataHandler.HasModelsToReopen());
        g_ItemDataHandler.ReopenChangedModels({});
        CHECK_FALSE(clothDropped());
        CHECK(WornClothScope::GetChangedSlots().empty());
    }
}

// The lists of the item tools: no slot, no wing tier and, for the weapon kind,
// the kind the group gives (which then follows the data) and none.
TEST_CASE("The item tool lists offer no slot, no wing tier and the weapon kind the group gives [editor][items]")
{
    using MuEditor::Items::Fields::EnumEntries;
    const auto slots = EnumEntries<ItemSlot>({{ItemSlot::None, "None"}});
    CHECK(slots.size() == GetEnumNames(ItemSlot{}).size() + 1);
    CHECK(slots.front().value == ItemSlot::None);
    CHECK(EnumEntries<WingTier>({{WingTier::None, "None"}}).front().value == WingTier::None);

    const auto sword =
        EnumEntries<WeaponKind>({{WeaponKind::Sword, "By the group: sword"}, {WeaponKind::None, "None"}});
    CHECK(sword.size() == GetEnumNames(WeaponKind{}).size() + 1);
    CHECK(std::string(sword[0].name) == "By the group: sword");
    CHECK(sword[1].value == WeaponKind::None);
    CHECK(std::ranges::count(sword, WeaponKind::Sword, &EnumName<WeaponKind>::value) == 1);
    // A group that gives none: the second none is left out.
    const auto none = EnumEntries<WeaponKind>({{WeaponKind::None, "By the group: None"}, {WeaponKind::None, "None"}});
    CHECK(none.size() == GetEnumNames(WeaponKind{}).size() + 1);
    CHECK(std::string(none[0].name) == "By the group: None");
}

// A field typed in before the game took the item is stored when it is left or
// drawn again: the size and slot of the held item stay, other fields change.
TEST_CASE("The item tools keep the size and slot of an item the game holds [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    const ItemDefinition blade = *g_ItemDatabase.Find(BladeType);
    ItemDefinition wider = kris;
    ++wider.width;
    ItemDefinition ring = kris;
    ring.slot = ItemSlot::Ring;
    ItemDefinition widerBlade = blade;
    ++widerBlade.width;
    {
        const WornItemsScope worn({KrisType});
        CHECK_FALSE(g_ItemEdits.SetItem("Width", wider));
        CHECK_FALSE(g_ItemEdits.SetItem("Slot", ring));
        // Nothing of a step that reaches a held item is stored.
        const ItemDefinition both[] = {widerBlade, wider};
        CHECK_FALSE(g_ItemEdits.SetItems("Width", both));
        CHECK(g_ItemDatabase.Find(BladeType)->width == blade.width);
        CHECK(g_ItemDatabase.Find(KrisType)->width == kris.width);
        CHECK(g_ItemDatabase.Find(KrisType)->slot == kris.slot);
        CHECK(ItemAttribute[KrisType].Width == kris.width);
        CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());

        ItemDefinition stronger = kris;
        stronger.damageMax = 99;
        CHECK(g_ItemEdits.SetItem("Damage", stronger));
        CHECK(g_ItemDatabase.Find(KrisType)->damageMax == 99);
    }
    // Not held any more.
    ItemDefinition widened = *g_ItemDatabase.Find(KrisType);
    ++widened.width;
    CHECK(g_ItemEdits.SetItem("Width", widened));
    CHECK(g_ItemDatabase.Find(KrisType)->width == widened.width);
    g_ItemEdits.GetHistory().Clear();
}

// The import keeps the size and slot of a held item; a held type that had no
// data takes the imported item, since it has no size or slot to keep.
TEST_CASE("A bmd import keeps the size and slot of the held items that had data [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    ItemDefinition widerBlade = *g_ItemDatabase.Find(BladeType);
    ++widerBlade.width;

    // Before the import: the Kris has no data, the blade another size.
    REQUIRE(g_ItemDataHandler.RemoveItem(KrisType));
    g_ItemDataHandler.SetItem(widerBlade);
    const WornItemsScope worn({KrisType, BladeType});
    std::vector<ItemDefinition> held;
    for (const int itemType : MuEditor::Items::FindHeldItemTypes())
    {
        held.push_back(g_ItemDatabase.GetAllSlots()[itemType]);
    }
    REQUIRE(held.size() == 2);

    REQUIRE_FALSE(HasFatal(g_ItemDataHandler.ImportFromBmd().issues));
    CItemEditorActions::KeepHeldItemShapes(held);
    CHECK(g_ItemDatabase.Find(BladeType)->width == widerBlade.width);
    const ItemDefinition* imported = g_ItemDatabase.Find(KrisType);
    REQUIRE(imported != nullptr);
    CHECK(imported->width == kris.width);
    CHECK(imported->height == kris.height);
    CHECK(imported->slot == kris.slot);
    CHECK(ItemAttribute[KrisType].Width == kris.width);
}

TEST_CASE("Item editor changes go into the item database [data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    SUBCASE("a changed value goes into the database and ItemAttribute, the names stay")
    {
        ItemDefinition blade = *g_ItemDatabase.Find(BladeType);
        blade.width = 2;
        g_ItemDataHandler.SetItem(blade);

        CHECK(g_ItemDatabase.Find(BladeType)->width == 2);
        CHECK(ItemAttribute[BladeType].Width == 2);
        CHECK(g_ItemDatabase.Find(BladeType)->names.Get("pt") == "L\xC3\xA2mina");
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

    SUBCASE("a new weapon gets its group's weapon kind")
    {
        ItemDefinition spear;
        spear.group = 3;
        spear.number = 100;
        spear.names = Data::LocalizedString::Parse("New Spear");
        spear.slot = ItemSlot::MainHand;
        REQUIRE(g_ItemDataHandler.AddItem(spear, nullptr));
        CHECK(g_ItemDatabase.GetWeaponKind(MakeItemType(3, 100)) == WeaponKind::Spear);
    }

    SUBCASE("a copy keeps a weapon kind and a stat profile its source picked")
    {
        // Horn of Fenrir: its data gives "helper", it names "default".
        const ItemDefinition* fenrir = g_ItemDatabase.Find(13, 37);
        REQUIRE(fenrir != nullptr);
        REQUIRE(fenrir->statProfile == "default");
        ItemDefinition copy = *fenrir;
        copy.number = 300;
        REQUIRE(g_ItemDataHandler.AddItem(copy, nullptr, fenrir));
        CHECK(g_ItemDatabase.Find(13, 300)->statProfile == "default");

        ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        kris.weaponKind = WeaponKind::Axe;
        g_ItemDataHandler.SetItem(kris);
        REQUIRE(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Axe);
        const ItemDefinition krisSource = *g_ItemDatabase.Find(KrisType);
        ItemDefinition krisCopy = krisSource;
        krisCopy.number = 200;
        REQUIRE(g_ItemDataHandler.AddItem(krisCopy, nullptr, &krisSource));
        CHECK(g_ItemDatabase.GetWeaponKind(MakeItemType(0, 200)) == WeaponKind::Axe);

        // Without the source, a new item follows its data.
        ItemDefinition fresh = *fenrir;
        fresh.number = 301;
        REQUIRE(g_ItemDataHandler.AddItem(fresh, nullptr));
        CHECK(g_ItemDatabase.Find(13, 301)->statProfile == "helper");
    }

    SUBCASE("picking no stat profile follows the data at once")
    {
        ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        REQUIRE(kris.statProfile == "weapon");
        kris.statProfile = "default";
        g_ItemDataHandler.SetItem(kris);
        REQUIRE(g_ItemDatabase.Find(KrisType)->statProfile == "default");
        kris = *g_ItemDatabase.Find(KrisType);
        kris.statProfile.clear();
        g_ItemDataHandler.SetItem(kris);
        CHECK(g_ItemDatabase.Find(KrisType)->statProfile == "weapon");
    }

    SUBCASE("a new bow is a bow once it is two-handed and on offHand, in any order")
    {
        const int newBowType = MakeItemType(4, 100);
        ItemDefinition bow;
        bow.group = 4;
        bow.number = 100;
        bow.names = Data::LocalizedString::Parse("New Bow");
        bow.slot = ItemSlot::MainHand;
        REQUIRE(g_ItemDataHandler.AddItem(bow, nullptr));
        bow = *g_ItemDatabase.Find(newBowType);
        bow.twoHanded = true;
        g_ItemDataHandler.SetItem(bow);
        CHECK(g_ItemDatabase.GetWeaponKind(newBowType) == WeaponKind::Crossbow);

        bow = *g_ItemDatabase.Find(newBowType);
        bow.slot = ItemSlot::OffHand;
        g_ItemDataHandler.SetItem(bow);
        CHECK(g_ItemDatabase.GetWeaponKind(newBowType) == WeaponKind::Bow);

        // The other order: on offHand first, then two-handed.
        const int otherBowType = MakeItemType(4, 101);
        ItemDefinition other;
        other.group = 4;
        other.number = 101;
        other.names = Data::LocalizedString::Parse("New Bow");
        other.slot = ItemSlot::OffHand;
        REQUIRE(g_ItemDataHandler.AddItem(other, nullptr));
        CHECK(g_ItemDatabase.GetWeaponKind(otherBowType) == WeaponKind::None);
        other = *g_ItemDatabase.Find(otherBowType);
        other.twoHanded = true;
        g_ItemDataHandler.SetItem(other);
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
        const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        EditItem(KrisType,
                 [](ItemDefinition& item)
                 {
                     for (int characterClass = 0; characterClass < MAX_CLASS; ++characterClass)
                     {
                         item.classRequirements[characterClass] = characterClass == CLASS_RAGEFIGHTER ? 1 : 0;
                     }
                 });
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Glove);

        g_ItemDataHandler.SetItem(kris);
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Sword);
    }

    SUBCASE("a staff is a book while it is on offHand")
    {
        REQUIRE(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Staff);
        EditItem(ITEM_STAFF, [](ItemDefinition& item) { item.slot = ItemSlot::OffHand; });
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Book);

        EditItem(ITEM_STAFF, [](ItemDefinition& item) { item.slot = ItemSlot::MainHand; });
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_STAFF) == WeaponKind::Staff);
    }

    SUBCASE("an undone edit undoes the weapon kind")
    {
        EditItem(ITEM_BOLT, [](ItemDefinition& item) { item.twoHanded = true; });
        CHECK(g_ItemDatabase.GetWeaponKind(ITEM_BOLT) == WeaponKind::Bow);

        EditItem(ITEM_BOLT, [](ItemDefinition& item) { item.twoHanded = false; });
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
        EditItem(ITEM_HORN_OF_FENRIR, [](ItemDefinition& item) { item.requirements.level = 0; });
        EditItem(ITEM_HORN_OF_FENRIR, [](ItemDefinition& item) { item.requirements.level = 300; });
        CHECK(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->statProfile == "default");
    }

    SUBCASE("an item without a stat profile takes the one its data gives when edited")
    {
        ItemDefinition shield = *g_ItemDatabase.Find(ITEM_SMALL_SHIELD);
        shield.statProfile.clear();
        g_ItemDatabase.Set(shield);
        EditItem(ITEM_SMALL_SHIELD, [](ItemDefinition&) {});
        CHECK(g_ItemDatabase.Find(ITEM_SMALL_SHIELD)->statProfile == "shield");
    }

    SUBCASE("a wing's stat profile follows its slot")
    {
        REQUIRE(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "firstTierWing");
        EditItem(ITEM_WINGS_OF_ELF, [](ItemDefinition& item) { item.slot = ItemSlot::Helm; });
        CHECK(g_ItemDatabase.Find(ITEM_WINGS_OF_ELF)->statProfile == "armor");

        EditItem(ITEM_WINGS_OF_ELF, [](ItemDefinition& item) { item.slot = ItemSlot::Wings; });
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
    CHECK_FALSE(HasFatal(saved.issues));
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
    REQUIRE_FALSE(HasFatal(result.issues));
    REQUIRE(g_ItemDatabase.Find(KrisType) != nullptr);
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    CHECK(g_ItemModelDatabase.Find(KrisType)->file == krisModelFile);
    CHECK(g_ItemModelDatabase.Find(emptyType) == nullptr);

    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
    const std::string swordFile = GetItemGroupFileName(0);
    CHECK(TestFiles::ReadWholeFile(folder.Directory() / GetItemModelDataDirectory() / swordFile) ==
          TestFiles::ReadWholeFile(DataDirectory / "Items" / "Models" / swordFile));
}

TEST_CASE("A bmd import puts a moved entry without a name back with its model [data][items][editor]")
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
    // Sphere 4 moves to a free number; the bmd files have no item at either number.
    const int sphere4Type = MakeItemType(12, 73);
    const int movedType = MakeItemType(12, GetNextFreeItemNumber(12, 75));
    REQUIRE(g_ItemDatabase.GetAllSlots()[sphere4Type].entryWithoutName);
    REQUIRE(g_ItemModelDatabase.Find(sphere4Type) != nullptr);
    const std::string sphere4ModelFile = g_ItemModelDatabase.Find(sphere4Type)->file;
    REQUIRE(g_ItemDataHandler.MoveItem(sphere4Type, movedType));
    REQUIRE(g_ItemModelDatabase.Find(sphere4Type) == nullptr);

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasFatal(result.issues));
    CHECK(g_ItemDatabase.GetAllSlots()[sphere4Type].entryWithoutName);
    REQUIRE(g_ItemModelDatabase.Find(sphere4Type) != nullptr);
    CHECK(g_ItemModelDatabase.Find(sphere4Type)->file == sphere4ModelFile);
    CHECK(IsFreeItemType(movedType));
    std::vector<ItemDataIssue> issues;
    ValidateModelsHaveItemEntries(g_ItemDatabase.GetAllSlots(), g_ItemModelDatabase.GetAllSlots(), issues);
    CHECK(issues.empty());
}

TEST_CASE("After a bmd import every number with a model has an entry [data][items][editor]")
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
    // Items the bmd files do not have: Sphere 4 named since the start, and an item added with a model.
    const int sphere4Type = MakeItemType(12, 73);
    ItemDefinition sphere4 = g_ItemDatabase.GetAllSlots()[sphere4Type];
    sphere4.names = Data::LocalizedString::Parse("Sphere 4");
    g_ItemDataHandler.SetItem(sphere4);
    REQUIRE(g_ItemDatabase.Find(sphere4Type) != nullptr);
    const int addedType = MakeItemType(0, 100);
    REQUIRE(IsFreeItemType(addedType));
    ItemDefinition added;
    added.group = 0;
    added.number = 100;
    added.names = Data::LocalizedString::Parse("Added Sword");
    REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
    REQUIRE(g_ItemDataHandler.AddItem(added, g_ItemModelDatabase.Find(KrisType)));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasFatal(result.issues));
    for (const int itemType : {sphere4Type, addedType})
    {
        CAPTURE(itemType);
        CHECK(g_ItemDatabase.Find(itemType) == nullptr);
        CHECK(g_ItemDatabase.GetAllSlots()[itemType].entryWithoutName);
        CHECK(g_ItemModelDatabase.Find(itemType) != nullptr);
    }
    std::vector<ItemDataIssue> issues;
    ValidateModelsHaveItemEntries(g_ItemDatabase.GetAllSlots(), g_ItemModelDatabase.GetAllSlots(), issues);
    CHECK(issues.empty());
}

// The names of a language whose item.bmd is missing stay as they are, like
// the names of the languages that have none (German).
// Export as bmd writes the weights the original client grows into the requirements the client
// keeps, and Import from bmd takes them back.
TEST_CASE("Export as bmd and Import from bmd keep the orbs', crystals' and scrolls' requirements [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    std::vector<ItemDefinition> weighted;
    for (const ItemDefinition& item : g_ItemDatabase.GetAllSlots())
    {
        if (item.Exists() && HasRequirementWeightsInBmd(g_ItemDatabase.GetStatProfiles(), item) &&
            (item.requirements.strength > 0 || item.requirements.dexterity > 0 || item.requirements.energy > 0 ||
             item.requirements.vitality > 0))
        {
            weighted.push_back(item);
        }
    }
    REQUIRE(weighted.size() == 44);

    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    CHECK(changeLog.find("no weight gives") == std::string::npos);
    // The file holds the weight: the Healing Orb's energy of 52 as 100.
    ItemDataLoader::RawItemFile raw;
    REQUIRE(ItemDataLoader::ReadRawFile(CItemDataHandler::GetItemFilePath(L"Eng").c_str(), raw));
    REQUIRE_FALSE(raw.isLegacyFormat);
    REQUIRE(raw.recordSize == static_cast<int>(sizeof(ITEM_ATTRIBUTE_FILE)));
    ITEM_ATTRIBUTE_FILE healingOrb;
    std::memcpy(&healingOrb, raw.GetRecord(MakeItemType(12, 8)), sizeof(healingOrb));
    CHECK(healingOrb.RequireEnergy == 100);
    REQUIRE_FALSE(HasFatal(g_ItemDataHandler.ImportFromBmd().issues));
    for (const ItemDefinition& item : weighted)
    {
        CAPTURE(g_ItemDatabase.GetLogName(MakeItemType(item.group, item.number)));
        const ItemDefinition* imported = g_ItemDatabase.Find(item.group, item.number);
        REQUIRE(imported != nullptr);
        CHECK(imported->requirements == item.requirements);
    }
}

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
    REQUIRE_FALSE(HasFatal(result.issues));
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
    REQUIRE_FALSE(HasFatal(result.issues));
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

TEST_CASE("A bmd import keeps the entries without a name as they are and does not count them [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    std::string changeLog;
    REQUIRE(g_ItemDataHandler.ExportAsBmd(changeLog));
    // An entry without a name with the Healing Orb's profile and requirements: values, which the
    // import must not take for the weights of a bmd file.
    const ItemDefinition* healingOrb = g_ItemDatabase.Find(12, 8);
    REQUIRE(healingOrb != nullptr);
    REQUIRE(HasRequirementWeightsInBmd(g_ItemDatabase.GetStatProfiles(), *healingOrb));
    const int entryType = MakeItemType(12, 20);
    ItemDefinition entry = g_ItemDatabase.GetAllSlots()[entryType];
    REQUIRE(entry.entryWithoutName);
    entry.statProfile = healingOrb->statProfile.empty()
                            ? DeriveStatProfile(g_ItemDatabase.GetStatProfiles(), *healingOrb)
                            : healingOrb->statProfile;
    entry.level = healingOrb->level;
    entry.requirements = healingOrb->requirements;
    g_ItemDataHandler.RestoreItems(std::span<const ItemDefinition>(&entry, 1));
    REQUIRE(HasRequirementWeightsInBmd(g_ItemDatabase.GetStatProfiles(), entry));

    const ItemBmdImportResult result = g_ItemDataHandler.ImportFromBmd();
    REQUIRE_FALSE(HasFatal(result.issues));
    CHECK(g_ItemDatabase.GetAllSlots()[entryType] == entry);
    // The imported items are the ones of the bmd files.
    CHECK(std::ranges::none_of(result.items, [](const ItemDefinition& item) { return item.entryWithoutName; }));
    CHECK(static_cast<int>(result.items.size()) == g_ItemDatabase.GetExistingItemCount());
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
    REQUIRE_FALSE(HasFatal(result.issues));
    CHECK_FALSE(HasNameIn(*g_ItemDatabase.Find(BladeType), "pt"));
    CHECK(GetOwnName(*g_ItemDatabase.Find(KrisType), "es") == "Kris");
    CHECK(HasNameIn(*g_ItemDatabase.Find(BladeType), "es"));
}

TEST_CASE("The item tools set whole items, and the derived fields follow the data unless set [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    ItemDefinition blade = *g_ItemDatabase.Find(BladeType);

    SUBCASE("a value change reaches the database and ItemAttribute")
    {
        blade.damageMax = 99;
        blade.tags.Set(ItemTag::Valuable);
        g_ItemDataHandler.SetItem(blade);
        CHECK(g_ItemDatabase.Find(BladeType)->damageMax == 99);
        CHECK(g_ItemDatabase.HasTag(BladeType, ItemTag::Valuable));
        CHECK(ItemAttribute[BladeType].DamageMax == 99);
    }

    SUBCASE("a weapon kind the user picks stays")
    {
        blade.weaponKind = WeaponKind::Glove;
        g_ItemDataHandler.SetItem(blade);
        CHECK(g_ItemDatabase.Find(BladeType)->weaponKind == WeaponKind::Glove);
    }

    SUBCASE("a picked weapon kind follows the data again once the group's kind is picked")
    {
        blade.weaponKind = WeaponKind::Axe;
        g_ItemDataHandler.SetItem(blade);
        ItemDefinition picked = *g_ItemDatabase.Find(BladeType);
        REQUIRE(picked.weaponKind == WeaponKind::Axe);
        // Item Basics' "By the group".
        picked.weaponKind = GetGroupWeaponKind(picked);
        g_ItemDataHandler.SetItem(picked);
        ItemDefinition following = *g_ItemDatabase.Find(BladeType);
        CHECK(following.weaponKind == WeaponKind::Sword);
        // The slot list's "None": a sword only on its slot.
        following.slot = ItemSlot::None;
        g_ItemDataHandler.SetItem(following);
        CHECK(g_ItemDatabase.Find(BladeType)->slot == ItemSlot::None);
        CHECK(g_ItemDatabase.Find(BladeType)->weaponKind == WeaponKind::None);
    }

    SUBCASE("the weapon kind follows the classes in group 0 while it is the derived one")
    {
        // Only the Rage Fighter: group 0 makes it a glove.
        const WeaponKind before = blade.weaponKind;
        blade.classRequirements.fill(0);
        blade.classRequirements[CLASS_RAGEFIGHTER] = 1;
        g_ItemDataHandler.SetItem(blade);
        CHECK(g_ItemDatabase.Find(BladeType)->weaponKind != before);
        CHECK(g_ItemDatabase.Find(BladeType)->weaponKind == WeaponKind::Glove);
    }

    SUBCASE("an item without names leaves the database and empties its row")
    {
        blade.names = Data::LocalizedString{};
        g_ItemDataHandler.SetItem(blade);
        CHECK(g_ItemDatabase.Find(BladeType) == nullptr);
        CHECK(ItemAttribute[BladeType].Name[0] == L'\0');
        CHECK(ItemAttribute[BladeType].Width == 0);
    }

    SUBCASE("several items at once")
    {
        ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        kris.tradable = false;
        blade.tradable = false;
        const std::vector<ItemDefinition> changed = {kris, blade};
        g_ItemDataHandler.SetItems(changed);
        CHECK_FALSE(g_ItemDatabase.Find(KrisType)->tradable);
        CHECK_FALSE(g_ItemDatabase.Find(BladeType)->tradable);
    }
}

TEST_CASE("The item tools add, remove and move items with their models [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    const int freeType = MakeItemType(0, 100);
    REQUIRE(IsFreeItemType(freeType));
    CHECK_FALSE(IsFreeItemType(KrisType));

    SUBCASE("a new item takes a free number, with a model")
    {
        ItemDefinition item = *g_ItemDatabase.Find(KrisType);
        item.number = 100;
        item.names = Data::LocalizedString::Parse("Training Kris");
        ItemModelDefinition model = *g_ItemModelDatabase.Find(KrisType);
        REQUIRE(g_ItemDataHandler.AddItem(item, &model));
        REQUIRE(g_ItemDatabase.Find(freeType) != nullptr);
        CHECK(g_ItemDatabase.Find(freeType)->names.GetNeutral() == "Training Kris");
        CHECK(std::wstring(ItemAttribute[freeType].Name) == L"Training Kris");
        REQUIRE(g_ItemModelDatabase.Find(freeType) != nullptr);
        CHECK(g_ItemModelDatabase.Find(freeType)->number == 100);
        CHECK(g_ItemModelDatabase.Find(freeType)->file == model.file);
        CHECK(g_ItemDataHandler.HasModelsToReopen());
        // The number is taken now.
        CHECK_FALSE(g_ItemDataHandler.AddItem(item, nullptr));
    }

    SUBCASE("an item without names is not added")
    {
        ItemDefinition item;
        item.number = 100;
        CHECK_FALSE(g_ItemDataHandler.AddItem(item, nullptr));
        CHECK(IsFreeItemType(freeType));
    }

    SUBCASE("a removed item frees its number, its model too")
    {
        REQUIRE(g_ItemModelDatabase.Find(KrisType) != nullptr);
        REQUIRE(g_ItemDataHandler.RemoveItem(KrisType));
        CHECK(g_ItemDatabase.Find(KrisType) == nullptr);
        CHECK(g_ItemModelDatabase.Find(KrisType) == nullptr);
        CHECK(ItemAttribute[KrisType].Name[0] == L'\0');
        // Nothing is left of its values.
        CHECK(g_ItemDatabase.GetAllSlots()[KrisType].width == 0);
        CHECK(IsFreeItemType(KrisType));
        CHECK_FALSE(g_ItemDataHandler.RemoveItem(KrisType));
    }

    SUBCASE("a moved item takes its row and its model to the free number")
    {
        REQUIRE(g_ItemDataHandler.MoveItem(KrisType, freeType));
        CHECK(g_ItemDatabase.Find(KrisType) == nullptr);
        REQUIRE(g_ItemDatabase.Find(freeType) != nullptr);
        CHECK(g_ItemDatabase.Find(freeType)->names.GetNeutral() == "Kris");
        CHECK(std::wstring(ItemAttribute[freeType].Name) == L"Kris");
        CHECK(g_ItemModelDatabase.Find(freeType) != nullptr);
        CHECK_FALSE(g_ItemDataHandler.MoveItem(freeType, BladeType));
    }

    SUBCASE("the undo puts back exactly what was stored")
    {
        const ItemDefinition before = *g_ItemDatabase.Find(BladeType);
        ItemDefinition changed = before;
        changed.weaponKind = WeaponKind::Glove;
        g_ItemDataHandler.SetItem(changed);
        g_ItemDataHandler.RestoreItems(std::span<const ItemDefinition>(&before, 1));
        CHECK(*g_ItemDatabase.Find(BladeType) == before);
    }
}

TEST_CASE("The item tools change models and shared models [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    REQUIRE_FALSE(g_ItemModelDatabase.GetSharedModels().empty());
    const SharedItemModel shared = g_ItemModelDatabase.GetSharedModels().front();
    std::vector<int> users;
    for (const ItemModelDefinition& model : g_ItemModelDatabase.GetAllSlots())
    {
        if (model.Exists() && model.model == shared.name)
        {
            users.push_back(MakeItemType(model.group, model.number));
        }
    }
    REQUIRE(users.size() > 1);

    SUBCASE("a display value is not opened again, a file is")
    {
        ItemModelDefinition kris = *g_ItemModelDatabase.Find(KrisType);
        kris.inventory.scale = 0.004;
        g_ItemDataHandler.SetItemModel(kris);
        CHECK(g_ItemModelDatabase.Find(KrisType)->inventory.scale == 0.004);
        CHECK_FALSE(g_ItemDataHandler.HasModelsToReopen());
        kris.file = "Data/Item/Sword02.bmd";
        g_ItemDataHandler.SetItemModel(kris);
        CHECK(g_ItemDataHandler.HasModelsToReopen());
        // The game has no model slots in this test; nothing is opened.
        CHECK(g_ItemDataHandler.ReopenChangedModels({}).empty());
        CHECK_FALSE(g_ItemDataHandler.HasModelsToReopen());
    }

    SUBCASE("an item that names a shared model takes its file")
    {
        ItemModelDefinition kris = *g_ItemModelDatabase.Find(KrisType);
        kris.model = shared.name;
        kris.file.clear();
        g_ItemDataHandler.SetItemModel(kris);
        CHECK(g_ItemModelDatabase.Find(KrisType)->file == shared.file);
        CHECK(g_ItemModelDatabase.Find(KrisType)->textureFolders == shared.textureFolders);
    }

    SUBCASE("a changed shared model changes all its items")
    {
        SharedItemModel changed = shared;
        changed.file = "Data/Item/Sword03.bmd";
        g_ItemDataHandler.SetSharedModel(changed);
        for (const int user : users)
        {
            CHECK(g_ItemModelDatabase.Find(user)->file == "Data/Item/Sword03.bmd");
        }
        CHECK(g_ItemModelDatabase.FindSharedModel(shared.name)->file == "Data/Item/Sword03.bmd");
    }

    SUBCASE("a renamed shared model is named by all its items")
    {
        SharedItemModel renamed = shared;
        renamed.name = "renamedModel";
        g_ItemDataHandler.SetSharedModel(renamed, shared.name);
        CHECK(g_ItemModelDatabase.FindSharedModel(shared.name) == nullptr);
        for (const int user : users)
        {
            CHECK(g_ItemModelDatabase.Find(user)->model == "renamedModel");
        }
    }

    SUBCASE("a shared model in use is not removed")
    {
        CHECK_FALSE(g_ItemDataHandler.RemoveSharedModel(shared.name));
        CHECK(g_ItemModelDatabase.FindSharedModel(shared.name) != nullptr);
        SharedItemModel unused{"unusedModel", "Data/Item/Sword04.bmd", {"Item"}, {}};
        g_ItemDataHandler.SetSharedModel(unused);
        CHECK(g_ItemDataHandler.RemoveSharedModel("unusedModel"));
        CHECK(g_ItemModelDatabase.FindSharedModel("unusedModel") == nullptr);
    }

    SUBCASE("the save writes the changed model group and the shared model file")
    {
        SharedItemModel unused{"unusedModel", "Data/Item/Sword04.bmd", {"Item"}, {}};
        g_ItemDataHandler.SetSharedModel(unused);
        ItemModelDefinition kris = *g_ItemModelDatabase.Find(KrisType);
        kris.inventory.scale = 0.004;
        g_ItemDataHandler.SetItemModel(kris);
        std::vector<ItemDataIssue> issues;
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
        const std::filesystem::path models = folder.Directory() / GetItemModelDataDirectory();
        CHECK(TestFiles::ReadWholeFile(models / "SharedModels.json").find("unusedModel") != std::string::npos);
        CHECK(TestFiles::ReadWholeFile(models / GetItemGroupFileName(0)).find("0.004") != std::string::npos);
    }
}

TEST_CASE("The item tools' changes can be undone and redone [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    MuEditor::Items::ItemEdits& edits = g_ItemEdits;
    edits.GetHistory().Clear();
    const MuEditor::Items::ItemSlotState kris = MuEditor::Items::ItemEdits::ReadSlot(KrisType);

    SUBCASE("a field")
    {
        ItemDefinition changed = kris.item;
        changed.damageMax = 99;
        edits.SetItem("Damage", changed);
        REQUIRE(edits.Undo());
        CHECK(MuEditor::Items::ItemEdits::ReadSlot(KrisType) == kris);
        CHECK(ItemAttribute[KrisType].DamageMax == kris.item.damageMax);
        CHECK(g_ItemSelection.GetItemType() == KrisType);
        REQUIRE(edits.Redo());
        CHECK(g_ItemDatabase.Find(KrisType)->damageMax == 99);
    }

    SUBCASE("a removed item comes back with its model")
    {
        REQUIRE(edits.RemoveItem("Remove", KrisType));
        REQUIRE(edits.Undo());
        CHECK(MuEditor::Items::ItemEdits::ReadSlot(KrisType) == kris);
        CHECK(std::wstring(ItemAttribute[KrisType].Name) == L"Kris");
    }

    SUBCASE("a name")
    {
        REQUIRE(edits.SetItemName("Name", KrisType, "de", "Dolch"));
        REQUIRE(edits.Undo());
        CHECK(GetOwnName(*g_ItemDatabase.Find(KrisType), "de") != "Dolch");
        CHECK(MuEditor::Items::ItemEdits::ReadSlot(KrisType) == kris);
    }

    SUBCASE("a move")
    {
        const int target = MakeItemType(0, 100);
        REQUIRE(edits.MoveItem("Move", KrisType, target));
        REQUIRE(edits.Undo());
        CHECK(g_ItemDatabase.Find(KrisType) != nullptr);
        CHECK(g_ItemDatabase.Find(target) == nullptr);
        CHECK(g_ItemModelDatabase.Find(KrisType) != nullptr);
    }

    SUBCASE("a move to another group and back keeps a picked weapon kind")
    {
        ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
        kris.weaponKind = WeaponKind::Axe;
        edits.SetItem("Weapon kind", kris);
        const int target = MakeItemType(1, 100);
        REQUIRE(edits.MoveItem("Move", KrisType, target));
        REQUIRE(edits.Undo());
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Axe);
        // The step before the move is still as it left the item.
        REQUIRE(edits.Undo());
        CHECK(g_ItemDatabase.GetWeaponKind(KrisType) == WeaponKind::Sword);
    }

    SUBCASE("a renamed shared model")
    {
        const SharedItemModel shared = g_ItemModelDatabase.GetSharedModels().front();
        SharedItemModel renamed = shared;
        renamed.name = "renamedModel";
        edits.SetSharedModel("Rename", renamed, shared.name);
        REQUIRE(edits.Undo());
        REQUIRE(g_ItemModelDatabase.FindSharedModel(shared.name) != nullptr);
        CHECK(*g_ItemModelDatabase.FindSharedModel(shared.name) == shared);
        CHECK(g_ItemModelDatabase.FindSharedModel("renamedModel") == nullptr);
        for (const ItemModelDefinition& model : g_ItemModelDatabase.GetAllSlots())
        {
            CHECK(model.model != "renamedModel");
        }
    }

    SUBCASE("a change past the history is not overwritten")
    {
        ItemDefinition changed = kris.item;
        changed.damageMax = 99;
        edits.SetItem("Damage", changed);
        changed.damageMax = 50;
        g_ItemDataHandler.SetItem(changed);
        CHECK_FALSE(edits.Undo());
        CHECK(g_ItemDatabase.Find(KrisType)->damageMax == 50);
    }
    edits.GetHistory().Clear();
}

TEST_CASE("The usage of an item lists its tags, its model and the code that names it [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));

    const MuEditor::Items::ItemUsage kris = MuEditor::Items::FindItemUsage(KrisType);
    CHECK(kris.hasModel);
    CHECK(kris.IsUsed());
    // The editor was built from these sources, and the code names the Kris.
    REQUIRE(kris.codeKnown);
    CHECK_FALSE(kris.code.empty());
    // The Kris is in no ancient set, whether or not an earlier test loaded the set files.
    CHECK_FALSE(kris.inAncientSet);

    const MuEditor::Items::ItemUsage nothing = MuEditor::Items::FindItemUsage(MakeItemType(0, 100));
    CHECK_FALSE(nothing.hasModel);
    CHECK(nothing.tags.empty());
}

TEST_CASE("A number that is not free names what takes it [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));

    const MuEditor::Items::ItemNumberHolder kris = MuEditor::Items::FindItemNumberHolder(KrisType);
    CHECK(kris.item == "Kris (0,0)");
    CHECK(kris.model.empty());

    // Sphere 4 has its entry without a name and its model, but no item yet.
    const int sphere4Type = MakeItemType(12, 73);
    REQUIRE_FALSE(IsFreeItemType(sphere4Type));
    const MuEditor::Items::ItemNumberHolder sphere4 = MuEditor::Items::FindItemNumberHolder(sphere4Type);
    CHECK(sphere4.item.empty());
    CHECK(sphere4.entryWithoutName);
    CHECK(sphere4.model == "Data/Item/s30_sphere_body04.bmd");
    CHECK(MuEditor::Items::GetEntryLabel(sphere4Type) == "Entry without a name (12,73)");
    CHECK(MuEditor::Items::GetEntryLabel(KrisType) == "Kris (0,0)");
    CHECK(std::string(MuEditor::Items::GetNumberWithoutItemLabel(sphere4Type)) == "Entry without a name");
    CHECK(std::string(MuEditor::Items::GetNumberWithoutItemLabel(MakeItemType(0, 100))) == "No item at this number");
    // The editor was built from these sources, which name it.
    CHECK(sphere4.codeName == "ITEM_SPHERE_4");

    const MuEditor::Items::ItemNumberHolder freeNumber = MuEditor::Items::FindItemNumberHolder(MakeItemType(0, 100));
    CHECK(freeNumber.item.empty());
    CHECK_FALSE(freeNumber.entryWithoutName);
    CHECK(freeNumber.model.empty());
    CHECK(freeNumber.codeName.empty());
}

TEST_CASE("An entry without a name moves and goes as an item does [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    const int sphere4Type = MakeItemType(12, 73);
    const int freeType = MakeItemType(12, GetNextFreeItemNumber(12, 75));
    REQUIRE(IsFreeItemType(freeType));

    REQUIRE(g_ItemEdits.MoveItem("Move", sphere4Type, freeType));
    CHECK(IsFreeItemType(sphere4Type));
    CHECK(g_ItemDatabase.GetAllSlots()[freeType].entryWithoutName);
    CHECK(g_ItemModelDatabase.Find(freeType) != nullptr);
    REQUIRE(g_ItemEdits.Undo());
    CHECK(g_ItemDatabase.GetAllSlots()[sphere4Type].entryWithoutName);

    REQUIRE(g_ItemEdits.RemoveItem("Remove", sphere4Type));
    CHECK(IsFreeItemType(sphere4Type));
    REQUIRE(g_ItemEdits.Undo());
    CHECK(g_ItemDatabase.GetAllSlots()[sphere4Type].entryWithoutName);
    CHECK(g_ItemModelDatabase.Find(sphere4Type) != nullptr);
    g_ItemEdits.GetHistory().Clear();
}

namespace
{
// The id of a field an item tool window draws, inside the ID scopes it pushes
// (the item type, a requirement), with its label.
ImGuiID ItemToolFieldId(const char* windowId, std::initializer_list<int> scopes, const char* label)
{
    const ImGuiWindow* window = ImGui::FindWindowByName(windowId);
    REQUIRE(window != nullptr);
    ImGuiID seed = window->ID;
    for (const int scope : scopes)
    {
        seed = ImGui::GetIDWithSeed(scope, seed);
    }
    return ImGui::GetIDWithSeed(label, nullptr, seed);
}

// Item Basics and Item Requirements for a frame, in the editor's order, after
// the typed text.
void RenderBasicsAndRequirements(const char* typed = nullptr)
{
    if (typed != nullptr)
    {
        ImGui::GetIO().AddInputCharactersUTF8(typed);
    }
    bool open = true;
    ImGui::NewFrame();
    g_MuItemBasicsUI.Render(&open);
    g_MuItemRequirementsUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// One item tool for a frame, after the typed text; `draw` false hides it, as
// F12 hides the editor.
template <typename TTool> void RenderItemTool(TTool& tool, bool draw, const char* typed = nullptr)
{
    if (typed != nullptr)
    {
        ImGui::GetIO().AddInputCharactersUTF8(typed);
    }
    bool open = true;
    ImGui::NewFrame();
    if (draw)
    {
        tool.Render(&open);
    }
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// The Add item dialog for a frame; `open` opens it with the selected item.
void RenderAddDialog(MuEditor::Items::CItemAddDialog& dialog, bool open)
{
    ImGui::NewFrame();
    if (open)
    {
        dialog.OpenAdd(g_ItemSelection.GetItemType());
    }
    dialog.Render();
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// A widget of the Add item dialog, as a click or a click into a field.
void ActivateInAddDialog(const char* label)
{
    const ImGuiWindow* window = ImGui::FindWindowByName("###ItemAddDialog");
    REQUIRE(window != nullptr);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(label, nullptr, window->ID));
}

void RenderItemTools()
{
    bool open = true;
    g_MuItemBrowserUI.Render(&open);
    g_MuItemBasicsUI.Render(&open);
    g_MuItemRequirementsUI.Render(&open);
    g_MuItemRulesUI.Render(&open);
    g_MuItemCategoriesUI.Render(&open);
    g_MuItemModelUI.Render(&open);
    g_MuItemGlowColorsUI.Render(&open);
    g_MuItemSyncUI.Render(&open);
    g_MuItemFormulasUI.Render(&open);
    g_MuDataChecksUI.Render(&open);
    g_MuItemTranslationsUI.Render(&open);
}

void RenderModelTool()
{
    bool open = true;
    g_MuItemModelUI.BeforeFrame();
    ImGui::NewFrame();
    g_MuItemModelUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}

// A widget of the shared model form: the tool pushes the selected item's ID,
// the form's fields "SharedModel" too.
ImGuiID SharedModelFormId(const char* label, const char* scope = nullptr)
{
    const ImGuiWindow* tool = ImGui::FindWindowByName(MuEditor::WindowIds::ItemModel);
    REQUIRE(tool != nullptr);
    ImGuiID id = ImGui::GetIDWithSeed(g_ItemSelection.GetItemType(), tool->ID);
    if (scope != nullptr)
    {
        id = ImGui::GetIDWithSeed(scope, nullptr, id);
    }
    return ImGui::GetIDWithSeed(label, nullptr, id);
}

// As a click: pressed in the next frame; a disabled button is not.
void PressInModelTool(ImGuiID id)
{
    ImGui::ActivateItemByID(id);
    RenderModelTool();
}

// Activation selects all of the field's text; the typed text replaces it.
void TypeIntoModelTool(ImGuiID id, const char* text)
{
    ImGui::ActivateItemByID(id);
    RenderModelTool();
    ImGui::GetIO().AddInputCharactersUTF8(text);
    RenderModelTool();
}

ImGuiWindow* OpenPopupWindow()
{
    const ImGuiContext& g = *ImGui::GetCurrentContext();
    return g.OpenPopupStack.empty() ? nullptr : g.OpenPopupStack.back().Window;
}
} // namespace

// Draws every item tool for a few frames, with an item that has a model and a
// number without anything: ImGui counts what the windows get wrong (Begin and
// End, Push and Pop out of step) as errors of the frame.
TEST_CASE("The item tools draw their frames without ImGui errors [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    TestModelSlots slots;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    TestImGuiContext context;

    constexpr int Frames = 5;
    // Item Formulas with each kind of entry picked.
    using FormulaEntry = CMuItemFormulasUI::Entry;
    const FormulaEntry entries[] = {{FormulaEntry::Kind::DropLevelBonus, {}},
                                    {FormulaEntry::Kind::Default, {}},
                                    {FormulaEntry::Kind::Profile, "shield"},
                                    {FormulaEntry::Kind::Table, "defenseIncreaseShields"},
                                    {FormulaEntry::Kind::Rules, {}}};
    size_t entry = 0;
    for (const int itemType : {KrisType, MakeItemType(0, 100)})
    {
        g_ItemSelection.Select(itemType);
        for (int frame = 0; frame < Frames; ++frame)
        {
            g_MuItemFormulasUI.Show(entries[entry++ % std::size(entries)]);
            // Between frames, as the editor does.
            g_MuItemModelUI.BeforeFrame();
            ImGui::NewFrame();
            RenderItemTools();
            ImGui::Render();
            INFO("item type " << itemType << ", frame " << frame);
            CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        }
    }
    // The model tool's inventory display shown before it is stored ends.
    g_MuItemModelUI.BeforeFrame();
}

// A click into a field of one tool leaves the field of another, which is stored
// in that frame; the clicked tool takes the stored item, so its own edit does
// not bring the old values back.
TEST_CASE("A field clicked into while another item tool stores its edit keeps that edit [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    g_ItemSelection.Select(KrisType);
    REQUIRE(g_ItemDatabase.Find(KrisType)->requirements.level != 42);
    REQUIRE(g_ItemDatabase.Find(KrisType)->durability != 30);

    TestImGuiContext context;
    RenderBasicsAndRequirements();
    const ImGuiID level =
        ItemToolFieldId(MuEditor::WindowIds::ItemRequirements, {KrisType, static_cast<int>(ItemRequirement::Level)},
                        I18N::Metadata::ReqLevel);
    const ImGuiID durability = ItemToolFieldId(MuEditor::WindowIds::ItemBasics, {KrisType}, I18N::Metadata::Durability);
    // Activation selects the field's text: the typed text replaces it.
    ImGui::ActivateItemByID(level);
    RenderBasicsAndRequirements();
    REQUIRE(ImGui::GetActiveID() == level);
    RenderBasicsAndRequirements("42");

    // The click into Durability stores the level.
    ImGui::ActivateItemByID(durability);
    RenderBasicsAndRequirements();
    REQUIRE(ImGui::GetActiveID() == durability);
    REQUIRE(g_ItemDatabase.Find(KrisType)->requirements.level == 42);
    RenderBasicsAndRequirements("30");
    // Left by a click into the level: stored the frame after.
    ImGui::ActivateItemByID(level);
    RenderBasicsAndRequirements();
    RenderBasicsAndRequirements();

    CHECK(g_ItemDatabase.Find(KrisType)->durability == 30);
    CHECK(g_ItemDatabase.Find(KrisType)->requirements.level == 42);
    g_ItemEdits.GetHistory().Clear();
}

// The checks of a field left also hold for an edit stored without it: the
// window hidden while the field is typed in.
TEST_CASE(
    "Item Basics refuses an English name also when it is stored without its field being left [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    g_ItemSelection.Select(KrisType);
    const std::string kris = g_ItemDatabase.Find(KrisType)->names.GetNeutral();

    TestImGuiContext context;
    RenderItemTool(g_MuItemBasicsUI, true);
    const ImGuiID name = ItemToolFieldId(MuEditor::WindowIds::ItemBasics, {KrisType}, I18N::Editor::EnglishName);
    ImGui::ActivateItemByID(name);
    RenderItemTool(g_MuItemBasicsUI, true);
    REQUIRE(ImGui::GetActiveID() == name);
    RenderItemTool(g_MuItemBasicsUI, true, "Kr||is");

    SUBCASE("the window hidden, as F12 does")
    {
        RenderItemTool(g_MuItemBasicsUI, false);
        RenderItemTool(g_MuItemBasicsUI, false);
        RenderItemTool(g_MuItemBasicsUI, true);
    }
    SUBCASE("the field left")
    {
        ImGui::ActivateItemByID(
            ItemToolFieldId(MuEditor::WindowIds::ItemBasics, {KrisType}, I18N::Metadata::Durability));
        RenderItemTool(g_MuItemBasicsUI, true);
        RenderItemTool(g_MuItemBasicsUI, true);
    }
    CHECK(g_ItemDatabase.Find(KrisType)->names.GetNeutral() == kris);
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());
    g_ItemEdits.GetHistory().Clear();
}

// A +0 value that several weights give needs one picked from the list, which
// a store without its field being left cannot ask: it is dropped.
TEST_CASE("Item Requirements does not store a +0 value that needs a weight picked when its window is hidden "
          "[data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    g_ItemSelection.Select(KrisType);
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    REQUIRE(MuEditor::Items::HasWeight(kris, ItemRequirement::Strength));
    int value = MuEditor::Items::ComputeZeroValue(kris, ItemRequirement::Strength) + 1;
    while (MuEditor::Items::FindWeightsGiving(kris, ItemRequirement::Strength, value).GetCount() < 2)
    {
        ++value;
        REQUIRE(value < 1000);
    }
    REQUIRE(MuEditor::Items::ChooseWeight(kris, ItemRequirement::Strength, value, kris.requirements.strength).kind ==
            MuEditor::Items::WeightChoice::Kind::Pick);

    TestImGuiContext context;
    RenderItemTool(g_MuItemRequirementsUI, true);
    const ImGuiID strength =
        ItemToolFieldId(MuEditor::WindowIds::ItemRequirements, {KrisType, static_cast<int>(ItemRequirement::Strength)},
                        I18N::Metadata::ReqStrength);
    ImGui::ActivateItemByID(strength);
    RenderItemTool(g_MuItemRequirementsUI, true);
    REQUIRE(ImGui::GetActiveID() == strength);
    RenderItemTool(g_MuItemRequirementsUI, true, std::to_string(value).c_str());
    RenderItemTool(g_MuItemRequirementsUI, false);
    RenderItemTool(g_MuItemRequirementsUI, false);
    RenderItemTool(g_MuItemRequirementsUI, true);

    CHECK(g_ItemDatabase.Find(KrisType)->requirements.strength == kris.requirements.strength);
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());
    g_ItemEdits.GetHistory().Clear();
}

// The click from the weight field into the +0 field of the same requirement
// leaves the weight field, which stores under the same label: a +0 value
// typed next that several weights give still asks which.
TEST_CASE(
    "A +0 value typed right after the weight field of its requirement still asks for its weight [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    g_ItemSelection.Select(KrisType);
    const int typedWeight = g_ItemDatabase.Find(KrisType)->requirements.strength + 10;

    TestImGuiContext context;
    RenderItemTool(g_MuItemRequirementsUI, true);
    const std::initializer_list<int> strengthScope = {KrisType, static_cast<int>(ItemRequirement::Strength)};
    const ImGuiID weight = ItemToolFieldId(MuEditor::WindowIds::ItemRequirements, strengthScope, I18N::Editor::Weight);
    const ImGuiID zeroValue =
        ItemToolFieldId(MuEditor::WindowIds::ItemRequirements, strengthScope, I18N::Metadata::ReqStrength);
    ImGui::ActivateItemByID(weight);
    RenderItemTool(g_MuItemRequirementsUI, true);
    REQUIRE(ImGui::GetActiveID() == weight);
    RenderItemTool(g_MuItemRequirementsUI, true, std::to_string(typedWeight).c_str());
    ImGui::ActivateItemByID(zeroValue);
    RenderItemTool(g_MuItemRequirementsUI, true);
    REQUIRE(ImGui::GetActiveID() == zeroValue);
    REQUIRE(g_ItemDatabase.Find(KrisType)->requirements.strength == typedWeight);

    // A +0 value several other weights give.
    const ItemDefinition weighted = *g_ItemDatabase.Find(KrisType);
    int value = MuEditor::Items::ComputeZeroValue(weighted, ItemRequirement::Strength) + 1;
    while (MuEditor::Items::FindWeightsGiving(weighted, ItemRequirement::Strength, value).GetCount() < 2)
    {
        ++value;
        REQUIRE(value < 2000);
    }
    REQUIRE(MuEditor::Items::ChooseWeight(weighted, ItemRequirement::Strength, value, typedWeight).kind ==
            MuEditor::Items::WeightChoice::Kind::Pick);
    RenderItemTool(g_MuItemRequirementsUI, true, std::to_string(value).c_str());
    // Left by a click into the weight field.
    ImGui::ActivateItemByID(weight);
    RenderItemTool(g_MuItemRequirementsUI, true);
    RenderItemTool(g_MuItemRequirementsUI, true);

    // The weight list asks; nothing is stored until one is taken. Also not when
    // the list takes the keyboard from the weight field: activated by
    // navigation, as this test clicks, it became active a frame before the +0
    // field was reported left, with the weight of the typed value.
    for (int frame = 0; frame < 3; ++frame)
    {
        INFO("frame " << frame);
        CHECK(OpenPopupWindow() != nullptr);
        CHECK(g_ItemDatabase.Find(KrisType)->requirements.strength == typedWeight);
        RenderItemTool(g_MuItemRequirementsUI, true);
    }
    g_ItemEdits.GetHistory().Clear();
}

// A click from a +0 value into the +0 field of a requirement drawn above it
// makes that field active, earlier in the same frame, before the first one is
// reported left: the typed value still asks for its own weight.
TEST_CASE("A +0 value left by a click into the +0 field above it still asks for its weight [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    g_ItemSelection.Select(KrisType);
    const ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    REQUIRE(MuEditor::Items::HasWeight(kris, ItemRequirement::Strength));
    REQUIRE(MuEditor::Items::HasWeight(kris, ItemRequirement::Dexterity));
    int value = MuEditor::Items::ComputeZeroValue(kris, ItemRequirement::Dexterity) + 1;
    while (MuEditor::Items::FindWeightsGiving(kris, ItemRequirement::Dexterity, value).GetCount() < 2)
    {
        ++value;
        REQUIRE(value < 1000);
    }
    REQUIRE(MuEditor::Items::ChooseWeight(kris, ItemRequirement::Dexterity, value, kris.requirements.dexterity).kind ==
            MuEditor::Items::WeightChoice::Kind::Pick);

    TestImGuiContext context;
    RenderItemTool(g_MuItemRequirementsUI, true);
    const ImGuiID strength =
        ItemToolFieldId(MuEditor::WindowIds::ItemRequirements, {KrisType, static_cast<int>(ItemRequirement::Strength)},
                        I18N::Metadata::ReqStrength);
    const ImGuiID dexterity =
        ItemToolFieldId(MuEditor::WindowIds::ItemRequirements, {KrisType, static_cast<int>(ItemRequirement::Dexterity)},
                        I18N::Metadata::ReqDexterity);
    ImGui::ActivateItemByID(dexterity);
    RenderItemTool(g_MuItemRequirementsUI, true);
    REQUIRE(ImGui::GetActiveID() == dexterity);
    RenderItemTool(g_MuItemRequirementsUI, true, std::to_string(value).c_str());
    ImGui::ActivateItemByID(strength);
    RenderItemTool(g_MuItemRequirementsUI, true);

    // The list asks, and nothing is stored until a weight is taken.
    for (int frame = 0; frame < 3; ++frame)
    {
        INFO("frame " << frame);
        RenderItemTool(g_MuItemRequirementsUI, true);
        CHECK(OpenPopupWindow() != nullptr);
        CHECK(g_ItemDatabase.Find(KrisType)->requirements.strength == kris.requirements.strength);
        CHECK(g_ItemDatabase.Find(KrisType)->requirements.dexterity == kris.requirements.dexterity);
    }
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());

    // It is the list of the typed dexterity value.
    const ImGuiWindow* list = OpenPopupWindow();
    REQUIRE(list != nullptr);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(I18N::Editor::TakeThisWeight, nullptr, list->ID));
    RenderItemTool(g_MuItemRequirementsUI, true);
    RenderItemTool(g_MuItemRequirementsUI, true);
    CHECK(OpenPopupWindow() == nullptr);
    CHECK(MuEditor::Items::FindWeightsGiving(kris, ItemRequirement::Dexterity, value)
              .Contains(g_ItemDatabase.Find(KrisType)->requirements.dexterity));
    CHECK(g_ItemDatabase.Find(KrisType)->requirements.strength == kris.requirements.strength);
    CHECK(g_ItemEdits.GetHistory().CanUndo());
    g_ItemEdits.GetHistory().Clear();
}

// An item added with a size of its own starts without a slot, so no class can
// wear it, as most items of groups 12 to 15.
TEST_CASE("Add item without copying makes an item without a slot [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    // Selected: the dialog adds to its group.
    const int potionType = MakeItemType(14, 1);
    REQUIRE(g_ItemDatabase.Find(potionType) != nullptr);
    g_ItemSelection.Select(potionType);

    TestImGuiContext context;
    MuEditor::Items::CItemAddDialog dialog;
    RenderAddDialog(dialog, true);
    RenderAddDialog(dialog, false);
    // Unchecked: a size and slot of its own.
    ActivateInAddDialog(I18N::Editor::CopyTheSelectedItem);
    RenderAddDialog(dialog, false);
    ActivateInAddDialog(I18N::Editor::EnglishName);
    RenderAddDialog(dialog, false);
    ImGui::GetIO().AddInputCharactersUTF8("Test Potion");
    RenderAddDialog(dialog, false);
    ActivateInAddDialog(I18N::Editor::Add);
    RenderAddDialog(dialog, false);

    // Add selects the new item.
    REQUIRE(g_ItemSelection.GetItemType() != potionType);
    const ItemDefinition* added = g_ItemDatabase.Find(g_ItemSelection.GetItemType());
    REQUIRE(added != nullptr);
    CHECK(added->names.GetNeutral() == "Test Potion");
    CHECK(added->group == 14);
    CHECK(added->slot == ItemSlot::None);
    CHECK(added->weaponKind == WeaponKind::None);
    CHECK(std::ranges::all_of(added->classRequirements, [](BYTE step) { return step == 0; }));
    g_ItemEdits.GetHistory().Clear();
}

// A click into the world leaves no editor window focused, and the game window
// has the keyboard: the Map Editor's keys still reach it then, but not while
// a text field of another window is typed in.
TEST_CASE("The Map Editor's Ctrl+Z and Delete reach it after a click into the world [editor]")
{
    TestImGuiContext context;
    // A test's editor core has no window: the game has the keyboard.
    REQUIRE_FALSE(g_MuEditorCore.IsKeyboardWithEditor());
    ImGuiIO& io = ImGui::GetIO();
    int undos = 0;
    int deletes = 0;
    bool focusText = false;
    char text[16] = "abc";
    const auto frame = [&]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin(MuEditor::WindowIds::MapEditor);
        undos += CMapEditorUI::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z) ? 1 : 0;
        deletes += CMapEditorUI::Shortcut(ImGuiKey_Delete) ? 1 : 0;
        ImGui::End();
        ImGui::SetNextWindowPos(ImVec2(400.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin("Other tool");
        if (focusText)
        {
            ImGui::SetKeyboardFocusHere();
            focusText = false;
        }
        ImGui::InputText("Name", text, sizeof(text));
        ImGui::End();
        ImGui::Render();
    };
    const auto press = [&](ImGuiKeyChord chord)
    {
        const auto key = static_cast<ImGuiKey>(chord & ~ImGuiMod_Mask_);
        const bool ctrl = (chord & ImGuiMod_Ctrl) != 0;
        if (ctrl)
        {
            io.AddKeyEvent(ImGuiMod_Ctrl, true);
        }
        io.AddKeyEvent(key, true);
        frame();
        io.AddKeyEvent(key, false);
        if (ctrl)
        {
            io.AddKeyEvent(ImGuiMod_Ctrl, false);
        }
        frame();
    };
    frame();
    frame();

    SUBCASE("after a click into the world")
    {
        // No window there.
        io.AddMousePosEvent(1000.0f, 800.0f);
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        frame();
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        frame();
        frame();
        REQUIRE(ImGui::GetCurrentContext()->NavWindow == nullptr);
        press(ImGuiMod_Ctrl | ImGuiKey_Z);
        press(ImGuiKey_Delete);
        CHECK(undos == 1);
        CHECK(deletes == 1);
    }
    SUBCASE("not while a text field of another window is typed in")
    {
        focusText = true;
        frame();
        frame();
        frame();
        REQUIRE(ImGui::GetActiveID() != 0);
        press(ImGuiKey_Delete);
        press(ImGuiMod_Ctrl | ImGuiKey_Z);
        CHECK(deletes == 0);
        CHECK(undos == 0);
    }
}

// ImGui keeps the focus in an item tool when the game window takes the
// keyboard back (Alt+Tab with the editor inside a full-screen game): a Ctrl+Z
// typed while playing does not undo the last item edit.
TEST_CASE("An item tool's Ctrl+Z does not undo while the game window has the keyboard [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    ++kris.damageMax;
    REQUIRE(g_ItemEdits.SetItem("Change the Kris", kris));
    g_ItemSelection.Select(KrisType);

    TestImGuiContext context;
    // A test's editor core has no window: the game has the keyboard.
    REQUIRE_FALSE(g_MuEditorCore.IsKeyboardWithEditor());
    ImGuiIO& io = ImGui::GetIO();
    bool ctrlZPressed = false;
    const auto frame = [&](bool focusTool)
    {
        bool open = true;
        ImGui::NewFrame();
        if (focusTool)
        {
            ImGui::SetWindowFocus(MuEditor::WindowIds::ItemRules);
        }
        g_MuItemRulesUI.Render(&open);
        ctrlZPressed = ctrlZPressed || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z);
        ImGui::Render();
    };
    frame(false);
    frame(true);
    frame(false);
    REQUIRE(ImGui::GetCurrentContext()->NavWindow == ImGui::FindWindowByName(MuEditor::WindowIds::ItemRules));
    io.AddKeyEvent(ImGuiMod_Ctrl, true);
    io.AddKeyEvent(ImGuiKey_Z, true);
    frame(false);
    io.AddKeyEvent(ImGuiKey_Z, false);
    io.AddKeyEvent(ImGuiMod_Ctrl, false);
    frame(false);

    // The press arrived, and the step is still there.
    CHECK(ctrlZPressed);
    CHECK(g_ItemEdits.GetHistory().CanUndo());
    CHECK(g_ItemDatabase.Find(KrisType)->damageMax == kris.damageMax);
    g_ItemEdits.GetHistory().Clear();
}

// ImGui keeps the row a Shift click starts from as an index into the rows: a
// search that lists other rows moves it to its item's row, or drops it.
TEST_CASE("The Shift click source of Item Rules follows its item when other rows are listed [data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    std::vector<int> swords;
    std::vector<int> bows;
    for (const ItemDefinition& item : g_ItemDatabase.GetAllSlots())
    {
        if (item.Exists() && item.group == 0 && swords.size() < 20)
        {
            swords.push_back(MakeItemType(item.group, item.number));
        }
        if (item.Exists() && item.group == 4)
        {
            bows.push_back(MakeItemType(item.group, item.number));
        }
    }
    REQUIRE(swords.size() == 20);

    TestImGuiContext context;
    MuEditor::Items::CItemRulesTable table;
    const auto draw = [&](const std::vector<int>& rows)
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
        ImGui::Begin("Rules");
        table.Render(rows, -1, false);
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    draw(swords);
    ImPool<ImGuiMultiSelectState>& states = ImGui::GetCurrentContext()->MultiSelectStorage;
    REQUIRE(states.GetAliveCount() == 1);
    ImGuiMultiSelectState* state = states.GetByIndex(0);
    // As a click on row 10 leaves it.
    state->RangeSrcItem = 10;
    draw(std::vector<int>(swords.begin() + 7, swords.end()));
    // The same sword, now in row 3.
    CHECK(state->RangeSrcItem == 3);
    draw(bows);
    CHECK(state->RangeSrcItem == ImGuiSelectionUserData_Invalid);
}

// A layout saved before the item tools: the item table (now the browser) in
// the tools above the console. An item tool opened the first time docks with
// the browser, not into the console.
TEST_CASE(
    "An item tool opened the first time in a layout from before the item tools docks with the browser [editor][items]")
{
    TestImGuiContext context;
    ImGuiID tools = 0;
    ImGuiID console = 0;
    bool browserOpen = true;
    bool browserSaved = true;
    bool translationsSaved = false;
    const auto frame = [&]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(1200.0f, 800.0f));
        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoDocking);
        // As BuildDefaultLayout made it before the item tools.
        const ImGuiID dockSpace = ImGui::GetID("ToolWindows");
        if (ImGui::DockBuilderGetNode(dockSpace) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockSpace, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockSpace, ImVec2(1200.0f, 780.0f));
            console = ImGui::DockBuilderSplitNode(dockSpace, ImGuiDir_Down, 0.25f, nullptr, &tools);
            ImGui::DockBuilderDockWindow(MuEditor::WindowIds::Console, console);
            if (browserSaved)
            {
                ImGui::DockBuilderDockWindow(MuEditor::WindowIds::ItemBrowser, tools);
            }
            if (translationsSaved)
            {
                // Saved by phase 5, not open.
                ImGui::DockBuilderDockWindow(MuEditor::WindowIds::ItemTranslations, tools);
            }
            ImGui::DockBuilderFinish(dockSpace);
        }
        // The node is there: it is not built again.
        MuEditor::Layout::SubmitDockSpace(false);
        ImGui::End();
        if (browserOpen)
        {
            ImGui::Begin(MuEditor::WindowIds::ItemBrowser);
            ImGui::End();
        }
        MuEditor::Layout::DockBesideItemTools(MuEditor::WindowIds::ItemBasics);
        ImGui::Begin(MuEditor::WindowIds::ItemBasics);
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    SUBCASE("the browser open") {}
    SUBCASE("the browser not open")
    {
        browserOpen = false;
    }
    SUBCASE("only a tool saved where the browser was, not open")
    {
        // The browser gives no place: the tool's saved one does.
        browserOpen = false;
        browserSaved = false;
        translationsSaved = true;
    }
    frame();
    frame();
    const ImGuiWindow* basics = ImGui::FindWindowByName(MuEditor::WindowIds::ItemBasics);
    REQUIRE(basics != nullptr);
    CHECK(basics->DockId == tools);
}

// The warning before a shared model changes lists its items; a click on one
// selects it, and the warning stays open to apply the change.
TEST_CASE("A click in the shared model warning's list keeps the warning open [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    TestModelSlots slots;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    const SharedItemModel shared = g_ItemModelDatabase.GetSharedModels().front();
    const std::vector<int> users = MuEditor::Items::FindSharedModelUsers(shared.name);
    REQUIRE(users.size() > 1);
    REQUIRE(shared.noneBlendMeshes != std::vector<int>{1, 2});

    TestImGuiContext context;
    g_ItemSelection.Select(users[0]);
    for (int frame = 0; frame < 3; ++frame)
    {
        RenderModelTool();
    }
    TypeIntoModelTool(SharedModelFormId(I18N::Editor::NoneBlendMeshes, "SharedModel"), "1, 2");
    PressInModelTool(SharedModelFormId(I18N::Editor::ApplyToTheSharedModel));
    RenderModelTool();
    REQUIRE(OpenPopupWindow() != nullptr);

    // As a click on the second item in the warning's list does.
    g_ItemSelection.Select(users[1]);
    for (int frame = 0; frame < 3; ++frame)
    {
        RenderModelTool();
    }
    ImGuiWindow* warning = OpenPopupWindow();
    REQUIRE(warning != nullptr);
    PressInModelTool(ImGui::GetIDWithSeed(I18N::Editor::ApplyToTheSharedModel, nullptr, warning->ID));
    CHECK(OpenPopupWindow() == nullptr);
    CHECK(g_ItemModelDatabase.FindSharedModel(shared.name)->noneBlendMeshes == std::vector<int>{1, 2});
    CHECK(g_ItemEdits.GetHistory().GetUndoLabel() == I18N::Editor::ChangeASharedModel);
    g_ItemEdits.GetHistory().Clear();
}

// The meshes typed without the spaces the field writes are applied as numbers;
// the form follows the undo of that change.
TEST_CASE("The shared model form follows an undo of meshes typed without spaces [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    TestModelSlots slots;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    g_ItemEdits.GetHistory().Clear();
    const SharedItemModel shared = g_ItemModelDatabase.GetSharedModels().front();
    const std::vector<int> users = MuEditor::Items::FindSharedModelUsers(shared.name);
    REQUIRE(users.size() > 1);
    REQUIRE(shared.noneBlendMeshes != std::vector<int>{1, 2});

    TestImGuiContext context;
    g_ItemSelection.Select(users[0]);
    for (int frame = 0; frame < 3; ++frame)
    {
        RenderModelTool();
    }
    TypeIntoModelTool(SharedModelFormId(I18N::Editor::NoneBlendMeshes, "SharedModel"), "1,2");
    PressInModelTool(SharedModelFormId(I18N::Editor::ApplyToTheSharedModel));
    RenderModelTool();
    ImGuiWindow* warning = OpenPopupWindow();
    REQUIRE(warning != nullptr);
    PressInModelTool(ImGui::GetIDWithSeed(I18N::Editor::ApplyToTheSharedModel, nullptr, warning->ID));
    REQUIRE(g_ItemModelDatabase.FindSharedModel(shared.name)->noneBlendMeshes == std::vector<int>{1, 2});

    // What Ctrl+Z does.
    REQUIRE(g_ItemEdits.Undo());
    RenderModelTool();
    CHECK(g_ItemModelDatabase.FindSharedModel(shared.name)->noneBlendMeshes == shared.noneBlendMeshes);
    // The form shows the undone model, so there is nothing to apply.
    PressInModelTool(SharedModelFormId(I18N::Editor::ApplyToTheSharedModel));
    CHECK(OpenPopupWindow() == nullptr);
    g_ItemEdits.GetHistory().Clear();
}

// Items with the same name: the lucky sets have a tag their namesakes do not,
// the Seals of Wealth and the seed sphere tiers count up by number (13/44's
// noClassLines only leaves tooltip lines out, so it does not tell them apart).
TEST_CASE("The item lists tell items with the same name apart [data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    MuEditor::Items::ItemNamesakes& namesakes = g_ItemNamesakes;

    CHECK(namesakes.GetQualifier(KrisType).empty());
    CHECK(namesakes.GetQualifier(MakeItemType(7, 1)).empty());
    CHECK(namesakes.GetQualifier(MakeItemType(7, 67)) == "luckyItem");
    CHECK(namesakes.GetQualifier(MakeItemType(13, 44)) == "#1");
    CHECK(namesakes.GetQualifier(MakeItemType(13, 116)) == "#2");
    // The Divine and the Faith set have names of their own.
    CHECK(namesakes.GetQualifier(MakeItemType(7, 19)).empty());
    CHECK(namesakes.GetQualifier(MakeItemType(7, 50)).empty());
    CHECK(namesakes.GetQualifier(MakeItemType(12, 100)) == "#1");
    CHECK(namesakes.GetQualifier(MakeItemType(12, 124)) == "#5");

    // A renamed item has a namesake no more.
    ItemDefinition renamed = *g_ItemDatabase.Find(MakeItemType(13, 116));
    renamed.name = L"Seal of Plenty";
    renamed.names.Set("en", "Seal of Plenty");
    g_ItemDataHandler.SetItem(renamed);
    CHECK(namesakes.GetQualifier(MakeItemType(13, 44)).empty());
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
}

// The stats a weapon, a staff, a shield, a helm and wings have at each level,
// normal, excellent and ancient, as the game computes them for the stored item.
TEST_CASE("Item Basics shows the stats at each level, from the values being typed [data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    using MuEditor::Items::ComputeItemLevelValues;
    using MuEditor::Items::ItemLevelValues;
    using MuEditor::Items::RequirementVariant;

    for (const int itemType : {KrisType, static_cast<int>(ITEM_SKULL_STAFF), static_cast<int>(ITEM_SMALL_SHIELD),
                               static_cast<int>(ITEM_DRAGON_HELM), static_cast<int>(ITEM_WINGS_OF_ELF)})
    {
        const ItemDefinition item = *g_ItemDatabase.Find(itemType);
        for (const RequirementVariant variant :
             {RequirementVariant{}, RequirementVariant{true, false, 0}, RequirementVariant{false, true, 0}})
        {
            const ItemLevelValues values = ComputeItemLevelValues(item, variant);
            for (int level = 0; level < Data::Items::ItemLevelCount; ++level)
            {
                const GameLogic::Items::ItemStatValues game = GameLogic::Items::ComputeItemStats(
                    ItemAttribute[itemType], {itemType, level, static_cast<BYTE>(variant.excellent ? 1 : 0),
                                              static_cast<BYTE>(variant.ancient ? 1 : 0), 0});
                const GameLogic::Items::ItemStatValues& shown = values[static_cast<size_t>(level)];
                INFO(g_ItemDatabase.GetLogName(itemType)
                     << " +" << level << " excellent " << variant.excellent << " ancient " << variant.ancient);
                CHECK(shown.damageMin == game.damageMin);
                CHECK(shown.damageMax == game.damageMax);
                CHECK(shown.magicPower == game.magicPower);
                CHECK(shown.defense == game.defense);
                CHECK(shown.magicDefense == game.magicDefense);
                CHECK(shown.blockRate == game.blockRate);
            }
        }
    }

    // A base value being typed counts at once.
    ItemDefinition stronger = *g_ItemDatabase.Find(KrisType);
    const int before = ComputeItemLevelValues(stronger, {})[5].damageMax;
    stronger.damageMax = static_cast<BYTE>(stronger.damageMax + 10);
    CHECK(ComputeItemLevelValues(stronger, {})[5].damageMax == before + 10);
}

TEST_CASE("Item Requirements shows what the game asks at each level, from the values being typed "
          "[data][items][editor]")
{
    ClientDataScope client(ShippedClientDirectory);
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    using MuEditor::Items::ComputeRequirementTable;
    using MuEditor::Items::FindRequirementBase;
    using MuEditor::Items::RequirementTable;
    constexpr size_t Strength = static_cast<size_t>(ItemRequirement::Strength);

    ItemDefinition kris = *g_ItemDatabase.Find(KrisType);
    // As the game's /Kris window shows it: 20 + 40 * (6 + 3 * level) * 3 / 100.
    const RequirementTable normal = ComputeRequirementTable(kris, {});
    const int expected[] = {27, 30, 34, 38, 41, 45, 48, 52, 56, 59, 63, 66, 70, 74, 77, 81};
    for (size_t level = 0; level < normal.size(); ++level)
    {
        CHECK(normal[level][Strength] == expected[level]);
    }
    CHECK(ComputeRequirementTable(kris, {true, false, 0})[0][Strength] == 57);
    CHECK(ComputeRequirementTable(kris, {false, false, 4})[0][Strength] == 47);
    // The weapon profile adds strength per option level, the default one none.
    CHECK(MuEditor::Items::OptionLevelAddsStrength(kris));
    ItemDefinition defaultProfile = kris;
    defaultProfile.statProfile = "default";
    CHECK_FALSE(MuEditor::Items::OptionLevelAddsStrength(defaultProfile));

    // A base value being typed counts at once.
    ItemDefinition stronger = kris;
    stronger.requirements.strength = 80;
    CHECK(ComputeRequirementTable(stronger, {})[0][Strength] > normal[0][Strength]);

    // By item level: the values a normal item has, so nothing changes for it;
    // turning it off finds the base value back.
    RequirementByLevel values{};
    for (size_t level = 0; level < values.size(); ++level)
    {
        values[level] = static_cast<WORD>(normal[level][Strength]);
    }
    ItemDefinition byLevel = kris;
    byLevel.requirementsByLevel.Set(ItemRequirement::Strength, values);
    byLevel.requirements.strength = 0;
    const RequirementTable fromLevels = ComputeRequirementTable(byLevel, {});
    for (size_t level = 0; level < fromLevels.size(); ++level)
    {
        CHECK(fromLevels[level][Strength] == normal[level][Strength]);
    }
    CHECK(FindRequirementBase(byLevel, ItemRequirement::Strength, values) == kris.requirements.strength);
    CHECK(FindRequirementBase(byLevel, ItemRequirement::Level, RequirementByLevel{12, 12}) == 12);
    // Also with a strength at the highest value, which the addition cannot raise.
    ItemDefinition capped = byLevel;
    RequirementByLevel cappedValues = values;
    cappedValues[0] = 65535;
    capped.requirementsByLevel.Set(ItemRequirement::Strength, cappedValues);
    CHECK(MuEditor::Items::OptionLevelAddsStrength(capped));

    // A +0 value no base gives: the base with the nearest one.
    const ItemDefinition darkBreaker = *g_ItemDatabase.Find(MakeItemType(0, 17));
    const RequirementTable darkBreakerTable = ComputeRequirementTable(darkBreaker, {});
    RequirementByLevel typed{};
    for (size_t level = 0; level < typed.size(); ++level)
    {
        typed[level] = static_cast<WORD>(darkBreakerTable[level][Strength]);
    }
    // A base step adds about 3 to the +0 value of this sword (581 at 180, 584
    // at 181), so 582 is nearest to the base it has.
    typed[0] = static_cast<WORD>(typed[0] + 1);
    CHECK(FindRequirementBase(darkBreaker, ItemRequirement::Strength, typed) == darkBreaker.requirements.strength);

    // With drop level 0 every base above 0 gives the same +0 value; the other
    // levels still find the base back.
    ItemDefinition dropLevelZero = kris;
    dropLevelZero.level = 0;
    dropLevelZero.requirements.strength = 1500;
    const RequirementTable dropLevelZeroTable = ComputeRequirementTable(dropLevelZero, {});
    RequirementByLevel fromDropLevelZero{};
    for (size_t level = 0; level < fromDropLevelZero.size(); ++level)
    {
        fromDropLevelZero[level] = static_cast<WORD>(dropLevelZeroTable[level][Strength]);
    }
    ItemDefinition weakest = dropLevelZero;
    weakest.requirements.strength = 1;
    CHECK(ComputeRequirementTable(weakest, {})[0][Strength] == dropLevelZeroTable[0][Strength]);
    CHECK(FindRequirementBase(dropLevelZero, ItemRequirement::Strength, fromDropLevelZero) == 1500);
}

// A number field is typed into, then its form is not drawn for a frame (its
// window hidden, the editor closed): the edit is stored, as when it is left.
TEST_CASE("An item tool's edit is stored when its form stops being drawn [editor][items]")
{
    TestImGuiContext context;
    int stored = 0;
    int storedTimes = 0;
    MuEditor::Items::ItemEditDraft<int> draft([&](int) { return std::optional<int>(stored); },
                                              [&](const std::string&, const int& value)
                                              {
                                                  stored = value;
                                                  ++storedTimes;
                                              });
    const auto drawForm = [&](bool typeDigit)
    {
        if (typeDigit)
        {
            ImGui::GetIO().AddInputCharacter('7');
        }
        ImGui::NewFrame();
        ImGui::Begin("Form");
        int* value = draft.Begin(0, 0);
        REQUIRE(value != nullptr);
        if (ImGui::GetFrameCount() == 1)
        {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputInt("Value", value);
        draft.AfterField("Value");
        draft.End();
        ImGui::End();
        ImGui::Render();
    };
    const auto skipFrames = [](int frames)
    {
        for (int frame = 0; frame < frames; ++frame)
        {
            ImGui::NewFrame();
            ImGui::Render();
        }
    };
    drawForm(false);
    drawForm(false);
    drawForm(true);
    REQUIRE(storedTimes == 0);

    SUBCASE("for frames")
    {
        skipFrames(3);
    }
    SUBCASE("for one frame, after which ImGui still reports the field as left")
    {
        skipFrames(1);
    }
    drawForm(false);
    CHECK(storedTimes == 1);
    CHECK(stored == 7);
}

// The arrow of a header closes its section on mouse down, before ImGui reports
// the field in it left; the closed section does not draw the field to report
// it. SetNextItemOpen(false) closes it as the arrow does.
TEST_CASE("An item tool's edit is stored when its section is closed while it is typed [editor][items]")
{
    TestImGuiContext context;
    int stored = 0;
    int storedTimes = 0;
    MuEditor::Items::ItemEditDraft<int> draft([&](int) { return std::optional<int>(stored); },
                                              [&](const std::string&, const int& value)
                                              {
                                                  stored = value;
                                                  ++storedTimes;
                                              });
    const auto drawForm = [&](bool sectionOpen, bool typeDigit)
    {
        if (typeDigit)
        {
            ImGui::GetIO().AddInputCharacter('7');
        }
        ImGui::NewFrame();
        ImGui::Begin("Form");
        int* value = draft.Begin(0, 0);
        REQUIRE(value != nullptr);
        ImGui::SetNextItemOpen(sectionOpen);
        if (ImGui::CollapsingHeader("Section"))
        {
            if (ImGui::GetFrameCount() == 1)
            {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputInt("Value", value);
            draft.AfterField("Value");
        }
        draft.End();
        ImGui::End();
        ImGui::Render();
    };
    drawForm(true, false);
    drawForm(true, false);
    drawForm(true, true);
    REQUIRE(storedTimes == 0);

    drawForm(false, false);
    CHECK(storedTimes == 1);
    CHECK(stored == 7);
    SUBCASE("the section stays closed")
    {
        for (int frame = 0; frame < 3; ++frame)
        {
            drawForm(false, false);
        }
        drawForm(true, false);
    }
    SUBCASE("the section opens while ImGui still reports the field as left")
    {
        drawForm(true, false);
    }
    // Stored once.
    CHECK(storedTimes == 1);
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

namespace
{
// The issues of the item at (group, number) with this severity and field.
bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, int group, int number,
              std::string_view field)
{
    return std::ranges::any_of(issues,
                               [&](const ItemDataIssue& issue)
                               {
                                   return issue.severity == severity && issue.group == group &&
                                          issue.number == number && issue.field == field;
                               });
}
} // namespace

// The checks the item tools show and the save runs before it writes.
TEST_CASE("The editor's checks report problems of the item models [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    g_ItemDataHandler.SetLookNames(Render::Items::Styles::Exists, Render::Items::ItemEffects::Exists);

    std::vector<ItemDataIssue> issues;
    g_ItemDataHandler.Validate(issues);
    REQUIRE_FALSE(HasFatal(issues));
    // The folder has the data files but not the .bmd files.
    CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, 0, 0, "file"));
    issues.clear();

    ItemModelDefinition kris = *g_ItemModelDatabase.Find(KrisType);
    SUBCASE("a shared model that does not exist")
    {
        kris.model = "noSuchModel";
        g_ItemModelDatabase.Set(kris, g_GlowColors);
        g_ItemDataHandler.Validate(issues);
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Fatal, 0, 0, "model"));
        std::vector<ItemDataIssue> saveIssues;
        CHECK(g_ItemDataHandler.Save(saveIssues) == ItemDataSaveResult::InvalidData);
    }
    SUBCASE("a look that does not exist")
    {
        kris.renderStyle = "noSuchStyle";
        kris.itemEffect = "noSuchEffect";
        g_ItemModelDatabase.Set(kris, g_GlowColors);
        g_ItemDataHandler.Validate(issues);
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, 0, 0, "renderStyle"));
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, 0, 0, "itemEffect"));
        CHECK_FALSE(HasFatal(issues));
    }
    SUBCASE("a value the model file reader refuses")
    {
        kris.inventory.scale = 0.0;
        g_ItemModelDatabase.Set(kris, g_GlowColors);
        g_ItemDataHandler.Validate(issues);
        CHECK(HasFatal(issues));
    }
    SUBCASE("a glow color that is not in the list")
    {
        kris.glow.color = "noSuchColor";
        g_ItemModelDatabase.Set(kris, g_GlowColors);
        g_ItemDataHandler.Validate(issues);
        CHECK(HasFatal(issues));
    }
    SUBCASE("a value of a shared model the reader refuses")
    {
        REQUIRE_FALSE(g_ItemModelDatabase.GetSharedModels().empty());
        SharedItemModel shared = g_ItemModelDatabase.GetSharedModels().front();
        SUBCASE("a file with backslashes")
        {
            shared.file = "Data\\Item\\Sword01.bmd";
        }
        SUBCASE("no file")
        {
            shared.file.clear();
        }
        SUBCASE("a texture folder with a slash at the end")
        {
            shared.textureFolders = {"Item/"};
        }
        g_ItemModelDatabase.SetSharedModel(shared);
        g_ItemDataHandler.Validate(issues);
        CHECK(std::ranges::any_of(
            issues, [](const ItemDataIssue& issue)
            { return issue.severity == ItemDataIssueSeverity::Fatal && issue.source == SharedItemModelsFileName; }));
    }
    SUBCASE("a file that two items name themselves")
    {
        ItemModelDefinition copy = kris;
        copy.number = 30;
        g_ItemModelDatabase.Set(copy, g_GlowColors);
        g_ItemDataHandler.Validate(issues);
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, 0, 0, "file"));
        CHECK(HasIssue(issues, ItemDataIssueSeverity::Warning, 0, 30, "file"));
    }
}

// A stat file that lacks a profile items name (removed outside the editor) stops the save of
// the items, together with the other errors of the item checks.
TEST_CASE("An item that names a profile the item stat file does not have blocks the save [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));

    ItemStatsLoadResult stats = LoadItemStatsFile(folder.Directory() / GetItemStatsFile());
    REQUIRE_FALSE(HasFatal(stats.issues));
    std::erase_if(stats.file.profiles, [](const WrittenStatProfile& profile) { return profile.name == "shield"; });
    std::erase_if(stats.file.profileRules, [](const StatProfileRule& rule) { return rule.profile == "shield"; });
    ItemStatProfilesScope restoreProfiles;
    g_ItemDatabase.SetStatProfiles(ItemStatProfiles(stats.file));

    // An item without an English name is an error of the item checks.
    ItemDefinition untranslated = *g_ItemDatabase.Find(KrisType);
    untranslated.names = Data::LocalizedString{};
    untranslated.names.Set("pt", "Cris");
    g_ItemDatabase.Set(untranslated);

    std::vector<ItemDataIssue> issues;
    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::InvalidData);
    CHECK(std::ranges::any_of(issues,
                              [](const ItemDataIssue& issue)
                              {
                                  return issue.severity == ItemDataIssueSeverity::Fatal &&
                                         issue.field == "statProfile" &&
                                         issue.message.find("\"shield\"") != std::string::npos;
                              }));
    CHECK(std::ranges::any_of(issues,
                              [](const ItemDataIssue& issue)
                              {
                                  return issue.severity == ItemDataIssueSeverity::Fatal && issue.group == 0 &&
                                         issue.number == 0 && issue.field != "statProfile";
                              }));
}

TEST_CASE("Save Items writes the item stat file when the editor changed it [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    ItemStatProfilesScope restoreProfiles;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const std::filesystem::path statsFile = folder.Directory() / GetItemStatsFile();
    const std::string shippedText = TestFiles::ReadWholeFile(statsFile);
    const ItemStatsLoadResult shipped = LoadItemStatsFile(statsFile);
    REQUIRE(shipped.issues.empty());
    CHECK(g_ItemDataHandler.GetStatFile() == shipped.file);

    SUBCASE("unchanged, it is left as it is")
    {
        std::vector<ItemDataIssue> issues;
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
        CHECK(TestFiles::ReadWholeFile(statsFile) == shippedText);
    }

    SUBCASE("changed, it is written and reads back the same")
    {
        ItemStatFile changed = shipped.file;
        changed.excellentDropLevelBonus = 26;
        // The handler keeps the writer's order.
        std::swap(changed.profiles.front(), changed.profiles.back());
        g_ItemDataHandler.SetStatFile(changed);
        CHECK(g_ItemDatabase.GetStatProfiles().GetExcellentDropLevelBonus() == 26);
        CHECK(g_ItemDataHandler.GetStatFile().profiles.front().name == shipped.file.profiles.front().name);

        std::vector<ItemDataIssue> issues;
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
        const ItemStatsLoadResult saved = LoadItemStatsFile(statsFile);
        CHECK(saved.issues.empty());
        CHECK(saved.file == g_ItemDataHandler.GetStatFile());
        CHECK(TestFiles::ReadWholeFile(statsFile) != shippedText);
    }

    SUBCASE("invalid, it stops the save")
    {
        ItemStatFile invalid = shipped.file;
        invalid.tables.front().name = "no name";
        g_ItemDataHandler.SetStatFile(invalid);

        std::vector<ItemDataIssue> issues;
        CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::InvalidData);
        CHECK(std::ranges::any_of(issues,
                                  [](const ItemDataIssue& issue)
                                  {
                                      return issue.severity == ItemDataIssueSeverity::Fatal &&
                                             issue.source == GetItemStatsFile().generic_string();
                                  }));
        CHECK(TestFiles::ReadWholeFile(statsFile) == shippedText);
    }
}

// A stat file written by hand need not have its profiles in name order; the
// editor keeps it in the writer's order, so the undo history matches it.
TEST_CASE("A change of a stat file whose profiles are not in name order is undone and redone [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    const std::filesystem::path statsFile = folder.Directory() / GetItemStatsFile();
    std::string text = TestFiles::ReadWholeFile(statsFile);
    // armor is the first profile and weapon the last, with the same terms:
    // swapping their names puts the profiles out of order without changing them.
    const size_t armor = text.find(R"("name": "armor")");
    const size_t weapon = text.rfind(R"("name": "weapon")");
    REQUIRE(armor != std::string::npos);
    REQUIRE(weapon != std::string::npos);
    text.replace(weapon, std::string(R"("name": "weapon")").size(), R"("name": "armor")");
    text.replace(armor, std::string(R"("name": "armor")").size(), R"("name": "weapon")");
    std::ofstream(statsFile, std::ios::binary) << text;

    ClientDataScope client(folder.Directory());
    ItemStatProfilesScope restoreProfiles;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    CHECK(g_ItemDataHandler.GetStatFile().profiles.front().name == "armor");
    MuEditor::Items::ItemEdits& edits = g_ItemEdits;
    edits.GetHistory().Clear();

    ItemStatFile changed = g_ItemDataHandler.GetStatFile();
    changed.excellentDropLevelBonus = 26;
    edits.SetStatFile("Change the item formulas", changed);
    REQUIRE(edits.Undo());
    CHECK(g_ItemDataHandler.GetStatFile().excellentDropLevelBonus == 25);
    CHECK(edits.Redo());
    CHECK(g_ItemDataHandler.GetStatFile().excellentDropLevelBonus == 26);
    edits.GetHistory().Clear();
}

TEST_CASE("A change of the profile rules gives the items that follow them the new profile, as one undo step "
          "[data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    ItemStatProfilesScope restoreProfiles;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    MuEditor::Items::ItemEdits& edits = g_ItemEdits;
    edits.GetHistory().Clear();
    const ItemStatFile shipped = g_ItemDataHandler.GetStatFile();

    // A boots item picked as a weapon keeps its pick.
    const int pickedBoots = MakeItemType(11, 0);
    EditItem(pickedBoots, [](ItemDefinition& boots) { boots.statProfile = "weapon"; });
    REQUIRE(g_ItemDatabase.Find(pickedBoots)->statProfile == "weapon");
    std::vector<int> boots;
    for (const ItemDefinition& item : g_ItemDatabase.GetAllSlots())
    {
        if (item.Exists() && item.slot == ItemSlot::Boots && item.statProfile == "armor")
        {
            boots.push_back(MakeItemType(item.group, item.number));
        }
    }
    REQUIRE(boots.size() > 10);

    SUBCASE("the boots leave the armour rule")
    {
        ItemStatFile changed = shipped;
        const auto armor = std::ranges::find_if(changed.profileRules,
                                                [](const StatProfileRule& rule) { return rule.profile == "armor"; });
        REQUIRE(armor != changed.profileRules.end());
        armor->slots.reset(static_cast<size_t>(ItemSlot::Boots));
        const std::vector<MuEditor::Items::StatProfileChange> changes = MuEditor::Items::FindStatProfileChanges(
            g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(), ItemStatProfiles(changed));
        REQUIRE(changes.size() == boots.size());
        edits.SetStatFile("Change the profile rules", changed,
                          MuEditor::Items::ApplyStatProfileChanges(g_ItemDatabase.GetAllSlots(), changes));
        for (const int itemType : boots)
        {
            CHECK(g_ItemDatabase.Find(itemType)->statProfile.empty());
        }
        CHECK(g_ItemDatabase.Find(pickedBoots)->statProfile == "weapon");
        CHECK(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->statProfile == "default");

        REQUIRE(edits.Undo());
        CHECK(g_ItemDataHandler.GetStatFile() == shipped);
        for (const int itemType : boots)
        {
            CHECK(g_ItemDatabase.Find(itemType)->statProfile == "armor");
        }
        REQUIRE(edits.Redo());
        CHECK(g_ItemDatabase.Find(boots.front())->statProfile.empty());

        // An item edited after the change refuses the undo.
        EditItem(boots.front(), [](ItemDefinition& item) { item.durability = 99; });
        CHECK_FALSE(edits.GetHistory().Undo());
    }

    SUBCASE("a renamed profile renames the rules and the items that name it")
    {
        const GameLogic::Items::ItemStatInput input{KrisType, 9, 1, 0, 4};
        const GameLogic::Items::ItemStatValues krisBefore =
            GameLogic::Items::ComputeItemStats(*g_ItemDataHandler.GetItemAttribute(KrisType), input);

        ItemStatFile renamed = shipped;
        MuEditor::Items::RenameStatProfile(renamed, "weapon", "blade");
        const std::vector<MuEditor::Items::StatProfileChange> changes =
            MuEditor::Items::FindStatProfileChanges(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(),
                                                    ItemStatProfiles(renamed), "weapon", "blade");
        // The 113 weapons and the boots picked as a weapon.
        CHECK(changes.size() == 114);
        edits.SetStatFile("Rename the stat profile", renamed,
                          MuEditor::Items::ApplyStatProfileChanges(g_ItemDatabase.GetAllSlots(), changes));
        CHECK(g_ItemDatabase.Find(KrisType)->statProfile == "blade");
        CHECK(g_ItemDatabase.Find(pickedBoots)->statProfile == "blade");
        const GameLogic::Items::ItemStatValues krisAfter =
            GameLogic::Items::ComputeItemStats(*g_ItemDataHandler.GetItemAttribute(KrisType), input);
        CHECK(krisAfter.damageMin == krisBefore.damageMin);
        CHECK(krisAfter.damageMax == krisBefore.damageMax);
        CHECK(krisAfter.requireStrength == krisBefore.requireStrength);
        CHECK(krisAfter.requireDexterity == krisBefore.requireDexterity);

        std::vector<ItemDataIssue> issues;
        g_ItemDataHandler.Validate(issues);
        CHECK_FALSE(HasFatal(issues));

        REQUIRE(edits.Undo());
        CHECK(g_ItemDatabase.Find(KrisType)->statProfile == "weapon");
        CHECK(g_ItemDataHandler.GetStatFile() == shipped);
    }
    edits.GetHistory().Clear();
}

TEST_CASE("The bmd exports refuse damage or magic power past 255 and name the items [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    ClientDataScope client(folder.Directory());
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    const WORD krisDamageMin = g_ItemDatabase.Find(KrisType)->damageMin;
    const WORD bladeMagicPower = g_ItemDatabase.Find(BladeType)->magicPower;
    EditItem(KrisType, [](ItemDefinition& item) { item.damageMin = 300; });
    EditItem(BladeType, [](ItemDefinition& item) { item.magicPower = 256; });
    const std::string named = "Item (0,0) DamageMin is 300; a .bmd item file holds up to 255\n"
                              "Item (0,5) MagicPower is 256; a .bmd item file holds up to 255\n";

    // Every such item is named once, and no file is written.
    std::string changeLog;
    CHECK_FALSE(g_ItemDataHandler.ExportAsBmd(changeLog));
    CHECK(changeLog == named);
    for (const ItemBmdLanguage& language : GetItemBmdLanguages())
    {
        CHECK_FALSE(std::filesystem::exists(folder.LegacyItemFilePath(language.folder)));
    }

    std::wstring legacyFile = (folder.Directory() / "Item_S6E3.bmd").wstring();
    std::string problems;
    std::string notes;
    CHECK_FALSE(g_ItemDataHandler.ExportAsS6E3(legacyFile.data(), problems, notes));
    CHECK(problems == named);
    CHECK_FALSE(std::filesystem::exists(legacyFile));

    // With the values back, both write.
    EditItem(KrisType, [&](ItemDefinition& item) { item.damageMin = krisDamageMin; });
    EditItem(BladeType, [&](ItemDefinition& item) { item.magicPower = bladeMagicPower; });
    std::string savedChangeLog;
    CHECK(g_ItemDataHandler.ExportAsBmd(savedChangeLog));
    problems.clear();
    CHECK(g_ItemDataHandler.ExportAsS6E3(legacyFile.data(), problems, notes));
    CHECK(problems.empty());
    CHECK(std::filesystem::exists(legacyFile));
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

namespace
{
using Data::Effects::GlowColor;

std::vector<GlowColor> CurrentGlowColors()
{
    const std::span<const GlowColor> colors = g_GlowColors.GetColors();
    return {colors.begin(), colors.end()};
}

// Whether one of the item's glow colors (glow, shine, ancient) is this one.
bool GlowsWith(int itemType, const std::array<float, 3>& color)
{
    const ItemGlowColors* colors = g_ItemModelDatabase.FindGlowColors(itemType);
    return colors != nullptr &&
           (colors->color == color || colors->shineColor == color || colors->ancientColor == color);
}
} // namespace

// D26 for the glow colors: a new color an item picks, a changed value that
// reaches every item that uses it, and only unused colors removed.
TEST_CASE("The item tools add glow colors, change them and remove unused ones [data][items][editor]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope restoreModels;
    I18N::SetLocale("en");
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
    const std::vector<GlowColor> shipped = CurrentGlowColors();
    const std::filesystem::path colorsFile = folder.Directory() / GetGlowColorsFile();
    const std::string shippedText = TestFiles::ReadWholeFile(colorsFile);
    MuEditor::Items::ItemEdits& edits = g_ItemEdits;
    edits.GetHistory().Clear();
    constexpr std::array<float, 3> Teal{0.0f, 0.5f, 0.5f};

    SUBCASE("a new color an item picks is saved, and undone with the pick")
    {
        const MuEditor::Items::ItemSlotState kris = MuEditor::Items::ItemEdits::ReadSlot(KrisType);
        // As the model tool adds it from a color field.
        edits.BeginGroup("New glow color");
        edits.SetGlowColor("New glow color", {"teal", {0, 0.5, 0.5}});
        ItemModelDefinition model = *g_ItemModelDatabase.Find(KrisType);
        model.glow.color = "teal";
        edits.SetItemModel("Glow color", model);
        edits.EndGroup();
        CHECK(g_GlowColors.GetColors().back() == GlowColor{"teal", {0, 0.5, 0.5}});
        CHECK(g_ItemModelDatabase.FindGlowColors(KrisType)->color == Teal);
        CHECK(MuEditor::Items::FindGlowColorUsers("teal") == std::vector<int>{KrisType});

        std::vector<ItemDataIssue> issues;
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
        const GlowColorsLoadResult saved = LoadGlowColorsFile(colorsFile);
        CHECK(saved.issues.empty());
        CHECK(saved.colors == CurrentGlowColors());

        REQUIRE(edits.Undo());
        CHECK(MuEditor::Items::ItemEdits::ReadSlot(KrisType) == kris);
        CHECK(CurrentGlowColors() == shipped);
        REQUIRE(edits.Redo());
        CHECK(g_ItemModelDatabase.FindGlowColors(KrisType)->color == Teal);

        // The next start reads it back.
        REQUIRE(g_ItemDataHandler.LoadModels(errorMessage));
        CHECK(g_ItemModelDatabase.Find(KrisType)->glow.color == "teal");
        CHECK(g_ItemModelDatabase.FindGlowColors(KrisType)->color == Teal);
    }

    SUBCASE("a changed value reaches every item that uses the color")
    {
        // Nothing changed: the save leaves the file as it is.
        std::vector<ItemDataIssue> issues;
        REQUIRE(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::Saved);
        CHECK(TestFiles::ReadWholeFile(colorsFile) == shippedText);

        const std::vector<int> users = MuEditor::Items::FindGlowColorUsers("gold");
        REQUIRE(users.size() > 1);
        const std::array<float, 3> gold = MuEditor::Items::ToColorField(*g_GlowColors.Find("gold"));
        constexpr std::array<float, 3> Yellow{1.0f, 1.0f, 0.0f};
        edits.SetGlowColor("Change a glow color", {"gold", {1, 1, 0}});
        for (const int user : users)
        {
            INFO("item type " << user);
            CHECK(GlowsWith(user, Yellow));
        }
        // The color keeps its place in the file.
        const std::vector<GlowColor> changed = CurrentGlowColors();
        REQUIRE(changed.size() == shipped.size());
        for (size_t index = 0; index < shipped.size(); ++index)
        {
            CHECK(changed[index].name == shipped[index].name);
        }

        REQUIRE(edits.Undo());
        CHECK(CurrentGlowColors() == shipped);
        CHECK(GlowsWith(users.front(), gold));
    }

    SUBCASE("only colors no item uses are removed, never the default colors")
    {
        edits.SetGlowColor("New glow color", {"unused", {0.2, 0.2, 0.2}});
        const std::string names[] = {"unused", Data::Items::ItemGlow::DefaultColor, "gold"};
        REQUIRE(edits.RemoveGlowColors("Remove unused glow colors", names));
        CHECK(g_GlowColors.Find("unused") == nullptr);
        CHECK(g_GlowColors.Find(Data::Items::ItemGlow::DefaultColor) != nullptr);
        CHECK(g_GlowColors.Find("gold") != nullptr);
        const std::string used[] = {"gold"};
        CHECK_FALSE(edits.RemoveGlowColors("Remove a glow color", used));

        REQUIRE(edits.Undo());
        CHECK(g_GlowColors.Find("unused") != nullptr);

        // With no model left to use them, only the default rule keeps them.
        g_ItemModelDatabase.Build({}, g_GlowColors);
        for (const char* name : {Data::Items::ItemGlow::DefaultColor, Data::Items::ItemGlow::DefaultShineColor,
                                 Data::Items::ItemGlow::DefaultAncientColor})
        {
            INFO(name);
            CHECK_FALSE(g_ItemDataHandler.RemoveGlowColor(name));
            CHECK(g_GlowColors.Find(name) != nullptr);
        }
    }

    SUBCASE("a name the file would refuse stops the save")
    {
        g_ItemDataHandler.SetGlowColor({"light blue", {0, 0, 1}});
        std::vector<ItemDataIssue> issues;
        CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::InvalidData);
        CHECK(
            std::ranges::any_of(issues, [](const ItemDataIssue& issue) { return issue.field == "colors.light blue"; }));
        CHECK(TestFiles::ReadWholeFile(colorsFile) == shippedText);
    }

    edits.GetHistory().Clear();
    g_GlowColors.Build(shipped);
}

TEST_CASE("A description placeholder that names no table stops the save [editor][items]")
{
    TemporaryClientFolder folder;
    folder.CopyShippedItems();
    folder.CopyShippedGlowColors();
    ClientDataScope client(folder.Directory());
    ItemModelDatabaseScope models;
    std::string errorMessage;
    REQUIRE(g_ItemDataHandler.Load(errorMessage));
    REQUIRE_MESSAGE(g_ItemDataHandler.LoadModels(errorMessage), errorMessage);
    const std::filesystem::path potions = folder.Directory() / GetItemDataDirectory() / "Group14_Potion.json";
    const std::string shippedText = TestFiles::ReadWholeFile(potions);

    const int blessType = MakeItemType(14, 13);
    EditItem(blessType,
             [](ItemDefinition& bless)
             {
                 REQUIRE_FALSE(bless.description.empty());
                 bless.description[0].text = {};
                 bless.description[0].text.Set("en", "Up to +{table:noSuchTable}");
             });
    std::vector<ItemDataIssue> issues;
    CHECK(g_ItemDataHandler.Save(issues) == ItemDataSaveResult::InvalidData);
    CHECK(
        std::ranges::any_of(issues, [](const ItemDataIssue& issue)
                            { return issue.message.starts_with("has the unknown placeholder {table:noSuchTable}"); }));
    CHECK(TestFiles::ReadWholeFile(potions) == shippedText);
}
#endif
