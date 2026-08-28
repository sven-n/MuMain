#include "TerrainLightRows.h"

namespace Render::Terrain::LightRows
{
    namespace
    {
        constexpr int RowMask = RowCount - 1;
        static_assert((RowCount & RowMask) == 0, "RowCount must be a power of two for the wrap mask to work");

        bool s_DirtyRows[BufferCount][RowCount] = {};

        bool IsValidBuffer(int bufferIndex)
        {
            return bufferIndex >= 0 && bufferIndex < BufferCount;
        }
    }

    void MarkAll()
    {
        for (int buffer = 0; buffer < BufferCount; buffer++)
        {
            for (int row = 0; row < RowCount; row++)
            {
                s_DirtyRows[buffer][row] = true;
            }
        }
    }

    void MarkRange(int firstRow, int lastRow)
    {
        if (lastRow < firstRow) return;

        // A range this wide covers every row once wrapped, so skip the per-row work.
        if (lastRow - firstRow >= RowCount - 1)
        {
            MarkAll();
            return;
        }

        for (int row = firstRow; row <= lastRow; row++)
        {
            const int wrappedRow = row & RowMask;
            for (int buffer = 0; buffer < BufferCount; buffer++)
            {
                s_DirtyRows[buffer][wrappedRow] = true;
            }
        }
    }

    int CollectRuns(int bufferIndex, Run* outRuns, int maxRuns)
    {
        if (outRuns == nullptr || maxRuns <= 0) return 0;
        if (!IsValidBuffer(bufferIndex)) return 0;

        const bool* dirtyRows = s_DirtyRows[bufferIndex];
        int runCount = 0;
        int row = 0;
        while (row < RowCount && runCount < maxRuns)
        {
            if (!dirtyRows[row])
            {
                row++;
                continue;
            }

            const int firstRow = row;
            while (row < RowCount && dirtyRows[row])
            {
                row++;
            }

            outRuns[runCount].firstRow = firstRow;
            outRuns[runCount].rowCount = row - firstRow;
            runCount++;
        }

        return runCount;
    }

    void ClearRuns(int bufferIndex)
    {
        if (!IsValidBuffer(bufferIndex)) return;

        for (int row = 0; row < RowCount; row++)
        {
            s_DirtyRows[bufferIndex][row] = false;
        }
    }
}
