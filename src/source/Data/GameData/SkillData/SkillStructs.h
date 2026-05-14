#pragma once

#include <windows.h>

// Forward declarations for constants
#ifndef MAX_CLASS
#define MAX_CLASS 7
#endif

#ifndef MAX_DUTY_CLASS
#define MAX_DUTY_CLASS 3
#endif

#ifndef MAX_SKILL_NAME
#define MAX_SKILL_NAME 50
#endif

// Include X-macro field definitions
#include "SkillFieldDefs.h"

// Legacy file format structure (32-byte name)
// Used for backward compatibility with old BMD files
typedef struct
{
    char Name[32];  // Legacy format used 32 bytes
    SKILL_ATTRIBUTE_FIELDS
} SKILL_ATTRIBUTE_FILE_LEGACY;

// Current file format structure with MAX_SKILL_NAME byte name
// Used for reading/writing BMD files
typedef struct
{
    char Name[MAX_SKILL_NAME];
    SKILL_ATTRIBUTE_FIELDS
} SKILL_ATTRIBUTE_FILE;

// Runtime structure with wide-character name
// Used in-memory during gameplay/editor
typedef struct
{
    wchar_t Name[MAX_SKILL_NAME];
    SKILL_ATTRIBUTE_FIELDS
} SKILL_ATTRIBUTE;

// ============================================================================
// COPY HELPERS (mirrors ItemStructs.h pattern)
// ============================================================================

// Generate field copy statements from X-macro
#define COPY_SKILL_FIELD(name, type, arraySize, width) (dest).name = (source).name;

// Macro to copy all non-name fields from source to dest
#define COPY_SKILL_ATTRIBUTE_FIELDS(dest, source) \
    do { \
        SKILL_FIELDS_SIMPLE(COPY_SKILL_FIELD) \
        memcpy((dest).RequireDutyClass, (source).RequireDutyClass, sizeof((source).RequireDutyClass)); \
        memcpy((dest).RequireClass, (source).RequireClass, sizeof((source).RequireClass)); \
        SKILL_FIELDS_AFTER_ARRAYS(COPY_SKILL_FIELD) \
    } while(0)

// Helper template to copy from file structure to runtime structure
// Requires: #include "MultiLanguage.h"
template<typename TSource>
inline void CopySkillAttributeFromSource(SKILL_ATTRIBUTE& dest, const TSource& source)
{
    CMultiLanguage::ConvertFromUtf8(dest.Name, source.Name, MAX_SKILL_NAME);
    COPY_SKILL_ATTRIBUTE_FIELDS(dest, source);
}

// Helper template to copy from runtime structure to file structure
// Requires: #include "MultiLanguage.h"
template<typename TDest>
inline void CopySkillAttributeToDestination(TDest& dest, const SKILL_ATTRIBUTE& source)
{
    CMultiLanguage::ConvertToUtf8(dest.Name, source.Name, sizeof(dest.Name));
    COPY_SKILL_ATTRIBUTE_FIELDS(dest, source);
}
