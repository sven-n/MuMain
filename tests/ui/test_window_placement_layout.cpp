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

TEST_CASE("Workspace shell reserves regions and collapses hidden slots [ui][window-placement]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("workspace-shell", {1920, 1080});
    REQUIRE(context != nullptr);
    for (const char* theme : {"legacy", "modern"})
    {
        CAPTURE(theme);
        auto* document = context->LoadDocumentFromMemory(WorkspaceFor(theme));
        REQUIRE(document != nullptr);
        auto* header = document->GetElementById("shell_header");
        auto* left = document->GetElementById("shell_left");
        auto* content = document->GetElementById("safe_area");
        header->SetProperty("height", "60px");
        left->SetProperty("width", "180px");
        Open(document, "main_hud", 1.f);
        Slot(document, "main_hud")->SetProperty("width", "1280px");
        Slot(document, "main_hud")->SetProperty("height", "102px");
        document->UpdateDocument();
        CHECK(content->GetAbsoluteOffset().x == doctest::Approx(180.f));
        CHECK(content->GetAbsoluteOffset().y == doctest::Approx(60.f));
        CHECK(content->GetBox().GetSize().x == doctest::Approx(1740.f));
        CHECK(content->GetBox().GetSize().y == doctest::Approx(918.f));
        CHECK(Slot(document, "main_hud")->GetAbsoluteOffset().x == doctest::Approx(320.f));
        CHECK(Slot(document, "main_hud")->GetAbsoluteOffset().y == doctest::Approx(978.f));
        // The NPC panel stage centres on the whole screen, not on the area the shell leaves.
        Rml::ElementList stages;
        document->QuerySelectorAll(stages, ".panel-stage");
        REQUIRE(stages.size() == 1);
        CHECK(stages[0]->GetAbsoluteOffset().y == doctest::Approx(300.f));

        // Overlay keeps its placement but gives its space back to content.
        header->SetAttribute("data-participation", "overlay");
        document->UpdateDocument();
        CHECK(content->GetAbsoluteOffset().y == doctest::Approx(0.f));
        CHECK(content->GetBox().GetSize().y == doctest::Approx(978.f));
        Slot(document, "main_hud")->SetClass("open", false);
        document->UpdateDocument();
        CHECK(content->GetBox().GetSize().y == doctest::Approx(1080.f));

        context->UnloadDocument(document);
        context->Update();
    }
    Rml::RemoveContext("workspace-shell");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}

// The header holds the MU Helper bar (left) and the top bar (right). Modern caps its docks at the
// content area, so the service draws them smaller instead of over the top bar; legacy keeps the
// original's docks, which reach above it.
TEST_CASE("The header's corners and the docks the content area caps [ui][window-placement]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("workspace-header", {1024, 768});
    REQUIRE(context != nullptr);
    constexpr float Scale = 1.6f;
    context->SetDensityIndependentPixelRatio(Scale);
    for (const char* theme : {"legacy", "modern"})
    {
        CAPTURE(theme);
        auto* document = context->LoadDocumentFromMemory(WorkspaceFor(theme));
        REQUIRE(document != nullptr);
        const auto openSized = [&](const char* window, float width, float height)
        {
            Rml::Element* slot = Slot(document, window);
            REQUIRE(slot != nullptr);
            slot->SetClass("open", true);
            slot->SetProperty(Rml::PropertyId::Width, Rml::Property(width * Scale, Rml::Unit::PX));
            slot->SetProperty(Rml::PropertyId::Height, Rml::Property(height * Scale, Rml::Unit::PX));
        };
        openSized("mu_helper_bar", 207.f, 25.f);
        openSized("top_bar", 256.f, 24.f);
        openSized("main_hud", 640.f, 51.f);
        Rml::ElementList docks;
        document->QuerySelectorAll(docks, ".dock-right");
        REQUIRE(docks.size() == 1);
        docks[0]->SetProperty(Rml::PropertyId::Height, Rml::Property(432.f * Scale, Rml::Unit::PX));
        document->UpdateDocument();

        CHECK(Slot(document, "mu_helper_bar")->GetAbsoluteOffset().x == doctest::Approx(0.f));
        CHECK(Slot(document, "mu_helper_bar")->GetAbsoluteOffset().y == doctest::Approx(0.f));
        CHECK(Slot(document, "top_bar")->GetAbsoluteOffset().x == doctest::Approx(1024.f - 256.f * Scale));
        CHECK(Slot(document, "top_bar")->GetAbsoluteOffset().y == doctest::Approx(0.f));
        const float content = 768.f - 25.f * Scale - 51.f * Scale;
        CHECK(document->GetElementById("safe_area")->GetBox().GetSize().y == doctest::Approx(content));
        const float dock = docks[0]->GetBox().GetSize().y;
        CHECK(dock == doctest::Approx(std::string(theme) == "modern" ? content : 432.f * Scale));

        context->UnloadDocument(document);
        context->Update();
    }
    Rml::RemoveContext("workspace-header");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}

