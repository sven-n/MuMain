#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemModelProblem.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"

#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
const std::string StormHardGloveEntry = "Storm Hard Glove (0,33)";
const std::string SwordModels = "(Data/Items/Models/Group00_Sword.json)";

ItemDatabase MakeItems()
{
    ItemDefinition glove;
    glove.group = 0;
    glove.number = 33;
    glove.names = Data::LocalizedString::Parse("Storm Hard Glove");
    const std::vector<ItemDefinition> definitions{glove};

    ItemDatabase items;
    items.Build(definitions);
    return items;
}

ItemModelProblem MakeMissingTexture(int mesh, const std::string& texture)
{
    ItemModelProblem problem;
    problem.type = ItemModelProblemType::TextureMissing;
    problem.group = 0;
    problem.number = 33;
    problem.modelFile = "Data/Item/Sword34.bmd";
    problem.mesh = mesh;
    problem.texture = texture;
    problem.searchedFolders = {"Item"};
    return problem;
}

bool Contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}
} // namespace

TEST_CASE("A missing item model texture names the item, the model, the file and the folders [data][items]")
{
    const ItemDatabase items = MakeItems();

    CHECK(MakeMissingTexture(1, "Item762_Armor.jpg").ToString(items) ==
          StormHardGloveEntry +
              ": texture Item762_Armor.jpg (Item762_Armor.OZJ) of mesh 1 of Data/Item/Sword34.bmd "
              "not found or not readable in Data/Item/ " +
              SwordModels);

    ItemModelProblem tga = MakeMissingTexture(0, "hair.tga");
    tga.searchedFolders = {"Item", "Player", "Effect"};
    CHECK(Contains(tga.ToString(items), "hair.tga (hair.OZT)"));
    CHECK(Contains(tga.ToString(items), "not found or not readable in Data/Item/, Data/Player/ or Data/Effect/"));
}

TEST_CASE("The log entry of an item model problem names the item type instead of the item [data][items]")
{
    CHECK(MakeMissingTexture(1, "Item762_Armor.jpg").ToLogString() ==
          "(0,33): texture Item762_Armor.jpg (Item762_Armor.OZJ) of mesh 1 of Data/Item/Sword34.bmd "
          "not found or not readable in Data/Item/ " +
              SwordModels);
}

TEST_CASE("A texture of a type the game cannot load is an error [data][items]")
{
    ItemModelProblem problem = MakeMissingTexture(2, "Sword34.bmp");
    problem.type = ItemModelProblemType::TextureTypeUnsupported;

    CHECK(problem.IsError());
    CHECK(problem.ToString(MakeItems()) == StormHardGloveEntry +
                                               ": texture Sword34.bmp of mesh 2 of Data/Item/Sword34.bmd is not a "
                                               ".jpg or .tga texture, which the game cannot load " +
                                               SwordModels);
}

TEST_CASE("The game reads .jpg and .tga textures from their encrypted copies [data][items]")
{
    CHECK(GetStoredTextureFileName("Sword01.jpg") == "Sword01.OZJ");
    CHECK(GetStoredTextureFileName("hair.TGA") == "hair.OZT");
    CHECK(GetStoredTextureFileName("level.2.jpg") == "level.2.OZJ");
    CHECK_FALSE(GetStoredTextureFileName("Sword01.bmp").has_value());
    CHECK_FALSE(GetStoredTextureFileName("Sword01.jpeg").has_value());
    CHECK_FALSE(GetStoredTextureFileName("Sword01").has_value());

    CHECK(IsHiddenTexture("hide.jpg"));
    CHECK_FALSE(IsHiddenTexture("Hide.jpg"));
}

TEST_CASE("A missing item model file names the item and the model entry [data][items]")
{
    ItemModelProblem problem;
    problem.type = ItemModelProblemType::ModelFileMissing;
    problem.group = 13;
    problem.number = 116;
    problem.modelFile = "Data/Item/monmark02.bmd";

    CHECK(problem.IsError());
    CHECK(problem.ToString(MakeItems()) ==
          "<unknown item> (13,116): model file Data/Item/monmark02.bmd could not be opened (missing or not a valid "
          ".bmd file) (Data/Items/Models/Group13_Helper.json)");
}

TEST_CASE("A texture found only outside the texture folders is a warning that names the folder to add [data][items]")
{
    ItemModelProblem problem = MakeMissingTexture(1, "Item762_Armor.jpg");
    problem.type = ItemModelProblemType::TextureOutsideFolders;
    problem.usedInstead = "Data/Player/Item762_Armor.jpg";

    CHECK_FALSE(problem.IsError());
    CHECK(Contains(problem.ToString(MakeItems()),
                   "the one another model loaded (Data/Player/Item762_Armor.jpg) is used, add its folder to "
                   "textureFolders"));
}

TEST_CASE("A glow on a mesh the model does not have is a warning [data][items]")
{
    ItemModelProblem problem = MakeMissingTexture(0, "");
    problem.type = ItemModelProblemType::GlowMeshMissing;
    problem.field = "glow.meshes";
    problem.mesh = 3;
    problem.meshCount = 3;

    CHECK_FALSE(problem.IsError());
    CHECK(problem.ToLogString() ==
          "(0,33): glow.meshes has mesh 3, but Data/Item/Sword34.bmd has 3 meshes " + SwordModels);
}

TEST_CASE("The item model error message lists the errors only, up to a limit [data][items]")
{
    const ItemDatabase items = MakeItems();
    ItemModelProblem warning = MakeMissingTexture(0, "Warning.jpg");
    warning.type = ItemModelProblemType::TextureOutsideFolders;
    const std::vector<ItemModelProblem> problems{MakeMissingTexture(0, "First.jpg"), warning,
                                                 MakeMissingTexture(1, "Second.jpg"),
                                                 MakeMissingTexture(2, "Third.jpg")};

    const std::string message = DescribeItemModelErrors(problems, items, 2);

    CHECK(Contains(message, "First.jpg"));
    CHECK(Contains(message, "Second.jpg"));
    CHECK_FALSE(Contains(message, "Third.jpg"));
    CHECK_FALSE(Contains(message, "Warning.jpg"));
    CHECK(Contains(message, "... and 1 more"));
    CHECK(Contains(message, "MuError.log"));

    const std::vector<ItemModelProblem> warnings{warning};
    CHECK(DescribeItemModelErrors(warnings, items, 2).empty());
}
