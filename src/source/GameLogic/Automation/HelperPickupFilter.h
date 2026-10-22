#pragma once

#include "MUHelper/MuHelperData.h"

typedef struct tagITEM ITEM;

namespace GameLogic::Automation
{
bool ShouldPickUpHelperItem(const MUHelper::ConfigData& config, ITEM* item);
} // namespace GameLogic::Automation
