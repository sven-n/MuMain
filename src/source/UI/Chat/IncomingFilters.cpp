#include "UI/Chat/IncomingFilters.h"

#include <utility>
#include <vector>

namespace UI::Chat
{
namespace
{
struct Entry
{
    int id;
    IncomingFilter filter;
};

// In a function, so that a filter can be added from another file's static initialisation.
std::vector<Entry>& Filters()
{
    static std::vector<Entry> filters;
    return filters;
}
} // namespace

int AddIncomingFilter(IncomingFilter filter)
{
    static int lastId = 0;
    Filters().push_back({++lastId, std::move(filter)});
    return lastId;
}

void RemoveIncomingFilter(int id)
{
    std::erase_if(Filters(), [id](const Entry& entry) { return entry.id == id; });
}

bool ApplyIncomingFilters(IncomingMessage& message)
{
    for (const Entry& entry : Filters())
    {
        if (!entry.filter(message))
        {
            return false;
        }
    }
    return true;
}
} // namespace UI::Chat
