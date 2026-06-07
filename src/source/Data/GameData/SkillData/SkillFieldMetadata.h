#pragma once

#ifdef _EDITOR

#include "Core/Platform/WinCompat.h"
#include "SkillStructs.h"
#include "SkillFieldDefs.h"
#include "Data/GameData/Common/FieldMetadataHelper.h"
#include "I18N/All.h"

// ============================================================================
// SKILL-SPECIFIC METADATA
// ============================================================================

// Type alias for clarity - skills use the same type system as items
using ESkillFieldType = EFieldType;

// Skill field descriptor (specialized from generic FieldDescriptor)
using SkillFieldDescriptor = FieldDescriptor<SKILL_ATTRIBUTE>;

// Macros for descriptor generation
#define MAKE_SKILL_FIELD_DESCRIPTOR(name, type, arraySize, width, i18nName) \
    { #name, &I18N::Metadata::i18nName, EFieldType::type, offsetof(SKILL_ATTRIBUTE, name), width },

#define MAKE_SKILL_ARRAY_DESCRIPTOR(nameWithIndex, baseName, index, type, width, i18nName) \
    { #nameWithIndex, &I18N::Metadata::i18nName, EFieldType::type, offsetof(SKILL_ATTRIBUTE, baseName[index]), width },

// Static descriptor array - automatically generated from X-macros
namespace SkillFieldMetadataInternal
{
    static const SkillFieldDescriptor s_descriptors[] = {
        // Name field is special (UTF-16 string) and stands outside the X-macros.
        { "Name", &I18N::Metadata::Name, EFieldType::WCharArray, offsetof(SKILL_ATTRIBUTE, Name), 150.0f },
        SKILL_FIELDS_SIMPLE(MAKE_SKILL_FIELD_DESCRIPTOR)
        SKILL_FIELDS_ARRAYS(MAKE_SKILL_ARRAY_DESCRIPTOR)
        SKILL_FIELDS_AFTER_ARRAYS(MAKE_SKILL_FIELD_DESCRIPTOR)
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

// Get display name for a skill field descriptor (follows active locale).
inline const char* GetSkillFieldDisplayName(const SkillFieldDescriptor& desc)
{
    return GetFieldDisplayName(desc);
}

// Helper to render a skill field by descriptor (uses generic helper)
template<typename TColumns>
inline void RenderSkillFieldByDescriptor(const SkillFieldDescriptor& desc, TColumns* cols, SKILL_ATTRIBUTE& skill,
                                          int& colIdx, int skillIndex, bool& rowInteracted, bool isVisible)
{
    ::RenderFieldByDescriptor(desc, cols, skill, colIdx, skillIndex, rowInteracted, isVisible, MAX_SKILL_NAME);
}

#endif // _EDITOR
