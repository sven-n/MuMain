// w_MapHeaders.h: interface for the BaseMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _MAPHEADERS_H
#define _MAPHEADERS_H

#pragma once

#ifdef PSW_ADD_MAPSYSTEM

#include "w_MapProcess.h"

template<typename T>
T& TheWorld( int type ) 
{
	//return static_cast<T&>(TheMapProcess().GetMap(type));
	return dynamic_cast<T&>(TheMapProcess().GetMap(type));
}

// ¶óÅ¬¸®¿Â(ºùÇÏµµ½Ã) ¸Ê
#ifdef CSK_ADD_MAP_ICECITY
#include "GM_Raklion.h"
#endif // CSK_ADD_MAP_ICECITY

#ifdef CSK_ADD_MAP_ICECITY
#define g_Raklion	TheWorld<SEASON4A::CGM_Raklion>(WD_57ICECITY)
#endif // CSK_ADD_MAP_ICECITY

// »êÅ¸¸¶À» ¸Ê
#ifdef YDG_ADD_MAP_SANTA_TOWN
#include "GMSantaTown.h"
#endif // YDG_ADD_MAP_SANTA_TOWN

#ifdef YDG_ADD_MAP_SANTA_TOWN
#define g_SantaTown TheWorld<CGMSantaTown>(WD_62SANTA_TOWN)
#endif // YDG_ADD_MAP_SANTA_TOWN

#ifdef PBG_ADD_PKFIELD
#include "GM_PK_Field.h"
#define g_PKField TheWorld<CGM_PK_Field>(WD_63PK_FIELD)
#endif //PBG_ADD_PKFIELD

#ifdef YDG_ADD_MAP_DUEL_ARENA
#include "GMDuelArena.h"
#define g_DuelArena TheWorld<CGMDuelArena>(WD_64DUELARENA)
#endif	// YDG_ADD_MAP_DUEL_ARENA

#ifdef YDG_ADD_MAP_DOPPELGANGER1
#include "GMDoppelGanger1.h"
#define g_DoppelGanger1 TheWorld<CGMDoppelGanger1>(WD_65DOPPLEGANGER1)
#endif	// YDG_ADD_MAP_DOPPELGANGER1

#ifdef YDG_ADD_MAP_DOPPELGANGER2
#include "GMDoppelGanger2.h"
#define g_DoppelGanger2 TheWorld<CGMDoppelGanger2>(WD_66DOPPLEGANGER2)
#endif	// YDG_ADD_MAP_DOPPELGANGER2

#ifdef YDG_ADD_MAP_DOPPELGANGER3
#include "GMDoppelGanger3.h"
#define g_DoppelGanger3 TheWorld<CGMDoppelGanger3>(WD_67DOPPLEGANGER3)
#endif	// YDG_ADD_MAP_DOPPELGANGER3

#ifdef YDG_ADD_MAP_DOPPELGANGER4
#include "GMDoppelGanger4.h"
#define g_DoppelGanger4 TheWorld<CGMDoppelGanger4>(WD_68DOPPLEGANGER4)
#endif	// YDG_ADD_MAP_DOPPELGANGER4

#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1
#include "GMEmpireGuardian1.h"
#define g_EmpireGuardian1 TheWorld<GMEmpireGuardian1>(WD_69EMPIREGUARDIAN1)
#endif //LDK_ADD_MAP_EMPIREGUARDIAN1

#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2
#include "GMEmpireGuardian2.h"
#define g_EmpireGuardian2 TheWorld<GMEmpireGuardian2>(WD_70EMPIREGUARDIAN2)
#endif //LDS_ADD_MAP_EMPIREGUARDIAN2

#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3
#include "GMEmpireGuardian3.h"
#define g_EmpireGuardian3 TheWorld<GMEmpireGuardian3>(WD_71EMPIREGUARDIAN3)
#endif //LDK_ADD_MAP_EMPIREGUARDIAN3

#ifdef LDS_ADD_MAP_EMPIREGUARDIAN4
#include "GMEmpireGuardian4.h"
#define g_EmpireGuardian4 TheWorld<GMEmpireGuardian4>(WD_72EMPIREGUARDIAN4)
#endif //LDS_ADD_MAP_EMPIREGUARDIAN4

#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
#include "GMUnitedMarketPlace.h"
#define g_UnitedMarketPlace TheWorld<GMUnitedMarketPlace>(WD_79UNITEDMARKETPLACE)
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE

#ifdef ASG_ADD_MAP_KARUTAN
#include "GMKarutan1.h"
#define g_Karutan1 TheWorld<CGMKarutan1>(WD_80KARUTAN1)
#endif // ASG_ADD_MAP_KARUTAN

#endif // PSW_ADD_MAPSYSTEM
#endif //_MAPHEADERS_H
