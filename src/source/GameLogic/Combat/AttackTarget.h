#pragma once

class CHARACTER;
class OBJECT;

namespace GameLogic::Combat
{
// Slot bounds only: callers retain the existing delayed-feedback lifetime semantics.
OBJECT* AttackTargetObject(const CHARACTER& attacker);
} // namespace GameLogic::Combat
