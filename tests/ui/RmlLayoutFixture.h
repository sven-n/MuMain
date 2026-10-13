#pragma once

#include <RmlUi/Core.h>
#include "UI/RmlBridge/ThemeFileInterface.h"
#include <filesystem>
#include <string>

namespace UI::Tests
{
std::string ReadFile(const std::filesystem::path& path);
std::string AssetMarkup(const std::string& theme, const std::string& name);
class RmlLayoutFixture
{
    struct NullRenderer : Rml::RenderInterface
    {
        Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override { return 1; }
        void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
        void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
        Rml::TextureHandle LoadTexture(Rml::Vector2i& size, const Rml::String&) override { size = {4096, 4096}; return 1; }
        Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override { return 1; }
        void ReleaseTexture(Rml::TextureHandle) override {}
        void EnableScissorRegion(bool) override {}
        void SetScissorRegion(Rml::Rectanglei) override {}
    };

public:
    RmlLayoutFixture();
    ~RmlLayoutFixture();
    void Configure(Rml::Vector2i viewport, int percent, float contentScale = 1.f);
    Rml::ElementDocument* Load(const std::string& theme, const std::string& name);
    Rml::ElementDocument* LoadMarkup(const std::string& markup);
    void Refresh();
    Rml::Context* context = nullptr;

private:
    NullRenderer m_Renderer;
    UI::RmlBridge::ThemeFileInterface m_Files;
    int m_PreviousPercent;
    float m_PreviousContentScale;
};

} // namespace UI::Tests
