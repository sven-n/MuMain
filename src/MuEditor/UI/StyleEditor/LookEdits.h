#pragma once

#ifdef _EDITOR

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"
#include "Data/GameData/LookData/LookJson.h"

#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Looks
{
// The looks of one kind the Style Editor changes (design L18, L31, L39): the looks and folders of
// Data/Effects/RenderStyles.json or ItemEffects.json as the editor has them (CItemDataHandler::GetLookFile). Each
// change is an item edit (one undo step with the item tools, ItemEdits::SetLookFile) and reaches the game at once, so
// it draws what the editor shows; Save Items writes the file.
class LookEdits final
{
public:
    // The looks, sorted by name, and their folders.
    using State = Data::Looks::RenderStylesFile;

    // The render styles' edits.
    static LookEdits& GetInstance();
    // The edits of the looks of `kind`.
    static LookEdits& Of(Data::Looks::LookKind kind);

    [[nodiscard]] Data::Looks::LookKind GetKind() const
    {
        return m_kind;
    }

    [[nodiscard]] const State& GetState() const;
    [[nodiscard]] const Data::Looks::LookDefinition* Find(std::string_view name) const;
    // Counts the changes, also undone and redone, for what the windows keep between frames.
    [[nodiscard]] int GetVersion() const;
    // The problems of the file's read, and whether it had no fatal one: otherwise the looks are the game's and Save
    // Items leaves the file alone.
    [[nodiscard]] const std::vector<Data::Items::ItemDataIssue>& GetIssues() const;
    [[nodiscard]] bool IsComplete() const;

    // One undo step each, named `step`. SetLook replaces the look `name` (its new name may differ: the folders follow
    // it); AddLook puts the look into the folder at `folder` (the empty path: under the root).
    void SetLook(const char* step, const std::string& name, Data::Looks::LookDefinition look);
    void AddLook(const char* step, Data::Looks::LookDefinition look, const Data::Colors::ColorFolderPath& folder);
    void RemoveLook(const char* step, const std::string& name);
    void SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders);

    // Apply for one item (design L36): `copy`, the edited look under a name no look has, goes into the folder of
    // `original`, and only the item at `itemType` names it (as its render style or item effect, by the kind); or a new
    // item at the free item type `newItemType` with the English name `itemName`, duplicated from `itemType` with its
    // model (Duplicate item), names it. The look and the item are one undo step `step`. Nothing happens for an item
    // without a model.
    void ApplyToItemOnly(const char* step, const std::string& original, Data::Looks::LookDefinition copy, int itemType);
    void ApplyToNewItem(const char* step, const std::string& original, Data::Looks::LookDefinition copy, int itemType,
                        int newItemType, const std::string& itemName);

    // A test's state, as if read from the file, with nothing to undo.
    void ResetForTest(State state);

private:
    explicit LookEdits(Data::Looks::LookKind kind) : m_kind(kind) {}
    void Change(const char* step, State next);
    // The folder the look is in; the empty path for the root.
    [[nodiscard]] Data::Colors::ColorFolderPath FolderOf(const std::string& name) const;

    // The change count of the looks whose textures were loaded last: the first read can give the game looks it did
    // not have (the file changed since the game started).
    mutable int m_texturesVersion = -1;
    Data::Looks::LookKind m_kind;
};
} // namespace MuEditor::Looks

#define g_LookEdits MuEditor::Looks::LookEdits::GetInstance()
#define g_ItemEffectEdits MuEditor::Looks::LookEdits::Of(Data::Looks::LookKind::ItemEffect)

#endif // _EDITOR
