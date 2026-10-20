#include "stdafx.h"

#include "doctest.h"

// The source lines that name an item, which the item tools show before an
// item is removed (phase 6).
#ifdef _EDITOR
#include "Data/GameData/ItemData/ItemType.h"
#include "UI/ItemTools/ItemCodeReferences.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

using Data::Items::MakeItemType;
using MuEditor::Items::ItemCodeReference;
using MuEditor::Items::ItemCodeReferences;

namespace
{
using Kind = ItemCodeReference::Kind;

// A source folder whose files are written by the test.
class TemporarySourceFolder
{
public:
    TemporarySourceFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_item_code_references_" + std::to_string(std::random_device{}())))
    {
        std::filesystem::remove_all(m_directory);
        std::filesystem::create_directories(m_directory);
    }

    ~TemporarySourceFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_directory, ignored);
    }

    TemporarySourceFolder(const TemporarySourceFolder&) = delete;
    TemporarySourceFolder& operator=(const TemporarySourceFolder&) = delete;

    void Write(std::string_view relativePath, std::string_view text) const
    {
        const std::filesystem::path path = m_directory / std::filesystem::path(relativePath);
        std::filesystem::create_directories(path.parent_path());
        std::ofstream(path, std::ios::binary) << text;
    }

    const std::filesystem::path& Directory() const
    {
        return m_directory;
    }

private:
    std::filesystem::path m_directory;
};

constexpr std::string_view EnumFile = "Core/Globals/_enum.h";
constexpr std::string_view EnumSource = R"source(enum
{
    MODEL_KRIS = MODEL_SWORD + 0,
    MODEL_BLADE = MODEL_SWORD + 5,
    MODEL_BOLT = MODEL_BOW + 7,
    MODEL_FACE = MODEL_CARD + 4,
};

enum
{
    ITEM_KRIS = ITEM_SWORD + 0,
    ITEM_SHORT_SWORD = ITEM_SWORD + 1,
    ITEM_BLADE = ITEM_SWORD + 5,
    ITEM_APPLE = ITEM_POTION + 0,
    ITEM_ANTIDOTE = ITEM_POTION + 8,
    // ITEM_OLD_ANTIDOTE = ITEM_POTION + 9,
    ITEM_JEWEL_OF_BLESS = ITEM_POTION + 13,
    ITEM_NUMBER_DARK_SPIRIT = 5,
};
)source";

constexpr int KrisType = MakeItemType(0, 0);
constexpr int ShortSwordType = MakeItemType(0, 1);
constexpr int BladeType = MakeItemType(0, 5);
constexpr int BoltType = MakeItemType(4, 7);
constexpr int AppleType = MakeItemType(14, 0);
constexpr int AntidoteType = MakeItemType(14, 8);
constexpr int OldAntidoteType = MakeItemType(14, 9);
constexpr int JewelOfBlessType = MakeItemType(14, 13);

constexpr std::string_view UseFile = "GameLogic/ItemUse.cpp";
constexpr std::string_view UseSource = R"source(bool IsKris(const ITEM* ip)
{
    return ip->Type == ITEM_KRIS || ip->Type == ITEM_KRIS; // Kris
}
bool IsKrisModel(int type)
{
    return type == MODEL_KRIS;
}
)source";
constexpr int KrisTwiceLine = 3;
constexpr int KrisModelLine = 7;

constexpr std::string_view RawIdFile = "GameLogic/ItemRawIds.cpp";
constexpr std::string_view RawIdSource = R"source(void Use(const ITEM* ip)
{
    if (ip->Type == ITEM_POTION + 100 || ip->Type == MODEL_ARMOR + 53)
        return;
    if (ip->Type == ITEM_HELPER+3 || ip->Type == ITEM_POTION + 2 * 3)
        return;
    if (ip->Type == ITEM_SWORD || ip->Type == ITEM_WING + 512)
        return;
    if (ip->Type == ITEM_SWORD + 0 || ip->Type == ITEM_KRIS)
        return;
}
)source";
constexpr int RawIdLine = 3;
constexpr int RawIdWithoutSpacesLine = 5;
constexpr int GroupBaseLine = 7;
constexpr int RawIdAndNameLine = 9;

constexpr std::string_view RangeFile = "UI/ItemRanges.h";
constexpr std::string_view RangeSource = R"source(inline bool InRanges(int type, const ITEM* ip)
{
    if (ip->Type >= ITEM_SWORD + 2 && ip->Type <= ITEM_SWORD + 4)
        return true;
    if (ITEM_ANTIDOTE >= type && ITEM_APPLE < type)
        return true;
    if (type >= MODEL_BOW && type < MODEL_BOLT)
        return true;
    if (type >= ITEM_SWORD && type < ITEM_BOW)
        return true;
    if (ip->Type >= ITEM_SHORT_SWORD && type <= ITEM_BLADE)
        return true;
    if (type >= ITEM_SWORD && type < ITEM_SWORD + MAX_ITEM_INDEX)
        return true;
    return (type > ITEM_POTION + 20) && (type < ITEM_POTION + 23);
}
)source";
constexpr int RawIdRangeLine = 3;
constexpr int SwappedRangeLine = 5;
constexpr int GroupBaseRangeLine = 7;
constexpr int WholeGroupsLine = 9;
constexpr int TwoVariablesLine = 11;
constexpr int OpenRangeLine = 13;
constexpr int ParenthesesRangeLine = 15;

