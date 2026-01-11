#pragma once

#ifdef _EDITOR

#include <windows.h>
#include "GameData/ItemData/ItemFieldMetadata.h"
#include "_struct.h"

// Forward declaration
class CItemEditorTable;

// Handles all column rendering for the Item Editor
// Uses metadata-driven approach for automatic field rendering
class CItemEditorColumns
{
public:
    CItemEditorColumns() : m_pTable(nullptr), m_lastEditedIndex(-1), m_errorLogged(false) {}
    ~CItemEditorColumns() = default;

    // Set the parent table instance (needed for invalidating filter)
    void SetTable(CItemEditorTable* table) { m_pTable = table; }

    // X-macro-driven rendering - automatically renders any field based on descriptor
    void RenderFieldByDescriptor(const FieldDescriptor& desc, int& colIdx, int itemIndex,
                                 ITEM_ATTRIBUTE& item, bool& rowInteracted, bool isVisible);

    // Render Index column (special - not part of ITEM_ATTRIBUTE fields)
    void RenderIndexColumn(int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible);

    // Friend declaration for template helper in ItemFieldMetadata.h
    template<typename TColumns>
    friend void ::RenderFieldByDescriptor(const FieldDescriptor& desc, TColumns* cols, ITEM_ATTRIBUTE& item,
                                          int& colIdx, int itemIndex, bool& rowInteracted, bool isVisible);

private:
    // Low-level type-specific rendering helpers
    void RenderByteColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         BYTE& value, bool& rowInteracted, bool isVisible);
    void RenderWordColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         WORD& value, bool& rowInteracted, bool isVisible);
    void RenderIntColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                        int& value, bool& rowInteracted, bool isVisible);
    void RenderDWordColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                          DWORD& value, bool& rowInteracted, bool isVisible);
    void RenderBoolColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                         bool& value, bool& rowInteracted, bool isVisible);
    void RenderWCharArrayColumn(const char* columnName, int& colIdx, int itemIndex, int uniqueId,
                               wchar_t* value, int arraySize, bool& rowInteracted, bool isVisible);

    // Parent table reference
    CItemEditorTable* m_pTable;

    // Index column state tracking
    int m_lastEditedIndex;
    bool m_errorLogged;
};

#endif // _EDITOR
