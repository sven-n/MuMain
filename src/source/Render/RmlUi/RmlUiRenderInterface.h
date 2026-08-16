#pragma once

#include "stdafx.h"
#include <RmlUi/Core/RenderInterface.h>
#include "Render/RHI/RHI.h"
#include <unordered_map>

// Rml::RenderInterface implementation targeting this engine's RHI:: abstraction directly --
// deliberately bypasses IR:: (ImmediateRenderer)'s per-vertex Begin/Vertex/End API, which is
// built for CPU-accumulated immediate-mode drawing, not RmlUi's bulk compile-once/render-many
// contract (a CompiledGeometryHandle is reused across frames until an element's layout/style
// actually changes, so the conversion cost in CompileGeometry below is not a per-frame cost).
//
// Only the "required functions for basic rendering" section of Rml::RenderInterface is
// implemented -- the optional advanced-rendering functions (SetTransform, layers, filters,
// shaders) are left at their base-class no-op defaults, meaning CSS transforms/backdrop-filter/
// box-shadow-blur are not yet supported. See the RmlUi migration plan's Phase 0.3 for the
// deliberate MVP scope cut.
class RmlUiRenderInterface : public Rml::RenderInterface
{
public:
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

private:
    struct CompiledGeom
    {
        RHI::BufferHandle vbo;
        RHI::BufferHandle ibo;
        uint32_t indexCount = 0;
    };

    // RmlUi's TextureHandle is an opaque uintptr_t we hand out ourselves from an incrementing
    // counter -- never alias it onto a raw CGlobalBitmap index or an RHI::TextureHandle id
    // directly. Both of those come from independent, not-guaranteed-disjoint counters
    // (CGlobalBitmap::GenerateTextureIndex() vs RHI_GL.cpp's own GL texture-id counter), so
    // misrouting a ReleaseTexture() call between the file-backed and generated paths below would
    // be a real correctness bug (freeing/using the wrong texture), not just a style concern.
    enum class TextureKind { FileBacked, Generated };
    struct TextureRecord { TextureKind kind; uint32_t id; };

    Rml::TextureHandle RegisterTexture(TextureKind kind, uint32_t id);

    std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeom*> m_Geometry;
    std::unordered_map<Rml::TextureHandle, TextureRecord> m_Textures;
    uintptr_t m_NextTextureHandle = 1;

    bool m_ScissorEnabled = false;
};
