#pragma once

#ifdef _EDITOR

#include "ItemModelFiles.h"

#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Items
{
// The model tool's list of model files to open for the item (Data/Item and
// Data/Player), with a search. Picking one gives the item that file of its
// own (an item without a model entry gets one, with the file's folder as its
// texture folder); a shared model that opens the same file is offered instead.
class CItemModelFilePicker
{
public:
    // `model`: the item's model entry, nullptr when it has none.
    void Render(int itemType, const Data::Items::ItemModelDefinition* model);

private:
    void Pick(int itemType, const Data::Items::ItemModelDefinition* model, std::string_view file);
    void RenderOtherUsers(int itemType, const Data::Items::ItemModelDefinition& model);

    ItemModelFiles m_files;
    std::string m_search;
    std::string m_foundFor;
    bool m_foundValid = false;
    std::vector<std::string_view> m_found;
};
} // namespace MuEditor::Items

#endif // _EDITOR
