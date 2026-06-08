#include "stdafx.h"
#include "Core/Input/KeyState.h"
#include "Input/Selection.h"
#include "Character/CharacterManager.h" // gCharacterManager

// Includes mirror ZzzInterface.cpp, the unit this was extracted from.
#include "Core/Platform/Imm.h"
#include "UI/Legacy/UIManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/PlayerActionState.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "Core/Input/ImeInput.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Effects/ZzzEffect.h"
#include "Scenes/SceneCore.h"
#include "Engine/Pathing/ZzzPath.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include "GameLogic/Events/MatchEvent.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Character/CSParts.h"
#include "UI/Legacy/UIMapName.h"	// rozy
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Events/Event.h"
#include "UI/NewUI/NewUISystem.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "UI/Legacy/UIControls.h"
#include "GameLogic/Social/PartyManager.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "World/MapInfra/w_MapHeaders.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "UI/NewUI/HUD/NewUIGensRanking.h"

// File-scope state still owned by ZzzInterface.cpp (no shared header yet).
extern int SelectedCharacter, SelectedNpc, SelectedItem, SelectedOperate;
extern int Attacking;

namespace Input::Selection
{
int SelectItem()
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        if (o->Live && o->Visible)
        {
            o->LightEnable = true;
            Vector(0.2f, 0.2f, 0.2f, o->Light);
        }
    }
    float Luminosity = 1.5f;

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        if (o->Live && o->Visible)
        {
            if (CollisionDetectLineToOBB(MousePosition, MouseTarget, o->OBB))
            {
                {
                    o->LightEnable = false;
                    Vector(Luminosity, Luminosity, Luminosity, o->Light);
                    return i;
                }
            }
        }
    }
    return -1;
}

int SelectCharacter(BYTE Kind)
{
    bool Main = true;
    if (SceneFlag == CHARACTER_SCENE)
        Main = false;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if ((Kind & o->Kind) == o->Kind && o->Live && o->Visible && o->Alpha > 0.f)
        {
            o->LightEnable = true;
            switch (c->Level)
            {
            case 0:Vector(0.2f, 0.2f, 0.2f, o->Light); break;
            case 1:Vector(-0.4f, -0.4f, -0.4f, o->Light); break;
            case 2:Vector(0.2f, -0.6f, -0.6f, o->Light); break;
            case 3:Vector(1.5f, 1.5f, 1.5f, o->Light); break;
            case 4:Vector(0.3f, 0.2f, -0.5f, o->Light); break;
            }
            if (c->PK >= PVP_MURDERER2)
            {
                Vector(-0.4f, -0.4f, -0.4f, o->Light);
            }
        }
    }
    vec3_t Light;
    Vector(0.8f, 0.8f, 0.8f, Light);
    int iSelected = -1;
    float fNearestDist = 1000000000000.0f;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;

        if (o->Live && o->Visible && o->Alpha > 0.f && c->Dead == 0 && !g_isCharacterBuff(o, eBuff_CrywolfNPCHide))
        {
            if (o->Kind == KIND_PLAYER)		//. bug fixed by soyaviper
            {
                for (int j = 0; j < PartyNumber; ++j)
                {
                    PARTY_t* p = &Party[j];

                    if (p->index != -2) continue;
                    if (p->index > -1) continue;

                    int length = std::max<int>(wcslen(p->Name), std::max<int>(1, wcslen(c->ID)));

                    if (!wcsncmp(p->Name, c->ID, length))
                    {
                        p->index = i;
                        break;
                    }
                }
            }

            if (Main && c == Hero)
            {
                continue;
            }

            if (c->m_bIsSelected == false)
            {
                continue;
            }

            if ((Kind & o->Kind) == o->Kind)
            {
                if (o->Type == MODEL_SMELTING_NPC)
                {
                    o->OBB.StartPos[2] += 300.0f;
                }
                else if (o->Type == MODEL_MAYA_HAND_LEFT || o->Type == MODEL_MAYA_HAND_RIGHT)
                {
                    o->OBB.StartPos[2] += 200.0f;
                }
                else if (o->Type == MODEL_KANTURU2ND_ENTER_NPC)
                {
                    o->OBB.StartPos[0] -= 100.f;
                    o->OBB.StartPos[2] += 100.0f;
                    o->OBB.XAxis[0] += 100.f;
                    o->OBB.ZAxis[2] += 100.f;
                }

                // Character scene uses a generous synthesized box (model OBB is too
                // tight at the steep camera angle). Main scene uses the model OBB.
                OBB_t pickOBB;
                if (!Main)
                    BuildCharacterScenePickOBB(o, pickOBB);
                else
                    pickOBB = o->OBB;

                if (CollisionDetectLineToOBB(MousePosition, MouseTarget, pickOBB))
                {
                    vec3_t vSub;
                    VectorSubtract(o->Position, g_Camera.Position, vSub);

                    float fNewDist = DotProduct(vSub, vSub);

                    if (fNewDist < fNearestDist)
                    {
                        BOOL bCanTalk = TRUE;
                        if (gMapManager.WorldActive == WD_0LORENCIA || gMapManager.WorldActive == WD_2DEVIAS)
                        {
                            int Index = ((int)o->Position[1] / (int)TERRAIN_SCALE) * 256 + ((int)o->Position[0] / (int)TERRAIN_SCALE);
                            if ((gMapManager.WorldActive == WD_0LORENCIA && TerrainMappingLayer1[Index] == 4) ||
                                (gMapManager.WorldActive == WD_2DEVIAS && TerrainMappingLayer1[Index] == 3))
                            {
                                if (TerrainMappingLayer1[Index] != HeroTile && (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 11))
                                    bCanTalk = FALSE;
                            }
                        }
                        if (bCanTalk == TRUE)
                        {
                            iSelected = i;
                            fNearestDist = fNewDist;
                        }
                    }
                }
            }
        }
    }

    for (int j = 0; j < PartyNumber; ++j)
    {
        PARTY_t* p = &Party[j];

        if (p->index >= 0) continue;

        int length = std::max<int>(wcslen(p->Name), std::max<int>(1, wcslen(Hero->ID)));

        if (!wcsncmp(p->Name, Hero->ID, length))
        {
            p->index = -3;
        }
        else
        {
            p->index = -1;
        }
    }

    return iSelected;
}