constexpr std::string_view CommentFile = "Engine/Comments.cpp";
constexpr std::string_view CommentSource = R"source(// ITEM_BLADE in a comment
/* MODEL_BLADE
   ITEM_POTION + 9 */
void Log()
{
    Print("ITEM_BLADE and ITEM_POTION + 9");
    char letter = 'I'; // ITEM_JEWEL_OF_BLESS
    int jewel = ITEM_JEWEL_OF_BLESS; /* ITEM_BLADE */
}
)source";
constexpr int JewelLine = 8;

void WriteSources(const TemporarySourceFolder& folder)
{
    folder.Write(EnumFile, EnumSource);
    folder.Write(UseFile, UseSource);
    folder.Write(RawIdFile, RawIdSource);
    folder.Write(RangeFile, RangeSource);
    folder.Write(CommentFile, CommentSource);
    // Only .cpp and .h files are read.
    folder.Write("Notes.txt", "ITEM_BLADE\n");
}

// How the line names the item; nothing when it does not.
std::optional<Kind> KindOnLine(const ItemCodeReferences& references, int itemType, std::string_view file, int line)
{
    const std::span<const ItemCodeReference> found = references.Find(itemType);
    const auto reference = std::find_if(found.begin(), found.end(), [&](const ItemCodeReference& candidate)
                                        { return candidate.file == file && candidate.line == line; });
    if (reference == found.end())
        return std::nullopt;
    return reference->kind;
}
} // namespace

TEST_CASE("The item code references find the lines that name an item by its constant [editor][items]")
{
    TemporarySourceFolder folder;
    WriteSources(folder);
    ItemCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));
    CHECK(references.IsLoaded());

    // Sorted by file and line; a line that names the item twice, or by its
    // constant and its raw id, is one reference. _enum.h defines the names and
    // is not read for them.
    const std::span<const ItemCodeReference> kris = references.Find(KrisType);
    REQUIRE(kris.size() == 3);
    CHECK(kris[0].file == RawIdFile);
    CHECK(kris[0].line == RawIdAndNameLine);
    CHECK(kris[0].kind == Kind::Name);
    CHECK(kris[1].file == UseFile);
    CHECK(kris[1].line == KrisTwiceLine);
    CHECK(kris[1].text == "return ip->Type == ITEM_KRIS || ip->Type == ITEM_KRIS; // Kris");
    CHECK(kris[1].kind == Kind::Name);
    // The MODEL_ constant names the item too.
    CHECK(kris[2].file == UseFile);
    CHECK(kris[2].line == KrisModelLine);
    CHECK(kris[2].text == "return type == MODEL_KRIS;");

    // In a header, and as a range bound that does not enclose the other item.
    const std::span<const ItemCodeReference> shortSword = references.Find(ShortSwordType);
    REQUIRE(shortSword.size() == 1);
    CHECK(shortSword[0].file == RangeFile);
    CHECK(shortSword[0].line == TwoVariablesLine);
}

TEST_CASE("The item code references find raw ids written as a group base plus a number [editor][items]")
{
    TemporarySourceFolder folder;
    WriteSources(folder);
    ItemCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));

    CHECK(KindOnLine(references, MakeItemType(14, 100), RawIdFile, RawIdLine) == Kind::RawId);
    CHECK(KindOnLine(references, MakeItemType(8, 53), RawIdFile, RawIdLine) == Kind::RawId);
    CHECK(KindOnLine(references, MakeItemType(13, 3), RawIdFile, RawIdWithoutSpacesLine) == Kind::RawId);
    // ITEM_POTION + 2 * 3 is not item 2 of the group, nor item 6.
    CHECK_FALSE(KindOnLine(references, MakeItemType(14, 2), RawIdFile, RawIdWithoutSpacesLine).has_value());
    CHECK_FALSE(KindOnLine(references, MakeItemType(14, 6), RawIdFile, RawIdWithoutSpacesLine).has_value());
    // A group base alone names no item, and a number past the group is no item of it.
    CHECK_FALSE(KindOnLine(references, KrisType, RawIdFile, GroupBaseLine).has_value());
    CHECK(references.Find(MakeItemType(13, 0)).empty());
    // The raw id and the constant on one line: one reference, by the constant.
    const std::span<const ItemCodeReference> kris = references.Find(KrisType);
    CHECK(std::count_if(kris.begin(), kris.end(), [](const ItemCodeReference& reference)
                        { return reference.file == RawIdFile && reference.line == RawIdAndNameLine; }) == 1);
    CHECK(KindOnLine(references, KrisType, RawIdFile, RawIdAndNameLine) == Kind::Name);
}

