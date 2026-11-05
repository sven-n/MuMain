#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/LookData/LookJson.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

using namespace Data::Looks;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

namespace
{
const std::string Source = "RenderStyles.json";

struct ReadResult
{
    std::vector<LookDefinition> looks;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(std::string_view text)
{
    ReadResult result;
    ReadRenderStylesJson(text, Source, result.looks, result.issues);
    return result;
}

std::string File(const std::string& looks)
{
    return R"({ "formatVersion": 1, "kind": "renderStyle", "looks": )" + looks + " }";
}

// The issue at `field` with `severity`, or nullptr.
const ItemDataIssue* Find(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity,
                          const std::string& field)
{
    const auto it = std::find_if(issues.begin(), issues.end(), [&](const ItemDataIssue& issue)
                                 { return issue.severity == severity && issue.field == field; });
    return it == issues.end() ? nullptr : &*it;
}

std::string Describe(const std::vector<ItemDataIssue>& issues)
{
    std::string text;
    for (const ItemDataIssue& issue : issues)
    {
        text += issue.ToString() + "\n";
    }
    return text;
}
} // namespace

TEST_CASE("The shipped render style file reads without problems and is in the written format [looks][files]")
{
    const auto text = Data::Items::Json::ReadTextFile(std::filesystem::path(MU_TEST_DATA_DIR) / "Effects" /
                                                      std::string(GetRenderStylesFileName()));
    REQUIRE(text);
    const ReadResult result = Read(*text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    CHECK(WriteRenderStylesJson(result.looks) == *text);
}

TEST_CASE("A look's layers read every field, and write back as they were read [looks][files]")
{
    const std::string text = WriteRenderStylesJson({
        {"chromeMesh0",
         {Layer{}, Layer{.kind = LayerKind::Reflection, .blend = Blend::Glow, .meshes = {{0}}, .glowMesh = 0}},
         {}},
        {"albatrossBow",
         {Layer{.meshes = {{1}, true}},
          Layer{.motion = Motion::SlowDrift,
                .meshes = {{2}},
                .texture = "Effect/Chrome01.jpg",
                .opacity = 0.5f,
                .glowMesh = NoGlowMesh{},
                .glowStrength = 1.f,
                .scrollU = 0.25f,
                .scrollV = 0.75f},
          Layer{.kind = LayerKind::Reflection,
                .motion = Motion::StrongSwirl,
                .blend = Blend::AsDrawing,
                .opacity = FromDrawing{}}},
         {.glowMesh = 1, .glowStrength = 1.f, .hiddenMesh = 1}},
    });
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 2);
    // Sorted by name.
    CHECK(result.looks[0].name == "albatrossBow");
    CHECK(result.looks[1].name == "chromeMesh0");
    const Layer& texture = result.looks[0].layers[1];
    CHECK(texture.kind == LayerKind::Texture);
    CHECK(texture.texture == "Effect/Chrome01.jpg");
    CHECK(texture.opacity == Value{0.5f});
    CHECK(texture.glowMesh == GlowMesh{NoGlowMesh{}});
    CHECK(texture.scrollV == Value{0.75f});
    CHECK(result.looks[0].layers[0].meshes == MeshSet{{1}, true});
    CHECK(result.looks[0].layers[2].motion == Motion::StrongSwirl);
    CHECK(result.looks[0].layers[2].opacity == Value{FromDrawing{}});
    CHECK(result.looks[0].itemValuesAfter.hiddenMesh == 1);
    CHECK(result.looks[1].layers[1] ==
          Layer{.kind = LayerKind::Reflection, .blend = Blend::Glow, .meshes = {{0}}, .glowMesh = 0});
    CHECK(WriteRenderStylesJson(result.looks) == text);
}

TEST_CASE("A layer leaves out what is the item's, and a texture layer writes no motion [looks][files]")
{
    const std::string text = WriteRenderStylesJson({{"textured", {Layer{}}, {}}});
    CHECK(text.find(R"({"kind": "texture"})") != std::string::npos);
    CHECK(text.find("motion") == std::string::npos);
    CHECK(text.find("itemValuesAfter") == std::string::npos);
}

TEST_CASE("Problems that make a render style file unusable are fatal [looks][files]")
{
    struct Case
    {
        const char* text;
        const char* field;
    };
    const Case cases[] = {
        {R"([{"name": "a", "layers": [{"kind": "texture"}]}, {"name": "a", "layers": []}])", "looks[1].name"},
        {R"([{"name": "a b", "layers": []}])", "looks[0].name"},
        {R"([{"layers": []}])", "looks[0].name"},
        {R"([{"name": "a"}])", "looks[0].layers"},
        {R"([{"name": "a", "layers": [{}]}])", "looks[0].layers[0].kind"},
        {R"([{"name": "a", "layers": [{"kind": "paint"}]}])", "looks[0].layers[0].kind"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "motion": "swirl"}]}])", "looks[0].layers[0].motion"},
        {R"([{"name": "a", "layers": [{"kind": "reflection", "motion": "spin"}]}])", "looks[0].layers[0].motion"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "blend": "add"}]}])", "looks[0].layers[0].blend"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "meshes": []}]}])", "looks[0].layers[0].meshes"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "meshes": [50]}]}])", "looks[0].layers[0].meshes"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "meshes": {"only": [1]}}]}])", "looks[0].layers[0].meshes"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowMesh": -1}]}])", "looks[0].layers[0].glowMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "opacity": "item"}]}])", "looks[0].layers[0].opacity"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowStrength": "drawing"}]}])",
         "looks[0].layers[0].glowStrength"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "texture": ""}]}])", "looks[0].layers[0].texture"},
        {R"([{"name": "a", "layers": [], "itemValuesAfter": {"hiddenMesh": "all"}}])",
         "looks[0].itemValuesAfter.hiddenMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowMesh": "every"}]}])", "looks[0].layers[0].glowMesh"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "opacity": {"from": 0}}]}])",
         "looks[0].layers[0].opacity.shape"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "opacity": {"shape": "zigzag"}}]}])",
         "looks[0].layers[0].opacity.shape"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowStrength": {"shape": "wave", "from": 0, "to": 1}}]}])",
         "looks[0].layers[0].glowStrength.seconds"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "glowStrength": {"shape": "wave", "from": 0, "to": 1,
            "seconds": 0}}]}])",
         "looks[0].layers[0].glowStrength.seconds"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollV": {"shape": "frames", "count": 0, "step": 0.25,
            "secondsEach": 0.08}}]}])",
         "looks[0].layers[0].scrollV.count"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollU": {"shape": "random", "from": 0, "to": 1,
            "step": 0}}]}])",
         "looks[0].layers[0].scrollU.step"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollU": {"shape": "random", "from": 1, "to": 0,
            "step": 0.1}}]}])",
         "looks[0].layers[0].scrollU.to"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": [1, 1]}]}])", "looks[0].layers[0].light"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": {"color": [1, 1, 1]}}]}])",
         "looks[0].layers[0].light"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": {"tint": [1, 1]}}]}])",
         "looks[0].layers[0].light.tint"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "light": [1, "white", 1]}]}])",
         "looks[0].layers[0].light[1]"},
        {R"([{"name": "a", "layers": [{"kind": "texture", "scrollingMesh": 50}]}])",
         "looks[0].layers[0].scrollingMesh"},
        {R"([{"name": "a", "layers": [], "itemValuesAfter": {"glowStrength": "drawing"}}])",
         "looks[0].itemValuesAfter.glowStrength"},
        {R"([{"name": "a", "layers": [], "itemValuesBefore": [{"hiddenMesh": 1}]}])", "looks[0].itemValuesBefore"},
    };
    for (const Case& c : cases)
    {
        CAPTURE(c.text);
        const ReadResult result = Read(File(c.text));
        INFO(Describe(result.issues));
        CHECK(Find(result.issues, ItemDataIssueSeverity::Fatal, c.field) != nullptr);
        CHECK(Data::Items::HasFatal(result.issues));
    }

    CHECK(Find(Read(R"({ "formatVersion": 1, "kind": "itemEffect", "looks": [] })").issues,
               ItemDataIssueSeverity::Fatal, "kind") != nullptr);
    CHECK(Find(Read(R"({ "formatVersion": 1, "kind": "renderStyle" })").issues, ItemDataIssueSeverity::Fatal,
               "looks") != nullptr);
    CHECK(Data::Items::HasFatal(Read(R"({ "formatVersion": 2, "kind": "renderStyle", "looks": [] })").issues));
}

