#pragma once

#ifdef _EDITOR

#include <Windows.h>
#include "GameData/ItemData/ItemFieldMetadata.h"
#include "_struct.h"

// Forward declaration
class CItemEditorTable;

// Handles all column rendering for the Item Editor
// Uses metadata-driven approach for automatic field rendering
class CItemEditorColumns
{
public:
    CItemEditorColumns() = default;
    ~CItemEditorColumns() = default;

    // Metadata-driven rendering - automatically renders any field based on metadata
    void RenderFieldByMetadata(const ItemFieldMetadata& meta, int& colIdx, int itemIndex,
                               ITEM_ATTRIBUTE& item, bool& rowInteracted, bool isVisible);

    // Render Index column (special - not part of ITEM_ATTRIBUTE fields)
    void RenderIndexColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible);

private:
    // Low-level type-specific rendering helpers
    void RenderByteColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         BYTE& value, bool& rowInteracted, bool isVisible);
    void RenderWordColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         WORD& value, bool& rowInteracted, bool isVisible);
    void RenderIntColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                        int& value, bool& rowInteracted, bool isVisible);
    void RenderBoolColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         bool& value, bool& rowInteracted, bool isVisible);
    void RenderWCharArrayColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                               wchar_t* value, int arraySize, bool& rowInteracted, bool isVisible);
};

#endif // _EDITOR
