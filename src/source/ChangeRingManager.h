#pragma once

class CChangeRingManager
{
public:
    CChangeRingManager();
    virtual ~CChangeRingManager();
    static CChangeRingManager* GetInstance() { static CChangeRingManager s_Instance; return &s_Instance; }
    void LoadItemModel();
    void LoadItemTexture();
    bool CheckDarkLordHair(int iType);
    bool CheckDarkCloak(CLASS_TYPE iClass, int iType);
    bool CheckChangeRing(short RingType);
    bool CheckRepair(int iType);
    bool CheckMoveMap(short sLeftRingType, short sRightRingType);
    bool CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType);
};

#define g_ChangeRingMgr CChangeRingManager::GetInstance()