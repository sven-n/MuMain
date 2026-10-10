#include <doctest.h>
#include <RmlUi/Core.h>

#include <algorithm>
#include <optional>
#include <string>

namespace
{
const Rml::Vector2i TextureSize{256, 128};
constexpr float ArtworkWidth = 166.f;
constexpr float NameHeight = 90.f;
constexpr float StrifeHeight = 28.f;

class ImageCropFixture
{
    struct Renderer : Rml::RenderInterface
    {
        std::optional<Rml::Vector2f> crop;

        Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int>) override
        {
            if (vertices.size() != 4)
                return 0;
            crop = Rml::Vector2f{};
            for (const auto& vertex : vertices)
            {
                crop->x = std::max(crop->x, vertex.tex_coord.x);
                crop->y = std::max(crop->y, vertex.tex_coord.y);
            }
            return 1;
        }
        void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
        void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
        Rml::TextureHandle LoadTexture(Rml::Vector2i& size, const Rml::String&) override { size = TextureSize; return 1; }
        Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override { return 1; }
        void ReleaseTexture(Rml::TextureHandle) override {}
        void EnableScissorRegion(bool) override {}
        void SetScissorRegion(Rml::Rectanglei) override {}
    };

public:
    ImageCropFixture()
    {
        Rml::SetRenderInterface(&m_Renderer);
        REQUIRE(Rml::Initialise());
        m_Context = Rml::CreateContext("image-crop", {640, 480});
        REQUIRE(m_Context != nullptr);
        auto constructor = m_Context->CreateDataModel("image_crop");
        REQUIRE(constructor);
        constructor.Bind("source", &m_Source);
        m_Model = constructor.GetModelHandle();
    }

    ~ImageCropFixture()
    {
        Rml::Shutdown();
        Rml::SetRenderInterface(nullptr);
    }

    void Load(float height)
    {
        const Rml::String markup =
            "<rml><head></head><body data-model='image_crop'>"
            "<img data-attr-src='source' rect='0 0 " + std::to_string(ArtworkWidth) + " " + std::to_string(height) + "'/></body></rml>";
        auto* document = m_Context->LoadDocumentFromMemory(markup);
        REQUIRE(document != nullptr);
        document->Show();
        Refresh();
    }

    void SetSource(const Rml::String& source)
    {
        m_Source = source;
        m_Model.DirtyVariable("source");
        m_Renderer.crop.reset();
        Refresh();
    }

    void CheckCrop(float height) const
    {
        REQUIRE(m_Renderer.crop.has_value());
        CHECK(m_Renderer.crop->x == doctest::Approx(ArtworkWidth / TextureSize.x));
        CHECK(m_Renderer.crop->y == doctest::Approx(height / TextureSize.y));
    }

private:
    void Refresh() { m_Context->Update(); m_Context->Render(); }

    Renderer m_Renderer;
    Rml::Context* m_Context = nullptr;
    Rml::DataModelHandle m_Model;
    Rml::String m_Source;
};
} // namespace

TEST_CASE("a bound image keeps its crop on first load and after clearing its source [ui][rmlui][image]")
{
    float height = NameHeight;
    SUBCASE("map name") {}
    SUBCASE("strife banner") { height = StrifeHeight; }

    ImageCropFixture fixture;
    fixture.Load(height);
    fixture.SetSource("first.tga");
    fixture.CheckCrop(height);
    fixture.SetSource("");
    fixture.SetSource("second.tga");
    fixture.CheckCrop(height);
}
