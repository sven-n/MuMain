#pragma once

#include <Windows.h>
#include <vector>
#include <string>

// Enum to represent different field types
enum class EItemFieldType
{
    Bool,
    Byte,
    Word,
    Int,
    WCharArray,  // For Name field (wchar_t[])
    ByteArray    // For RequireClass[], Resistance[]
};

// Metadata for a single field in ITEM_ATTRIBUTE
struct ItemFieldMetadata
{
    const char* fieldName;           // Internal field name (e.g., "TwoHand")
    const char* defaultDisplayName;  // Default English display name (e.g., "Two-Hand")
    size_t offset;                   // Offset from start of ITEM_ATTRIBUTE
    EItemFieldType type;             // Field type
    int arraySize;                   // Size for arrays (1 for non-arrays)
    float defaultColumnWidth;        // Default ImGui column width

    ItemFieldMetadata(const char* name, const char* displayName, size_t off, EItemFieldType t, int arrSize = 1, float colWidth = 70.0f)
        : fieldName(name), defaultDisplayName(displayName), offset(off), type(t), arraySize(arrSize), defaultColumnWidth(colWidth)
    {}

    // Get translated display name with fallback to defaultDisplayName
    const char* GetDisplayName() const;
};

// Helper macros to calculate field offsets
#define FIELD_OFFSET(type, field) offsetof(type, field)

// Get all item field metadata
class CItemFieldMetadataRegistry
{
public:
    static const std::vector<ItemFieldMetadata>& GetAllFields();
    static const ItemFieldMetadata* FindFieldByName(const char* fieldName);
    static int GetFieldCount();

private:
    static void InitializeMetadata();
    static std::vector<ItemFieldMetadata> s_metadata;
    static bool s_initialized;
};
