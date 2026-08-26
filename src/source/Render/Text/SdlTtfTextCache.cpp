#include "SdlTtfTextCache.h"

#include <new>
#include <utility>

namespace Render::Text
{
SdlTtfTextCache::SdlTtfTextCache(TTF_TextEngine* engine, std::size_t capacity)
    : m_engine(engine), m_capacity(capacity)
{
}

SdlTtfTextCache::~SdlTtfTextCache() = default;

SdlTtfTextCache::PreparedText SdlTtfTextCache::Prepare(TTF_Font* font, std::string_view utf8)
{
    if (m_engine == nullptr || font == nullptr)
        return {};

    for (const Entry& entry : m_entries)
    {
        if (entry.font == font && entry.utf8 == utf8)
            return {entry.text.get(), entry.width, entry.height};
    }

    TextOwner text(TTF_CreateText(m_engine, font, utf8.data(), utf8.size()), TTF_DestroyText);
    if (text == nullptr)
        return {};

    int width = 0;
    int height = 0;
    if (!TTF_GetTextSize(text.get(), &width, &height))
        return {};

    PreparedText prepared{text.get(), width, height};
    if (m_capacity == 0)
    {
        prepared.transient = std::move(text);
        return prepared;
    }

    try
    {
        std::string key(utf8);
        if (m_entries.size() >= m_capacity)
            Clear();
        if (m_entries.capacity() < m_capacity)
            m_entries.reserve(m_capacity);
        m_entries.push_back({font, std::move(key), std::move(text), width, height});
    }
    catch (const std::bad_alloc&)
    {
        prepared.transient = std::move(text);
        return prepared;
    }

    const Entry& entry = m_entries.back();
    return {entry.text.get(), entry.width, entry.height};
}

void SdlTtfTextCache::Reset(TTF_TextEngine* engine)
{
    Clear();
    m_engine = engine;
}

void SdlTtfTextCache::Clear()
{
    m_entries.clear();
}

std::size_t SdlTtfTextCache::Size() const
{
    return m_entries.size();
}
}
