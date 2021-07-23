//////////////////////////////////////////////////////////////////////////
//  
//  npcBreeder.cpp
//  
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <process.h>
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "zzzinfomation.h"
#include "zzzobject.h"
#include "zzzcharacter.h"
#include "zzzinterface.h"
#include "zzzinventory.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "wsclientinline.h"

#include "npcBreeder.h"

#ifdef PET_SYSTEM
#include "GIPetManager.h"
#endif// PET_SYSTEM

///////////////////////////////////////
//  Extern.
///////////////////////////////////////
extern  int SrcInventoryIndex;

///////////////////////////////////////
//  npcBreeder
///////////////////////////////////////
namespace npcBreeder
{
    //////////////////////////////////////////////////////////////////////////
    //  원하는 팻의 생명회복을 위한 젠을 계산한다.
    //////////////////////////////////////////////////////////////////////////
    int     CalcRecoveryZen ( BYTE type, char* Text )
    {
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
		ITEM* ip = NULL;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
        ITEM* ip;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

		g_pMyInventory->SetRepairEnableLevel(false);
        
        switch ( type )
        {
        case REVIVAL_DARKHORSE:
            ip = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            if ( ip->Type!=ITEM_HELPER+4 )
            {
				// 1229 "팻을 장착하지 않았습니다."
                sprintf ( Text, GlobalText[1229] );
                return -1;
            }
            break;

        case REVIVAL_DARKSPIRIT:
            ip = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
            if ( ip->Type!=ITEM_HELPER+5 )
            {
				// 1229 "팻을 장착하지 않았습니다."
                sprintf ( Text, GlobalText[1229] );
                return -1;
            }
            break;

#ifdef DARK_WOLF
        case REVIVAL_DARKWOLF:
            ip = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            if ( ip->Type!=ITEM_HELPER+6 )
            {
                sprintf ( Text, GlobalText[1229] );
                return -1;
            }
            break;
#endif// DARK_WOLF
        }
	    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

		int Level = (ip->Level>>3)&15;
        int maxDurability = calcMaxDurability ( ip, p, Level );

        int Gold = 0;
        if ( ip->Durability < maxDurability )
        {
			DWORD dwValue = 0;
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
			dwValue = giPetManager::GetPetItemValue( giPetManager::GetPetInfo(ip) );	// 250만젠
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
			if(type == REVIVAL_DARKSPIRIT)
			{
				dwValue = giPetManager::ItemValue( PET_TYPE_DARK_SPIRIT );	// 250만젠
			}
			else if(type == REVIVAL_DARKHORSE)
			{
				dwValue = giPetManager::ItemValue( PET_TYPE_DARK_HORSE );	// 250만젠
			}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
			
            Gold = ConvertRepairGold( dwValue, ip->Durability, maxDurability, ip->Type, Text );
        }
#ifndef KJH_FIX_REPAIR_DARKLOAD_PET_DURABILITY_ZERO
        //  완전히 생명이 없을때.
        if ( ip->Durability==0 )
        {
            Gold *= 2;
        }
#endif // KJH_FIX_REPAIR_DARKLOAD_PET_DURABILITY_ZERO

        switch ( Gold )
        {
        case 0:
            sprintf ( Text, GlobalText[1230] );
            break;

        default :
            {
                char  Text2[100];
				memset(Text2, 0, sizeof(char) * 100);

#ifdef _VS2008PORTING
				if ( (int)CharacterMachine->Gold<Gold )
#else // _VS2008PORTING
                if ( CharacterMachine->Gold<Gold )
#endif // _VS2008PORTING
                {
	                ConvertGold ( Gold-CharacterMachine->Gold, Text);
                    sprintf ( Text2, GlobalText[1231], Text );
                }
                else
                {
                    sprintf ( Text2, GlobalText[1232], Text );
                }

                int   Length = strlen ( Text2 );
                memcpy ( Text, Text2, sizeof( char )*Length );
                Text[Length] = 0;

            }
            break;
        }

        return Gold;
    }

	void RecoverPet( BYTE type )
	{
        char Text[100];
        int Gold = CalcRecoveryZen ( type, Text );

#ifdef _VS2008PORTING
		if ( (int)CharacterMachine->Gold>=Gold && Gold!=-1 )
#else // _VS2008PORTING
        if ( CharacterMachine->Gold>=Gold && Gold!=-1 )
#endif // _VS2008PORTING
        {
            switch ( type )
            {
            case REVIVAL_DARKHORSE:
				SendRequestRepair ( EQUIPMENT_HELPER, Gold );
                break;

            case REVIVAL_DARKSPIRIT:
				SendRequestRepair ( EQUIPMENT_WEAPON_LEFT, Gold );
                break;
            }
#ifdef PET_SYSTEM
            giPetManager::InitItemBackup ();
#endif// PET_SYSTEM
        }
	}
}

