#pragma once

#include <list>
#include <string>
#include <vector>

#include "GameLogic/Warps/ServerWarpList.h"
#include "Network/MoveCommandData.h"

namespace GameLogic::Warps
{
// Builds the warp list which the client shows from the warps of the server and the warp list of its
// data files (movereq_<lang>.bmd):
// - A warp which is in both keeps the entry of the data files (its translated names and its position),
//   with the level requirement and costs of the server.
// - A warp which only the server has is added after them, in the order of the server, with the name
//   of the server.
// - A warp which only the data files have is left out: the server can't warp there.
std::vector<SEASON3B::CMoveCommandData::MOVEREQINFO>
MergeWarpList(const std::vector<SEASON3B::CMoveCommandData::MOVEREQINFO>& dataFileWarps,
              const std::vector<ServerWarp>& serverWarps);
} // namespace GameLogic::Warps
