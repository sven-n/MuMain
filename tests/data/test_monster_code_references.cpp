#include "stdafx.h"

#include "doctest.h"

// The source lines that name a monster number, which the Monster Browser
// shows before a monster is removed (MN1).
#ifdef _EDITOR
#include "Core/Globals/_enum.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "UI/MonsterTools/MonsterCodeLine.h"
#include "UI/MonsterTools/MonsterCodeReferenceIndex.h"
#include "UI/MonsterTools/MonsterCodeReferences.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <optional>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <system_error>

using MuEditor::CodeReference;
using MuEditor::Monsters::MonsterCodeReferences;

namespace
{
using Kind = CodeReference::Kind;

// A source folder whose files are written by the test.
class TemporarySourceFolder
{
public:
    TemporarySourceFolder()
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_monster_code_references_" + std::to_string(std::random_device{}())))
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
constexpr std::string_view EnumSource = R"source(enum EMonsterModelType : int
{
    MONSTER_MODEL_UNDEFINED = -1,
    MONSTER_MODEL_HOUND = 1,
    MONSTER_MODEL_LICH = 4,
};

enum EMonsterType : int
{
    MONSTER_UNDEFINED = -1,
    MONSTER_START = 0,

    MONSTER_BULL_FIGHTER = 0,
    MONSTER_HOUND = 1,
    MONSTER_BUDGE_DRAGON = 2,
    MONSTER_GOLDEN_WHEEL = 81,
    MONSTER_GATE_TO_KALIMA_1 = 152,
    MONSTER_GATE_TO_KALIMA_7 = 158,
    // MONSTER_TRAP = 523,
    MONSTER_GUARD = 600,

    MONSTER_END = 668
};

enum
{
    MONSTER_NOT_A_MONSTER = 7,
};
)source";

constexpr int BullFighter = 0;
constexpr int Hound = 1;
constexpr int BudgeDragon = 2;
constexpr int GoldenWheel = 81;
constexpr int Guard = 600;

constexpr std::string_view NameFile = "GameLogic/MonsterNames.cpp";
constexpr std::string_view NameSource = R"source(bool IsHound(const CHARACTER* c)
{
    return c->MonsterIndex == MONSTER_HOUND || c->MonsterIndex == MONSTER_HOUND; // Hound
}
int Model()
{
    return MONSTER_MODEL_LICH + MONSTER_NOT_A_MONSTER;
}
)source";
constexpr int HoundTwiceLine = 3;
constexpr int ModelLine = 7;

constexpr std::string_view RawIdFile = "Engine/MonsterRawIds.cpp";
constexpr std::string_view RawIdSource = R"source(void Show(CHARACTER* c, int iMonsterIndex, int rawType)
{
    if (iMonsterIndex == 495 || 319 == c->MonsterIndex)
        return;
    if (rawType == 566 || CharactersClient[0].MonsterIndex != (497))
        return;
    getMonsterName(236);
    if (c->MonsterIndex == 1024 || c->MonsterIndex == 5 + 1 || FindCharacterIndexByMonsterIndex(216) == 3)
        return;
    if (c->MonsterIndex == 2 || c->MonsterIndex == MONSTER_BUDGE_DRAGON)
        return;
}
)source";
constexpr int RawIdLine = 3;
constexpr int OtherNamesLine = 5;
constexpr int CallLine = 7;
constexpr int NotNumbersLine = 8;
constexpr int RawIdAndNameLine = 10;

