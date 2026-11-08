#include "stdafx.h"

#include "doctest.h"

// The Style Editor (design L18 to L20, L31, L42, L43): its edits of the render style file, which reach the game at once
// and are undo steps, the window on a look of the file or one still in code, and Data Checks opening it.
#ifdef _EDITOR

#include "LookEditsScope.h"
#include "TestImGuiContext.h"
#include "ToolWindowsScope.h"

#include "Core/MuEditorCore.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookEffects.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"
#include "I18N/All.h"
#include "Render/Items/ItemRenderStyles.h"
#include "UI/Common/MuEditorToolWindows.h"
#include "UI/Common/MuEditorWindowIds.h"
#include "UI/ItemTools/ItemEdits.h"
#include "UI/ItemTools/ItemSelection.h"
#include "UI/StyleEditor/LookEdits.h"
#include "UI/StyleEditor/LookFields.h"
#include "UI/StyleEditor/LookParts.h"
#include "UI/StyleEditor/MuStyleEditorUI.h"
#include "UI/StyleEditor/RenderStyleCheckArea.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
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

TEST_CASE("A change of the looks that leaves the file as it is records no step and leaves the file alone [editor]"
          "[looks]")
{
    const LookEditsScope looks({Plain("brightBlade"), Plain("darkBlade")}, {{"Blades", {}, {"brightBlade"}}});
    const int version = g_ItemDataHandler.GetRenderStylesVersion();

    // The same folders, and the same looks in another order: nothing to undo, and no change for Save or the drafts.
    g_LookEdits.SetFolders("Same folders", g_LookEdits.GetState().folders);
    Data::Looks::RenderStylesFile reversed = g_LookEdits.GetState();
    std::ranges::reverse(reversed.looks);
    g_ItemEdits.SetRenderStyles("Same looks", reversed);
    CHECK(g_ItemDataHandler.GetRenderStylesVersion() == version);
    CHECK_FALSE(g_ItemEdits.GetHistory().CanUndo());

    // Other folders are a change.
    g_LookEdits.SetFolders("No folders", {});
    CHECK(g_ItemDataHandler.GetRenderStylesVersion() != version);
    CHECK(g_LookEdits.GetState().folders.empty());
    CHECK(g_ItemEdits.GetHistory().GetUndoLabel() == "No folders");
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

TEST_CASE("The Style Editor shows a look as parts with effects, and the gallery adds one on the picked part (design "
          "L35, L40) [editor][looks]")
{
    TestImGuiContext context;
    ToolWindowsScope windows;
    g_ItemSelection.Select(MuEditor::Items::ItemSelection::NoItem);
    LookDefinition look = Plain("brightBlade");
    Layer glow;
    glow.meshes.meshes = {1};
    glow.glowMesh = 1;
    glow.glowStrength = 1.0f;
    look.layers.push_back(glow);
    const LookEditsScope looks({look});

    g_MuStyleEditorUI.ShowRenderStyle("brightBlade");
    RenderStyleEditor();
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetPickedPart() == MuEditor::Looks::EveryPart);

    // Without an item the parts are the meshes the look names.
    g_MuStyleEditorUI.PickPart(1);
    g_MuStyleEditorUI.AddEffect(Data::Looks::EffectKind::Pulse);
    REQUIRE(g_MuStyleEditorUI.GetDraft().layers.size() == 3);
    const Layer& pulse = g_MuStyleEditorUI.GetDraft().layers[2];
    CHECK(pulse.meshes.meshes == std::vector<int>{1});
    CHECK(Data::Looks::DescribeEffect(pulse).kind == Data::Looks::EffectKind::Pulse);
    CHECK(g_MuStyleEditorUI.GetPickedLine() == 2);
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetPickedPart() == 1);
    CHECK(g_MuStyleEditorUI.GetDraft().layers.size() == 3);
    // The game draws the look as applied until Apply.
    CHECK(GameLook("brightBlade")->layers.size() == 2);

    // On every part, an effect draws every mesh.
    g_MuStyleEditorUI.PickPart(MuEditor::Looks::EveryPart);
    g_MuStyleEditorUI.AddEffect(Data::Looks::EffectKind::Shine);
    CHECK(g_MuStyleEditorUI.GetDraft().layers.back().meshes.IsAll());
    RenderStyleEditor();
    // A flowing texture moves on one part only.
    g_MuStyleEditorUI.AddEffect(Data::Looks::EffectKind::Flowing);
    CHECK(g_MuStyleEditorUI.GetDraft().layers.size() == 4);
    g_MuStyleEditorUI.PickPart(1);
    g_MuStyleEditorUI.AddEffect(Data::Looks::EffectKind::Flowing);
    REQUIRE(g_MuStyleEditorUI.GetDraft().layers.size() == 5);
    CHECK(g_MuStyleEditorUI.GetDraft().layers.back().glowMesh == Data::Looks::GlowMesh(1));
    RenderStyleEditor();
}