// A theme can remove the original dock height and let one fill-capable window occupy a fraction
// of the safe area. The service reads this resolved border box and hands it to the window.
TEST_CASE("A theme-sized slot fills the safe area's height [ui][window-placement]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("window-fill", {1920, 1080});
    REQUIRE(context != nullptr);

    for (const char* theme : {"legacy", "modern"})
    for (const std::string window : {"character", "pet"})
    {
        CAPTURE(theme);
        CAPTURE(window);
        std::string rml = WorkspaceFor(theme);
        const std::string region = "class=\"region dock-right\" data-ref-height=\"432\"";
        const auto regionAt = rml.find(region);
        REQUIRE(regionAt != std::string::npos);
        rml.replace(regionAt, region.size(), "class=\"region dock-right\"");
        const std::string slot = "class=\"slot\" data-window=\"" + window + "\"";
        const auto slotAt = rml.find(slot);
        REQUIRE(slotAt != std::string::npos);
        rml.replace(slotAt, slot.size(),
                    "class=\"slot fill-window\" data-window=\"" + window + "\" data-fit=\"fill\"");
        const auto headEnd = rml.find("</head>");
        REQUIRE(headEnd != std::string::npos);
        rml.insert(headEnd, "<style>.dock-right { top: 0; } "
                            ".fill-window { width: 35%; height: 100%; }</style>");

        auto* document = context->LoadDocumentFromMemory(rml);
        REQUIRE(document != nullptr);
        Open(document, "main_hud", 1.f);
        Slot(document, "main_hud")->SetProperty(Rml::PropertyId::Height, Rml::Property(102.f, Rml::Unit::PX));
        Rml::Element* fillSlot = Slot(document, window);
        REQUIRE(fillSlot != nullptr);
        fillSlot->SetClass("open", true);
        document->UpdateDocument();

        // A theme's header (modern keeps one) takes its height off the content area's top.
        const float header = document->GetElementById("shell_header")->GetBox().GetSize(Rml::BoxArea::Border).y;
        const Rml::Vector2f size = fillSlot->GetBox().GetSize(Rml::BoxArea::Border);
        const Rml::Vector2f offset = fillSlot->GetAbsoluteOffset(Rml::BoxArea::Border);
        CHECK(size.x == doctest::Approx(672.f));
        CHECK(size.y == doctest::Approx(978.f - header));
        CHECK(offset.x == doctest::Approx(1248.f));
        CHECK(offset.y == doctest::Approx(header));

        // The service gives a fill slot the window's content size as its minimum.
        fillSlot->SetProperty(Rml::PropertyId::MinWidth, Rml::Property(800.f, Rml::Unit::PX));
        document->UpdateDocument();
        CHECK(fillSlot->GetBox().GetSize(Rml::BoxArea::Border).x == doctest::Approx(800.f));
        CHECK(fillSlot->GetAbsoluteOffset(Rml::BoxArea::Border).x == doctest::Approx(1120.f));

        context->UnloadDocument(document);
        context->Update();
    }

    Rml::RemoveContext("window-fill");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}

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

        Open(document, "main_hud", 1.f);
        Slot(document, "main_hud")->SetProperty(Rml::PropertyId::Height, Rml::Property(Height - HudTop, Rml::Unit::PX));
        Rml::ElementList regions;
        document->QuerySelectorAll(regions, ".region");
        for (Rml::Element* region : regions)
        {
            if (region->HasAttribute("data-ref-height"))
                region->SetProperty(Rml::PropertyId::Height,
                                    Rml::Property(region->GetAttribute<float>("data-ref-height", 0.f) * Scale,
                                                  Rml::Unit::PX));
        }

        // Opened out of document order: placement follows the slots' order, not the opening order.
        Open(document, "storage", Scale);
        Open(document, "inventory", Scale);
        Open(document, "character", Scale);
        document->UpdateDocument();

        // A theme that caps its docks at the content area starts them below its header; the
        // service then draws the windows smaller to fit (not part of this RCSS-only check).
        const float header = document->GetElementById("shell_header")->GetBox().GetSize(Rml::BoxArea::Border).y;
        const float dockTop = std::string(theme) == "modern" ? header : HudTop - 432.f * Scale;
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