int SelectOperate()
{
    for (int i = 0; i < MAX_OPERATES; i++)
    {
        OPERATE* n = &Operates[i];
        OBJECT* o = n->Owner;
        if (n->Live && o->Visible && o->HiddenMesh == -1)
        {
            float* Light = &o->Light[0];
            Vector(0.2f, 0.2f, 0.2f, Light);
        }
    }
    if (IsBattleCastleStart() && gMapManager.WorldActive == WD_30BATTLECASTLE)
        return -1;

    for (int i = 0; i < MAX_OPERATES; i++)
    {
        OPERATE* n = &Operates[i];
        OBJECT* o = n->Owner;
        if (n->Live && o->Visible)
        {
            float* Light = &o->Light[0];
            if (CollisionDetectLineToOBB(MousePosition, MouseTarget, o->OBB))
            {
                Vector(1.5f, 1.5f, 1.5f, Light);
                return i;
            }
        }
    }
    return -1;
}

void SelectObjects()
{
    BYTE CKind_1, CKind_2;

    if (g_pOption->IsAutoAttack() && gMapManager.WorldActive != WD_6STADIUM && gMapManager.InChaosCastle() == false)
    {
        if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_CLIENT + 1)
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
        else
        {
            CHARACTER* sc = &CharactersClient[SelectedCharacter];

            if (sc->Dead > 0 || sc->Object.Kind != KIND_MONSTER)
            {
                SelectedCharacter = -1;
                Attacking = -1;
            }

            if (Attacking != -1)
            {
                if (MouseLButton || MouseLButtonPush || MouseRButton || MouseRButtonPush || Hero->Dead > 0)
                {
                    SelectedCharacter = -1;
                }
            }
            else
            {
                SelectedCharacter = -1;
            }
        }
    }
    else
    {
        SelectedCharacter = -1;
        Attacking = -1;
    }

    SelectedItem = -1;
    SelectedNpc = -1;
    SelectedOperate = -1;

    if (!MouseOnWindow && false == g_pNewUISystem->CheckMouseUse() && SEASON3B::CheckMouseIn(0, 0, GetScreenWidth(), 429))
    {
        if (Core::Input::IsKeyDown(VK_MENU))
        {
            if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem() == NULL)
                SelectedItem = SelectItem();

            if (SelectedItem == -1)
            {
                SelectedNpc = SelectCharacter(KIND_NPC);
                if (SelectedNpc == -1)
                {
                    SelectedCharacter = SelectCharacter(KIND_MONSTER | KIND_EDIT);
                    if (SelectedCharacter == -1)
                    {
                        SelectedCharacter = SelectCharacter(KIND_PLAYER);
                        if (SelectedCharacter == -1)
                        {
                            SelectedOperate = SelectOperate();
                        }
                    }
                }
            }
            else
            {
                g_pPartyManager->SearchPartyMember();
            }
        }
        else
        {
            CKind_1 = KIND_MONSTER | KIND_EDIT;
            CKind_2 = KIND_PLAYER;

            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_WIZARD)
            {
                auto Skill = CharacterAttribute->Skill[Hero->CurrentSkill];

                if (Skill == AT_SKILL_HEALING
                    || Skill == AT_SKILL_HEALING_STR
                    || Skill == AT_SKILL_DEFENSE
                    || Skill == AT_SKILL_DEFENSE_STR
                    || Skill == AT_SKILL_DEFENSE_MASTERY
                    || Skill == AT_SKILL_ATTACK
                    || Skill == AT_SKILL_ATTACK_STR
                    || Skill == AT_SKILL_ATTACK_MASTERY
                    || Skill == AT_SKILL_TELEPORT_ALLY
                    || Skill == AT_SKILL_SOUL_BARRIER
                    || Skill == AT_SKILL_SOUL_BARRIER_STR
                    || Skill == AT_SKILL_SOUL_BARRIER_PROFICIENCY
                    )
                {
                    CKind_1 = KIND_PLAYER;
                    CKind_2 = KIND_MONSTER | KIND_EDIT;
                }
            }

            if (g_pPartyListWindow && g_pPartyListWindow->GetSelectedCharacter() != -1)
            {
                g_pPartyManager->SearchPartyMember();
            }
            else
            {
                if (SelectedCharacter == -1)
                {
                    SelectedCharacter = SelectCharacter(CKind_1);
                }
                if (SelectedCharacter == -1)
                {
                    SelectedCharacter = SelectCharacter(CKind_2);
                    if (SelectedCharacter == -1)
                    {
                        SelectedNpc = SelectCharacter(KIND_NPC);
                        if (SelectedNpc == -1)
                        {
                            if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem() == NULL)
                            {
                                SelectedItem = SelectItem();
                            }
                            if (SelectedItem == -1)
                            {
                                SelectedOperate = SelectOperate();
                            }
                        }
                    }
                }
                else if (Attacking != -1)
                {
                    g_pPartyManager->SearchPartyMember();
                }
            }
        }
    }
    else
    {
        g_pPartyManager->SearchPartyMember();
    }

    if (SelectedCharacter == -1)
    {
        Attacking = -1;
    }

    if (g_pPartyListWindow)
    {
        g_pPartyListWindow->SetListBGColor();
    }
}
}
