// MapManager.h: interface for the CMapManager class.
//////////////////////////////////////////////////////////////////////

#pragma once


enum ENUM_WORLD
{
    WD_0LORENCIA = 0,
    WD_1DUNGEON,
    WD_2DEVIAS,
    WD_3NORIA,
    WD_4LOSTTOWER,
    WD_5UNKNOWN,
    WD_6STADIUM,
    WD_7ATLANSE,
    WD_8TARKAN,
    WD_9DEVILSQUARE,
    WD_10HEAVEN,
    WD_11BLOODCASTLE1,
    WD_11BLOODCASTLE_END = WD_11BLOODCASTLE1 + 6,
    WD_18CHAOS_CASTLE,
    WD_18CHAOS_CASTLE_END = WD_18CHAOS_CASTLE + (CHAOS_NUM - 1),
    WD_24HELLAS,
    WD_24HELLAS_END = WD_24HELLAS + (HELLAS_NUM - 2),
    WD_30BATTLECASTLE,
    WD_31HUNTING_GROUND = 31,
    WD_33AIDA = 33,
    WD_34CRYWOLF_1ST = 34,
    WD_35CRYWOLF_2ND = 35,
    WD_24HELLAS_7 = 36,
    WD_37KANTURU_1ST = 37,
    WD_38KANTURU_2ND = 38,
    WD_39KANTURU_3RD = 39,
    WD_40AREA_FOR_GM = 40,
    WD_41CHANGEUP3RD_1ST = 41,
    WD_42CHANGEUP3RD_2ND = 42,
    WD_45CURSEDTEMPLE_LV1 = 45,
    WD_45CURSEDTEMPLE_LV2,
    WD_45CURSEDTEMPLE_LV3,
    WD_45CURSEDTEMPLE_LV4,
    WD_45CURSEDTEMPLE_LV5,
    WD_45CURSEDTEMPLE_LV6,
    WD_51HOME_6TH_CHAR = 51,
    WD_52BLOODCASTLE_MASTER_LEVEL = 52,
    WD_53CAOSCASTLE_MASTER_LEVEL = 53,
    WD_54CHARACTERSCENE = 54,
    WD_55LOGINSCENE = 55,
    WD_56MAP_SWAMP_OF_QUIET = 56,
    WD_57ICECITY = 57,
    WD_58ICECITY_BOSS = 58,
    WD_62SANTA_TOWN = 62,
    WD_63PK_FIELD = 63,
    WD_64DUELARENA = 64,
    WD_65DOPPLEGANGER1 = 65,
    WD_66DOPPLEGANGER2 = 66,
    WD_67DOPPLEGANGER3 = 67,
    WD_68DOPPLEGANGER4 = 68,
    WD_69EMPIREGUARDIAN1 = 69,
    WD_70EMPIREGUARDIAN2 = 70,
    WD_71EMPIREGUARDIAN3 = 71,
    WD_72EMPIREGUARDIAN4 = 72,
    WD_73NEW_LOGIN_SCENE = 73,
    WD_74NEW_CHARACTER_SCENE = 74,
    WD_77NEW_LOGIN_SCENE = 77,
    WD_78NEW_CHARACTER_SCENE = 78,
    WD_79UNITEDMARKETPLACE = 79,
    WD_80KARUTAN1 = 80,
    WD_81KARUTAN2 = 81,
    NUM_WD
};

class CMapManager
{
public:
    CMapManager();
    virtual ~CMapManager();
    void Load();
    void LoadWorld(int Map);
    void DeleteObjects();
    bool InChaosCastle(int iMap = -1);
    bool InBloodCastle(int iMap = -1);
    bool InDevilSquare();
    bool InHellas(int iMap = -1);
    bool InHiddenHellas(int iMap = -1);
    bool IsPKField();
    bool IsCursedTemple();
    bool IsEmpireGuardian1();
    bool IsEmpireGuardian2();
    bool IsEmpireGuardian3();
    bool IsEmpireGuardian4();
    bool IsEmpireGuardian();
    bool InBattleCastle(int iMap = -1);
    const wchar_t* GetMapName(int iMap);
public:
    int WorldActive;
};

extern CMapManager gMapManager;
