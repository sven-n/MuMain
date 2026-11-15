#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace Core
{
// A long load as a list of steps, run a few per frame so the frame can draw a loading screen and answer the
// window in between.
class StagedLoad
{
public:
    using Step = std::function<void()>;

    explicit StagedLoad(std::vector<Step> steps) : m_steps(std::move(steps)) {}

    // Runs steps until the budget is spent, always at least one; true once every step has run.
    bool Pump(std::chrono::milliseconds budget)
    {
        const auto start = std::chrono::steady_clock::now();
        while (m_next < m_steps.size())
        {
            m_steps[m_next++]();
            if (std::chrono::steady_clock::now() - start >= budget)
                break;
        }
        return m_next == m_steps.size();
    }

    std::size_t Done() const
    {
        return m_next;
    }

    std::size_t Total() const
    {
        return m_steps.size();
    }

private:
    std::vector<Step> m_steps;
    std::size_t m_next = 0;
};
} // namespace Core
