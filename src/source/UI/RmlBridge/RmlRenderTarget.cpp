#include "stdafx.h"
#include "UI/RmlBridge/RmlRenderTarget.h"

#include "Render/Renderer/MuRenderer.h"
#include "Render/RmlUi/RmlUiRuntime.h"

#include <RmlUi/Core/Core.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace UI::RmlBridge
{
namespace
{
constexpr char kSourcePrefix[] = "rtt:";

// A retired texture is released only once no document can still be sampling it. A new Source() is
// picked up by the view's next sync and applied by RmlUi's next update, which may fall a frame
// after that, so the old one stays alive for a few frames past the switch.
constexpr std::uint64_t kRetireLag = 3;

struct Published
{
    std::uint32_t id = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct Retired
{
    Rml::String source;
    std::uint32_t id = 0;
    std::uint64_t releaseFrame = 0;
};

std::uint64_t s_frame = 0;
// Never reused, unlike texture ids: a recycled id matching a stale RmlUi cache entry would hand
// RmlUi a released texture without it ever asking LoadTexture() again.
std::uint64_t s_nextSerial = 1;

std::vector<RenderTarget*>& Targets()
{
    static std::vector<RenderTarget*> targets;
    return targets;
}

std::unordered_map<Rml::String, Published>& PublishedSources()
{
    static std::unordered_map<Rml::String, Published> sources;
    return sources;
}

std::vector<Retired>& RetiredTextures()
{
    static std::vector<Retired> retired;
    return retired;
}

void Retire(std::uint32_t id, const Rml::String& source)
{
    if (id != 0)
        RetiredTextures().push_back({source, id, s_frame + kRetireLag});
}

void ReleaseRetired()
{
    const bool rmlReady = RmlUiRuntime::Instance().IsCreated();
    auto& retired = RetiredTextures();
    for (auto it = retired.begin(); it != retired.end();)
    {
        if (it->releaseFrame > s_frame)
        {
            ++it;
            continue;
        }
        if (!it->source.empty())
        {
            PublishedSources().erase(it->source);
            // Before the texture goes, so RmlUi's cache never holds a handle whose address the
            // renderer could hand out again.
            if (rmlReady)
                Rml::ReleaseTexture(it->source);
        }
        mu::GetRenderer().ReleaseRenderTarget(it->id);
        it = retired.erase(it);
    }
}
} // namespace

RenderTarget::RenderTarget(Drawer drawer) : m_drawer(std::move(drawer))
{
    Targets().push_back(this);
}

RenderTarget::~RenderTarget()
{
    auto& targets = Targets();
    targets.erase(std::remove(targets.begin(), targets.end(), this), targets.end());
    // Never shown, so nothing can be sampling it.
    if (m_pending.id != 0)
        mu::GetRenderer().ReleaseRenderTarget(m_pending.id);
    Retire(m_current.id, m_source);
}

void RenderTarget::Resize(std::uint32_t width, std::uint32_t height)
{
    if (width == 0 || height == 0)
        return;
    const Texture& latest = m_pending.id != 0 ? m_pending : m_current;
    if (latest.id != 0 && latest.width == width && latest.height == height)
        return;
    // Superseded before it was ever shown.
    if (m_pending.id != 0)
        mu::GetRenderer().ReleaseRenderTarget(m_pending.id);
    m_pending = {};
    if (m_current.id != 0 && m_current.width == width && m_current.height == height)
        return;
    if (const std::uint32_t id = mu::GetRenderer().CreateRenderTarget(width, height))
        m_pending = {id, width, height};
}

void RenderTarget::Draw()
{
    if (!m_enabled || !m_drawer)
        return;
    const bool promoting = m_pending.id != 0;
    const Texture target = promoting ? m_pending : m_current;
    if (target.id == 0)
        return;

    auto& renderer = mu::GetRenderer();
    if (renderer.BeginOffscreenCapture(target.id, target.width, target.height) == 0)
        return;
    m_drawer(target.width, target.height);
    renderer.EndOffscreenCapture();

    if (!promoting)
        return;
    Retire(m_current.id, m_source);
    m_current = target;
    m_pending = {};
    m_source = kSourcePrefix + std::to_string(s_nextSerial++);
    PublishedSources()[m_source] = {target.id, target.width, target.height};
}

void RenderTarget::RenderAll()
{
    ++s_frame;
    ReleaseRetired();
    // A drawer could destroy a target, its own or another's.
    const auto targets = Targets();
    for (RenderTarget* target : targets)
    {
        const auto& live = Targets();
        if (std::find(live.begin(), live.end(), target) != live.end())
            target->Draw();
    }
}

bool RenderTarget::IsSource(const Rml::String& source)
{
    return source.rfind(kSourcePrefix, 0) == 0;
}

void* RenderTarget::Resolve(const Rml::String& source, int& width, int& height)
{
    const auto found = PublishedSources().find(source);
    if (found == PublishedSources().end())
        return nullptr;
    width = static_cast<int>(found->second.width);
    height = static_cast<int>(found->second.height);
    return mu::GetRenderer().GetRawTexture(found->second.id);
}
} // namespace UI::RmlBridge
