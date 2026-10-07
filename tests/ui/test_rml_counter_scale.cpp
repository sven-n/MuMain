#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <RmlUi/Core.h>

#include <chrono>
#include <string>

namespace
{
struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override { return 1; }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override { return 0; }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override { return 1; }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

struct ScaleModel
{
    float rootScale = 1.f;
};

class Fixture
{
public:
    Fixture()
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("counter-scale", {1920, 1080});
        REQUIRE(context != nullptr);
        Rml::DataModelConstructor constructor = context->CreateDataModel("scale");
        REQUIRE(constructor);
        constructor.Bind("root_scale", &model.rootScale);
        handle = constructor.GetModelHandle();
    }

    ~Fixture()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
    }

    Rml::ElementDocument* Load(const std::string& markup)
    {
        Rml::ElementDocument* document = context->LoadDocumentFromMemory(markup);
        REQUIRE(document != nullptr);
        document->Show();
        context->Update();
        return document;
    }

    void SetScale(float scale)
    {
        model.rootScale = scale;
        handle.DirtyVariable("root_scale");
        context->Update();
    }

    Rml::Context* context = nullptr;
    ScaleModel model;
    Rml::DataModelHandle handle;

private:
    NullRenderer m_Renderer;
};

// The theme's half: a panel scaled by --root-scale, and layers that cancel it with lengths
// multiplied by it, as base.rcss's .sharp-text does.
constexpr const char* kStyle = R"(
<style>
body { width: 100%; height: 100%; }
#panel { position: absolute; left: 0; top: 0; width: 200px; height: 100px; transform-origin: left top;
         transform: scale(var(--root-scale, 1)); }
.layer { position: absolute; left: 0; top: 0; transform-origin: left top;
         transform: scale(calc(1 / var(--root-scale, 1))); }
#fixed { width: calc(190px * var(--root-scale, 1)); height: calc(10px * var(--root-scale, 1)); }
#whole { width: calc(100% * var(--root-scale, 1)); height: 5px; }
#middle { top: 50%; height: calc(10px * var(--root-scale, 1)); width: 10px; transform-origin: left center;
          transform: translateY(-50%) scale(calc(1 / var(--root-scale, 1))); }
</style>)";

std::string Document(const char* panelAttributes)
{
    return std::string("<rml><head>") + kStyle + R"(</head><body data-model="scale"><div id="panel" )" +
           panelAttributes +
           R"(><div><div id="fixed" class="layer"/></div><div id="whole" class="layer"/><div id="middle" class="layer"/></div></body></rml>)";
}

Rml::Vector2f LayoutSize(Rml::ElementDocument* document, const char* id)
{
    return document->GetElementById(id)->GetBox().GetSize();
}

Rml::Vector2f DrawnSize(Rml::ElementDocument* document, const char* id)
{
    Rml::Vector2f offset, size;
    REQUIRE(UI::RmlBridge::DrawnContentBox(*document->GetElementById(id), offset, size));
    return size;
}

Rml::Vector2f DrawnOffset(Rml::ElementDocument* document, const char* id)
{
    Rml::Vector2f offset, size;
    REQUIRE(UI::RmlBridge::DrawnContentBox(*document->GetElementById(id), offset, size));
    return offset;
}

void CheckScale(Rml::ElementDocument* document, float scale)
{
    // Transforms are resolved when the context renders.
    document->GetContext()->Render();
    CHECK(LayoutSize(document, "fixed").x == doctest::Approx(190.f * scale));
    CHECK(LayoutSize(document, "fixed").y == doctest::Approx(10.f * scale));
    CHECK(LayoutSize(document, "whole").x == doctest::Approx(200.f * scale));
    // The panel scales by `scale` and the layer back by its inverse: drawn at reference size.
    CHECK(DrawnSize(document, "fixed").x == doctest::Approx(190.f * scale).epsilon(0.01));
    CHECK(DrawnSize(document, "whole").x == doctest::Approx(200.f * scale).epsilon(0.01));
    // Centred on the panel's middle at any scale.
    CHECK(DrawnOffset(document, "middle").y + DrawnSize(document, "middle").y / 2 ==
          doctest::Approx(50.f * scale).epsilon(0.01));
}
} // namespace

TEST_CASE("a custom property set at runtime recomputes every calc() under it [ui][counter-scale]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load(Document(""));
    CheckScale(document, 1.f);

    Rml::Element* panel = document->GetElementById("panel");
    panel->SetProperty("--root-scale", "2");
    fixture.context->Update();
    CheckScale(document, 2.f);

    panel->SetProperty("--root-scale", "0.5");
    fixture.context->Update();
    CheckScale(document, 0.5f);

    panel->RemoveProperty("--root-scale");
    fixture.context->Update();
    CheckScale(document, 1.f);
}

TEST_CASE("a data-style binding sets the custom property [ui][counter-scale]")
{
    Fixture fixture;
    Rml::ElementDocument* document = fixture.Load(Document(R"(data-style---root-scale="root_scale")"));
    CheckScale(document, 1.f);
    fixture.SetScale(2.f);
    CheckScale(document, 2.f);
    fixture.SetScale(0.75f);
    CheckScale(document, 0.75f);
}

namespace
{
std::string ManyLayers(bool bound, int count)
{
    std::string markup = std::string("<rml><head>") + kStyle +
                         R"(</head><body data-model="scale"><div id="panel" )" +
                         (bound ? R"(data-style-transform="'scale(' + root_scale + ')'")"
                                : R"(data-style---root-scale="root_scale")") +
                         ">";
    for (int i = 0; i < count; ++i)
    {
        const std::string width = std::to_string(100 + i % 90);
        if (bound)
            markup += R"(<span style="position: absolute; transform-origin: left top;" data-style-width="()" + width +
                      R"( * root_scale) + 'px'" data-style-transform="'scale(' + (1 / root_scale) + ')'">x</span>)";
        else
            markup += "<span class=\"layer w" + width + "\">x</span>";
    }
    markup += "</div></body></rml>";
    if (!bound)
    {
        std::string rules = "<style>";
        for (int w = 100; w < 190; ++w)
            rules += ".w" + std::to_string(w) + " { width: calc(" + std::to_string(w) + "px * var(--root-scale, 1)); }\n";
        rules += "</style>";
        markup.insert(markup.find("</head>"), rules);
    }
    return markup;
}

double MillisecondsPerScaleChange(Fixture& fixture, int changes)
{
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < changes; ++i)
        fixture.SetScale(i % 2 ? 1.5f : 2.f);
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    return elapsed.count() / changes;
}
} // namespace

// Not a pass/fail measure: prints the cost of one UI-scale change both ways. Meaningful only in an
// optimised build.
TEST_CASE("cost of a scale change, bound layers against calc() layers [ui][counter-scale][!benchmark]")
{
    constexpr int kLayers = 500;
    constexpr int kChanges = 40;
    double bound = 0.0;
    double calc = 0.0;
    {
        Fixture fixture;
        fixture.Load(ManyLayers(true, kLayers));
        bound = MillisecondsPerScaleChange(fixture, kChanges);
    }
    {
        Fixture fixture;
        fixture.Load(ManyLayers(false, kLayers));
        calc = MillisecondsPerScaleChange(fixture, kChanges);
    }
    MESSAGE("one scale change over " << kLayers << " layers: bound " << bound << " ms, calc() " << calc << " ms");
    CHECK(calc > 0.0);
}
