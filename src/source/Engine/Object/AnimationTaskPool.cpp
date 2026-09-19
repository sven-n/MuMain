#include "stdafx.h"
#include "AnimationTaskPool.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include <algorithm>

AnimationTaskPool& AnimationTaskPool::Instance()
{
    static AnimationTaskPool instance;
    return instance;
}

void AnimationTaskPool::Initialize(size_t threadCount)
{
    if (m_running) return;

    if (threadCount == 0)
    {
        unsigned int hardwareCores = std::thread::hardware_concurrency();
        m_workerCount = std::max(2u, (hardwareCores / 2 > 1) ? (hardwareCores / 2 - 1) : 2u);
    }
    else
    {
        m_workerCount = std::max(2u, static_cast<unsigned int>(threadCount));
    }

    m_running = true;
    m_remainingTasks = 0;

    m_queues.reserve(m_workerCount);
    for (size_t i = 0; i < m_workerCount; ++i)
    {
        m_queues.push_back(std::make_unique<WorkerQueue>());
    }

    m_workers.reserve(m_workerCount);
    for (size_t i = 0; i < m_workerCount; ++i)
    {
        m_workers.emplace_back(&AnimationTaskPool::WorkerLoop, this, i);
    }
}

void AnimationTaskPool::Shutdown()
{
    if (!m_running) return;

    m_running = false;
    m_workerCV.notify_all();

    for (std::thread& worker : m_workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }

    m_workers.clear();
    m_queues.clear();
    m_workerCount = 0;
}

bool AnimationTaskPool::PopTaskLocal(size_t workerIndex, Task& task)
{
    if (workerIndex >= m_queues.size()) return false;

    WorkerQueue& q = *m_queues[workerIndex];
    std::lock_guard<std::mutex> lock(q.mutex);
    if (q.queue.empty()) return false;

    task = std::move(q.queue.front());
    q.queue.pop_front();
    return true;
}

bool AnimationTaskPool::StealTask(size_t thiefIndex, Task& task)
{
    size_t count = m_queues.size();
    if (count <= 1) return false;

    for (size_t i = 1; i < count; ++i)
    {
        size_t targetIndex = (thiefIndex + i) % count;
        WorkerQueue& targetQ = *m_queues[targetIndex];

        if (targetQ.mutex.try_lock())
        {
            if (!targetQ.queue.empty())
            {
                task = std::move(targetQ.queue.back());
                targetQ.queue.pop_back();
                targetQ.mutex.unlock();
                return true;
            }
            targetQ.mutex.unlock();
        }
    }
    return false;
}

void AnimationTaskPool::WorkerLoop(size_t workerIndex)
{
    while (m_running)
    {
        Task task;
        if (PopTaskLocal(workerIndex, task) || StealTask(workerIndex, task))
        {
            task();
            int remaining = m_remainingTasks.fetch_sub(1, std::memory_order_acq_rel) - 1;
            if (remaining <= 0)
            {
                std::lock_guard<std::mutex> lock(m_cvMutex);
                m_waiterCV.notify_all();
            }
        }
        else
        {
            std::unique_lock<std::mutex> lock(m_cvMutex);
            m_workerCV.wait(lock, [this]() {
                return !m_running || m_remainingTasks.load(std::memory_order_relaxed) > 0;
            });
        }
    }
}

void AnimationTaskPool::DispatchCharacters(std::span<CHARACTER* const> activeChars)
{
    if (!m_running || m_workerCount == 0 || activeChars.empty()) return;

    // Chunk size of 5 characters per task for optimal CPU cache locality
    constexpr size_t CHUNK_SIZE = 5;
    size_t totalTasks = (activeChars.size() + CHUNK_SIZE - 1) / CHUNK_SIZE;

    m_remainingTasks.store(static_cast<int>(totalTasks), std::memory_order_release);

    for (size_t t = 0; t < totalTasks; ++t)
    {
        size_t startIdx = t * CHUNK_SIZE;
        size_t endIdx = std::min(startIdx + CHUNK_SIZE, activeChars.size());

        struct Chunk {
            std::array<CHARACTER*, CHUNK_SIZE> chars;
            size_t count;
        } chunk{};
        chunk.count = endIdx - startIdx;
        for (size_t i = 0; i < chunk.count; ++i) {
            chunk.chars[i] = activeChars[startIdx + i];
        }

        auto chunkTask = [chunk]() {
            for (size_t i = 0; i < chunk.count; ++i)
            {
                CHARACTER* c = chunk.chars[i];
                OBJECT* o = &c->Object;
                if (!o->Live || !o->Visible) continue;

                BMD* model = &Models[o->Type];
                if (!model || model->NumBones <= 0) continue;

                model->Animation(
                    o->BoneTransform,
                    o->AnimationFrame,
                    o->PriorAnimationFrame,
                    o->PriorAction,
                    o->Angle,
                    o->HeadAngle,
                    false,
                    true,
                    nullptr,
                    o->CurrentAction
                );
                o->EnableBoneMatrix = true;
            }
        };

        size_t targetWorker = t % m_workerCount;
        WorkerQueue& q = *m_queues[targetWorker];
        {
            std::lock_guard<std::mutex> lock(q.mutex);
            q.queue.push_back(std::move(chunkTask));
        }
    }

    m_workerCV.notify_all();
}

void AnimationTaskPool::Wait()
{
    if (m_remainingTasks.load(std::memory_order_acquire) <= 0) return;

    std::unique_lock<std::mutex> lock(m_cvMutex);
    m_waiterCV.wait(lock, [this]() {
        return m_remainingTasks.load(std::memory_order_relaxed) <= 0;
    });
}
