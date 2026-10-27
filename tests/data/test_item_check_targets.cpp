#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ItemTools/ItemCheckTargets.h"
#include "UI/ItemTools/ItemFormulaEntry.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace Data::Items;
using namespace MuEditor::Items;

namespace
{
// Longer than any item name the game shows.
constexpr std::size_t LongNameLength = 100;
constexpr const char* SwordFile = "Data/Item/Sword01.bmd";

ItemDefinition MakeItem(int number, std::string_view locale, std::string name)
{
    ItemDefinition item;
    item.number = number;
    item.names.Set(locale, std::move(name));
    return item;
}

ItemModelDefinition MakeModel(int number, const char* file)
{
    ItemModelDefinition model;
    model.number = number;
    model.file = file;
    return model;
}

// The tool of the problem of `field` whose message has `messagePart`, or
// nothing when the checks did not find that problem.
std::optional<ItemTool> FindTool(const std::vector<ItemDataIssue>& issues, std::string_view field,
                                 std::string_view messagePart)
{
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.field == field && issue.message.find(messagePart) != std::string::npos)
        {
            return GetToolForIssue(issue);
        }
    }
    return std::nullopt;
}
} // namespace

TEST_CASE("The item checks send each field of an item to the tool that edits it [editor][items]")
{
    const std::pair<std::string_view, ItemTool> fields[] = {
        {"width", ItemTool::Basics},
        {"level", ItemTool::Basics},
        {"statProfile", ItemTool::Basics},
        {"resistances.ice", ItemTool::Basics},
        {"unknownField", ItemTool::Basics},
        {"requirements.level", ItemTool::Requirements},
        {"requirementsByLevel.energy[2]", ItemTool::Requirements},
        {"classRequirements.darkWizard", ItemTool::Requirements},
        {"excellentRaisesLevel", ItemTool::Requirements},
        {"tradable", ItemTool::Rules},
        {"sellableWhenRentalExpired", ItemTool::Rules},
        {"tags", ItemTool::Categories},
        {"name", ItemTool::Basics},
        {"name.en", ItemTool::Basics},
        {"name.de", ItemTool::Translations},
        {"number", ItemTool::Browser},
        {"", ItemTool::Browser},
    };
    for (const auto& field : fields)
    {
        INFO(std::string(field.first));
        CHECK(GetToolForField(field.first) == field.second);
    }
}

TEST_CASE("The item checks send each field of a model entry to the model tool [editor][items]")
{
    const std::string_view fields[] = {
        "file",  "model",         "textureFolders",    "noneBlendMeshes", "inventory.scale", "ground.rotation",
        "cloth", "glow.level[3]", "material.specular", "renderStyle",     "itemEffect"};
    for (const std::string_view field : fields)
    {
        INFO(std::string(field));
        CHECK(GetToolForField(field) == ItemTool::Model);
    }
}

TEST_CASE("The item checks send a name problem in English to the basics and in another language to the "
          "translations [editor][items]")
{
    const ItemDefinition withoutEnglish = MakeItem(1, "de", std::string(LongNameLength, 'a'));
    const ItemDefinition withSeparator = MakeItem(2, "en", "Sword||Axe");
    ItemDefinition withBadLanguage = MakeItem(3, "en", "Item");
    withBadLanguage.names.Set("d e", "Text");
    ItemDefinition withUnshownLanguage = MakeItem(4, "en", "Item");
    withUnshownLanguage.names.Set("xx", "Text");
    const std::vector<ItemDefinition> items = {withoutEnglish, withSeparator, withBadLanguage, withUnshownLanguage};
    const char* const uiLocales[] = {"en", "de"};

    std::vector<ItemDataIssue> issues;
    ValidateItems(items, issues);
    ValidateNameLocales(items, uiLocales, issues);

    CHECK(FindTool(issues, "name", "English name is missing") == ItemTool::Basics);
    CHECK(FindTool(issues, "name", "the en name must not contain") == ItemTool::Basics);
    CHECK(FindTool(issues, "name", "the de name is longer") == ItemTool::Translations);
    CHECK(FindTool(issues, "name", "is not a language code") == ItemTool::Translations);
    CHECK(FindTool(issues, "name", "the xx name is never shown") == ItemTool::Translations);
}

