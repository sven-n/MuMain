#include "Render/Renderer/Overlay2DRecorder.h"

namespace
{
Render::Renderer::IOverlay2DRecorder* g_activeRecorder = nullptr;
} // namespace

Render::Renderer::IOverlay2DRecorder* Render::Renderer::ActiveOverlay2DRecorder()
{
    return g_activeRecorder;
}

Render::Renderer::Overlay2DRecordScope::Overlay2DRecordScope(IOverlay2DRecorder* recorder)
    : m_previous(g_activeRecorder)
{
    g_activeRecorder = recorder;
}

Render::Renderer::Overlay2DRecordScope::~Overlay2DRecordScope()
{
    g_activeRecorder = m_previous;
}