TEST_CASE("Unknown fields, a look without layers and empty item values are warnings [looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "a", "layers": [], "colour": 1, "itemValuesAfter": {}},
        {"name": "b", "layers": [{"kind": "texture", "shine": true}]}])"));
    INFO(Describe(result.issues));
    CHECK_FALSE(Data::Items::HasFatal(result.issues));
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].layers") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].colour") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].itemValuesAfter") != nullptr);
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[1].layers[0].shine") != nullptr);
    CHECK(result.looks.size() == 2);
}

TEST_CASE("The Style Editor's folders are the editor's: the file reads without a warning about them [looks][files]")
{
    const ReadResult result =
        Read(R"({ "formatVersion": 1, "kind": "renderStyle", "lookFolders": [{"name": "Tickets"}], "looks": [] })");
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
}

TEST_CASE("The Style Editor's folders write each on a line before the looks, and read back [looks][files]")
{
    const std::vector<LookDefinition> looks{{"b", {Layer{}}, {}}, {"a", {Layer{}}, {}}};
    const std::vector<Data::Colors::ColorFolder> folders{
        {"Tickets", {{"Old", {}, {"a"}}}, {"b", "gone"}},
    };
    const std::string text = WriteRenderStylesJson(looks, folders);
    CHECK(text.find("\"lookFolders\"") < text.find("\"looks\": [\n"));
    CHECK(text.find(R"({"name": "Tickets", "folders": [{"name": "Old", "looks": ["a"]}], "looks": ["b"]})") !=
          std::string::npos);
    RenderStylesFile file;
    std::vector<ItemDataIssue> issues;
    ReadRenderStylesFile(text, Source, file, issues);
    INFO(Describe(issues));
    CHECK(issues.empty());
    // The look that is gone is left out.
    CHECK(file.folders == std::vector<Data::Colors::ColorFolder>{{"Tickets", {{"Old", {}, {"a"}}}, {"b"}}});
    REQUIRE(file.looks.size() == 2);
    CHECK(file.looks[0].name == "a");
}

