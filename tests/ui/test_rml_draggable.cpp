#include <doctest.h>

#include "UI/RmlBridge/RmlDraggable.h"

#include <RmlUi/Core.h>

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

int s_ends = 0;
} // namespace

// The drag listener is attached for three events; unloading its document detaches all three
// (a theme switch does this to the inventory). It used to delete itself on the first detach.
TEST_CASE("A draggable panel's document unloads without touching a freed listener [ui][rml-draggable]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("rml-draggable", {320, 240});
    REQUIRE(context != nullptr);

    for (int round = 0; round < 3; ++round)
    {
        auto* document = context->LoadDocumentFromMemory(
            "<rml><head></head><body><div id='panel' style='position:absolute;left:0;top:0;width:100px;height:100px'>"
            "<div id='title' style='display:block;height:20px'></div></div></body></rml>");
        REQUIRE(document != nullptr);
        UI::RmlBridge::MakeDraggable(document->GetElementById("title"), document->GetElementById("panel"), nullptr,
                                     [] { ++s_ends; });
        document->Show();
        context->Update();
        context->UnloadDocument(document);
        context->Update();
    }

    // The listener still serves its events, and its document still unloads cleanly after use.
    auto* document = context->LoadDocumentFromMemory(
        "<rml><head></head><body><div id='panel' style='position:absolute;left:0;top:0;width:100px;height:100px'>"
        "<div id='title' style='display:block;height:20px'></div></div></body></rml>");
    REQUIRE(document != nullptr);
    Rml::Element* title = document->GetElementById("title");
    UI::RmlBridge::MakeDraggable(title, document->GetElementById("panel"), nullptr, [] { ++s_ends; });
    Rml::Dictionary parameters;
    title->DispatchEvent(Rml::EventId::Dragend, parameters);
    CHECK(s_ends == 1);
    context->UnloadDocument(document);
    context->Update();

    Rml::RemoveContext("rml-draggable");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}
