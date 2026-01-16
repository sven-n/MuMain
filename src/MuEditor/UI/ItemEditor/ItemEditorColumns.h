#pragma once

#ifdef _EDITOR

#include <Windows.h>

// Forward declaration
class CItemEditorTable;

// Handles all column rendering for the Item Editor
// Contains reusable column rendering methods to eliminate duplication
class CItemEditorColumns
{
public:
    CItemEditorColumns() = default;
    ~CItemEditorColumns() = default;

    // Column rendering helpers
    void RenderByteColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         BYTE& value, bool& rowInteracted, bool isVisible);
    void RenderWordColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         WORD& value, bool& rowInteracted, bool isVisible);
    void RenderIntColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                        int& value, bool& rowInteracted, bool isVisible);
    void RenderBoolColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         bool& value, bool& rowInteracted, bool isVisible);
    void RenderByteArrayColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                              BYTE* array, int arraySize, bool& rowInteracted, bool isVisible);

    // Special columns
    void RenderIndexColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible);
    void RenderNameColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible);
    void RenderTwoHandColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible);
};

#endif // _EDITOR
