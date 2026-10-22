#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCodeReferenceIndex.h"

#include "MuEditorSourceDir.h"

#include <filesystem>

namespace MuEditor::Items
{
namespace
{
// The sources the editor was built from (src/source); UTF-8, as the path may
// hold any letter.
constexpr const char8_t* SourceDirectory = MU_EDITOR_SOURCE_DIR;

ItemCodeReferences LoadItemCodeReferences()
{
    ItemCodeReferences references;
    references.Load(std::filesystem::path(SourceDirectory));
    return references;
}
} // namespace

const ItemCodeReferences& GetItemCodeReferences()
{
    static const ItemCodeReferences references = LoadItemCodeReferences();
    return references;
}
} // namespace MuEditor::Items

#endif // _EDITOR
