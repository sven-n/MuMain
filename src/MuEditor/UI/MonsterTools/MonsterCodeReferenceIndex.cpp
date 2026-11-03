#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCodeReferenceIndex.h"

#include "../MuEditor/UI/Common/CodeReferenceScan.h"

namespace MuEditor::Monsters
{
namespace
{
MonsterCodeReferences LoadMonsterCodeReferences()
{
    MonsterCodeReferences references;
    references.Load(CodeReferenceScan::EditorSourceFolder());
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
