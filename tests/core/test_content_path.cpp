#include "doctest.h"

#include "Core/Content/ContentPath.h"

using Content::Path;

TEST_CASE("a content path is written with '/' and keeps its case [content]")
{
    CHECK(Path::From("Data\\Sound\\x.wav")->str() == "Data/Sound/x.wav");
    CHECK(Path::From("Data/sound/X.wav")->str() == "Data/sound/X.wav");
    CHECK(Path::From("./Data//Items/./Items.json")->str() == "Data/Items/Items.json");
    CHECK(Path::From("Data/Items/")->str() == "Data/Items");
}

TEST_CASE("a path that leaves the content or names nothing is not a content path [content]")
{
    CHECK_FALSE(Path::From(""));
    CHECK_FALSE(Path::From("."));
    CHECK_FALSE(Path::From("./"));
    CHECK_FALSE(Path::From("../Data/x.json"));
    CHECK_FALSE(Path::From("Data/../../x.json"));
    CHECK_FALSE(Path::From("/Data/x.json"));
    CHECK_FALSE(Path::From("\\Data\\x.json"));
    CHECK_FALSE(Path::From("C:\\MU\\Data\\x.json"));
    CHECK_FALSE(Path::From("C:"));
}

TEST_CASE("a content path splits into its folder and name [content]")
{
    const Path path = *Path::From("Data/Items/Items.json");
    CHECK(path.Folder() == "Data/Items");
    CHECK(path.Name() == "Items.json");

    const Path root = *Path::From("config.ini");
    CHECK(root.Folder().empty());
    CHECK(root.Name() == "config.ini");
}
