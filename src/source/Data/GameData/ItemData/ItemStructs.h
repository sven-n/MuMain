#pragma once

#include <windows.h>
#include "MultiLanguage.h"

// Forward declarations for constants
#ifndef MAX_CLASS
#define MAX_CLASS 7
#endif

#ifndef MAX_RESISTANCE
#define MAX_RESISTANCE 7
#endif

#ifndef MAX_ITEM_NAME
#define MAX_ITEM_NAME 50
#endif

// Include X-macro field definitions (single source of truth)
#include "ItemFieldDefs.h"

// Legacy file format structure (S6E3) with 30-byte name
// Used for backward compatibility with old BMD files
typedef struct
{
    char Name[30];
    ITEM_ATTRIBUTE_FIELDS
} ITEM_ATTRIBUTE_FILE_LEGACY;

// Current file format structure with MAX_ITEM_NAME byte name
// Used for reading/writing BMD files
typedef struct
{
    char Name[MAX_ITEM_NAME];
    ITEM_ATTRIBUTE_FIELDS
} ITEM_ATTRIBUTE_FILE;

// Runtime structure with wide-character name
// Used in-memory during gameplay
typedef struct
{
    wchar_t Name[MAX_ITEM_NAME];
    ITEM_ATTRIBUTE_FIELDS
} ITEM_ATTRIBUTE;

// ============================================================================
// COPY HELPERS (replaces ItemAttributeHelpers.h)
// ============================================================================

// Generate field copy statements from X-macro
#define COPY_FIELD(name, type, arraySize, width) (dest).name = (source).name;

// Macro to copy all non-name fields from source to dest
#define COPY_ITEM_ATTRIBUTE_FIELDS(dest, source) \
    do { \
        ITEM_FIELDS_SIMPLE(COPY_FIELD) \
        memcpy((dest).RequireClass, (source).RequireClass, sizeof((source).RequireClass)); \
        memcpy((dest).Resistance, (source).Resistance, sizeof((source).Resistance)); \
    } while(0)

// Helper template to copy from file structure to runtime structure
// Requires: #include "MultiLanguage.h"
template<typename TSource>
inline void CopyItemAttributeFromSource(ITEM_ATTRIBUTE& dest, const TSource& source)
{
    CMultiLanguage::ConvertFromUtf8(dest.Name, source.Name, MAX_ITEM_NAME);
    COPY_ITEM_ATTRIBUTE_FIELDS(dest, source);
}

// Helper template to copy from runtime structure to file structure
// Requires: #include "MultiLanguage.h"
template<typename TDest>
inline void CopyItemAttributeToDestination(TDest& dest, const ITEM_ATTRIBUTE& source)
{
    CMultiLanguage::ConvertToUtf8(dest.Name, source.Name, sizeof(dest.Name));
    COPY_ITEM_ATTRIBUTE_FIELDS(dest, source);
}
