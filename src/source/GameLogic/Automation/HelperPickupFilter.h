#pragma once

namespace MUHelper
{
struct _PetAttackConfig;
using ConfigData = _PetAttackConfig;
} // namespace MUHelper

typedef struct tagITEM ITEM;

namespace GameLogic::Automation
{
bool ShouldPickUpHelperItem(const MUHelper::ConfigData& config, ITEM* item);
} // namespace GameLogic::Automation
