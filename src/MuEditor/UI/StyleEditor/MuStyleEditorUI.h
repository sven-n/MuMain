#pragma once

#ifdef _EDITOR

#include "LookEdits.h"
#include "LookParts.h"
#include "LooksRoot.h"

#include "../MuEditor/UI/Common/NamedTree.h"
#include "../MuEditor/UI/ItemTools/ItemModelPreview.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"
#include "Data/GameData/LookData/LookEffects.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

// The Style Editor (design L18 to L20, L24, L31, L35): Windows > Style Editor, built like the Colors window. A tree
// with the roots Render styles and Item effects and the user's folders; each look with how many items use it, "(code)"
// for one still in code, which can be looked at but not changed. The picked look of Data/Effects/RenderStyles.json
// shows the parts of the item picked in the item tools, each with the effects on it, after a row for the look's base
// (the item's own texture) and one for every part; a part is picked in the list or by a click on it in the preview, Add
// effect puts an effect of the gallery on it (L40), and the picked effect shows only its own settings (All fields:
// every field of its layer). "Show the layers" shows the look as its layers in drawing order instead, with the item
// values it leaves after drawing as the last line, and the fields of the picked line. Below them, the problems of the
// look as it is now (Apply is off while a fatal one is left) and the items that use it. Above them, staying where it is
// while they scroll, a preview of the item in three views (L41): the item as the game draws it or without a look, the
// look as applied, and the look as the fields have it, before it is applied, with one camera, the time stopped or
// moving, an eye per effect or layer (for the edited one) and "Draw as: doppelganger". Apply asks what the change is
// for (L36): all the items that use the look, which it lists (D26), only the picked item (as a copy of the look) or a
// new item. The changes are item edits (L39): one undo history with the item tools, and Save Items writes the file.
namespace MuEditor::Looks
{
// A look ForItem made for the preview, kept with what it was made from, so it is made again only when that changes.
struct ItemPreviewLook
{
    std::optional<Data::Looks::LookDefinition> from;
    Data::Looks::LookKind kind = Data::Looks::LookKind::RenderStyle;
    int itemType = -1;
    int modelsVersion = -1;
    std::optional<Data::Looks::LookDefinition> forItem;
};
} // namespace MuEditor::Looks

class CMuStyleEditorUI
{
public:
    static CMuStyleEditorUI& GetInstance();

    void Render(bool* open);
    // Opens the window on the render style of that name, in the file or in code, and brings it to the front.
    void ShowRenderStyle(const std::string& name);
    // The same for an item effect.
    void ShowItemEffect(const std::string& name);
    // Between frames: the preview's texture can only be released then.
    void BeforeFrame()
    {
        m_preview.BeforeFrame();
    }

    // The look the tree has picked; "" while a root or a folder is.
    [[nodiscard]] const std::string& GetPickedLook() const
    {
        return m_tree.GetPick().entry;
    }
    // The picked look as its fields have it now.
    [[nodiscard]] const Data::Looks::LookDefinition& GetDraft() const
    {
        return m_draft;
    }
    // Whether the look has changes not applied yet, which stay with it while another look is picked; the tree marks
    // its row.
    [[nodiscard]] bool HasUnappliedChanges(const std::string& name,
                                           Data::Looks::LookKind kind = Data::Looks::LookKind::RenderStyle) const;
    // No draft, picked or kept, as before a look was first picked: for a test that brings looks of its own.
    void ForgetDraftsForTest();
    // The picked row of the parts list: a mesh, MuEditor::Looks::EveryPart or BasePart.
    [[nodiscard]] int GetPickedPart() const
    {
        return m_part;
    }
    // A part shows alone in the preview, the others see-through, as a row of the mesh list keeps it; the base and every
    // part show the whole item.
    void PickPart(int part);
    // Puts the gallery's effect on the picked part (every part for the base row) into the draft and picks it; an effect
    // that needs one part (Data::Looks::NeedsOnePart) only on a part.
    void AddEffect(Data::Looks::EffectKind kind);
    // The picked line: a layer, or past the last one (the item values of the layers view, no effect in the parts view).
    [[nodiscard]] std::size_t GetPickedLine() const
    {
        return m_line;
    }

private:
    enum class NameDialog
    {
        None,
        New,
        Copy,
        Rename,
    };
    // What Apply changes (design L36).
    enum class ApplyTo
    {
        AllItems,
        ThisItem,
        NewItem,
    };