TEST_CASE("The item checks send a doubled item to the browser and a requirement problem to the "
          "requirements [editor][items]")
{
    ItemDefinition item = MakeItem(1, "en", "Item");
    item.excellentRaisesLevel = false;
    const std::vector<ItemDefinition> items = {item, item};

    std::vector<ItemDataIssue> issues;
    ValidateItems(items, issues);

    CHECK(FindTool(issues, "number", "more than once") == ItemTool::Browser);
    CHECK(FindTool(issues, "excellentRaisesLevel", "level requirement") == ItemTool::Requirements);
}

TEST_CASE("The item checks send the problems the model checks find to the model tool [editor][items]")
{
    std::vector<ItemModelDefinition> models = {MakeModel(1, SwordFile), MakeModel(2, SwordFile), MakeModel(3, "")};
    models[0].renderStyle = "noSuchStyle";
    models[0].itemEffect = "noSuchEffect";
    models[1].glow.color = "noSuchColor";
    models[2].model = "noSuchSharedModel";
    const auto noLook = [](std::string_view) { return false; };
    const std::vector<Data::Effects::GlowColor> colors = {
        {ItemGlow::DefaultColor}, {ItemGlow::DefaultShineColor}, {ItemGlow::DefaultAncientColor}};

    std::vector<ItemDataIssue> issues;
    ValidateItemModelLooks(models, noLook, noLook, issues);
    ValidateItemModelGlowColors(models, colors, issues);
    ValidateDirectlySharedModelFiles(models, issues);
    ApplySharedItemModels(models, {}, "SharedModels.json", issues);

    CHECK(FindTool(issues, "renderStyle", "noSuchStyle") == ItemTool::Model);
    CHECK(FindTool(issues, "itemEffect", "noSuchEffect") == ItemTool::Model);
    CHECK(FindTool(issues, "glow.color", "noSuchColor") == ItemTool::Model);
    CHECK(FindTool(issues, "file", SwordFile) == ItemTool::Model);
    CHECK(FindTool(issues, "model", "noSuchSharedModel") == ItemTool::Model);
}

TEST_CASE("The item checks open the window of each item tool [editor][items]")
{
    struct ExpectedWindow
    {
        ItemTool tool;
        const char* windowId;
        bool MuEditor::ToolWindows::* open;
    };
    const ExpectedWindow expected[] = {
        {ItemTool::Basics, MuEditor::WindowIds::ItemBasics, &MuEditor::ToolWindows::itemBasics},
        {ItemTool::Requirements, MuEditor::WindowIds::ItemRequirements, &MuEditor::ToolWindows::itemRequirements},
        {ItemTool::Rules, MuEditor::WindowIds::ItemRules, &MuEditor::ToolWindows::itemRules},
        {ItemTool::Categories, MuEditor::WindowIds::ItemCategories, &MuEditor::ToolWindows::itemCategories},
        {ItemTool::Model, MuEditor::WindowIds::ItemModel, &MuEditor::ToolWindows::itemModel},
        {ItemTool::Translations, MuEditor::WindowIds::ItemTranslations, &MuEditor::ToolWindows::itemTranslations},
        {ItemTool::Browser, MuEditor::WindowIds::ItemBrowser, &MuEditor::ToolWindows::itemBrowser},
    };
    for (const ExpectedWindow& entry : expected)
    {
        INFO(std::string(entry.windowId));
        const ItemToolWindowRef window = GetToolWindow(entry.tool);
        CHECK(std::string(window.windowId) == entry.windowId);

        MuEditor::ToolWindows windows;
        windows.*window.open = true;
        CHECK(windows.*entry.open);
    }
}

TEST_CASE("The item checks send a problem of the item stat file to its entry in Item Formulas [editor][items]")
{
    using Kind = FormulaEntry::Kind;
    CHECK(FindFormulaEntryForField("tables.steps[3]") == FormulaEntry{Kind::Table, "steps"});
    CHECK(FindFormulaEntryForField("tables.linear") == FormulaEntry{Kind::Table, "linear"});
    CHECK(FindFormulaEntryForField("profiles.shield.defense.ancient") == FormulaEntry{Kind::Profile, "shield"});
    CHECK(FindFormulaEntryForField("default.damage.table") == FormulaEntry{Kind::Default, ""});
    CHECK(FindFormulaEntryForField("dropLevelBonus.excellent") == FormulaEntry{Kind::DropLevelBonus, ""});
    CHECK(FindFormulaEntryForField("profileRules[2].slot") == FormulaEntry{Kind::Rules, ""});
    CHECK(FindFormulaEntryForField("formatVersion").kind == Kind::None);
}

#endif // _EDITOR
