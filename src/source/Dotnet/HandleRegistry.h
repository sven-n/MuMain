#pragma once

#include <cstdint>
#include <map>
#include <mutex>

namespace DotNetBridge
{
// Maps the handles the managed library hands out to the native objects behind them.
//
// The managed library calls back on thread-pool threads. Dispatch() holds the lock while the
// action runs, so Remove() waits for a callback in progress, and no callback for the handle
// starts after Remove() returns: the owner can be destroyed right after it. The lock is
// recursive, because an action may add or remove handles itself.
template <typename Target> class HandleRegistry
{
public:
    void Add(int32_t handle, Target* target)
    {
        const std::lock_guard lock(m_mutex);
        m_targets[handle] = target;
    }

    void Remove(int32_t handle)
    {
        const std::lock_guard lock(m_mutex);
        m_targets.erase(handle);
    }

    // Calls action(target) if the handle is registered and returns whether it did.
    template <typename Action> bool Dispatch(int32_t handle, Action&& action)
    {
        const std::lock_guard lock(m_mutex);
        const auto found = m_targets.find(handle);
        if (found == m_targets.end() || found->second == nullptr)
        {
            return false;
        }

        action(*found->second);
        return true;
    }

private:
    std::recursive_mutex m_mutex;
    std::map<int32_t, Target*> m_targets;
};
} // namespace DotNetBridge
