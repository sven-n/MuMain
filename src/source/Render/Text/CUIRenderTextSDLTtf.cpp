#include "stdafx.h"


#include "CUIRenderTextSDLTtf.h"

#include "Core/Utilities/FrameProfiler.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Text/SDLTtfColorPack.h"
#include "Render/Text/SdlTtfGpuTextProperties.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <SDL3_ttf/SDL_ttf.h>

#include <cmath>
#include <vector>

namespace
{
constexpr std::size_t kMaxRetainedVertexCapacity = 4096;

struct TextLayout
{
    float screenY;
    float renderX;
    float boxWidth;
    float boxHeight;
    float alignmentOffset;
};

struct ScaledTextMetrics
{
    UI::Scaling::Transform transform;
    float scale;
    float width;
    float height;
};

ScaledTextMetrics BuildScaledTextMetrics(UI::Scaling::FontRole role, int measuredWidth, int measuredHeight,
                                         int boxWidth, int boxHeight)
{
    const auto transform = UI::Scaling::GetActiveTransform();
    const float physicalBoxWidth = boxWidth > 0 ? UI::Scaling::SizeX(transform, static_cast<float>(boxWidth)) : 0.0f;
    const float physicalBoxHeight = boxHeight > 0 ? UI::Scaling::SizeY(transform, static_cast<float>(boxHeight)) : 0.0f;
    const float scale = UI::Scaling::FontScaleForBounds(role, transform, static_cast<float>(measuredWidth),
                                                       static_cast<float>(measuredHeight), physicalBoxWidth,
                                                       physicalBoxHeight);
    return {transform, scale, static_cast<float>(measuredWidth) * scale,
            static_cast<float>(measuredHeight) * scale};
}

TextLayout BuildTextLayout(int x, int y, int boxWidth, int boxHeight, int sort, float measuredWidth,
                           float measuredHeight)
{
    TextLayout layout{ConvertPositionY(static_cast<float>(y)), ConvertPositionX(static_cast<float>(x)),
                      ConvertX(static_cast<float>(boxWidth)), ConvertY(static_cast<float>(boxHeight)), 0.0f};
    if (layout.boxWidth == 0.0f)
        layout.boxWidth = static_cast<float>(measuredWidth);
    if (layout.boxHeight == 0.0f)
        layout.boxHeight = static_cast<float>(measuredHeight);

    const bool textFits = measuredWidth < layout.boxWidth;
    if ((sort == RT3_SORT_CENTER || sort == RT3_WRITE_CENTER) && textFits)
        layout.alignmentOffset = (layout.boxWidth - static_cast<float>(measuredWidth)) / 2.0f;
    else if ((sort == RT3_SORT_RIGHT || sort == RT3_WRITE_RIGHT_TO_LEFT) && textFits)
        layout.alignmentOffset = layout.boxWidth - static_cast<float>(measuredWidth);

    if (sort == RT3_WRITE_RIGHT_TO_LEFT)
        layout.renderX -= layout.boxWidth;
    else if (sort == RT3_WRITE_CENTER)
        layout.renderX -= layout.boxWidth / 2.0f;
    return layout;
}

void RenderTextBackground(mu::IMuRenderer& renderer, const TextLayout& layout, int windowHeight, DWORD color)
{
    if ((color >> 24) == 0)
        return;

    const float y = static_cast<float>(windowHeight) - layout.screenY;
    const mu::Vertex2D vertices[4] = {
        {layout.renderX, y, 0.0f, 0.0f, color},
        {layout.renderX, y - layout.boxHeight, 0.0f, 0.0f, color},
        {layout.renderX + layout.boxWidth, y - layout.boxHeight, 0.0f, 0.0f, color},
        {layout.renderX + layout.boxWidth, y, 0.0f, 0.0f, color},
    };
    renderer.RenderQuad2D(vertices, 0u);
}

void ConsumeGlyphUploads(TTF_Text* text)
{
    const SDL_PropertiesID properties = TTF_GetTextProperties(text);
    const Sint64 uploadedGlyphs =
        SDL_GetNumberProperty(properties, Render::Text::kUploadedGlyphCountProperty, 0);
    if (uploadedGlyphs <= 0)
        return;

    FrameProfiler::Count(FrameProfiler::Counter::GlyphUploads, static_cast<std::uint32_t>(uploadedGlyphs));
    SDL_SetNumberProperty(properties, Render::Text::kUploadedGlyphCountProperty, 0);
}

void SubmitTextDrawData(mu::IMuRenderer& renderer, const TTF_GPUAtlasDrawSequence* drawData, float drawX, float drawY,
                        float glyphScale, DWORD color)
{
    static thread_local std::vector<mu::Vertex2D> vertices;
    if (vertices.capacity() > kMaxRetainedVertexCapacity)
        std::vector<mu::Vertex2D>().swap(vertices);

    for (const TTF_GPUAtlasDrawSequence* sequence = drawData; sequence != nullptr; sequence = sequence->next)
    {
        if (sequence->num_indices <= 0 || sequence->atlas_texture == nullptr)
            continue;

        vertices.clear();
        vertices.reserve(static_cast<std::size_t>(sequence->num_indices));
        for (int i = 0; i < sequence->num_indices; ++i)
        {
            const int index = sequence->indices[i];
            vertices.push_back({sequence->xy[index].x * glyphScale + drawX,
                                drawY + sequence->xy[index].y * glyphScale, sequence->uv[index].x,
                                sequence->uv[index].y, color});
        }
        renderer.SubmitTextTriangles(vertices, sequence->atlas_texture, nullptr);
    }
}
} // namespace

