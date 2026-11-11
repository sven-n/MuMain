#include "stdafx.h"
#include "GameLogic/Combat/CombatTarget.h"
#include "GameLogic/Combat/SkillAim.h"

// Includes mirror ZzzInterface.cpp, the unit this was extracted from.
#include "Core/Platform/Imm.h"
#include "UI/Core/UIManager.h"
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
#include "UI/Chat/ChatInput.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/SkillCast.h"
#include "GameLogic/Combat/SkillExecution.h"
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
#include "UI/HUD/UIMapName.h"	// rozy
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Events/Event.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "GameLogic/Social/PartyManager.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "World/MapInfra/w_MapHeaders.h"

extern int SelectedCharacter, TargetX, TargetY;
extern bool SelectFlag;

namespace GameLogic::Combat
{
namespace
{
// A direction skill only takes the direction from its aim, so an aim beyond the
// skill's range is pulled back along it instead of making the hero walk there.
void PullDirectionAimIntoRange(CHARACTER* c, int skill, float distance)
{
    if (c != Hero || !SkillAim::IsAimedByDirection(skill))
    {
        return;
    }
    SkillAim::PullIntoRange(c->Object.Position, c->TargetPosition, distance, TERRAIN_SCALE);
}

void SetTargetTile(const CHARACTER* c)
{
    TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
    TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
}
} // namespace

bool CheckTarget(CHARACTER* c, int skill, float distance)
{
    if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[SelectedCharacter].Object.Position, c->TargetPosition);
        // A character that can't be attacked only gives a direction skill its direction.
        if (!CheckAttack())
        {
            PullDirectionAimIntoRange(c, skill, distance);
        }
        SetTargetTile(c);
        return true;
    }

    RenderTerrain(true);
    if (!SelectFlag)
    {
        return false;
    }
    AimAtGround(c, skill, distance, CollisionPosition);
    return true;
}

void AimAtGround(CHARACTER* c, int skill, float distance, const float ground[3])
{
    VectorCopy(ground, c->TargetPosition);
    PullDirectionAimIntoRange(c, skill, distance);
    SetTargetTile(c);
}
}
