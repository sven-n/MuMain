#pragma once

#ifdef _EDITOR

#include <Windows.h>
#include <string>
#include "SkillStructs.h"
#include "SkillFieldDefs.h"
#include "GameData/Common/FieldMetadataHelper.h"

// ============================================================================
// SKILL-SPECIFIC METADATA
// ============================================================================

// Type alias for clarity - skills use the same type system as items
using ESkillFieldType = EFieldType;

// Skill field descriptor (specialized from generic FieldDescriptor)
using SkillFieldDescriptor = FieldDescriptor<SKILL_ATTRIBUTE>;

// Macros for descriptor generation
#define MAKE_SKILL_FIELD_DESCRIPTOR(name, type, arraySize, width) \
    { #name, EFieldType::type, offsetof(SKILL_ATTRIBUTE, name), width },

#define MAKE_SKILL_ARRAY_DESCRIPTOR(nameWithIndex, baseName, index, type, width) \
    { #nameWithIndex, EFieldType::type, offsetof(SKILL_ATTRIBUTE, baseName[index]), width },

// Static descriptor array - automatically generated from X-macros
namespace SkillFieldMetadataInternal
{
    static const SkillFieldDescriptor s_descriptors[] = {
        // Name field is special (UTF-16 string)
        { "Name", EFieldType::WCharArray, offsetof(SKILL_ATTRIBUTE, Name), 150.0f },
        // All other fields from X-macros
        SKILL_FIELDS_SIMPLE(MAKE_SKILL_FIELD_DESCRIPTOR)
        SKILL_FIELDS_ARRAYS(MAKE_SKILL_ARRAY_DESCRIPTOR)
    };
}

// Accessor functions
inline const SkillFieldDescriptor* GetSkillFieldDescriptors()
{
    return SkillFieldMetadataInternal::s_descriptors;
}

inline constexpr int GetSkillFieldCount()
{
    return sizeof(SkillFieldMetadataInternal::s_descriptors) / sizeof(SkillFieldDescriptor);
}

// Get display name for a skill field (with translation support)
inline const char* GetSkillFieldDisplayName(const char* fieldName)
{
    return GetFieldDisplayName(fieldName);
}

// Helper to render a skill field by descriptor (uses generic helper)
template<typename TColumns>
inline void RenderSkillFieldByDescriptor(const SkillFieldDescriptor& desc, TColumns* cols, SKILL_ATTRIBUTE& skill,
                                          int& colIdx, int skillIndex, bool& rowInteracted, bool isVisible)
{
    ::RenderFieldByDescriptor(desc, cols, skill, colIdx, skillIndex, rowInteracted, isVisible, MAX_SKILL_NAME);
}

#endif // _EDITOR
