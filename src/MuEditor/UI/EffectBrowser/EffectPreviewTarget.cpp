#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewTarget.h"

#include "Render/Renderer/MuRenderer.h"

namespace MuEditor::Effects
{
std::uint32_t EffectPreviewTarget::Begin(int width, int height, bool resizeNow)
{
    m_used = true;
    if (m_texture == 0)
    {
        m_width = width;
        m_height = height;
    }
    else if ((width != m_width || height != m_height) && resizeNow)
    {
        m_resize = true;
    }
    const std::uint32_t texture = mu::GetRenderer().BeginOffscreenCapture(
        m_texture, static_cast<std::uint32_t>(m_width), static_cast<std::uint32_t>(m_height));
    if (m_texture == 0)
        m_texture = texture;
    return texture;
}

void EffectPreviewTarget::BeforeFrame()
{
    if (m_texture != 0 && (!m_used || m_resize))
    {
        mu::GetRenderer().ReleaseTexture(m_texture);
        m_texture = 0;
    }
    m_used = false;
    m_resize = false;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
