#pragma once

#include <list>
#include "Core/Globals/_types.h" // BuffStateMap, eBuffState, DWORD

#include <cstdint>

SmartPointer(Buff);

namespace GameLogic::Buffs
{
struct StatBonuses
{
    int attackDamage = 0;
    int defense = 0;
};
} // namespace GameLogic::Buffs

class Buff
{
public:
    static BuffPtr Make();
    Buff();
    virtual ~Buff();

public:
    Buff& operator=(const Buff& buff);

public:
    void RegisterBuff(eBuffState buffstate);
    void RegisterBuff(std::list<eBuffState> buffstate);
    void UnRegisterBuff(eBuffState buffstate);
    void UnRegisterBuff(std::list<eBuffState> buffstate);

    bool isBuff() const;
    bool isBuff(eBuffState buffstate) const;
    const eBuffState isBuff(std::list<eBuffState> buffstatelist) const;
    void TokenBuff(eBuffState curbufftype);

public:
    const DWORD GetBuffSize() const;
    const eBuffState GetBuff(int iterindex) const;
    const DWORD GetBuffCount(eBuffState buffstate);
    void ClearBuff();
    GameLogic::Buffs::StatBonuses GetLevelScaledStatBonuses(std::uint16_t level) const;
    bool IsEqualBuffType(IN int iBuffType, OUT wchar_t* szBuffName);

public:
    BuffStateMap m_Buff;
};

inline Buff& Buff::operator=(const Buff& buff)
{
    m_Buff = buff.m_Buff;
    return *this;
}
