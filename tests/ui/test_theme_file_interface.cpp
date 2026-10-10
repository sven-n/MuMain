#include <doctest.h>

#include "stdafx.h"
#include "UI/RmlBridge/ThemeFileInterface.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/ComputedValues.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace
{
struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override { return 0; }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override { return 0; }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override { return 0; }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

struct Fixture
{
    Fixture()
    {
        root = std::filesystem::temp_directory_path() /
            ("rml_theme_file_interface_tests_" +
             std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Rml::SetFileInterface(&files);
        Rml::SetRenderInterface(&renderer);
        REQUIRE(Rml::Initialise());
        context = Rml::CreateContext("theme-file-interface", {320, 240});
        REQUIRE(context != nullptr);
    }

    ~Fixture()
    {
        Rml::Shutdown();
        Rml::SetFileInterface(nullptr);
        Rml::SetRenderInterface(nullptr);
        std::filesystem::remove_all(root);
    }

    std::filesystem::path Theme(const char* name) const
    {
        return root / "Data/Interface/RmlUi/themes" / name;
    }

    void Write(const std::filesystem::path& path, const std::string& contents) const
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream(path, std::ios::binary) << contents;
    }

    Rml::Colourb LoadColor(const char* theme, const char* linkAttributes) const
    {
        const std::string markup = std::string("<rml><head><link ") + linkAttributes +
            "/></head><body><div id='colored'>text</div></body></rml>";
        const std::string source = (Theme(theme) / "view.rml").generic_string();
        Rml::ElementDocument* document = context->LoadDocumentFromMemory(markup, source);
        REQUIRE(document != nullptr);
        document->Show();
        context->Update();
        Rml::Element* colored = document->GetElementById("colored");
        REQUIRE(colored != nullptr);
        const Rml::Colourb result = colored->GetComputedValues().color();
        context->UnloadDocument(document);
        return result;
    }

    std::filesystem::path root;
    UI::RmlBridge::ThemeFileInterface files;
    NullRenderer renderer;
    Rml::Context* context = nullptr;
};
} // namespace

TEST_CASE("themed external RCSS resolves tokens regardless of link attribute order [ui][rml-theme]")
{
    Fixture fixture;
    fixture.Write(fixture.Theme("alpha") / "tokens.ini", "[Tokens]\naccent=#123456\n");
    fixture.Write(fixture.Theme("alpha") / "paint.rcss", "#colored { color: token(accent); }");
    fixture.Write(fixture.Theme("beta") / "tokens.ini", "[Tokens]\naccent=#abcdef\n");
    fixture.Write(fixture.Theme("beta") / "paint.rcss", "#colored { color: token(accent); }");

    CHECK(fixture.LoadColor("alpha", "href='paint.rcss' type='text/rcss'") == Rml::Colourb(0x12, 0x34, 0x56));
    CHECK(fixture.LoadColor("beta", "type='text/rcss' href='paint.rcss'") == Rml::Colourb(0xab, 0xcd, 0xef));
    CHECK(fixture.LoadColor("alpha", "type='text/rcss' href='paint.rcss'") == Rml::Colourb(0x12, 0x34, 0x56));
}

TEST_CASE("theme file interface passes through ordinary files and plain RCSS [ui][rml-theme]")
{
    Fixture fixture;
    const auto plain = fixture.Theme("alpha") / "plain.rcss";
    const auto other = fixture.root / "fonts" / "sample.bin";
    fixture.Write(plain, "#colored { color: #987654; }");
    fixture.Write(other, "abcdef");

    CHECK(fixture.LoadColor("alpha", "href='plain.rcss' type='text/rcss'") == Rml::Colourb(0x98, 0x76, 0x54));
    Rml::FileHandle file = fixture.files.Open(other.generic_string());
    REQUIRE(file != 0);
    CHECK(fixture.files.Length(file) == 6);
    CHECK(fixture.files.Seek(file, 2, SEEK_SET));
    char bytes[3] = {};
    CHECK(fixture.files.Read(bytes, 2, file) == 2);
    CHECK(std::string(bytes, 2) == "cd");
    fixture.files.Close(file);
    CHECK(fixture.files.Open((fixture.root / "missing.rcss").generic_string()) == 0);
}
