#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemModelProblem.h"
#include "Data/GameData/ItemData/ItemDatabase.h"

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
              "not found in Data/Item/ " +
              SwordModels);

    ItemModelProblem tga = MakeMissingTexture(0, "hair.tga");
    tga.searchedFolders = {"Item", "Player", "Effect"};
    CHECK(Contains(tga.ToString(items), "hair.tga (hair.OZT)"));
    CHECK(Contains(tga.ToString(items), "not found in Data/Item/, Data/Player/ or Data/Effect/"));
}

TEST_CASE("A missing item model file names the item and the model entry [data][items]")
{
    ItemModelProblem problem;
    problem.type = ItemModelProblemType::ModelFileMissing;
    problem.group = 13;
    problem.number = 116;
    problem.modelFile = "Data/Item/monmark02.bmd";

    CHECK(problem.IsError());
    CHECK(problem.ToString(MakeItems()) == "<unknown item> (13,116): model file Data/Item/monmark02.bmd not found "
                                           "(Data/Items/Models/Group13_Helper.json)");
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
