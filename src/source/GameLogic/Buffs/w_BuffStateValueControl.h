#pragma once

SmartPointer(BuffStateValueControl);

// The buff strip's tooltip texts of a buff. The values and durations of the buffs are in Data/Buffs/Buffs.json
// (GameLogic/Buffs/BuffValues.h).
class BuffStateValueControl
{
public:
    static BuffStateValueControlPtr Make();
    virtual ~BuffStateValueControl();

public:
    void GetBuffInfoString(std::list<std::wstring>& outstr, eBuffState bufftype);

private:
    BuffStateValueControl();
};
