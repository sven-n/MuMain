#pragma once

#include <functional>
#include <utility>
#include <vector>

namespace Core
{
// Undo steps, run newest first. Each setup step that succeeds pushes how to undo it, so a setup that stops
// halfway and a normal shutdown both undo exactly what was set up, in reverse order.
class TeardownStack
{
public:
    void Push(std::function<void()> undo)
    {
        m_steps.push_back(std::move(undo));
    }

    void Unwind()
    {
        while (!m_steps.empty())
        {
            const std::function<void()> undo = std::move(m_steps.back());
            m_steps.pop_back();
            undo();
        }
    }

private:
    std::vector<std::function<void()>> m_steps;
};
} // namespace Core
