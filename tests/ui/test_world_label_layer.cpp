#include <doctest.h>

#include "stdafx.h"
#include "UI/Character/WorldLabelLayer.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/ElementText.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override
    {
        return 0;
    }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override
    {
        return 0;
    }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override
    {
        return 0;
    }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

// RmlUi running headless, in a folder holding an empty world_labels.rml.
class Fixture
{
public:
    Fixture()
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("world-label-layer", {640, 480});
        REQUIRE(context != nullptr);
        UI::RmlBridge::SetContextResolver([this](std::string_view) { return context; });
        m_Folder = std::filesystem::temp_directory_path() / "world_label_layer_tests";
        std::filesystem::remove_all(m_Folder);
        std::filesystem::create_directories(m_Folder / "Data/Interface/RmlUi/themes/legacy");
        std::ofstream(m_Folder / "Data/Interface/RmlUi/world_labels.rml") << "<rml><head></head><body></body></rml>";
        m_PreviousDirectory = std::filesystem::current_path();
        m_PreviousTheme = UI::RmlBridge::GetActiveThemeName();
        std::filesystem::current_path(m_Folder);
        UI::RmlBridge::SetActiveThemeName("legacy");
    }

    ~Fixture()
    {
        UI::RmlBridge::SetContextResolver({});
        std::filesystem::current_path(m_PreviousDirectory);
        UI::RmlBridge::SetActiveThemeName(m_PreviousTheme);
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
        std::filesystem::remove_all(m_Folder);
    }

    Rml::Context* context = nullptr;

private:
    NullRenderer m_Renderer;
    std::filesystem::path m_Folder;
    std::filesystem::path m_PreviousDirectory;
    std::string m_PreviousTheme;
};

Render::Renderer::RecordedText Label(std::string_view utf8, float x)
{
    Render::Renderer::RecordedText text;
    text.boxX = x;
    text.boxY = 10.0f;
    text.boxWidth = 40.0f;
    text.boxHeight = 12.0f;
    text.textX = x;
    text.textPixelSize = 12.0f;
    text.lineHeight = 12.0f;
    text.textColor = 0xFFFFFFFFu;
    text.utf8 = utf8;
    return text;
}

void DrawFrame(UI::Character::WorldLabelLayer& layer, std::initializer_list<std::string_view> labels)
{
    layer.BeginFrame();
    float x = 0.0f;
    for (const std::string_view utf8 : labels)
        layer.RecordText(Label(utf8, x += 50.0f));
    layer.EndFrame();
}

Rml::ElementDocument* LabelDocument(Rml::Context* context)
{
    for (int i = 0; i < context->GetNumDocuments(); ++i)
        if (context->GetDocument(i)->GetSourceURL().find("world_labels.rml") != Rml::String::npos)
            return context->GetDocument(i);
    return nullptr;
}

// The shown label box whose text is `utf8`.
Rml::Element* Showing(Rml::Context* context, const std::string& utf8)
{
    context->Update();
    Rml::ElementDocument* document = LabelDocument(context);
    for (int i = 0; document != nullptr && i < document->GetNumChildren(); ++i)
    {
        Rml::Element* box = document->GetChild(i);
        if (box->GetComputedValues().display() != Rml::Style::Display::None &&
            box->GetInnerRML().find(">" + utf8 + "<") != Rml::String::npos)
            return box;
    }
    return nullptr;
}

float ZIndex(Rml::Element* box)
{
    return box->GetComputedValues().z_index().value;
}

// Every shown label box with its text, in document order.
std::vector<std::pair<std::string, Rml::Element*>> ShownLabels(Rml::Context* context)
{
    context->Update();
    std::vector<std::pair<std::string, Rml::Element*>> labels;
    Rml::ElementDocument* document = LabelDocument(context);
    for (int i = 0; document != nullptr && i < document->GetNumChildren(); ++i)
    {
        Rml::Element* box = document->GetChild(i);
        if (box->GetComputedValues().display() == Rml::Style::Display::None || box->GetNumChildren() == 0)
            continue;
        const auto* text = dynamic_cast<Rml::ElementText*>(box->GetChild(0)->GetChild(0));
        if (text != nullptr)
            labels.emplace_back(text->GetText(), box);
    }
    return labels;
}

void DrawFrame(UI::Character::WorldLabelLayer& layer, const std::vector<std::string>& labels)
{
    layer.BeginFrame();
    float x = 0.0f;
    for (const std::string& utf8 : labels)
        layer.RecordText(Label(utf8, x += 1.0f));
    layer.EndFrame();
}
} // namespace

// Slots used to follow the draw order, so one label entering or leaving the view moved the text of
// every label after it to another element, each laid out and shaped again.
TEST_CASE("a label keeps its element when the draw order changes [ui][world-labels]")
{
    Fixture fixture;
    UI::Character::WorldLabelLayer layer;
    REQUIRE(layer.Create());

    DrawFrame(layer, {"Alpha", "Bravo"});
    Rml::Element* alpha = Showing(fixture.context, "Alpha");
    Rml::Element* bravo = Showing(fixture.context, "Bravo");
    REQUIRE(alpha != nullptr);
    REQUIRE(bravo != nullptr);
    CHECK(ZIndex(alpha) < ZIndex(bravo));

    DrawFrame(layer, {"Charlie", "Bravo", "Alpha"});
    CHECK(Showing(fixture.context, "Alpha") == alpha);
    CHECK(Showing(fixture.context, "Bravo") == bravo);
    REQUIRE(Showing(fixture.context, "Charlie") != nullptr);
    // Drawn later stacks higher, as the native labels did.
    CHECK(ZIndex(Showing(fixture.context, "Charlie")) < ZIndex(bravo));
    CHECK(ZIndex(bravo) < ZIndex(alpha));

    DrawFrame(layer, {"Alpha"});
    CHECK(Showing(fixture.context, "Alpha") == alpha);
    CHECK(Showing(fixture.context, "Bravo") == nullptr);
    CHECK(Showing(fixture.context, "Charlie") == nullptr);

    layer.Release();
}

// The last label drawn first (a selected ground item's name) once moved each later match to the end
// of the pool, so every label searched the whole pool again.
TEST_CASE("a large reorder keeps every label and its duplicates on their elements [ui][world-labels]")
{
    Fixture fixture;
    UI::Character::WorldLabelLayer layer;
    REQUIRE(layer.Create());

    std::vector<std::string> labels;
    for (int i = 0; i < 1000; ++i)
        labels.push_back("Item " + std::to_string(i));
    labels.push_back("Spider");
    labels.push_back("Spider");
    DrawFrame(layer, labels);
    const auto before = ShownLabels(fixture.context);
    REQUIRE(before.size() == labels.size());

    std::rotate(labels.rbegin(), labels.rbegin() + 3, labels.rend());
    DrawFrame(layer, labels);
    const auto after = ShownLabels(fixture.context);
    REQUIRE(after.size() == labels.size());
    // The same boxes show the same texts: none was emptied and refilled with another label.
    std::multimap<std::string, Rml::Element*> expected(before.begin(), before.end());
    std::multimap<std::string, Rml::Element*> actual(after.begin(), after.end());
    CHECK(actual == expected);
    for (const auto& [utf8, box] : after)
        if (utf8 == "Item 999")
            CHECK(ZIndex(box) == 0.0f);

    layer.Release();
}
