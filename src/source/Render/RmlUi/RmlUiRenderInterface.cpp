#include "stdafx.h"
#include "RmlUiRenderInterface.h"

#include "Render/Core/GlobalUBO.h"
#include "Render/Shaders/PassthroughShader.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Globals/_TextureIndex.h"
#include "Core/Utilities/StringUtils.h"
#include <algorithm>

Rml::CompiledGeometryHandle RmlUiRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
    // Convert RmlUi's native vertex (pos2 + premultiplied-alpha rgba8 + uv2, 20 bytes) into
    // RHI::VertexLayout::PosUvColor's float layout (pos3+uv2+rgba4, 36 bytes) -- the only layout
    // implemented on either backend today. See the RmlUi migration plan's Phase 0.2: adding a
    // 5th VertexLayout instead would need a new HLSL shader variant + RegisterVertexShaderBytecode
    // call on the in-progress, unmerged D3D11 branch, for a cost this conversion avoids entirely.
    //
    // Un-premultiplying alpha here (rather than adding a premultiplied-alpha RHI::BlendMode) is
    // the same trade: RmlUi vertex colour is premultiplied (Vertex.h), but every existing
    // straight-alpha BlendMode in RHI.h (Blend3 -- SRC_ALPHA, ONE_MINUS_SRC_ALPHA, see
    // RenderGeometry below) expects un-premultiplied colour. For a single blend pass with no
    // layer/filter compositing (this MVP implements none of RenderInterface's optional
    // layer/filter functions), un-premultiplying here and blending straight-alpha produces the
    // identical final pixel to blending the original premultiplied colour with (ONE,
    // ONE_MINUS_SRC_ALPHA) -- revisit only if/when layers or filters are implemented, which
    // genuinely need premultiplied compositing.
    std::vector<float> packed(vertices.size() * 9);
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        const Rml::Vertex& v = vertices[i];
        float* d = &packed[i * 9];
        d[0] = v.position.x;
        d[1] = v.position.y;
        d[2] = 0.0f; // pos3 -- RmlUi is strictly 2D, z is unused by PassthroughShader's ortho pass
        d[3] = v.tex_coord.x;
        d[4] = v.tex_coord.y;
        const float a = v.colour.alpha / 255.0f;
        if (a > 0.0f)
        {
            d[5] = (v.colour.red / 255.0f) / a;
            d[6] = (v.colour.green / 255.0f) / a;
            d[7] = (v.colour.blue / 255.0f) / a;
        }
        else
        {
            // Alpha is zero -- this pixel contributes nothing to the blend result regardless of
            // RGB, so any finite value is harmless. Avoid a divide-by-zero producing inf/NaN,
            // which could otherwise pollute interpolation across a triangle edge.
            d[5] = d[6] = d[7] = 0.0f;
        }
        d[8] = a;
    }

    std::vector<uint32_t> idx32(indices.begin(), indices.end()); // RHI index buffers are u32-only

    CompiledGeom* geom = new CompiledGeom{};
    geom->vbo = RHI::CreateVertexBuffer(packed.data(), packed.size() * sizeof(float), RHI::BufferUsage::Static);
    geom->ibo = RHI::CreateIndexBuffer(idx32.data(), idx32.size() * sizeof(uint32_t), RHI::BufferUsage::Static);
    geom->indexCount = static_cast<uint32_t>(idx32.size());

    auto handle = reinterpret_cast<Rml::CompiledGeometryHandle>(geom);
    m_Geometry.emplace(handle, geom);
    return handle;
}

void RmlUiRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
{
    auto it = m_Geometry.find(geometry);
    if (it == m_Geometry.end()) return;
    CompiledGeom* geom = it->second;

    PassthroughShader::Instance().Bind();
    // Blend3, not AlphaTest: AlphaTest (RHI::BlendMode::AlphaTest) enables the shader's alpha-test
    // discard threshold (g_AlphaRef) as a side effect -- built for hard-cutout foliage sprites, it
    // would silently clip RmlUi's anti-aliased text/element edges (low but nonzero alpha) instead
    // of blending them. Blend3 is the same SRC_ALPHA/ONE_MINUS_SRC_ALPHA blend func with alpha-test
    // explicitly disabled (see EnableAlphaBlend3(), ZzzOpenglUtil.cpp) -- confirmed by reading its
    // implementation, not assumed from the RHI.h enum comment alone.
    RHI::SetBlendMode(RHI::BlendMode::Blend3);

    auto texIt = m_Textures.find(texture);
    if (texture != 0 && texIt != m_Textures.end())
    {
        // FileBacked's stored id is a CGlobalBitmap bitmap-index, not a GL texture name -- must be
        // resolved through Bitmaps.GetTexture()->TextureNumber to get the real bindable handle
        // (see this class's header comment on why the two id spaces are never interchangeable).
        // Generated's id is already a real RHI::TextureHandle/GL name from RHI::CreateTexture, so
        // it's used as-is. This distinction was already respected in ReleaseTexture below but was
        // missed here -- latent since RmlUi's only textures until now were Generated (font glyph
        // atlases); the first FileBacked decorator image exposed it.
        GLuint glTextureName = static_cast<GLuint>(texIt->second.id);
        if (texIt->second.kind == TextureKind::FileBacked)
        {
            BITMAP_t* bmp = Bitmaps.GetTexture(texIt->second.id);
            glTextureName = bmp ? static_cast<GLuint>(bmp->TextureNumber) : 0;
        }

        PassthroughShader::Instance().SetTexture(glTextureName, 0);
        PassthroughShader::Instance().SetUseTexture(true);
    }
    else
    {
        PassthroughShader::Instance().SetUseTexture(false);
    }
    PassthroughShader::Instance().SetUseFog(false);

    // GlobalUBO::SetModel(origin, scale) already exists purely for "translate without a new
    // uniform" (per-draw offset without introducing a dedicated RmlUi uniform) -- see the RmlUi
    // migration plan's Phase 0.3.
    const float origin[3] = { translation.x, translation.y, 0.0f };
    GlobalUBO::Instance().SetModel(origin, 1.0f);

    RHI::BindVertexBuffer(geom->vbo, RHI::VertexLayout::PosUvColor);
    RHI::BindIndexBuffer(geom->ibo);
    RHI::DrawIndexed(RHI::Topology::TriangleList, geom->indexCount);
}

void RmlUiRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
    auto it = m_Geometry.find(geometry);
    if (it == m_Geometry.end()) return;
    RHI::DestroyBuffer(it->second->vbo);
    RHI::DestroyBuffer(it->second->ibo);
    delete it->second;
    m_Geometry.erase(it);
}

Rml::TextureHandle RmlUiRenderInterface::RegisterTexture(TextureKind kind, uint32_t id)
{
    const Rml::TextureHandle handle = m_NextTextureHandle++;
    m_Textures.emplace(handle, TextureRecord{ kind, id });
    return handle;
}

Rml::TextureHandle RmlUiRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
    // Routed through CGlobalBitmap (the same ref-counted, OZJ/OZT-aware pipeline every other
    // texture in the game uses -- GlobalBitmap.cpp) rather than loading the file directly, so
    // RmlUi-referenced images (RML/RCSS <img>/background-image) share its cache/eviction/ref-
    // counting instead of bypassing it. See the RmlUi migration plan's Phase 0.3.
    // RML/RCSS conventionally author asset paths with forward slashes; every existing in-tree
    // caller of Bitmaps.LoadImage passes backslash-separated paths (e.g. "Interface\\Foo.jpg"),
    // matching this codebase's own path-handling convention -- normalize rather than assume GL's
    // (and this engine's Linux port's) forward-slash tolerance extends to this legacy loader too.
    std::string normalized = source;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
    const std::wstring wpath = StringUtils::NarrowToWide(normalized); // Rml::String is std::string (RmlUi/Config/Config.h)
    const GLuint bitmapIndex = Bitmaps.LoadImage(wpath, GL_LINEAR, GL_CLAMP_TO_EDGE);
    if (bitmapIndex == BITMAP_UNKNOWN) return 0;

    BITMAP_t* bmp = Bitmaps.GetTexture(bitmapIndex);
    if (!bmp) return 0;

    texture_dimensions = { static_cast<int>(bmp->Width), static_cast<int>(bmp->Height) };
    return RegisterTexture(TextureKind::FileBacked, bitmapIndex);
}

Rml::TextureHandle RmlUiRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
    // Raw RGBA8 straight from RmlUi itself (rasterized text/SVG, no filename to cache by) --
    // goes straight through RHI::CreateTexture, bypassing CGlobalBitmap entirely (there is
    // nothing to ref-count by name).
    RHI::TextureDesc desc{ source_dimensions.x, source_dimensions.y, RHI::TexFilter::Linear, RHI::TexWrap::Clamp };
    RHI::TextureHandle h = RHI::CreateTexture(desc, source.data());
    if (!h.IsValid()) return 0;
    return RegisterTexture(TextureKind::Generated, h.id);
}

void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
{
    auto it = m_Textures.find(texture);
    if (it == m_Textures.end()) return;

    if (it->second.kind == TextureKind::FileBacked)
    {
        Bitmaps.UnloadImage(it->second.id, /*bForce=*/false); // ref-counted; frees + RHI::DestroyTexture at Ref==0
    }
    else
    {
        RHI::DestroyTexture(RHI::TextureHandle{ it->second.id }); // already invalidates BindState's cache
    }
    m_Textures.erase(it);
}

void RmlUiRenderInterface::EnableScissorRegion(bool enable)
{
    m_ScissorEnabled = enable;
    RHI::SetScissorEnabled(enable);
}

void RmlUiRenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
    RHI::SetScissorRect(region.Left(), region.Top(), region.Width(), region.Height());
}
