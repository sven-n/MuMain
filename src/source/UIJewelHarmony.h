#pragma once

#define MAXHARMONYJEWELOPTIONTYPE 3
#define MAXHARMONYJEWELOPTIONINDEX 10

//#define IS_BUTTON_SORT

struct HarmonyJewelOption
{
    int OptionType;
    wchar_t Name[60];
    int Minlevel;
    int HarmonyJewelLevel[14];
    int Zen[14];

    HarmonyJewelOption() : OptionType(-1), Minlevel(-1)
    {
        memset(Name, 0, sizeof(Name));
        for (int i = 0; i < 14; ++i)
        {
            HarmonyJewelLevel[i] = -1;
            Zen[i] = -1;
        }
    }
};

struct NaturalAbility
{
    int SI_force;
    int SI_activity;
    NaturalAbility() : SI_force(0), SI_activity(0) {}
};

struct StrikingPower
{
    int SI_minattackpower;
    int SI_maxattackpower;
    int SI_magicalpower;
    int SI_attackpowerRate;
    int SI_skillattackpower;

    StrikingPower() : SI_minattackpower(0),
        SI_maxattackpower(0),
        SI_magicalpower(0),
        SI_attackpowerRate(0),
        SI_skillattackpower(0) {}
};

struct StrengthenDefense
{
    int SI_defense;
    int SI_AG;
    int SI_HP;
    int SI_defenseRate;

    StrengthenDefense() : SI_defense(0), SI_AG(0), SI_HP(0), SI_defenseRate(0) {}
};

struct StrengthenCapability
{
    NaturalAbility SI_NB;
    bool		   SI_isNB;

    StrikingPower  SI_SP;
    bool           SI_isSP;

    StrengthenDefense SI_SD;
    bool			  SI_isSD;

    StrengthenCapability() : SI_isNB(false), SI_isSP(false), SI_isSD(false) {}
};

enum StrengthenItem
{
    SI_Weapon = 0,
    SI_Staff,
    SI_Defense,
    SI_None,
};

typedef HarmonyJewelOption HARMONYJEWELOPTION;

class JewelHarmonyInfo
{
public:
    virtual ~JewelHarmonyInfo();
    static JewelHarmonyInfo* MakeInfo();

public:
    const StrengthenItem		GetItemType(int type);
    const HARMONYJEWELOPTION& GetHarmonyJewelOptionInfo(int type, int option);
    void						GetStrengthenCapability(StrengthenCapability* pitemSC, const ITEM* pitem, const int index);

public:
    const bool IsHarmonyJewelOption(int type, int option);

private:
    JewelHarmonyInfo();
    const bool OpenJewelHarmonyInfoFile(const std::wstring& filename);

private:
    HARMONYJEWELOPTION		m_OptionData[MAXHARMONYJEWELOPTIONTYPE][MAXHARMONYJEWELOPTIONINDEX];
};

inline
const HARMONYJEWELOPTION& JewelHarmonyInfo::GetHarmonyJewelOptionInfo(int type, int option)
{
    return m_OptionData[type][option - 1];
}
