#pragma once

#include <vector>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <span>
#include <array>
#include <memory>

struct CHARACTER;

/**
 * @brief Industry-standard dynamic work-stealing task pool for skeletal animation.
 */
class AnimationTaskPool
{
public:
    using Task = std::function<void()>;

    static AnimationTaskPool& Instance();

    /**
     * @brief Initializes the worker thread pool.
     * @param threadCount Number of worker threads (0 = auto-calculate based on cores)
     */
    void Initialize(size_t threadCount = 0);

    /**
     * @brief Shuts down all worker threads cleanly.
     */
    void Shutdown();

    /**
     * @brief Non-blocking dispatch of character animation tasks across worker queues.
     */
    void DispatchCharacters(std::span<CHARACTER* const> activeChars);

    /**
     * @brief Hard synchronization barrier blocking until all dispatched tasks finish.
     */
    void Wait();

    /**
     * @brief Returns number of active worker threads.
     */
    size_t GetWorkerCount() const { return m_workerCount; }

private:
    AnimationTaskPool() = default;
    ~AnimationTaskPool() { Shutdown(); }

    AnimationTaskPool(const AnimationTaskPool&) = delete;
    AnimationTaskPool& operator=(const AnimationTaskPool&) = delete;

    void WorkerLoop(size_t workerIndex);
    bool PopTaskLocal(size_t workerIndex, Task& task);
    bool StealTask(size_t thiefIndex, Task& task);

    struct WorkerQueue
    {
        std::deque<Task> queue;
        mutable std::mutex mutex;
    };

    std::vector<std::thread> m_workers;
    std::vector<std::unique_ptr<WorkerQueue>> m_queues;
    
    std::atomic<int> m_remainingTasks{ 0 };
    std::atomic<bool> m_running{ false };
    size_t m_workerCount{ 0 };

    std::mutex m_cvMutex;
    std::condition_variable m_workerCV;
    std::condition_variable m_waiterCV;
};
