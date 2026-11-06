#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCodeReferenceIndex.h"

#include "UI/Common/CodeReferenceScan.h"

namespace MuEditor::Items
{
namespace
{
ItemCodeReferences LoadItemCodeReferences()
{
    ItemCodeReferences references;
    references.Load(CodeReferenceScan::EditorSourceFolder());
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