CUIRenderTextSDLTtf::CUIRenderTextSDLTtf() = default;

CUIRenderTextSDLTtf::~CUIRenderTextSDLTtf()
{
    Release();
}

bool CUIRenderTextSDLTtf::Create(HDC)
{
    auto& renderer = mu::GetRenderer();
    TTF_TextEngine* engine = renderer.GetTextEngine();
    if (engine == nullptr)
    {
        mu::log::Get("render")->error("CUIRenderTextSDLTtf::Create -- no TTF text engine available");
        return false;
    }

    m_activeFont = renderer.GetTtfFont();
    if (m_activeFont == nullptr)
    {
        mu::log::Get("render")->error("CUIRenderTextSDLTtf::Create -- no TTF font available");
        return false;
    }

    m_textCache.Reset(engine);
    return true;
}

void CUIRenderTextSDLTtf::Release()
{
    m_textCache.Reset(nullptr);
    m_activeFont = nullptr;
    m_activeRole = UI::Scaling::FontRole::Normal;
}

DWORD CUIRenderTextSDLTtf::GetTextColor() const
{
    return m_textColor;
}
DWORD CUIRenderTextSDLTtf::GetBgColor() const
{
    return m_backColor;
}

void CUIRenderTextSDLTtf::SetTextColor(BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
    m_textColor = mu::sdlttf::PackColorDWORD(red, green, blue, alpha);
}

void CUIRenderTextSDLTtf::SetTextColor(DWORD color)
{
    m_textColor = color;
}

void CUIRenderTextSDLTtf::SetBgColor(BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
    m_backColor = mu::sdlttf::PackColorDWORD(red, green, blue, alpha);
}

void CUIRenderTextSDLTtf::SetBgColor(DWORD color)
{
    m_backColor = color;
}

void CUIRenderTextSDLTtf::SetFont(HFONT font)
{
    auto& renderer = mu::GetRenderer();
    if (font == g_hFontBold)
    {
        m_activeFont = renderer.GetTtfFontBold();
        m_activeRole = UI::Scaling::FontRole::Bold;
    }
    else if (font == g_hFontBig)
    {
        m_activeFont = renderer.GetTtfFontBig();
        m_activeRole = UI::Scaling::FontRole::Big;
    }
    else if (font == g_hFixFont)
    {
        m_activeFont = renderer.GetTtfFontFixed();
        m_activeRole = UI::Scaling::FontRole::Fixed;
    }
    else
    {
        m_activeFont = renderer.GetTtfFont();
        m_activeRole = UI::Scaling::FontRole::Normal;
    }
}

SIZE CUIRenderTextSDLTtf::MeasureText(const wchar_t* text, int length) const
{
    SIZE size = {0, 0};
    if (text == nullptr || length <= 0)
    {
        return size;
    }

    auto& renderer = mu::GetRenderer();
    TTF_Font* font = m_activeFont != nullptr ? m_activeFont : renderer.GetTtfFont();
    if (font == nullptr)
    {
        return size;
    }

    m_measureWideScratch.assign(text, length);
    mu_wchar_to_utf8(m_measureWideScratch.c_str(), m_measureUtf8Scratch);
    auto prepared = m_textCache.Prepare(font, m_measureUtf8Scratch);
    if (prepared.text == nullptr)
        return size;

    const ScaledTextMetrics metrics = BuildScaledTextMetrics(m_activeRole, prepared.width, prepared.height, 0, 0);
    size.cx = static_cast<LONG>(std::lround(metrics.width / metrics.transform.scaleX));
    size.cy = static_cast<LONG>(std::lround(metrics.height / metrics.transform.scaleY));
    return size;
}

void CUIRenderTextSDLTtf::RenderText(int x, int y, const wchar_t* text, int boxWidth, int boxHeight, int sort,
                                     OUT SIZE* textSize)
{
    if (text == nullptr || text[0] == L'\0')
    {
        return;
    }

    auto& renderer = mu::GetRenderer();
    TTF_TextEngine* engine = renderer.GetTextEngine();
    TTF_Font* font = m_activeFont != nullptr ? m_activeFont : renderer.GetTtfFont();
    if (engine == nullptr || font == nullptr)
    {
        return;
    }

    mu_wchar_to_utf8(text, m_utf8Scratch);
    if (m_utf8Scratch.empty())
    {
        return;
    }

    auto prepared = m_textCache.Prepare(font, m_utf8Scratch);
    if (prepared.text == nullptr)
        return;

    const ScaledTextMetrics metrics =
        BuildScaledTextMetrics(m_activeRole, prepared.width, prepared.height, boxWidth, boxHeight);

    const TextLayout layout = BuildTextLayout(x, y, boxWidth, boxHeight, sort, metrics.width, metrics.height);

    if (textSize != nullptr)
    {
        textSize->cx = static_cast<LONG>(std::lround(metrics.width / metrics.transform.scaleX));
        textSize->cy = static_cast<LONG>(std::lround(metrics.height / metrics.transform.scaleY));
    }

    const int windowHeight = renderer.GetCachedWindowHeight();
    if (windowHeight <= 0)
    {
        return;
    }

    RenderTextBackground(renderer, layout, windowHeight, m_backColor);
    const TTF_GPUAtlasDrawSequence* drawData = TTF_GetGPUTextDrawData(prepared.text);
    ConsumeGlyphUploads(prepared.text);
    SubmitTextDrawData(renderer, drawData, layout.renderX + layout.alignmentOffset,
                       static_cast<float>(windowHeight) - layout.screenY, metrics.scale, m_textColor);
}