TEST_CASE("The item code references find the items that a range check on one line encloses [editor][items]")
{
    TemporarySourceFolder folder;
    WriteSources(folder);
    ItemCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));

    // Raw ids as bounds.
    CHECK(KindOnLine(references, MakeItemType(0, 2), RangeFile, RawIdRangeLine) == Kind::RawId);
    CHECK(KindOnLine(references, MakeItemType(0, 3), RangeFile, RawIdRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, MakeItemType(0, 4), RangeFile, RawIdRangeLine) == Kind::RawId);
    CHECK_FALSE(KindOnLine(references, BladeType, RangeFile, RawIdRangeLine).has_value());

    // The item on the left of the comparison, and a bound that leaves its item out.
    CHECK(KindOnLine(references, AppleType, RangeFile, SwappedRangeLine) == Kind::Name);
    CHECK(KindOnLine(references, MakeItemType(14, 1), RangeFile, SwappedRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, MakeItemType(14, 7), RangeFile, SwappedRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, AntidoteType, RangeFile, SwappedRangeLine) == Kind::Name);
    CHECK(references.Find(OldAntidoteType).empty());

    // A group base bounds a range, from the group's first item.
    CHECK(KindOnLine(references, MakeItemType(4, 0), RangeFile, GroupBaseRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, MakeItemType(4, 6), RangeFile, GroupBaseRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, BoltType, RangeFile, GroupBaseRangeLine) == Kind::Name);
    CHECK(references.Find(MakeItemType(4, 8)).empty());

    // Two group bases test whole groups; bounds of two variables, or a bound
    // that is not an item, make no range.
    CHECK_FALSE(KindOnLine(references, MakeItemType(2, 100), RangeFile, WholeGroupsLine).has_value());
    CHECK_FALSE(KindOnLine(references, MakeItemType(0, 3), RangeFile, TwoVariablesLine).has_value());
    CHECK(KindOnLine(references, BladeType, RangeFile, TwoVariablesLine) == Kind::Name);
    CHECK_FALSE(KindOnLine(references, MakeItemType(0, 10), RangeFile, OpenRangeLine).has_value());

    // Each comparison in its own parentheses.
    CHECK(KindOnLine(references, MakeItemType(14, 20), RangeFile, ParenthesesRangeLine) == Kind::RawId);
    CHECK(KindOnLine(references, MakeItemType(14, 21), RangeFile, ParenthesesRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, MakeItemType(14, 22), RangeFile, ParenthesesRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, MakeItemType(14, 23), RangeFile, ParenthesesRangeLine) == Kind::RawId);
}

TEST_CASE("The item code references ignore comments and string literals [editor][items]")
{
    TemporarySourceFolder folder;
    WriteSources(folder);
    ItemCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));

    const std::span<const ItemCodeReference> blade = references.Find(BladeType);
    REQUIRE(blade.size() == 1);
    CHECK(blade[0].file == RangeFile);
    CHECK(references.Find(OldAntidoteType).empty());

    // The text keeps the comment of the line.
    const std::span<const ItemCodeReference> jewel = references.Find(JewelOfBlessType);
    REQUIRE(jewel.size() == 1);
    CHECK(jewel[0].file == CommentFile);
    CHECK(jewel[0].line == JewelLine);
    CHECK(jewel[0].text == "int jewel = ITEM_JEWEL_OF_BLESS; /* ITEM_BLADE */");
}

TEST_CASE("The item code references give the constant an item is named by in code [editor][items]")
{
    TemporarySourceFolder folder;
    WriteSources(folder);
    ItemCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));

    // The ITEM_ constant, though _enum.h defines the MODEL_ one first.
    CHECK(references.GetCodeName(KrisType) == "ITEM_KRIS");
    CHECK(references.GetCodeName(BoltType) == "MODEL_BOLT");
    // Commented out, not defined, or no item type.
    CHECK(references.GetCodeName(OldAntidoteType).empty());
    CHECK(references.GetCodeName(MakeItemType(15, 0)).empty());
    CHECK(references.GetCodeName(-1).empty());
    CHECK(references.GetCodeName(MAX_ITEM).empty());
    CHECK(references.Find(MAX_ITEM).empty());
}

TEST_CASE("The item code references know nothing without the source folder or its _enum.h [editor][items]")
{
    TemporarySourceFolder folder;
    WriteSources(folder);
    ItemCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));

    // A failed load forgets what an earlier one read.
    CHECK_FALSE(references.Load(folder.Directory() / "no such folder"));
    CHECK_FALSE(references.IsLoaded());
    CHECK(references.Find(KrisType).empty());
    CHECK(references.GetCodeName(KrisType).empty());

    REQUIRE(std::filesystem::remove(folder.Directory() / std::filesystem::path(EnumFile)));
    CHECK_FALSE(references.Load(folder.Directory()));
    CHECK_FALSE(references.IsLoaded());
    CHECK(references.Find(KrisType).empty());
}
#endif // _EDITOR
