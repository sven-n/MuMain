#pragma once

#include <cstddef>
#include <sstream>
#include <span>

/**
 * Field metadata system - replaces X-macros with simple, readable arrays
 *
 * This is MUCH easier to understand and maintain than X-macros!
 */

enum class FieldType
{
    Bool,
    Byte,
    Word,
    Int,
    DWord,
    Float
};

struct FieldInfo
{
    const char* name;        // Field name for display
    FieldType type;          // Field type
    size_t offset;           // Offset in struct (use offsetof)
    size_t size;             // Size of field (use sizeof)
    int arrayIndex;          // -1 for simple fields, >= 0 for array elements
};

/**
 * Compare a single field between two structs
 *
 * Uses field metadata to determine how to compare and format the output.
 * No macros needed - just a simple function!
 */
template<typename T>
inline void CompareFieldByMetadata(
    const FieldInfo& field,
    const T& oldStruct,
    const T& newStruct,
    std::stringstream& changes,
    bool& hasChanged)
{
    // Get pointers to the actual field data
    const BYTE* oldPtr = (const BYTE*)&oldStruct + field.offset;
    const BYTE* newPtr = (const BYTE*)&newStruct + field.offset;

    bool fieldChanged = false;
    std::stringstream fieldChange;

    switch (field.type)
    {
        case FieldType::Bool:
        {
            bool oldVal, newVal;
            memcpy(&oldVal, oldPtr, sizeof(bool));
            memcpy(&newVal, newPtr, sizeof(bool));
            if (oldVal != newVal)
            {
                fieldChanged = true;
                fieldChange << "  " << field.name << ": "
                           << (oldVal ? "true" : "false") << " -> "
                           << (newVal ? "true" : "false") << "\n";
            }
            break;
        }

        case FieldType::Byte:
        {
            BYTE oldVal, newVal;
            memcpy(&oldVal, oldPtr, sizeof(BYTE));
            memcpy(&newVal, newPtr, sizeof(BYTE));
            if (oldVal != newVal)
            {
                fieldChanged = true;
                fieldChange << "  " << field.name << ": "
                           << (int)oldVal << " -> " << (int)newVal << "\n";
            }
            break;
        }

        case FieldType::Word:
        {
            WORD oldVal, newVal;
            memcpy(&oldVal, oldPtr, sizeof(WORD));
            memcpy(&newVal, newPtr, sizeof(WORD));
            if (oldVal != newVal)
            {
                fieldChanged = true;
                fieldChange << "  " << field.name << ": "
                           << oldVal << " -> " << newVal << "\n";
            }
            break;
        }

        case FieldType::Int:
        {
            int oldVal, newVal;
            memcpy(&oldVal, oldPtr, sizeof(int));
            memcpy(&newVal, newPtr, sizeof(int));
            if (oldVal != newVal)
            {
                fieldChanged = true;
                fieldChange << "  " << field.name << ": "
                           << oldVal << " -> " << newVal << "\n";
            }
            break;
        }

        case FieldType::DWord:
        {
            DWORD oldVal, newVal;
            memcpy(&oldVal, oldPtr, sizeof(DWORD));
            memcpy(&newVal, newPtr, sizeof(DWORD));
            if (oldVal != newVal)
            {
                fieldChanged = true;
                fieldChange << "  " << field.name << ": "
                           << oldVal << " -> " << newVal << "\n";
            }
            break;
        }

        case FieldType::Float:
        {
            float oldVal, newVal;
            memcpy(&oldVal, oldPtr, sizeof(float));
            memcpy(&newVal, newPtr, sizeof(float));
            if (oldVal != newVal)
            {
                fieldChanged = true;
                fieldChange << "  " << field.name << ": "
                           << oldVal << " -> " << newVal << "\n";
            }
            break;
        }
    }

    if (fieldChanged)
    {
        changes << fieldChange.str();
        hasChanged = true;
    }
}

/**
 * Compare all fields using metadata array
 * Simple loop - no macros!
 */
template<typename T>
inline void CompareAllFieldsByMetadata(
    const T& oldStruct,
    const T& newStruct,
    std::span<const FieldInfo> fields,
    std::stringstream& changes,
    bool& hasChanged)
{
    for (const auto& field : fields)
    {
        CompareFieldByMetadata(field, oldStruct, newStruct, changes, hasChanged);
    }
}
