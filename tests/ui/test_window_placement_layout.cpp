#include <doctest.h>

#include <RmlUi/Core.h>

#include <fstream>
#include <sstream>
#include <string>

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

std::string ReadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

// workspace.rml with the theme's stylesheet inlined, as the theme loader does.
std::string WorkspaceFor(const std::string& theme)
{
    const std::string root = MU_RMLUI_DIR;
    std::string rml = ReadFile(root + "/workspace.rml");
    const std::string link = "<link type=\"text/rcss\" href=\"workspace.rcss\"/>";
    const auto at = rml.find(link);
    REQUIRE(at != std::string::npos);
    rml.replace(at, link.size(), "<style>" + ReadFile(root + "/themes/" + theme + "/workspace.rcss") + "</style>");
    return rml;
}

Rml::Element* Slot(Rml::ElementDocument* document, const std::string& window)
{
    Rml::ElementList slots;
    document->QuerySelectorAll(slots, ".slot");
    for (Rml::Element* slot : slots)
        if (slot->GetAttribute<Rml::String>("data-window", "") == window)
            return slot;
    return nullptr;
}

void Open(Rml::ElementDocument* document, const std::string& window, float scale)
{
    Rml::Element* slot = Slot(document, window);
    REQUIRE(slot != nullptr);
    slot->SetClass("open", true);
    slot->SetProperty(Rml::PropertyId::Width, Rml::Property(190.f * scale, Rml::Unit::PX));
    slot->SetProperty(Rml::PropertyId::Height, Rml::Property(429.f * scale, Rml::Unit::PX));
}
} // namespace

// The game's inputs at 1920x1080: dock scale 2.25, HUD scale 2.0 (51 units tall). Open windows
// must land on the original's columns, 190 units each from the right, on the dock's top edge,
// even though the workspace document is never shown.
TEST_CASE("Both themes' workspaces place docked windows on the original columns [ui][window-placement]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    constexpr float Width = 1920.f;
    constexpr float Height = 1080.f;
    constexpr float Scale = 2.25f;
    constexpr float HudTop = Height - 51.f * 2.f;
    auto* context = Rml::CreateContext("window-placement", {1920, 1080});
    REQUIRE(context != nullptr);

    for (const char* theme : {"legacy", "modern"})
    {
        CAPTURE(theme);
        auto* document = context->LoadDocumentFromMemory(WorkspaceFor(theme));
        REQUIRE(document != nullptr);

        document->GetElementById("safe_area")
            ->SetProperty(Rml::PropertyId::Bottom, Rml::Property(Height - HudTop, Rml::Unit::PX));
        Rml::ElementList regions;
        document->QuerySelectorAll(regions, ".region");
        for (Rml::Element* region : regions)
            region->SetProperty(Rml::PropertyId::Height,
                                Rml::Property(region->GetAttribute<float>("data-ref-height", 0.f) * Scale,
                                              Rml::Unit::PX));

        // Opened out of document order: placement follows the slots' order, not the opening order.
        Open(document, "storage", Scale);
        Open(document, "inventory", Scale);
        Open(document, "character", Scale);
        document->UpdateDocument();

        const float dockTop = HudTop - 432.f * Scale;
        const auto column = [&](const char* window, int n)
        {
            CAPTURE(window);
            const Rml::Vector2f offset = Slot(document, window)->GetAbsoluteOffset(Rml::BoxArea::Border);
            CHECK(offset.x == doctest::Approx(Width - 190.f * Scale * static_cast<float>(n)));
            CHECK(offset.y == doctest::Approx(dockTop));
        };
        column("character", 1);
        column("inventory", 2);
        column("storage", 3);

        // A closed slot takes no space.
        Slot(document, "character")->SetClass("open", false);
        document->UpdateDocument();
        column("inventory", 1);
        column("storage", 2);

        context->UnloadDocument(document);
        context->Update();
    }

    Rml::RemoveContext("window-placement");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}
