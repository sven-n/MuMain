#include "stdafx.h"

#ifdef _EDITOR

#include "ItemGroupNames.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"

namespace MuEditor::Items
{
std::string GetItemGroupLabel(int group)
{
    // "Group00_Sword.json" -> "Sword"
    const std::string fileName = Data::Items::GetItemGroupFileName(group);
    const size_t start = fileName.find('_') + 1;
    const size_t end = fileName.rfind('.');
    return std::to_string(group) + " " + fileName.substr(start, end - start);
}
} // namespace MuEditor::Items

#endif // _EDITOR
