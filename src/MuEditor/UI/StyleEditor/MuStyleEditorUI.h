#pragma once

#ifdef _EDITOR

#include "StyleEditorRoots.h"

#include "../MuEditor/UI/Common/NamedTree.h"
#include "../MuEditor/UI/ItemTools/ItemModelPreview.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <cstdint>
#include <string>
#include <vector>

// The Style Editor (design L18 to L20, L24, L31): Windows > Style Editor, built like the Colors window. A tree with the
// roots Render styles and Item effects and the user's folders; each look with how many items use it, "(code)" for one
// still in code, which can be looked at but not changed. The picked look of Data/Effects/RenderStyles.json shows its
// layers in drawing order, one line each, and the item values it leaves after drawing as the last line; below, the
// fields of the picked line, then the problems of the look as it is now (Apply is off while a fatal one is left), the
// items that use it, and a preview of the item picked in the item tools drawn with the look before it is applied, with
// the time stopped or moving, an eye per layer and "Draw as: doppelganger". Apply lists the items a change reaches
// (D26). The changes are steps of the editor's look history (LookEdits); Save writes the file.
class CMuStyleEditorUI
{
public:
    static CMuStyleEditorUI& GetInstance();

    void Render(bool* open);
    // Opens the window on the render style of that name, in the file or in code, and brings it to the front.
    void ShowRenderStyle(const std::string& name);
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

private:
    enum class NameDialog
    {
        None,
        New,
        Copy,
        Rename,
    };

    CMuStyleEditorUI();

    [[nodiscard]] bool IsRenderStylesPicked() const;
    // The picked look of the file, or nullptr (a root, a folder, a look in code, an item effect).
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
    void RenderConfirm();
    void RenderLayers();
    void RenderLayerToolbar();
    void RenderChecks();
    void RenderUsers(const std::vector<int>& users);
    void RenderPreview(const Data::Looks::LookDefinition* look);
    // The fields follow the look (an undo, a redo) until they are changed.
    void UpdateDraft(const Data::Looks::LookDefinition& look);
    void CheckDraft();
    void Apply();

    MuEditor::Looks::RenderStylesRoot m_renderStyles;
    MuEditor::Looks::ItemEffectsRoot m_itemEffects;
    MuEditor::NamedTree m_tree;
    MuEditor::Items::CItemModelPreview m_preview;

    // The picked look as the fields have it, and its look when they took it.
    Data::Looks::LookDefinition m_draft;
    Data::Looks::LookDefinition m_draftLoaded;
    std::string m_draftOf;
    int m_draftVersion = -1;
    // The picked line: a layer, or the item values (the number of layers).
    std::size_t m_line = 0;
    // The layers the preview leaves out (the eyes), by layer: 1 for one left out.
    std::vector<std::uint8_t> m_hiddenLayers;
    // The problems of the draft, as checked last, and what they were checked for.
    std::vector<Data::Items::ItemDataIssue> m_draftIssues;
    Data::Looks::LookDefinition m_checkedDraft;
    int m_checkedModels = -1;
    bool m_checked = false;

    bool m_stopTime = false;
    float m_seconds = 0.0f;
    bool m_asDoppelganger = false;

    NameDialog m_nameDialog = NameDialog::None;
    std::string m_name;
    // The look Copy and Rename start from.
    std::string m_nameFrom;
    std::vector<int> m_confirmUsers;
    std::string m_status;
    bool m_statusIsError = false;
};

#define g_MuStyleEditorUI CMuStyleEditorUI::GetInstance()

#endif // _EDITOR
