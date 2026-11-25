#include "GameLogic/Monsters/MonsterPacks.h"

#include "Core/Content/ContentStore.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <algorithm>
#include <set>
#include <vector>

namespace GameLogic::Monsters::Packs
{
namespace
{
std::vector<Waiting> g_waiting;
// The numbers whose failure was logged.
std::set<int> g_failed;

void LogFailure(int number, const std::string& error)
{
    if (g_failed.insert(number).second)
    {
        MU_LOG_WARN(mu::log::Get("data"), "Monster {}: content pack {} failed: {}", number, PackOf(number), error);
    }
}
} // namespace

std::string PackOf(int number)
{
    return "monster" + std::to_string(number);
}

bool ReadyToCreate(int number, int key)
{
    const Content::PackStatus status = Content::RequestPack(PackOf(number));
    switch (status.state)
    {
    case Content::PackStatus::State::Pending:
        // The same key spawned again replaces its entry.
        std::erase_if(g_waiting, [key](const Waiting& waiting) { return waiting.key == key; });
        g_waiting.push_back({number, key});
        return false;
    case Content::PackStatus::State::Failed:
        LogFailure(number, status.error);
        return true;
    case Content::PackStatus::State::Ready:
        break;
    }
    return true;
}

void Pump(const std::function<void(const Waiting&)>& remake)
{
    if (g_waiting.empty())
    {
        return;
    }
    // `remake` may spawn, which may add to g_waiting: it works on a copy.
    std::vector<Waiting> waiting = std::move(g_waiting);
    g_waiting.clear();
    for (const Waiting& monster : waiting)
    {
        const Content::PackStatus status = Content::RequestPack(PackOf(monster.number));
        if (status.state == Content::PackStatus::State::Pending)
        {
            g_waiting.push_back(monster);
            continue;
        }
        if (status.state == Content::PackStatus::State::Failed)
        {
            LogFailure(monster.number, status.error);
        }
        remake(monster);
    }
}

void Cancel()
{
    g_waiting.clear();
}

int WaitingCount()
{
    return static_cast<int>(g_waiting.size());
}
} // namespace GameLogic::Monsters::Packs
