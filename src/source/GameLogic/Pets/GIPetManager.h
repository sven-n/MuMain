//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#ifndef __CIPET_MANAGER_H__
#define __CIPET_MANAGER_H__

#include <cstdint>
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "CSPetSystem.h"
#include "UI/HUD/Skills/SkillTooltipModel.h"

namespace giPetManager
{
    extern PET_INFO gs_PetInfo;
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

    // 2026-09-02, CMainFrameWindow pilot Phase 2: same Dark-Lord-only pet-command dispatch/
    // content RenderPetCmdInfo() itself performs, with no drawing -- lets
    // UI::Skills::Tooltip::BuildModelForSlot() (SkillTooltip.cpp) fold this into the same RmlUi-
    // bindable Model the regular skill tooltip uses, rather than this staying a separate hand-
    // rolled TextList writer. Returns false for the same case RenderPetCmdInfo() returns false for
    // (Type outside the pet-command range, or hero isn't a Dark Lord) -- caller falls through to
    // the normal skill-tooltip path exactly as before.
    bool    BuildPetCmdTooltipModel(int Type, UI::Skills::Tooltip::Model& outModel);
}

using namespace giPetManager;

#endif
