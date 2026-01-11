#pragma once

#ifdef _EDITOR

#include <Windows.h>
#include "GameData/SkillData/SkillFieldMetadata.h"
#include "_struct.h"

// Forward declaration
class CSkillEditorTable;

// Handles all column rendering for the Skill Editor
// Uses metadata-driven approach for automatic field rendering
class CSkillEditorColumns
{
public:
    CSkillEditorColumns() : m_pTable(nullptr), m_lastEditedIndex(-1), m_errorLogged(false) {}
    ~CSkillEditorColumns() = default;

    // Set the parent table instance (needed for invalidating filter)
    void SetTable(CSkillEditorTable* table) { m_pTable = table; }

    // X-macro-driven rendering - automatically renders any field based on descriptor
    void RenderFieldByDescriptor(const SkillFieldDescriptor& desc, int& colIdx, int skillIndex,
                                 SKILL_ATTRIBUTE& skill, bool& rowInteracted, bool isVisible);

    // Render Index column (special - not part of SKILL_ATTRIBUTE fields)
    void RenderIndexColumn(int& colIdx, int skillIndex, bool& rowInteracted, bool isVisible);

    // Friend declaration for template helper in FieldMetadataHelper.h
    template<typename TColumns, typename TStruct>
    friend void ::RenderFieldByDescriptor(const FieldDescriptor<TStruct>& desc, TColumns* cols, TStruct& data,
                                          int& colIdx, int dataIndex, bool& rowInteracted, bool isVisible, int maxStringLength);

private:
    // Low-level type-specific rendering helpers
    void RenderByteColumn(const char* columnName, int& colIdx, int skillIndex, int uniqueId,
                         BYTE& value, bool& rowInteracted, bool isVisible);
    void RenderWordColumn(const char* columnName, int& colIdx, int skillIndex, int uniqueId,
                         WORD& value, bool& rowInteracted, bool isVisible);
    void RenderIntColumn(const char* columnName, int& colIdx, int skillIndex, int uniqueId,
                        int& value, bool& rowInteracted, bool isVisible);
    void RenderDWordColumn(const char* columnName, int& colIdx, int skillIndex, int uniqueId,
                          DWORD& value, bool& rowInteracted, bool isVisible);
    void RenderBoolColumn(const char* columnName, int& colIdx, int skillIndex, int uniqueId,
                         bool& value, bool& rowInteracted, bool isVisible);
    void RenderWCharArrayColumn(const char* columnName, int& colIdx, int skillIndex, int uniqueId,
                               wchar_t* value, int arraySize, bool& rowInteracted, bool isVisible);

    // Parent table reference
    CSkillEditorTable* m_pTable;

    // Index column state tracking
    int m_lastEditedIndex;
    bool m_errorLogged;
};

#endif // _EDITOR