TEST_CASE("The Style Editor's mesh fields offer the item's parts by name, and meshes beyond the model apart (design "
          "L42) [editor][looks]")
{
    using MuEditor::Looks::Fields::MeshOption;
    using MuEditor::Looks::Fields::MeshOptions;
    using MuEditor::Looks::Fields::WithMesh;
    const std::vector<MuEditor::Looks::LookPart> parts{{0, "Crystal", "wand44_R.jpg"}, {1, "", "wand44_R.jpg"}};
    CHECK(MeshOptions(parts, {5, 1, 5, 3}) == std::vector<MeshOption>{{0, "0 \xc2\xb7 Crystal"},
                                                                      {1, "1 \xc2\xb7 wand44_R.jpg"},
                                                                      {3, "3 (not in the model)"},
                                                                      {5, "5 (not in the model)"}});
    CHECK(MeshOptions({}, {2}) == std::vector<MeshOption>{{2, "2 (not in the model)"}});

    // A tick adds the mesh at the end and the others keep their order, the order the layer draws them in.
    CHECK(WithMesh({2, 0}, 1, true) == std::vector<int>{2, 0, 1});
    CHECK(WithMesh({2, 0, 1}, 0, false) == std::vector<int>{2, 1});
    CHECK(WithMesh({2}, 2, true) == std::vector<int>{2});
    // The last mesh taken out leaves none, which the checkboxes refuse: a layer without meshes draws on every one.
    CHECK(WithMesh({2}, 2, false).empty());
}

