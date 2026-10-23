#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCodeReferenceIndex.h"

#include "MuEditorSourceDir.h"

#include <filesystem>

namespace MuEditor::Monsters
{
namespace
{
// The sources the editor was built from (src/source); UTF-8, as the path may
// hold any letter.
constexpr const char8_t* SourceDirectory = MU_EDITOR_SOURCE_DIR;

MonsterCodeReferences LoadMonsterCodeReferences()
{
    MonsterCodeReferences references;
    references.Load(std::filesystem::path(SourceDirectory));
    return references;
}
} // namespace

const MonsterCodeReferences& GetMonsterCodeReferences()
{
    static const MonsterCodeReferences references = LoadMonsterCodeReferences();
    return references;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