    CMuStyleEditorUI();

    [[nodiscard]] bool IsRenderStylesPicked() const;
    // The kind of the picked root's looks, its root and the edits of its file.
    [[nodiscard]] Data::Looks::LookKind PickedKind() const;
    [[nodiscard]] MuEditor::Looks::LooksRoot& RootOf(Data::Looks::LookKind kind);
    [[nodiscard]] const MuEditor::Looks::LooksRoot& RootOf(Data::Looks::LookKind kind) const;
    [[nodiscard]] static MuEditor::Looks::LookEdits& EditsOf(Data::Looks::LookKind kind);
    // The picked look of the file, or nullptr (a root, a folder, a look in code).
    [[nodiscard]] const Data::Looks::LookDefinition* GetPickedFileLook() const;
    void RenderFileBar();
    void RenderToolbar();
    void OpenNameDialog(NameDialog dialog, std::string name);
    void RenderNameDialog();
    void RenderPicked();
    void RenderRoot();
    void RenderCodeLook(const std::string& name, bool itemEffect);
    void RenderLook(const Data::Looks::LookDefinition& look);
    void RenderApply(const Data::Looks::LookDefinition& look);
    void OpenApply(const Data::Looks::LookDefinition& look);
    void RenderConfirm();
    // The Apply choices for one item, below its radio buttons; false while the copy or the new item can't be made.
    [[nodiscard]] bool RenderApplyCopy();
    [[nodiscard]] std::string UnusedLookName(const std::string& base) const;
    void RenderLayers();
    void RenderLayerToolbar();
    // An item effect: its entries before the drawing in their order, then the object's light, and the picked line's
    // fields.
    void RenderItemEffect();
    void RenderParts();
    void RenderPartRow(const char* label, int part, const std::vector<std::size_t>& layers);
    void RenderEffectChip(std::size_t layer, int part);
    void RenderAddEffect();
    void RenderEffectSettings();
    // The part rows: the meshes of the item's model, and the others the look names.
    [[nodiscard]] std::vector<MuEditor::Looks::LookPart> PartRows() const;
    void RenderChecks();
    // The items that use the look (an item effect's with `itemEffect`), looked for again when the item models change.
    [[nodiscard]] const std::vector<int>& UsersOf(const std::string& name, bool itemEffect);
    void RenderUsers(const std::vector<int>& users);
    void RenderPreview(const Data::Looks::LookDefinition* look);
    // What stands above a view of the preview: the original item's dropdown, or the view's name.
    void RenderViewHeader(int view, float width);
    // The fields follow the look (an undo, a redo) until they are changed.
    void UpdateDraft(const Data::Looks::LookDefinition& look);
    // The draft is the look again, every layer shown.
    void LoadDraft(const Data::Looks::LookDefinition& look);
    // The draft of the look picked before, when it has changes not applied; its draft again when it had them.
    void KeepDraft();
    [[nodiscard]] bool TakeKeptDraft(const std::string& name);
    // A look renamed: its draft, picked or kept, goes with it.
    void RenameDraft(const std::string& from, const std::string& to);
    // The draft's layers change with their eyes: a layer in at `at` (picked), one out, two swapped.
    void InsertLayer(std::size_t at, Data::Looks::Layer layer);
    void RemoveLayer(std::size_t at);
    void SwapLayers(std::size_t first, std::size_t second);
    // When the draft or the item models changed: its problems, and the textures it names.
    void CheckDraft();
    void Apply();
    void ApplyCopy(ApplyTo to, const std::string& copyName, int itemType, int newItemType, const std::string& itemName);

    MuEditor::Looks::LooksRoot m_renderStyles{Data::Looks::LookKind::RenderStyle};
    MuEditor::Looks::LooksRoot m_itemEffects{Data::Looks::LookKind::ItemEffect};
    MuEditor::NamedTree m_tree;
    MuEditor::Items::CItemModelPreview m_preview;

