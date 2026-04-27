#pragma once

#ifdef _EDITOR

#include <windows.h>
#include <string>
#include "Translation/i18n.h"

// ============================================================================
// SHARED FIELD TYPE SYSTEM
// ============================================================================
// This is used by both ItemEditor and SkillEditor (and future editors)

enum class EFieldType
{
    Bool,
    Byte,
    Word,
    Int,
    DWord,
    WCharArray
};

// Helper macros to convert type enum to actual C++ types
#define FIELD_TYPE_Bool bool
#define FIELD_TYPE_Byte BYTE
#define FIELD_TYPE_Word WORD
#define FIELD_TYPE_Int int
#define FIELD_TYPE_DWord DWORD

// ============================================================================
// GENERIC FIELD DESCRIPTOR
// ============================================================================

template<typename TStruct>
struct FieldDescriptor
{
    const char* name;
    EFieldType type;
    size_t offset;
    float width;
};

// ============================================================================
// SHARED HELPER FUNCTIONS
// ============================================================================

// Get translated field name with fallback to raw field name
inline const char* GetFieldDisplayName(const char* fieldName)
{
    std::string translationKey = std::string("field_") + fieldName;
    if (i18n::HasTranslation(i18n::Domain::Metadata, translationKey.c_str()))
    {
        return i18n::TranslateMetadata(translationKey.c_str(), fieldName);
    }
    return fieldName;  // Fallback to raw field name if no translation
}

// ============================================================================
// GENERIC FIELD RENDERING TEMPLATE
// ============================================================================
// This template renders any field based on its descriptor
// TColumns must provide: RenderBoolColumn, RenderByteColumn, RenderWordColumn,
//                        RenderIntColumn, RenderDWordColumn, RenderWCharArrayColumn
// TStruct is the data structure type (e.g., ITEM_ATTRIBUTE, SKILL_ATTRIBUTE)

template<typename TColumns, typename TStruct>
inline void RenderFieldByDescriptor(
    const FieldDescriptor<TStruct>& desc,
    TColumns* cols,
    TStruct& data,
    int& colIdx,
    int dataIndex,
    bool& rowInteracted,
    bool isVisible,
    int maxStringLength)
{
    if (!isVisible) return;

    BYTE* dataPtr = reinterpret_cast<BYTE*>(&data);
    void* fieldPtr = dataPtr + desc.offset;
    const char* displayName = GetFieldDisplayName(desc.name);

    // Generate unique ID for ImGui
    int uniqueId = 0;
    for (const char* p = desc.name; *p; ++p) uniqueId = (uniqueId * 31) + *p;

    switch (desc.type)
    {
    case EFieldType::Bool:
        cols->RenderBoolColumn(displayName, colIdx, dataIndex, uniqueId,
                               *reinterpret_cast<bool*>(fieldPtr), rowInteracted, isVisible);
        break;
    case EFieldType::Byte:
        cols->RenderByteColumn(displayName, colIdx, dataIndex, uniqueId,
                               *reinterpret_cast<BYTE*>(fieldPtr), rowInteracted, isVisible);
        break;
    case EFieldType::Word:
        cols->RenderWordColumn(displayName, colIdx, dataIndex, uniqueId,
                               *reinterpret_cast<WORD*>(fieldPtr), rowInteracted, isVisible);
        break;
    case EFieldType::Int:
        cols->RenderIntColumn(displayName, colIdx, dataIndex, uniqueId,
                              *reinterpret_cast<int*>(fieldPtr), rowInteracted, isVisible);
        break;
    case EFieldType::DWord:
        cols->RenderDWordColumn(displayName, colIdx, dataIndex, uniqueId,
                                *reinterpret_cast<DWORD*>(fieldPtr), rowInteracted, isVisible);
        break;
    case EFieldType::WCharArray:
        cols->RenderWCharArrayColumn(displayName, colIdx, dataIndex, uniqueId,
                                     reinterpret_cast<wchar_t*>(fieldPtr), maxStringLength,
                                     rowInteracted, isVisible);
        break;
    }
}

#endif // _EDITOR