constexpr std::string_view RangeFile = "UI/MonsterRanges.h";
constexpr std::string_view RangeSource = R"source(inline bool InRanges(int type, const CHARACTER* c, int iMonsterIndex)
{
    if (iMonsterIndex >= 493 && iMonsterIndex <= 502)
        return true;
    if (c->MonsterIndex == MONSTER_HOUND || (78 <= c->MonsterIndex && c->MonsterIndex <= MONSTER_GOLDEN_WHEEL))
        return true;
    if (type >= MONSTER_GATE_TO_KALIMA_1 && type <= MONSTER_GATE_TO_KALIMA_7)
        return true;
    if (type >= 700 && type <= 710)
        return true;
    if (c->MonsterIndex >= MONSTER_START && c->MonsterIndex <= MONSTER_END)
        return true;
    return c->MonsterIndex > MONSTER_GUARD && c->MonsterIndex < MONSTER_END;
}
)source";
constexpr int RawIdRangeLine = 3;
constexpr int MixedRangeLine = 5;
constexpr int ConstantRangeLine = 7;
constexpr int OtherVariableLine = 9;
constexpr int MarkersLine = 11;
constexpr int MarkerBoundLine = 13;

constexpr std::string_view CommentFile = "Engine/MonsterComments.cpp";
constexpr std::string_view CommentSource = R"source(// MONSTER_BUDGE_DRAGON in a comment
/* MONSTER_BUDGE_DRAGON
   c->MonsterIndex == 2 */
void Log(const CHARACTER* c)
{
    Print("MONSTER_BUDGE_DRAGON and c->MonsterIndex == 2");
    int dragon = MONSTER_BUDGE_DRAGON; /* MONSTER_HOUND */
}
)source";
constexpr int DragonLine = 7;

void WriteSources(const TemporarySourceFolder& folder)
{
    folder.Write(EnumFile, EnumSource);
    folder.Write(NameFile, NameSource);
    folder.Write(RawIdFile, RawIdSource);
    folder.Write(RangeFile, RangeSource);
    folder.Write(CommentFile, CommentSource);
    // Only .cpp and .h files are read.
    folder.Write("Notes.txt", "MONSTER_HOUND\n");
}

// How the line names the monster; nothing when it does not.
std::optional<Kind> KindOnLine(const MonsterCodeReferences& references, int number, std::string_view file, int line)
{
    const std::span<const CodeReference> found = references.Find(number);
    const auto reference = std::find_if(found.begin(), found.end(), [&](const CodeReference& candidate)
                                        { return candidate.file == file && candidate.line == line; });
    if (reference == found.end())
    {
        return std::nullopt;
    }
    return reference->kind;
}

MonsterCodeReferences LoadReferences(const TemporarySourceFolder& folder)
{
    WriteSources(folder);
    MonsterCodeReferences references;
    REQUIRE(references.Load(folder.Directory()));
    return references;
}
} // namespace

TEST_CASE("A line of enum EMonsterType defines a monster constant as NAME = number [editor][monsters]")
{
    using MuEditor::Monsters::ReadMonsterConstant;
    const auto hound = ReadMonsterConstant("    MONSTER_HOUND = 1,");
    REQUIRE(hound.has_value());
    CHECK(hound->first == "MONSTER_HOUND");
    CHECK(hound->second == 1);
    // The last one has no comma.
    CHECK(ReadMonsterConstant("MONSTER_END = 668") == std::pair<std::string_view, int>{"MONSTER_END", 668});
    // Not a monster number: a model constant, another constant, a number
    // outside 0-1023 or a negative one.
    CHECK_FALSE(ReadMonsterConstant("MONSTER_MODEL_HOUND = 1,").has_value());
    CHECK_FALSE(ReadMonsterConstant("MONSTER_X = MONSTER_HOUND,").has_value());
    CHECK_FALSE(ReadMonsterConstant("MONSTER_X = 1024,").has_value());
    CHECK_FALSE(ReadMonsterConstant("MONSTER_UNDEFINED = -1,").has_value());
    CHECK_FALSE(ReadMonsterConstant("ITEM_KRIS = 1,").has_value());

    CHECK(MuEditor::Monsters::IsMonsterMarker("MONSTER_START"));
    CHECK(MuEditor::Monsters::IsMonsterMarker("MONSTER_END"));
    CHECK_FALSE(MuEditor::Monsters::IsMonsterMarker("MONSTER_HOUND"));
}

