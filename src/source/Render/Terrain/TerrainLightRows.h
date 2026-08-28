#pragma once

// Tracks which rows of PrimaryTerrainLight changed since each terrain-light VBO was
// last written, so UploadTerrainLightVBO() can upload just those rows instead of the
// whole terrain-sized vec3 array on every single frame.
//
// A row is the unit of tracking (rather than a single vertex) because the light array
// is row-major -- one row is one contiguous byte range in the VBO, so a run of dirty
// rows collapses into a single buffer update.
namespace Render::Terrain::LightRows
{
    // Terrain grid size. Deliberately not taken from TERRAIN_SIZE in Core/Globals/_define.h:
    // that header is not standalone-includable (it needs Windows types declared before it),
    // and depending on it would drag the whole precompiled-header chain into this otherwise
    // self-contained, unit-testable module. ZzzLodTerrain.cpp static_asserts the two against
    // each other, so they cannot drift apart unnoticed.
    inline constexpr int RowCount = 256;

    // The terrain light VBO is double buffered: a frame writes and draws one buffer
    // while the GPU may still be reading the other. Each buffer therefore needs its
    // own dirty set -- a row written while buffer 0 was current is still stale in
    // buffer 1 until buffer 1's turn comes around. Marking always dirties both sets;
    // only the buffer that actually gets written clears its own.
    inline constexpr int BufferCount = 2;

    // Upper bound on the number of runs CollectRuns can produce (every other row
    // dirty), so callers can size a stack array for it without a heap allocation.
    inline constexpr int MaxRuns = RowCount / 2 + 1;

    // A contiguous span of dirty rows, ready to become one buffer update.
    struct Run
    {
        int firstRow;
        int rowCount;
    };

    // Marks every row of every buffer dirty. Used when the VBOs are (re)created and
    // their contents are undefined.
    void MarkAll();

    // Marks an inclusive row range dirty in every buffer. Rows are wrapped into
    // [0, RowCount) exactly like TERRAIN_INDEX_REPEAT does, so callers can pass
    // unwrapped loop bounds straight from the terrain code.
    void MarkRange(int firstRow, int lastRow);

    // Writes the dirty-row runs of one buffer into outRuns and returns how many were
    // written. Does not modify the dirty state -- call ClearRuns once the runs have
    // actually been uploaded.
    int CollectRuns(int bufferIndex, Run* outRuns, int maxRuns);

    // Drops one buffer's dirty rows, after its pending runs have been uploaded.
    void ClearRuns(int bufferIndex);
}