    // The picked look as the fields have it, and its look when they took it.
    Data::Looks::LookDefinition m_draft;
    Data::Looks::LookDefinition m_draftLoaded;
    std::string m_draftOf;
    Data::Looks::LookKind m_draftKind = Data::Looks::LookKind::RenderStyle;
    int m_draftVersion = -1;
    // The drafts with changes not applied of the looks picked before, by kind and name (DraftKey).
    struct KeptDraft
    {
        Data::Looks::LookDefinition draft;
        Data::Looks::LookDefinition loaded;
        std::vector<std::uint8_t> hiddenLayers;
        std::size_t line = 0;
    };
    std::map<std::string, KeptDraft, std::less<>> m_keptDrafts;
    // The users UsersOf found last, of which look, as of which version of the item models.
    struct LookUsers
    {
        std::string name;
        bool itemEffect = false;
        int modelsVersion = -1;
        std::vector<int> items;
    };
    LookUsers m_users;
    // The picked line: a layer, or the item values (the number of layers).
    std::size_t m_line = 0;
    // The layers the preview leaves out (the eyes), by layer: 1 for one left out. As many as the draft's layers: they
    // change together (LoadDraft, InsertLayer, RemoveLayer, SwapLayers).
    std::vector<std::uint8_t> m_hiddenLayers;
    // The looks the preview draws as the picked item has them: as applied and as edited.
    MuEditor::Looks::ItemPreviewLook m_appliedForItem;
    MuEditor::Looks::ItemPreviewLook m_editedForItem;
    bool m_showLayers = false;
    bool m_allFields = false;
    int m_part = MuEditor::Looks::EveryPart;
    // The mesh a click in the preview kept, as last seen: a new one picks its part.
    std::optional<int> m_keptMesh;
    // The problems of the draft, as checked last, and what they were checked for.
    std::vector<Data::Items::ItemDataIssue> m_draftIssues;
    Data::Looks::LookDefinition m_checkedDraft;
    Data::Looks::LookKind m_checkedKind = Data::Looks::LookKind::RenderStyle;
    int m_checkedModels = -1;
    bool m_checked = false;

    bool m_stopTime = false;
    float m_seconds = 0.0f;
    bool m_asDoppelganger = false;
    // The preview's first view draws the item without a look instead of as the game draws it.
    bool m_originalWithoutLook = false;

    NameDialog m_nameDialog = NameDialog::None;
    std::string m_name;
    // The look Copy and Rename start from.
    std::string m_nameFrom;
    std::vector<int> m_confirmUsers;
    ApplyTo m_applyTo = ApplyTo::AllItems;
    // The item Apply was opened for, -1 for none; the copy's name, and the new item's number and name.
    int m_applyItem = -1;
    std::string m_copyName;
    int m_newNumber = 0;
    std::string m_newItemName;
};

#define g_MuStyleEditorUI CMuStyleEditorUI::GetInstance()

namespace MuEditor::Looks
{
// The look as the item of that type draws it in the game, for the preview: with the item's values when the item names
// the look and sets values of it (design L12); nullopt when it draws the look as the look has it.
std::optional<Data::Looks::LookDefinition> ForItem(const Data::Looks::LookDefinition& look, Data::Looks::LookKind kind,
                                                   int itemType);
// ForItem, kept in `kept` while the look, its kind, the item and the item models are the same; the textures the made
// look names are found when it is made. nullptr when the item draws the look as the look has it.
const Data::Looks::LookDefinition* ForItemKept(ItemPreviewLook& kept, const Data::Looks::LookDefinition& look,
                                               Data::Looks::LookKind kind, int itemType);
// The look without the layers whose eye is off (`hidden` non-zero), all else as it is: Only when, Not for, Draws alone,
// the item values.
Data::Looks::LookDefinition WithoutHiddenLayers(const Data::Looks::LookDefinition& look,
                                                const std::vector<std::uint8_t>& hidden);
} // namespace MuEditor::Looks

#endif // _EDITOR