TEST_CASE("The monster code references find the lines that name a monster by its constant [editor][monsters]")
{
    TemporarySourceFolder folder;
    const MonsterCodeReferences references = LoadReferences(folder);
    CHECK(references.IsLoaded());

    // Sorted by file and line; a line that names the monster twice is one
    // reference. _enum.h defines the names and is not read for them.
    const std::span<const CodeReference> hound = references.Find(Hound);
    REQUIRE(hound.size() == 2);
    CHECK(hound[0].file == NameFile);
    CHECK(hound[0].line == HoundTwiceLine);
    CHECK(hound[0].text == "return c->MonsterIndex == MONSTER_HOUND || c->MonsterIndex == MONSTER_HOUND; // Hound");
    CHECK(hound[0].kind == Kind::Name);
    // In a header.
    CHECK(hound[1].file == RangeFile);
    CHECK(hound[1].line == MixedRangeLine);

    // A model constant and a MONSTER_ name outside enum EMonsterType name no
    // monster; neither does the commented-out constant.
    CHECK_FALSE(KindOnLine(references, 4, NameFile, ModelLine).has_value());
    CHECK_FALSE(KindOnLine(references, 7, NameFile, ModelLine).has_value());
    CHECK(references.Find(523).empty());
}

TEST_CASE("The monster code references find numbers compared with a MonsterIndex [editor][monsters]")
{
    TemporarySourceFolder folder;
    const MonsterCodeReferences references = LoadReferences(folder);

    // On either side of the comparison, and in parentheses.
    CHECK(KindOnLine(references, 495, RawIdFile, RawIdLine) == Kind::RawId);
    CHECK(KindOnLine(references, 319, RawIdFile, RawIdLine) == Kind::RawId);
    CHECK(KindOnLine(references, 497, RawIdFile, OtherNamesLine) == Kind::RawId);
    // Numbers compared with other names, or passed to a function, are not
    // found.
    CHECK(references.Find(566).empty());
    CHECK(references.Find(236).empty());
    CHECK(references.Find(216).empty());
    CHECK(references.Find(3).empty());
    // Past 1023, or part of a sum.
    CHECK_FALSE(KindOnLine(references, 5, RawIdFile, NotNumbersLine).has_value());
    CHECK_FALSE(KindOnLine(references, 6, RawIdFile, NotNumbersLine).has_value());
    CHECK(references.Find(1024).empty());
    // The number and the constant on one line: one reference, by the constant.
    const std::span<const CodeReference> dragon = references.Find(BudgeDragon);
    CHECK(std::count_if(dragon.begin(), dragon.end(), [](const CodeReference& reference)
                        { return reference.file == RawIdFile && reference.line == RawIdAndNameLine; }) == 1);
    CHECK(KindOnLine(references, BudgeDragon, RawIdFile, RawIdAndNameLine) == Kind::Name);
    CHECK_FALSE(KindOnLine(references, BudgeDragon, RawIdFile, CallLine).has_value());
}

TEST_CASE("The monster code references find the monsters that a range check on one line encloses "
          "[editor][monsters]")
{
    TemporarySourceFolder folder;
    const MonsterCodeReferences references = LoadReferences(folder);

    // Numbers compared with a MonsterIndex as bounds.
    CHECK(KindOnLine(references, 493, RangeFile, RawIdRangeLine) == Kind::RawId);
    CHECK(KindOnLine(references, 494, RangeFile, RawIdRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, 501, RangeFile, RawIdRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, 502, RangeFile, RawIdRangeLine) == Kind::RawId);
    CHECK(references.Find(503).empty());

    // A number and a constant, the number on the left.
    CHECK(KindOnLine(references, 78, RangeFile, MixedRangeLine) == Kind::RawId);
    CHECK(KindOnLine(references, 80, RangeFile, MixedRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, GoldenWheel, RangeFile, MixedRangeLine) == Kind::Name);
    CHECK(KindOnLine(references, Hound, RangeFile, MixedRangeLine) == Kind::Name);

    // Constants bound a range of any variable; numbers only of a MonsterIndex.
    CHECK(KindOnLine(references, 152, RangeFile, ConstantRangeLine) == Kind::Name);
    CHECK(KindOnLine(references, 155, RangeFile, ConstantRangeLine) == Kind::Range);
    CHECK(KindOnLine(references, 158, RangeFile, ConstantRangeLine) == Kind::Name);
    CHECK(references.Find(705).empty());

    // Two markers name nothing; a marker and a constant name what lies between.
    CHECK_FALSE(KindOnLine(references, 300, RangeFile, MarkersLine).has_value());
    CHECK_FALSE(KindOnLine(references, BullFighter, RangeFile, MarkersLine).has_value());
    CHECK(KindOnLine(references, Guard, RangeFile, MarkerBoundLine) == Kind::Name);
    CHECK(KindOnLine(references, Guard + 1, RangeFile, MarkerBoundLine) == Kind::Range);
    CHECK(KindOnLine(references, 667, RangeFile, MarkerBoundLine) == Kind::Range);
    CHECK_FALSE(KindOnLine(references, 668, RangeFile, MarkerBoundLine).has_value());
}

