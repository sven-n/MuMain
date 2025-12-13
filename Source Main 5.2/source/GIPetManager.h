//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef __CIPET_MANAGER_H__
#define __CIPET_MANAGER_H__

#include <cstdint>
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "CSPetSystem.h"

namespace giPetManager
{
    static PET_INFO	gs_PetInfo;
    void    InitPetManager(void);
    void    CreatePetDarkSpirit(CHARACTER* c);
    void    CreatePetDarkSpirit_Now(CHARACTER* c);

    void    MovePet(CHARACTER* c);
    void    RenderPet(CHARACTER* c);
    void    DeletePet(CHARACTER* c);

    void    InitItemBackup(void);
    void SetPetInfo(std::uint8_t InvType, std::uint8_t InvPos, PET_INFO* pPetinfo);
    PET_INFO* GetPetInfo(ITEM* pItem);
    void CalcPetInfo(PET_INFO* pPetInfo);
    void SetPetItemConvert(ITEM* ip, PET_INFO* pPetInfo);
    std::uint32_t GetPetItemValue(PET_INFO* pPetInfo);
    bool RequestPetInfo(int sx, int sy, ITEM* pItem);
    bool RenderPetItemInfo(int sx, int sy, ITEM* pItem, int iInvenType);

    bool    SelectPetCommand(void);
    void    MovePetCommand(CHARACTER* c);
    bool    SendPetCommand(CHARACTER* c, int Index);
    void    SetPetCommand(CHARACTER* c, int Key, std::uint8_t Cmd);
    void    SetAttack(CHARACTER* c, int Key, int attackType);
    bool    RenderPetCmdInfo(int sx, int sy, int Type);
}

using namespace giPetManager;

#endif