#include "stdafx.h"

#include "doctest.h"

// The Style Editor (design L18 to L20, L31): its edits of the render style file, which reach the game at once and
// are undo steps, the window on a look of the file or one still in code, and Data Checks opening it.
#ifdef _EDITOR

#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookStore.h"
#include "I18N/All.h"
#include "Render/Items/ItemRenderStyles.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ItemTools/ItemEdits.h"
#include "UI/ItemTools/ItemSelection.h"
#include "UI/StyleEditor/LookEdits.h"
#include "UI/StyleEditor/MuStyleEditorUI.h"
#include "UI/StyleEditor/RenderStyleCheckArea.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <memory>
#include <string>
#include <vector>

using Data::Colors::ColorFolder;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Looks::Blend;
using Data::Looks::Layer;
using Data::Looks::LookDefinition;

namespace
{
// The looks of the editor and of the game for one test, with nothing to undo and nothing to save, and the ones before
// it again after it.
class LookEditsScope
{
public:
    explicit LookEditsScope(std::vector<LookDefinition> looks, std::vector<ColorFolder> folders = {})
        : m_previous(Data::Looks::g_LookStore.GetRenderStyles().begin(),
                     Data::Looks::g_LookStore.GetRenderStyles().end())
    {
        g_LookEdits.ResetForTest({std::move(looks), std::move(folders)});
        g_ItemEdits.GetHistory().Clear();
        g_ItemEdits.GetHistory().MarkSaved();
    }
    ~LookEditsScope()
    {
        g_ItemEdits.GetHistory().Clear();
        g_ItemEdits.GetHistory().MarkSaved();
        g_LookEdits.ResetForTest({m_previous, {}});
    }
    LookEditsScope(const LookEditsScope&) = delete;
    LookEditsScope& operator=(const LookEditsScope&) = delete;

private:
    std::vector<LookDefinition> m_previous;
};

LookDefinition Plain(const char* name)
{
    return {name, {Layer{}}, {}};
}

const LookDefinition* GameLook(const char* name)
{
    return Data::Looks::g_LookStore.FindRenderStyle(name);
}

void RenderStyleEditor()
{
    ImGui::NewFrame();
    bool open = true;
    g_MuStyleEditorUI.Render(&open);
    ImGui::Render();
    CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
}
} // namespace

TEST_CASE("The Style Editor's changes of a look reach the game at once and are item edits (design L39) [editor][looks]")
{
    const LookEditsScope looks({Plain("brightBlade")});
    LookDefinition glowing = Plain("brightBlade");
    glowing.layers[0].blend = Blend::Glow;

    g_LookEdits.SetLook("Change a look", "brightBlade", glowing);
    REQUIRE(GameLook("brightBlade") != nullptr);
    CHECK(GameLook("brightBlade")->layers[0].blend == Blend::Glow);
    // A step of the item tools' history, which Save Items saves.
    CHECK(g_ItemEdits.GetHistory().IsModifiedSinceSave());
    CHECK(g_ItemEdits.GetHistory().GetUndoLabel() == "Change a look");

    CHECK(g_ItemEdits.Undo());
    CHECK(GameLook("brightBlade")->layers[0].blend == Blend::Normal);
    CHECK_FALSE(g_ItemEdits.GetHistory().IsModifiedSinceSave());
    CHECK(g_ItemEdits.GetHistory().GetRedoLabel() == "Change a look");

    CHECK(g_ItemEdits.Redo());
    CHECK(GameLook("brightBlade")->layers[0].blend == Blend::Glow);

    // A step that changes nothing is none.
    CHECK(g_ItemEdits.Undo());
    g_LookEdits.SetLook("Change a look", "brightBlade", Plain("brightBlade"));
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());

    // A look and an item changed together are one step.
    g_ItemEdits.BeginGroup("Only this item");
    g_LookEdits.AddLook("Only this item", Plain("brightBladeGold"), {});
    g_ItemEdits.EndGroup();
    CHECK(GameLook("brightBladeGold") != nullptr);
    CHECK(g_ItemEdits.Undo());
    CHECK(GameLook("brightBladeGold") == nullptr);
}