TEST_CASE("Every field of a look draws, as drag fields, part checkboxes and dropdowns and a color picker, without "
          "changing it (design L42) [editor][looks]")
{
    using Data::Looks::ItemValues;
    using Data::Looks::LayerLight;
    using Data::Looks::Shape;
    using Data::Looks::ShapeKind;
    TestImGuiContext context;
    Layer shaped;
    shaped.kind = Data::Looks::LayerKind::Reflection;
    shaped.meshes.meshes = {2, 0};
    shaped.light = LayerLight{
        .color = {0.4f, Shape{.kind = ShapeKind::Wave, .from = 0.27, .to = 0.33, .seconds = 4.1887902}, 0.8f}};
    shaped.opacity = Shape{.kind = ShapeKind::Bounce, .from = 0.1, .to = 1.1, .seconds = 3.14159265};
    shaped.glowMesh = 1;
    shaped.glowStrength = Shape{.kind = ShapeKind::Wave, .from = -0.2, .to = 1.0, .seconds = 6.283185};
    shaped.scrollU = Shape{.kind = ShapeKind::Scroll, .perSecond = 0.8};
    shaped.scrollV = Shape{.kind = ShapeKind::Frames, .seconds = 0.1, .step = 0.25, .count = 4};
    shaped.scrollingMesh = 1;
    Layer numbers;
    numbers.meshes = {{1, 7}, true, false};
    numbers.texture = "Item/Deye_R.jpg";
    numbers.light = LayerLight{.tint = true, .color = {0.4f, 0.4f, 0.8f}};
    numbers.opacity = Shape{.kind = ShapeKind::Random, .from = 0.5, .to = 1.0, .step = 0.1};
    numbers.glowStrength = Shape{.kind = ShapeKind::Ramp, .from = 0.0, .to = 1.0, .seconds = 2.0};
    numbers.scrollU = 0.25f;
    const ItemValues values{.opacity = 0.5f,
                            .glowMesh = 0,
                            .glowStrength = Shape{.kind = ShapeKind::Wave, .from = 0.4, .to = 1.0, .seconds = 6.283185},
                            .scrollU = 0.0f,
                            .scrollV = 0.0f,
                            .hiddenMesh = 1,
                            .scrollingMesh = 1};
    const std::vector<MuEditor::Looks::LookPart> parts{{0, "Crystal", ""}, {1, "", "wand44_R.jpg"}};
    for (const std::vector<MuEditor::Looks::LookPart>& offered : {parts, std::vector<MuEditor::Looks::LookPart>{}})
    {
        for (const Layer& original : {shaped, numbers, Layer{}})
        {
            CAPTURE(offered.size());
            CAPTURE(MuEditor::Looks::Fields::DescribeLayer(original));
            Layer asLayer = original;
            Layer asEffect = original;
            ItemValues left = values;
            for (int frame = 0; frame < 3; ++frame)
            {
                ImGui::NewFrame();
                ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowSize(ImVec2(900.0f, 980.0f));
                ImGui::Begin("Fields");
                ImGui::PushID("Layer");
                CHECK_FALSE(MuEditor::Looks::Fields::Layer(asLayer, offered));
                ImGui::PopID();
                ImGui::PushID("Effect");
                CHECK_FALSE(MuEditor::Looks::Fields::Effect(asEffect, Data::Looks::DescribeEffect(asEffect), offered));
                ImGui::PopID();
                ImGui::PushID("Values");
                CHECK_FALSE(MuEditor::Looks::Fields::ItemValues(left, offered));
                ImGui::PopID();
                ImGui::End();
                ImGui::Render();
                CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
            }
            // Drawn, a field changes nothing: the numbers keep every digit they were written with.
            CHECK(asLayer == original);
            CHECK(asEffect == original);
            CHECK(left == values);
        }
    }
}

TEST_CASE("A field of a look says what it does while the mouse rests on it (design L42) [editor][looks]")
{
    TestImGuiContext context;
    ImGuiIO& io = ImGui::GetIO();
    Layer layer;
    const auto frame = [&layer]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(900.0f, 600.0f));
        ImGui::Begin("Fields", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
        MuEditor::Looks::Fields::Layer(layer);
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
        const ImGuiWindow* tip = ImGui::FindWindowByName("##Tooltip_00");
        return tip != nullptr && tip->Active;
    };
    CHECK_FALSE(frame());
    // The mouse rests on the first field, the layer's kind.
    io.AddMousePosEvent(ImGui::GetStyle().WindowPadding.x + 20.0f,
                        ImGui::GetStyle().WindowPadding.y + ImGui::GetFrameHeight() * 0.5f);
    bool shown = false;
    for (int count = 0; count < 60 && !shown; ++count)
    {
        shown = frame();
    }
    CHECK(shown);
}

