#include "stdafx.h"
#include "Render/Sprites/TextureDecodePool.h"

#include <algorithm>

namespace Render::Sprites
{
TextureDecodePool::TextureDecodePool(unsigned threadCount)
{
    threadCount = std::max(1u, threadCount);
    m_threads.reserve(threadCount);
    for (unsigned i = 0; i < threadCount; ++i)
    {
        m_threads.emplace_back([this] { Run(); });
    }
}

TextureDecodePool::~TextureDecodePool()
{
    {
        const std::lock_guard lock(m_mutex);
        m_stopping = true;
        m_jobs.clear();
    }
    m_ready.notify_all();
    for (std::thread& thread : m_threads)
    {
        thread.join();
    }
}

void TextureDecodePool::Post(std::function<void()> job)
{
    {
        const std::lock_guard lock(m_mutex);
        m_jobs.push_back(std::move(job));
    }
    m_ready.notify_one();
}

unsigned TextureDecodePool::DefaultThreadCount()
{
    const unsigned hardware = std::thread::hardware_concurrency();
    return std::clamp(hardware > 1 ? hardware - 1 : 1u, 1u, 8u);
}

void TextureDecodePool::Run()
{
    for (;;)
    {
        std::function<void()> job;
        {
            std::unique_lock lock(m_mutex);
            m_ready.wait(lock, [this] { return m_stopping || !m_jobs.empty(); });
            if (m_stopping)
            {
                return;
            }
            job = std::move(m_jobs.front());
            m_jobs.pop_front();
        }
        job();
    }
}
} // namespace Render::Sprites