TEST_CASE("A folder naming a look the file doesn't have is a warning that leaves it out [looks][files]")
{
    RenderStylesFile file;
    std::vector<ItemDataIssue> issues;
    ReadRenderStylesFile(File(R"([{"name": "a", "layers": [{"kind": "texture"}]}], "lookFolders": [
        {"name": "Tickets", "looks": ["a", "missing"], "colors": ["a"]}])"),
                         Source, file, issues);
    INFO(Describe(issues));
    CHECK(Find(issues, ItemDataIssueSeverity::Warning, "lookFolders[0].looks[1]") != nullptr);
    CHECK(Find(issues, ItemDataIssueSeverity::Warning, "lookFolders[0].colors") != nullptr);
    CHECK_FALSE(Data::Items::HasFatal(issues));
    CHECK(file.folders == std::vector<Data::Colors::ColorFolder>{{"Tickets", {}, {"a"}}});
}

TEST_CASE("A layer of every mesh, the item's hidden one too, reads and writes as allEvenHidden [looks][files]")
{
    const std::string text = WriteRenderStylesJson({{"texturedAllMeshes", {Layer{.meshes = {{}, false, true}}}, {}}});
    CHECK(text.find(R"("meshes": "allEvenHidden")") != std::string::npos);
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    REQUIRE(result.looks.size() == 1);
    CHECK(result.looks[0].layers[0].meshes.evenHidden);
    CHECK(result.looks[0].layers[0].meshes.IsAll());
}

TEST_CASE("Shapes, a layer's light, its scrolling mesh and the glow of every mesh write back as they were read "
          "[looks][files]")
{
    const Shape wave{.kind = ShapeKind::Wave, .from = -0.2, .to = 1.2, .seconds = 7.853982};
    const Shape bounce{.kind = ShapeKind::Bounce, .from = 0.1, .to = 0.4, .seconds = 3.141593};
    const Shape scroll{.kind = ShapeKind::Scroll, .perSecond = -0.1};
    const Shape ramp{.kind = ShapeKind::Ramp, .from = 0.0, .to = -1.0, .seconds = 2.0};
    const Shape frames{.kind = ShapeKind::Frames, .seconds = 0.08, .step = 0.25, .count = 4};
    const Shape random{.kind = ShapeKind::Random, .from = 0.0, .to = 0.9, .step = 0.1};
    const std::vector<LookDefinition> looks{
        {"timed",
         {Layer{.meshes = {{1}},
                .opacity = wave,
                .glowMesh = AllGlowMeshes{},
                .glowStrength = bounce,
                .scrollU = scroll,
                .scrollV = ramp,
                .light = LayerLight{.color = {bounce, bounce, 1.f}},
                .scrollingMesh = 1},
          Layer{.scrollU = random, .scrollV = frames, .light = LayerLight{.tint = true, .color = {0.8f, 0.f, 0.8f}}}},
         {.glowStrength = wave, .scrollU = scroll, .scrollingMesh = -1}},
    };
    const std::string text = WriteRenderStylesJson(looks);
    INFO(text);
    // Each layer stays on one line, its shapes in it, with the numbers of the file.
    CHECK(text.find(R"("opacity": {"shape": "wave", "from": -0.2, "to": 1.2, "seconds": 7.853982})") !=
          std::string::npos);
    CHECK(text.find(R"("glowMesh": "all")") != std::string::npos);
    CHECK(text.find(R"("light": {"tint": [0.8, 0, 0.8]})") != std::string::npos);
    CHECK(text.find(R"("scrollV": {"shape": "frames", "count": 4, "step": 0.25, "secondsEach": 0.08})") !=
          std::string::npos);
    CHECK(text.find(R"("scrollingMesh": "none")") != std::string::npos);
    const ReadResult result = Read(text);
    INFO(Describe(result.issues));
    CHECK(result.issues.empty());
    CHECK(result.looks == looks);
    CHECK(WriteRenderStylesJson(result.looks) == text);
}

TEST_CASE("A field of another shape is a warning [looks][files]")
{
    const ReadResult result = Read(File(R"([{"name": "a", "layers": [{"kind": "texture",
        "glowStrength": {"shape": "scroll", "perSecond": 0.5, "seconds": 2}}]}])"));
    INFO(Describe(result.issues));
    CHECK_FALSE(Data::Items::HasFatal(result.issues));
    CHECK(Find(result.issues, ItemDataIssueSeverity::Warning, "looks[0].layers[0].glowStrength.seconds") != nullptr);
}
