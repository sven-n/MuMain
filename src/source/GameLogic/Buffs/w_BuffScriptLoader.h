#pragma once

#include "Data/GameData/BuffData/BuffDefinition.h"

#define MAX_BUFF_NAME_LENGTH (Data::Buffs::MaxDisplayNameLength + 1)

// One buff's names and links as the game code reads them, built from the buff table (Data/Buffs/Buffs.json,
// g_BuffDatabase) in the display language.
class BuffInfo
{
public:
    // What s_ItemType and s_ItemIndex hold for a buff without an item, as the old BuffEffect files wrote it.
    static constexpr short NoItem = 255;

    BuffInfo();
    virtual ~BuffInfo();

    bool HasItem() const
    {
        return s_ItemType != NoItem;
    }

public:
    short s_BuffIndex;
    // The buff's group: buffs of one group are the same kind of effect.
    BYTE s_BuffEffectType;
    // The item that gives it, or NoItem and NoItem for none.
    short s_ItemType;
    short s_ItemIndex;
    wchar_t s_BuffName[MAX_BUFF_NAME_LENGTH];
    // eBuffClass_Buff or eBuffClass_DeBuff.
    BYTE s_BuffClassType;
    std::list<std::wstring> s_BuffDescriptlist;
};

SmartPointer(BuffScriptLoader);
// The game's buff info, read from the buff table (g_BuffDatabase) at each call, so it follows a new display language.
class BuffScriptLoader
{
public:
    static BuffScriptLoaderPtr Make();
    virtual ~BuffScriptLoader();

public:
    const BuffInfo GetBuffinfo(eBuffState type) const;
    eBuffClass IsBuffClass(eBuffState type) const;

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
    int GetBuffIndex(int iItemCode);
    int GetBuffType(int iItemCode);
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

private:
    BuffScriptLoader();
};
