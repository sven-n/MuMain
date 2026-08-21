#include "stdafx.h"


#include "CUIRenderTextSDLTtf.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Text/SDLTtfColorPack.h"

#include <SDL3_ttf/SDL_ttf.h>

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

TextLayout BuildTextLayout(int x, int y, int boxWidth, int boxHeight, int sort, int measuredWidth, int measuredHeight)
{
    TextLayout layout{static_cast<float>(y) * g_fScreenRate_y, static_cast<float>(x) * g_fScreenRate_x,
                      static_cast<float>(boxWidth) * g_fScreenRate_x, static_cast<float>(boxHeight) * g_fScreenRate_y,
                      0.0f};
    if (layout.boxWidth == 0.0f)
        layout.boxWidth = static_cast<float>(measuredWidth);
    if (layout.boxHeight == 0.0f)
        layout.boxHeight = static_cast<float>(measuredHeight);

    const bool textFits = measuredWidth < static_cast<int>(layout.boxWidth);
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

bool ConfigureText(TTF_Text* text, TTF_Font* font, const std::string& utf8, DWORD color)
{
    const Uint8 red = static_cast<Uint8>(color & 0xFFu);
    const Uint8 green = static_cast<Uint8>((color >> 8) & 0xFFu);
    const Uint8 blue = static_cast<Uint8>((color >> 16) & 0xFFu);
    const Uint8 alpha = static_cast<Uint8>((color >> 24) & 0xFFu);
    return TTF_SetTextFont(text, font) && TTF_SetTextString(text, utf8.c_str(), utf8.size()) &&
           TTF_SetTextColor(text, red, green, blue, alpha);
}

void SubmitTextDrawData(mu::IMuRenderer& renderer, const TTF_GPUAtlasDrawSequence* drawData, float drawX, float drawY,
                        DWORD color)
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
            vertices.push_back({sequence->xy[index].x + drawX, drawY + sequence->xy[index].y, sequence->uv[index].x,
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

    m_ttfText = TTF_CreateText(engine, m_activeFont, "", 0);
    return m_ttfText != nullptr;
}

void CUIRenderTextSDLTtf::Release()
{
    if (m_ttfText != nullptr)
    {
        TTF_DestroyText(m_ttfText);
        m_ttfText = nullptr;
    }
    m_activeFont = nullptr;
}

HDC CUIRenderTextSDLTtf::GetFontDC() const
{
    return nullptr;
}
BYTE* CUIRenderTextSDLTtf::GetFontBuffer() const
{
    return nullptr;
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
    }
    else if (font == g_hFontBig)
    {
        m_activeFont = renderer.GetTtfFontBig();
    }
    else if (font == g_hFixFont)
    {
        m_activeFont = renderer.GetTtfFontFixed();
    }
    else
    {
        m_activeFont = renderer.GetTtfFont();
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
    int width = 0;
    int height = 0;
    if (TTF_GetStringSize(font, m_measureUtf8Scratch.c_str(), m_measureUtf8Scratch.size(), &width, &height))
    {
        size.cx = static_cast<LONG>(static_cast<float>(width) / g_fScreenRate_x);
        size.cy = static_cast<LONG>(static_cast<float>(height) / g_fScreenRate_y);
    }
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

    int measuredWidth = 0;
    int measuredHeight = 0;
    if (!TTF_GetStringSize(font, m_utf8Scratch.c_str(), m_utf8Scratch.size(), &measuredWidth, &measuredHeight))
    {
        return;
    }

    const TextLayout layout = BuildTextLayout(x, y, boxWidth, boxHeight, sort, measuredWidth, measuredHeight);

    if (textSize != nullptr)
    {
        textSize->cx = static_cast<LONG>(static_cast<float>(measuredWidth) / g_fScreenRate_x);
        textSize->cy = static_cast<LONG>(static_cast<float>(measuredHeight) / g_fScreenRate_y);
    }

    const int windowHeight = renderer.GetCachedWindowHeight();
    if (windowHeight <= 0)
    {
        return;
    }

    RenderTextBackground(renderer, layout, windowHeight, m_backColor);
    if (!ConfigureText(m_ttfText, font, m_utf8Scratch, m_textColor))
    {
        return;
    }

    SubmitTextDrawData(renderer, TTF_GetGPUTextDrawData(m_ttfText), layout.renderX + layout.alignmentOffset,
                       static_cast<float>(windowHeight) - layout.screenY, m_textColor);
}
