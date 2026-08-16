#include "doctest.h"

#include "Render/Terrain/TerrainLightRows.h"

using namespace Render::Terrain::LightRows;

namespace
{
    // The tracker is a file-scope singleton shared by every case, so each case starts by
    // draining both buffers rather than assuming it runs first.
    void ResetTracker()
    {
        for (int buffer = 0; buffer < BufferCount; buffer++)
        {
            ClearRuns(buffer);
        }
    }
}

TEST_CASE("terrain light rows: nothing is dirty until something marks it")
{
    ResetTracker();

    Run runs[MaxRuns];
    CHECK(CollectRuns(0, runs, MaxRuns) == 0);
    CHECK(CollectRuns(1, runs, MaxRuns) == 0);
}

TEST_CASE("terrain light rows: a marked range becomes one run")
{
    ResetTracker();
    MarkRange(10, 12);

    Run runs[MaxRuns];
    REQUIRE(CollectRuns(0, runs, MaxRuns) == 1);
    CHECK(runs[0].firstRow == 10);
    CHECK(runs[0].rowCount == 3);
}

TEST_CASE("terrain light rows: marks land in every buffer and clear independently")
{
    // This is the double-buffering invariant: the light VBO alternates between two GL
    // buffers, so a row written while one is current is still stale in the other until
    // that one's turn comes around. Clearing the uploaded buffer must not drop the
    // other's pending work.
    ResetTracker();
    MarkRange(10, 12);

    Run runs[MaxRuns];
    REQUIRE(CollectRuns(0, runs, MaxRuns) == 1);
    REQUIRE(CollectRuns(1, runs, MaxRuns) == 1);

    ClearRuns(0);

    CHECK(CollectRuns(0, runs, MaxRuns) == 0);
    CHECK(CollectRuns(1, runs, MaxRuns) == 1);
}

TEST_CASE("terrain light rows: a range crossing the wrap splits into head and tail runs")
{
    // Terrain indexing wraps with TERRAIN_SIZE_MASK, so callers legitimately pass ranges
    // that run off the end of the grid.
    ResetTracker();
    MarkRange(RowCount - 2, RowCount + 1);

    Run runs[MaxRuns];
    REQUIRE(CollectRuns(0, runs, MaxRuns) == 2);
    CHECK(runs[0].firstRow == 0);
    CHECK(runs[0].rowCount == 2);
    CHECK(runs[1].firstRow == RowCount - 2);
    CHECK(runs[1].rowCount == 2);
}

TEST_CASE("terrain light rows: negative rows wrap like TERRAIN_INDEX_REPEAT")
{
    ResetTracker();
    MarkRange(-2, -1);

    Run runs[MaxRuns];
    REQUIRE(CollectRuns(0, runs, MaxRuns) == 1);
    CHECK(runs[0].firstRow == RowCount - 2);
    CHECK(runs[0].rowCount == 2);
}

TEST_CASE("terrain light rows: an over-wide range collapses to the whole grid")
{
    ResetTracker();
    MarkRange(0, RowCount * 4);

    Run runs[MaxRuns];
    REQUIRE(CollectRuns(0, runs, MaxRuns) == 1);
    CHECK(runs[0].firstRow == 0);
    CHECK(runs[0].rowCount == RowCount);
}

TEST_CASE("terrain light rows: an inverted range marks nothing")
{
    ResetTracker();
    MarkRange(5, 4);

    Run runs[MaxRuns];
    CHECK(CollectRuns(0, runs, MaxRuns) == 0);
}

TEST_CASE("terrain light rows: worst-case fragmentation fits in MaxRuns")
{
    // Every other row dirty is the most runs the grid can produce; MaxRuns exists so
    // callers can size a stack array for it without a heap allocation.
    ResetTracker();
    for (int row = 0; row < RowCount; row += 2)
    {
        MarkRange(row, row);
    }

    Run runs[MaxRuns];
    const int runCount = CollectRuns(0, runs, MaxRuns);
    CHECK(runCount == RowCount / 2);
    CHECK(runCount <= MaxRuns);
}

TEST_CASE("terrain light rows: MarkAll dirties every row of every buffer")
{
    ResetTracker();
    MarkAll();

    Run runs[MaxRuns];
    REQUIRE(CollectRuns(0, runs, MaxRuns) == 1);
    CHECK(runs[0].rowCount == RowCount);
    REQUIRE(CollectRuns(1, runs, MaxRuns) == 1);
    CHECK(runs[0].rowCount == RowCount);
}

TEST_CASE("terrain light rows: out-of-range buffer indices are rejected")
{
    ResetTracker();
    MarkAll();

    Run runs[MaxRuns];
    CHECK(CollectRuns(-1, runs, MaxRuns) == 0);
    CHECK(CollectRuns(BufferCount, runs, MaxRuns) == 0);

    // Must not disturb the real buffers either.
    ClearRuns(-1);
    ClearRuns(BufferCount);
    CHECK(CollectRuns(0, runs, MaxRuns) == 1);
}

TEST_CASE("terrain light rows: a null or zero-length destination collects nothing")
{
    ResetTracker();
    MarkAll();

    Run runs[MaxRuns];
    CHECK(CollectRuns(0, nullptr, MaxRuns) == 0);
    CHECK(CollectRuns(0, runs, 0) == 0);
}
