#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR

#include "UI/GraphicsLook/LookEditHistory.h"

#include <string>

using Data::Maps::LookProfileData;
using Data::Maps::LookValue;
using MuEditor::GraphicsLook::LookEditHistory;

namespace
{
LookProfileData WithBloom(double bloom)
{
    LookProfileData profile;
    profile.defaultOverrides[LookValue::BloomStrength] = bloom;
    return profile;
}
} // namespace

TEST_CASE("the look edit history undoes and redoes whole profiles, with their labels [editor][look]")
{
    LookEditHistory history;
    LookProfileData live = WithBloom(1.0);
    const LookEditHistory::Apply apply = [&live](const LookProfileData& profile)
    {
        live = profile;
        return true;
    };

    CHECK_FALSE(history.CanUndo());
    CHECK(history.GetUndoLabel().empty());
    history.Record("default: bloom.strength 0.5", WithBloom(1.0), WithBloom(0.5));
    history.Record("default: bloom.strength 0.2", WithBloom(0.5), WithBloom(0.2));
    live = WithBloom(0.2);
    CHECK(history.GetUndoLabel() == "default: bloom.strength 0.2");

    REQUIRE(history.Undo(apply));
    CHECK(live == WithBloom(0.5));
    CHECK(history.GetRedoLabel() == "default: bloom.strength 0.2");
    REQUIRE(history.Undo(apply));
    CHECK(live == WithBloom(1.0));
    CHECK_FALSE(history.Undo(apply));

    REQUIRE(history.Redo(apply));
    CHECK(live == WithBloom(0.5));

    // A new change ends what could be redone.
    history.Record("default: bloom.strength 0.9", WithBloom(0.5), WithBloom(0.9));
    CHECK_FALSE(history.CanRedo());

    history.Clear();
    CHECK_FALSE(history.CanUndo());
    CHECK_FALSE(history.CanRedo());
}

TEST_CASE("a refused undo keeps its step, and the history keeps the last steps only [editor][look]")
{
    LookEditHistory history;
    history.Record("one", WithBloom(1.0), WithBloom(0.5));
    CHECK_FALSE(history.Undo([](const LookProfileData&) { return false; }));
    CHECK(history.CanUndo());
    CHECK(history.GetUndoLabel() == "one");

    for (std::size_t index = 0; index < LookEditHistory::MaxUndoSteps; ++index)
    {
        history.Record("step " + std::to_string(index), WithBloom(1.0), WithBloom(0.5));
    }
    std::size_t undone = 0;
    while (history.Undo([](const LookProfileData&) { return true; }))
    {
        ++undone;
    }
    CHECK(undone == LookEditHistory::MaxUndoSteps);
}

#endif // _EDITOR