TEST_CASE("A look added, renamed or removed in the Style Editor takes its place in the folders along [editor][looks]")
{
    const LookEditsScope looks({Plain("a")}, {{"Blades", {}, {"a"}}});

    g_LookEdits.AddLook("New look", Plain("c"), {0});
    CHECK(g_LookEdits.GetState().folders == std::vector<ColorFolder>{{"Blades", {}, {"a", "c"}}});
    CHECK(GameLook("c") != nullptr);

    g_LookEdits.SetLook("Rename a look", "a", Plain("b"));
    CHECK(g_LookEdits.GetState().folders == std::vector<ColorFolder>{{"Blades", {}, {"b", "c"}}});
    CHECK(GameLook("a") == nullptr);
    CHECK(GameLook("b") != nullptr);
    // Sorted by name, as the file and the game keep them.
    REQUIRE(g_LookEdits.GetState().looks.size() == 2);
    CHECK(g_LookEdits.GetState().looks[0].name == "b");

    g_LookEdits.RemoveLook("Remove a look", "b");
    CHECK(g_LookEdits.GetState().folders == std::vector<ColorFolder>{{"Blades", {}, {"c"}}});
    CHECK(GameLook("b") == nullptr);

    CHECK(g_ItemEdits.Undo());
    CHECK(g_ItemEdits.Undo());
    CHECK(g_ItemEdits.Undo());
    CHECK(g_LookEdits.GetState().folders == std::vector<ColorFolder>{{"Blades", {}, {"a"}}});
    CHECK(GameLook("a") != nullptr);
}

TEST_CASE("The Style Editor opens on a look of the file with its fields, and on one still in code [editor][looks]")
{
    TestImGuiContext context;
    ToolWindowsScope windows;
    g_ItemSelection.Select(MuEditor::Items::ItemSelection::NoItem);
    LookDefinition look = Plain("brightBlade");
    look.layers.push_back(Layer{.kind = Data::Looks::LayerKind::Reflection, .blend = Blend::Glow});
    const LookEditsScope looks({look});

    g_MuStyleEditorUI.ShowRenderStyle("brightBlade");
    CHECK(g_MuEditorCore.GetToolWindows().styleEditor);
    RenderStyleEditor();
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetPickedLook() == "brightBlade");
    CHECK(g_MuStyleEditorUI.GetDraft() == look);

    // A look still in code is picked too, to be seen.
    REQUIRE(Render::Items::Styles::ExistsInCode("characterCard"));
    g_MuStyleEditorUI.ShowRenderStyle("characterCard");
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetPickedLook() == "characterCard");

    // The draft follows an undo while it is not changed.
    LookDefinition plain = Plain("brightBlade");
    g_LookEdits.SetLook("Change a look", "brightBlade", plain);
    g_MuStyleEditorUI.ShowRenderStyle("brightBlade");
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetDraft() == plain);
    CHECK(g_ItemEdits.Undo());
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetDraft() == look);
}

TEST_CASE("Data Checks: a click on a render style problem selects its item and opens the Style Editor [editor]")
{
    TestImGuiContext context;
    ToolWindowsScope windows;
    const std::unique_ptr<MuEditor::DataChecks::DataCheckArea> area = MuEditor::Looks::CreateRenderStyleCheckArea();
    CHECK(std::string(area->GetName()) == I18N::Editor::RenderStyles);
    const ItemDataIssue mesh{ItemDataIssueSeverity::Warning, "Data/Effects/RenderStyles.json",    0, 5,
                             "looks[0].layers[1].meshes",    "mesh 3 is not in this item's model"};

    ImGui::NewFrame();
    area->Open(mesh, false);
    ImGui::Render();
    CHECK(g_ItemSelection.GetItemType() == Data::Items::MakeItemType(0, 5));
    CHECK_FALSE(g_MuEditorCore.GetToolWindows().styleEditor);

    ImGui::NewFrame();
    area->Open(mesh, true);
    ImGui::Render();
    CHECK(g_MuEditorCore.GetToolWindows().styleEditor);
}

#endif // _EDITOR