TEST_CASE("A shape's curve shows ten seconds of the preview's clock with fixed axes and marks the time (design L43) "
          "[editor][looks]")
{
    using Data::Looks::Shape;
    using Data::Looks::ShapeKind;
    using MuEditor::Looks::Fields::CurveValueAt;
    using MuEditor::Looks::Fields::CurveWindowOf;
    const Shape wave{.kind = ShapeKind::Wave, .from = -0.2, .to = 1.0, .seconds = 6.283185};
    const MuEditor::Looks::Fields::CurveWindow window = CurveWindowOf(wave, 2.5);
    CHECK(window.start == 0.0);
    CHECK(window.now == doctest::Approx(2.5));
    CHECK(window.low == doctest::Approx(-0.2).epsilon(0.001));
    CHECK(window.high == 1.0);

    // A lower To keeps the axes, so the curve gets flatter instead of filling its box again, and half the seconds
    // put twice the rounds into the same ten seconds.
    const Shape changed{.kind = ShapeKind::Wave, .from = -0.2, .to = 0.5, .seconds = 3.14159};
    const MuEditor::Looks::Fields::CurveWindow flatter = CurveWindowOf(changed, 2.5);
    CHECK(flatter.low == doctest::Approx(-0.2).epsilon(0.001));
    CHECK(flatter.high == 1.0);
    CHECK(CurveValueAt(changed, flatter, 3.14159 / 4.0) == doctest::Approx(0.5));
    CHECK(CurveValueAt(changed, flatter, 3.14159 * 5.0 / 4.0) == doctest::Approx(0.5));

    // Later on the clock the window moves on by whole ten seconds, and draws the values the look has then.
    const MuEditor::Looks::Fields::CurveWindow later = CurveWindowOf(wave, 1234.5);
    CHECK(later.start == 1230.0);
    CHECK(later.now == doctest::Approx(4.5));
    CHECK(CurveValueAt(wave, later, 4.5) == doctest::Approx(MuEditor::Looks::Fields::ShapeValueAt(wave, 1234.5)));

    // A scroll starts where the window starts and grows over its ten seconds.
    const Shape scroll{.kind = ShapeKind::Scroll, .perSecond = 0.8};
    const MuEditor::Looks::Fields::CurveWindow flowing = CurveWindowOf(scroll, 1234.5);
    CHECK(CurveValueAt(scroll, flowing, 0.0) == 0.0);
    CHECK(flowing.low == 0.0);
    CHECK(flowing.high == doctest::Approx(8.0));
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

TEST_CASE("Data Checks checks the looks as the Style Editor has them, before Save Items writes them [editor][looks]")
{
    TestImGuiContext context;
    ToolWindowsScope windows;
    LookDefinition broken = Plain("brokenLook");
    broken.layers[0].texture = "Item/no_such_look_texture.jpg";
    const LookEditsScope looks({Plain("brightBlade"), broken});
    const std::unique_ptr<MuEditor::DataChecks::DataCheckArea> area = MuEditor::Looks::CreateRenderStyleCheckArea();
    std::vector<ItemDataIssue> issues;
    area->Check(issues);

    // The looks sorted by name, as Save Items writes them: the broken one second.
    const auto missing = std::ranges::find_if(issues, [](const ItemDataIssue& issue)
                                              { return issue.field == "looks[1].layers[0].texture"; });
    REQUIRE(missing != issues.end());
    CHECK(missing->severity == ItemDataIssueSeverity::Error);
    ImGui::NewFrame();
    area->Open(*missing, true);
    ImGui::Render();
    CHECK(g_MuStyleEditorUI.GetPickedLook() == "brokenLook");
}

TEST_CASE("A look's changes that are not applied stay while another look is picked [editor][looks]")
{
    TestImGuiContext context;
    ToolWindowsScope windows;
    g_ItemSelection.Select(MuEditor::Items::ItemSelection::NoItem);
    const LookEditsScope looks({Plain("brightBlade"), Plain("darkBlade")});

    g_MuStyleEditorUI.ShowRenderStyle("brightBlade");
    RenderStyleEditor();
    RenderStyleEditor();
    g_MuStyleEditorUI.PickPart(MuEditor::Looks::EveryPart);
    g_MuStyleEditorUI.AddEffect(Data::Looks::EffectKind::Glow);
    REQUIRE(g_MuStyleEditorUI.GetDraft().layers.size() == 2);

    g_MuStyleEditorUI.ShowRenderStyle("darkBlade");
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetPickedLook() == "darkBlade");
    CHECK(g_MuStyleEditorUI.GetDraft() == Plain("darkBlade"));
    // The tree marks the look with changes not applied.
    CHECK(g_MuStyleEditorUI.HasUnappliedChanges("brightBlade"));
    CHECK_FALSE(g_MuStyleEditorUI.HasUnappliedChanges("darkBlade"));

    g_MuStyleEditorUI.ShowRenderStyle("brightBlade");
    RenderStyleEditor();
    CHECK(g_MuStyleEditorUI.GetDraft().layers.size() == 2);
    // The game draws the look as applied until Apply.
    CHECK(GameLook("brightBlade")->layers.size() == 1);
}

