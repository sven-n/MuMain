#include "stdafx.h"

#include "LegacyMonsterKind.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"

namespace Data::Monsters
{
namespace
{
bool IsForcedNpcMonsterType(int rawType)
{
    switch (rawType)
    {
    case 367:
    case 371:
    case 375:
    case 376:
    case 377:
    case 379:
    case 380:
    case 381:
    case 382:
    case 383:
    case 384:
    case 385:
    case 406:
    case 407:
    case 408:
    case 414:
    case 415:
    case 416:
    case 417:
    case 450:
    case 452:
    case 453:
    case 464:
    case 465:
    case 467:
    case 468:
    case 469:
    case 470:
    case 471:
    case 472:
    case 473:
    case 474:
    case 475:
    case 478:
    case 479:
    case 492:
    case 522:
    case 540:
    case 541:
    case 542:
    case 543:
    case 544:
    case 545:
    case 546:
    case 547:
    case 577:
    case 578:
    case 579:
    case static_cast<int>(MONSTER_WANDERING_MERCHANT_ZYRO):
        return true;
    default:
        return false;
    }
}
} // namespace

int GetLegacyObjectKind(int number)
{
    const int rawType = number;

    // Special hard overrides first.
    if (rawType == 451)
    {
        return KIND_TMP;
    }

    // Exception range that must stay monster even though values are > 200.
    if (rawType >= 480 && rawType <= 491)
    {
        return KIND_MONSTER;
    }

    // Explicit NPC overrides.
    if (rawType == 368 || rawType == 369 || rawType == 370 || IsForcedNpcMonsterType(rawType))
    {
        return KIND_NPC;
    }

    // Legacy fallback classifier.
    if (rawType == 200)
    {
        return KIND_MONSTER;
    }
    if (rawType >= 260)
    {
        return KIND_MONSTER;
    }
    if (rawType > 200)
    {
        return KIND_NPC;
    }
    if (rawType >= 150)
    {
        return KIND_MONSTER;
    }
    if (rawType > 110)
    {
        return KIND_MONSTER;
    }
    if (rawType >= 100)
    {
        return KIND_TRAP;
    }

    return KIND_MONSTER;
}
} // namespace Data::Monsters
