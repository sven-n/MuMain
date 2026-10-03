#include "stdafx.h"
#include "UI/RmlBridge/RmlOverlayRender.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace UI::RmlBridge::OverlayRender
{
namespace
{
struct Entry
{
    Owner owner;
    std::function<void()> draw;
};

std::vector<Entry>& Entries()
{
    static std::vector<Entry> entries;
    return entries;
}
} // namespace

void Register(Owner owner, std::function<void()> draw)
{
    if (owner == nullptr || !draw)
        return;
    auto& entries = Entries();
    const auto it = std::find_if(entries.begin(), entries.end(),
                                 [owner](const Entry& entry) { return entry.owner == owner; });
    if (it != entries.end())
        it->draw = std::move(draw);
    else
        entries.push_back({owner, std::move(draw)});
}

void Unregister(Owner owner)
{
    auto& entries = Entries();
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [owner](const Entry& entry) { return entry.owner == owner; }),
                  entries.end());
}

void RenderAll()
{
    // Copied: a draw can close the window it belongs to, which unregisters mid-iteration.
    const auto entries = Entries();
    for (const auto& entry : entries)
        entry.draw();
}
} // namespace UI::RmlBridge::OverlayRender
