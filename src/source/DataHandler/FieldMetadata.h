#pragma once

#include <cstddef>
#include <sstream>

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
            bool oldVal = *(const bool*)oldPtr;
            bool newVal = *(const bool*)newPtr;
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
            BYTE oldVal = *(const BYTE*)oldPtr;
            BYTE newVal = *(const BYTE*)newPtr;
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
            WORD oldVal = *(const WORD*)oldPtr;
            WORD newVal = *(const WORD*)newPtr;
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
            int oldVal = *(const int*)oldPtr;
            int newVal = *(const int*)newPtr;
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
            DWORD oldVal = *(const DWORD*)oldPtr;
            DWORD newVal = *(const DWORD*)newPtr;
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
            float oldVal = *(const float*)oldPtr;
            float newVal = *(const float*)newPtr;
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
