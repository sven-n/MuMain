#pragma once

namespace SEASON4A
{
    enum _SOCKET_OPTION_TYPE
    {
        SOT_SOCKET_ITEM_OPTIONS,
        SOT_MIX_SET_BONUS_OPTIONS,
        SOT_EQUIP_SET_BONUS_OPTIONS,
        MAX_SOCKET_OPTION_TYPES
    };

    const int MAX_SOCKET_OPTION = 50;
    const int MAX_SOCKET_OPTION_NAME_LENGTH = 64;
    const int MAX_SPHERE_LEVEL = 5;
    const int MAX_SOCKET_TYPES = 6;

    enum _SOCKET_OPTION_CATEGORY
    {
        SOC_NULL,
        SOC_IMPROVE_ATTACK,
        SOC_IMPROVE_DEFENSE,
        SOC_IMPROVE_WEAPON,
        SOC_IMPROVE_ARMOR,
        SOC_IMPROVE_BATTLE,
        SOC_IMPROVE_STATUS,
        SOC_UNIQUE_OPTION,
        MAX_SOCKET_OPTION_CATEGORY
    };

    enum _SOCKET_OPTIONS
    {
        SOPT_ATTACK_N_MAGIC_DAMAGE_BONUS_BY_LEVEL = 0,
        SOPT_ATTACK_SPEED_BONUS,
        SOPT_ATTACT_N_MAGIC_DAMAGE_MAX_BONUS,
        SOPT_ATTACK_N_MAGIC_DAMAGE_MIN_BONUS,
        SOPT_ATTACK_N_MAGIC_DAMAGE_BONUS,
        SOPT_DECREASE_AG_USE,

        SOPT_DEFENCE_RATE_BONUS = 10,
        SOPT_DEFENCE_BONUS,
        SOPT_SHIELD_DEFENCE_BONUS,
        SOPT_DECREASE_DAMAGE,
        SOPT_REFLECT_DAMAGE,

        SOPT_MONSTER_DEATH_LIFE_BONUS = 16,
        SOPT_MONSTER_DEATH_MANA_BONUS,
        SOPT_SKILL_DAMAGE_BONUS,
        SOPT_ATTACK_RATE_BONUS,
        SOPT_INCREASE_ITEM_DURABILITY,

        SOPT_LIFE_REGENERATION_BONUS = 21,
        SOPT_MAX_LIFE_BONUS,
        SOPT_MAX_MANA_BONUS,
        SOPT_MANA_REGENERATION_BONUS,
        SOPT_MAX_AG_BONUS,
        SOPT_AG_REGENERATION_BONUS,

        SOPT_EXCELLENT_DAMAGE_BONUS = 29,
        SOPT_EXCELLENT_DAMAGE_RATE_BONUS,
        SOPT_CRITICAL_DAMAGE_BONUS,
        SOPT_CRITICAL_DAMAGE_RATE_BONUS,

        SOPT_STRENGTH_BONUS = 34,
        SOPT_DEXTERITY_BONUS,
        SOPT_VITALITY_BONUS,
        SOPT_ENERGY_BONUS,
        SOPT_REQUIRED_STENGTH_BONUS,
        SOPT_REQUIRED_DEXTERITY_BONUS,

        SOPT_UNIQUE01 = 41,
        SOPT_UNIQUE02,
    };

    enum _SOCKET_BONUS_OPTIONS
    {
        SBOPT_ATTACK_DAMAGE_BONUS = 0,
        SBOPT_SKILL_DAMAGE_BONUS,
        SBOPT_MAGIC_POWER_BONUS,
        SBOPT_SKILL_DAMAGE_BONUS_2,
        SBOPT_DEFENCE_BONUS,
        SBOPT_MAX_LIFE_BONUS,
    };

    typedef struct
    {
        int m_iOptionID;
        int m_iOptionCategory;
        wchar_t m_szOptionName[MAX_SOCKET_OPTION_NAME_LENGTH];
        char m_bOptionType;
        int m_iOptionValue[5];
        BYTE m_bySocketCheckInfo[6];
    } SOCKET_OPTION_INFO;

    typedef struct
    {
        int m_iOptionID;
        int m_iOptionCategory;
        char m_szOptionName[MAX_SOCKET_OPTION_NAME_LENGTH];
        char m_bOptionType;
        int m_iOptionValue[5];
        BYTE m_bySocketCheckInfo[6];
    } SOCKET_OPTION_INFO_FILE;

    typedef struct _SOCKET_OPTION_STATUS_BONUS
    {
        int m_iAttackDamageMinBonus;
        int m_iAttackDamageMaxBonus;
        int m_iAttackRateBonus;
        int m_iSkillAttackDamageBonus;
        int m_iAttackSpeedBonus;
#ifdef YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
        int m_iMagicPowerMinBonus;
        int m_iMagicPowerMaxBonus;
#else	// YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS
        int m_iMagicPowerBonus;
#endif	// YDG_FIX_SOCKET_MISSING_MAGIC_POWER_BONUS

        int m_iDefenceBonus;
        float m_fDefenceRateBonus;
        int m_iShieldDefenceBonus;
        int m_iStrengthBonus;
        int m_iDexterityBonus;
        int m_iVitalityBonus;
        int m_iEnergyBonus;
    } SOCKET_OPTION_STATUS_BONUS;

    class CSocketItemMgr
    {
    public:
        CSocketItemMgr();
        virtual ~CSocketItemMgr();

        BOOL IsSocketItem(const ITEM* pItem);
        BOOL IsSocketItem(const OBJECT* pObject);
        int GetSeedShpereSeedID(const ITEM* pItem);
        int GetSocketCategory(int iSeedID);

        int AttachToolTipForSocketItem(const ITEM* pItem, int iTextNum);
        int AttachToolTipForSeedSphereItem(const ITEM* pItem, int iTextNum);

        void CheckSocketSetOption();
        BOOL IsSocketSetOptionEnabled();
        void RenderToolTipForSocketSetOption(int iPos_x, int iPos_y);

        void CreateSocketOptionText(wchar_t* pszOptionText, int iSeedID, int iSphereLv);

        __int64 CalcSocketBonusItemValue(const ITEM* pItem, __int64 iOrgGold);

        int GetSocketOptionValue(const ITEM* pItem, int iSocketIndex);

        void CalcSocketStatusBonus();
        SOCKET_OPTION_STATUS_BONUS m_StatusBonus;

        void OpenSocketItemScript(const wchar_t* szFileName);

    protected:
        BOOL IsSocketItem(int iItemType);

        void CalcSocketOptionValueText(wchar_t* pszOptionValueText, int iOptionType, float fOptionValue);
        int CalcSocketOptionValue(int iOptionType, float fOptionValue);

    protected:
        SOCKET_OPTION_INFO m_SocketOptionInfo[MAX_SOCKET_OPTION_TYPES][MAX_SOCKET_OPTION];
        int m_iNumEquitSetBonusOptions;

        std::deque<DWORD> m_EquipSetBonusList;
    };
}

extern SEASON4A::CSocketItemMgr g_SocketItemMgr;
