// w_MapHeaders.h: interface for the BaseMap class.
//////////////////////////////////////////////////////////////////////
#pragma once
#include "w_MapProcess.h"
#include "MapManager.h"

template<typename T>
T& TheWorld( int type ) 
{
	//return static_cast<T&>(TheMapProcess().GetMap(type));
	return dynamic_cast<T&>(TheMapProcess().GetMap(type));
}

#include "GM_Raklion.h"

#define g_Raklion	TheWorld<SEASON4A::CGM_Raklion>(WD_57ICECITY)

#include "GMSantaTown.h"

#define g_SantaTown TheWorld<CGMSantaTown>(WD_62SANTA_TOWN)

#include "GM_PK_Field.h"
#define g_PKField TheWorld<CGM_PK_Field>(WD_63PK_FIELD)

#include "GMDuelArena.h"
#define g_DuelArena TheWorld<CGMDuelArena>(WD_64DUELARENA)

#include "GMDoppelGanger1.h"
#define g_DoppelGanger1 TheWorld<CGMDoppelGanger1>(WD_65DOPPLEGANGER1)

#include "GMDoppelGanger2.h"
#define g_DoppelGanger2 TheWorld<CGMDoppelGanger2>(WD_66DOPPLEGANGER2)

#include "GMDoppelGanger3.h"
#define g_DoppelGanger3 TheWorld<CGMDoppelGanger3>(WD_67DOPPLEGANGER3)

#include "GMDoppelGanger4.h"
#define g_DoppelGanger4 TheWorld<CGMDoppelGanger4>(WD_68DOPPLEGANGER4)

#include "GMEmpireGuardian1.h"
#define g_EmpireGuardian1 TheWorld<GMEmpireGuardian1>(WD_69EMPIREGUARDIAN1)

#include "GMEmpireGuardian2.h"
#define g_EmpireGuardian2 TheWorld<GMEmpireGuardian2>(WD_70EMPIREGUARDIAN2)

#include "GMEmpireGuardian3.h"
#define g_EmpireGuardian3 TheWorld<GMEmpireGuardian3>(WD_71EMPIREGUARDIAN3)

#include "GMEmpireGuardian4.h"
#define g_EmpireGuardian4 TheWorld<GMEmpireGuardian4>(WD_72EMPIREGUARDIAN4)

#include "GMUnitedMarketPlace.h"
#define g_UnitedMarketPlace TheWorld<GMUnitedMarketPlace>(WD_79UNITEDMARKETPLACE)

#ifdef ASG_ADD_MAP_KARUTAN
#include "GMKarutan1.h"
#define g_Karutan1 TheWorld<CGMKarutan1>(WD_80KARUTAN1)
#endif // ASG_ADD_MAP_KARUTAN