TEST_CASE("The monster code references ignore comments and string literals [editor][monsters]")
{
    TemporarySourceFolder folder;
    const MonsterCodeReferences references = LoadReferences(folder);

    const std::span<const CodeReference> dragon = references.Find(BudgeDragon);
    REQUIRE(dragon.size() == 2);
    // The text keeps the comment of the line.
    CHECK(dragon[0].file == CommentFile);
    CHECK(dragon[0].line == DragonLine);
    CHECK(dragon[0].text == "int dragon = MONSTER_BUDGE_DRAGON; /* MONSTER_HOUND */");
    CHECK(dragon[1].file == RawIdFile);
    CHECK(dragon[1].line == RawIdAndNameLine);
}

TEST_CASE("The monster code references know nothing without the source folder or its _enum.h "
          "[editor][monsters]")
{
    TemporarySourceFolder folder;
    folder.Write(NameFile, NameSource);
    MonsterCodeReferences references;
    CHECK_FALSE(references.Load(folder.Directory()));
    CHECK_FALSE(references.IsLoaded());
    CHECK(references.Find(Hound).empty());
    CHECK_FALSE(references.Load(folder.Directory() / "missing"));
    CHECK(references.Find(-1).empty());
}

// The sources the editor was built from: enum EMonsterType is where the
// reader looks for it, and the code has each kind of reference.
TEST_CASE("The monster code references of the editor's sources [editor][monsters]")
{
    const MonsterCodeReferences& references = MuEditor::Monsters::GetMonsterCodeReferences();
    REQUIRE(references.IsLoaded());

    const std::span<const CodeReference> dragon = references.Find(MONSTER_BUDGE_DRAGON);
    REQUIRE_FALSE(dragon.empty());
    CHECK(std::all_of(dragon.begin(), dragon.end(),
                      [](const CodeReference& reference) { return reference.file != EnumFile; }));
    bool rawId = false;
    bool range = false;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        for (const CodeReference& reference : references.Find(number))
        {
            rawId = rawId || reference.kind == Kind::RawId;
            range = range || reference.kind == Kind::Range;
        }
    }
    CHECK(rawId);
    CHECK(range);
}

// Removing a monster that a transformation ring turns into lists the ring.
TEST_CASE("The monster code references list the transformation ring for each of its forms [editor][monsters]")
{
    const MonsterCodeReferences& references = MuEditor::Monsters::GetMonsterCodeReferences();
    REQUIRE(references.IsLoaded());

    constexpr std::string_view RingFile = "GameLogic/Items/TransformationRing.cpp";
    for (const int form : {MONSTER_BUDGE_DRAGON, MONSTER_GIANT, MONSTER_SKELETON_WARRIOR, MONSTER_POISON_BULL,
                           MONSTER_THUNDER_LICH, MONSTER_DEATH_COW})
    {
        CAPTURE(form);
        const std::span<const CodeReference> found = references.Find(form);
        CHECK(std::any_of(found.begin(), found.end(),
                          [&](const CodeReference& reference) { return reference.file == RingFile; }));
    }
}

#endif // _EDITOR