TEST_CASE("A layer's texture changes when its field is left after typing, not with each key [editor][looks]")
{
    TestImGuiContext context;
    Layer layer;
    bool changed = false;
    const auto frame = [&layer, &changed]
    {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(900.0f, 900.0f));
        ImGui::Begin("Fields");
        changed = MuEditor::Looks::Fields::Layer(layer);
        ImGui::End();
        ImGui::Render();
        CHECK(ImGui::GetCurrentContext()->ErrorCountCurrentFrame == 0);
    };
    frame();
    const ImGuiWindow* window = ImGui::FindWindowByName("Fields");
    REQUIRE(window != nullptr);
    ImGui::ActivateItemByID(ImGui::GetIDWithSeed(I18N::Editor::Texture, nullptr, window->ID));
    frame();
    ImGui::GetIO().AddInputCharactersUTF8("Effect/Chrome01.jpg");
    frame();
    frame();
    CHECK(layer.texture.empty());
    CHECK_FALSE(changed);
    ImGui::ClearActiveID();
    frame();
    CHECK(changed);
    CHECK(layer.texture == "Effect/Chrome01.jpg");
}

namespace
{
// A client folder whose look file holds `text`; the working directory while it lives.
class LookFileFolder
{
public:
    explicit LookFileFolder(const std::string& text)
        // CTest runs the test cases as parallel processes; each needs its own folder.
        : m_directory(std::filesystem::temp_directory_path() /
                      ("mu_test_style_editor_" + std::to_string(std::random_device{}()))),
          m_previousDirectory(std::filesystem::current_path())
    {
        // Set up the log file before switching directories, so none is written there.
        mu::log::Init();
        std::filesystem::create_directories(m_directory / "Data" / "Effects");
        std::ofstream(m_directory / "Data" / "Effects" / "RenderStyles.json", std::ios::binary) << text;
        std::filesystem::current_path(m_directory);
    }

    ~LookFileFolder()
    {
        std::error_code ignored;
        std::filesystem::current_path(m_previousDirectory, ignored);
        std::filesystem::remove_all(m_directory, ignored);
    }

    LookFileFolder(const LookFileFolder&) = delete;
    LookFileFolder& operator=(const LookFileFolder&) = delete;

private:
    std::filesystem::path m_directory;
    std::filesystem::path m_previousDirectory;
};
} // namespace

TEST_CASE("The editor's first read of the look file gives the game the file's looks [editor][looks]")
{
    const LookEditsScope looks({Plain("brightBlade")});
    // The file changed since the game read it: it holds another look now.
    const LookFileFolder folder(Data::Looks::WriteRenderStylesJson({Plain("fromDisk")}));
    g_ItemDataHandler.ForgetRenderStylesForTest();

    REQUIRE(g_LookEdits.Find("fromDisk") != nullptr);
    CHECK(GameLook("fromDisk") != nullptr);
    CHECK(GameLook("brightBlade") == nullptr);
}

#endif // _EDITOR
