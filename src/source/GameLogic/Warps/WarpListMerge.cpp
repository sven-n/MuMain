#include "GameLogic/Warps/WarpListMerge.h"

#include <algorithm>
#include <cwchar>
#include <iterator>
#include <unordered_set>

namespace GameLogic::Warps
{
namespace
{
using MoveReqInfo = SEASON3B::CMoveCommandData::MOVEREQINFO;

template <size_t N> void CopyName(wchar_t (&target)[N], const std::wstring& name)
{
    const size_t length = std::min(name.size(), N - 1);
    std::wmemcpy(target, name.c_str(), length);
    target[length] = L'\0';
}

MoveReqInfo FromServer(const ServerWarp& warp)
{
    MoveReqInfo info{};
    info.index = warp.index;
    // The sub map name is the one which the warp chat command also accepts; the server has only one.
    CopyName(info.szMainMapName, warp.name);
    CopyName(info.szSubMapName, warp.name);
    info.iReqLevel = warp.levelRequirement;
    info.iReqZen = static_cast<int>(std::min<uint32_t>(warp.costs, INT32_MAX));
    return info;
}
} // namespace

std::vector<MoveReqInfo> MergeWarpList(const std::vector<MoveReqInfo>& dataFileWarps,
                                       const std::vector<ServerWarp>& serverWarps)
{
    std::vector<MoveReqInfo> result;
    result.reserve(serverWarps.size());

    // Of warps with the same index, the first one counts, like on the server.
    std::unordered_set<int> added;
    for (const auto& dataFileWarp : dataFileWarps)
    {
        const auto serverWarp = std::find_if(serverWarps.begin(), serverWarps.end(),
                                             [&](const ServerWarp& warp) { return warp.index == dataFileWarp.index; });
        if (serverWarp == serverWarps.end() || !added.insert(dataFileWarp.index).second)
        {
            continue;
        }

        auto info = dataFileWarp;
        info.iReqLevel = serverWarp->levelRequirement;
        info.iReqZen = static_cast<int>(std::min<uint32_t>(serverWarp->costs, INT32_MAX));
        result.push_back(info);
    }

    for (const auto& serverWarp : serverWarps)
    {
        if (added.insert(serverWarp.index).second)
        {
            result.push_back(FromServer(serverWarp));
        }
    }

    return result;
}
} // namespace GameLogic::Warps
