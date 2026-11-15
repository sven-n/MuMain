#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace Render::Sprites
{
// Worker threads that run texture decodes. Destroying the pool drops the jobs
// no worker has started and joins the threads.
class TextureDecodePool final
{
public:
    explicit TextureDecodePool(unsigned threadCount = DefaultThreadCount());
    ~TextureDecodePool();
    TextureDecodePool(const TextureDecodePool&) = delete;
    TextureDecodePool& operator=(const TextureDecodePool&) = delete;

    void Post(std::function<void()> job);
    unsigned ThreadCount() const
    {
        return static_cast<unsigned>(m_threads.size());
    }

    // One fewer than the hardware threads, leaving one to the game thread; 1 to 8.
    static unsigned DefaultThreadCount();

private:
    void Run();

    std::mutex m_mutex;
    std::condition_variable m_ready;
    std::deque<std::function<void()>> m_jobs;
    bool m_stopping = false;
    std::vector<std::thread> m_threads;
};
} // namespace Render::Sprites
