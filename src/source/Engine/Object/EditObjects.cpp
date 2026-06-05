#include "stdafx.h"
#include "Engine/Object/EditObjects.h"

// Includes mirror ZzzInterface.cpp, the unit this was extracted from.
#include <imm.h>
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

// File-scope editor state still owned by ZzzInterface.cpp (no shared header yet).
extern int MonsterKey;
extern int EditFlag;
extern int SelectMonster, SelectModel, SelectMapping, SelectColor, SelectWall;
extern float SelectMappingAngle, WallAngle;
extern int BrushSize;
extern bool EnableRandomObject;
extern OBJECT* PickObject;
extern vec3_t PickObjectAngle;
extern float PickObjectHeight;
extern bool PickObjectLockHeight;

namespace Editor
{
void EditObjects()
{
    if (EditFlag == EDIT_MONSTER)
    {
        if (MouseLButtonPush)
        {
            MouseLButtonPush = false;
            bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);
            if (Success)
            {
                CHARACTER* c = CreateMonster((EMonsterType)MonsterScript[SelectMonster].Type, (BYTE)(CollisionPosition[0] / TERRAIN_SCALE), (BYTE)(CollisionPosition[1] / TERRAIN_SCALE), MonsterKey++);
                c->Object.Kind = KIND_EDIT;
            }
        }
        if (MouseRButtonPush)
        {
            MouseRButtonPush = false;
            if (SelectedCharacter != -1)
            {
                CharactersClient[SelectedCharacter].Object.Live = false;
            }
        }
    }
    if (EditFlag == EDIT_OBJECT)
    {
        if (MouseRButtonPush)
        {
            MouseRButtonPush = false;
            bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);
            if (Success)
            {
                OBJECT* o = CreateObject(SelectModel, CollisionPosition, PickObjectAngle);
                int Scale = (int)TERRAIN_SCALE;
                o->Position[0] = (float)((int)o->Position[0] / Scale + 0.5f) * Scale;
                o->Position[1] = (float)((int)o->Position[1] / Scale + 0.5f) * Scale;
            }
        }
        if (MouseLButtonPush)
        {
            MouseLButtonPush = false;
            if (!PickObject)
            {
                if (MouseX < 100 && MouseY < 100)
                {
                    PickObject = CreateObject(SelectModel, MouseTarget, PickObjectAngle);
                }
                else
                {
                    PickObject = CollisionDetectObjects(PickObject);
                    if (PickObject)
                        PickObjectHeight = PickObject->Position[2] - RequestTerrainHeight(PickObject->Position[0], PickObject->Position[1]);
                }
            }
        }
        if (PickObject)
        {
            if (MouseLButton)
            {
                bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);
                if (Success)
                {
                    VectorCopy(CollisionPosition, PickObject->Position);
                    if (PickObjectLockHeight)
                    {
                        int Scale = (int)TERRAIN_SCALE / 2;
                        PickObject->Position[0] = (float)((int)PickObject->Position[0] / Scale * Scale);
                        PickObject->Position[1] = (float)((int)PickObject->Position[1] / Scale * Scale);
                    }
                    else
                        PickObject->Position[2] += PickObjectHeight;
                }
                if (HIBYTE(GetAsyncKeyState('Q')) == 128)
                    PickObject->Angle[0] -= 5.f;
                if (HIBYTE(GetAsyncKeyState('E')) == 128)
                    PickObject->Angle[0] += 5.f;
                if (HIBYTE(GetAsyncKeyState('A')) == 128)
                    PickObject->Angle[2] += 30.f;
                if (HIBYTE(GetAsyncKeyState('D')) == 128)
                    PickObject->Angle[2] -= 30.f;
                if (HIBYTE(GetAsyncKeyState('W')) == 128)
                    PickObjectHeight += 5.f;
                if (HIBYTE(GetAsyncKeyState('S')) == 128)
                    PickObjectHeight -= 5.f;
                if (HIBYTE(GetAsyncKeyState('R')) == 128)
                    PickObject->Scale += 0.02f;
                if (HIBYTE(GetAsyncKeyState('F')) == 128)
                    PickObject->Scale -= 0.02f;
                if (MouseX >= REFERENCE_WIDTH - 100 && MouseY < 100)
                {
                    DeleteObject(PickObject, &ObjectBlock[PickObject->Block]);
                    PickObject = NULL;
                }
            }
            else
            {
                VectorCopy(PickObject->Angle, PickObjectAngle);
                CreateObject(PickObject->Type, PickObject->Position, PickObject->Angle, PickObject->Scale);
                if (EnableRandomObject)
                {
                    vec3_t Position, Angle;
                    for (int i = 0; i < 9; i++)
                    {
                        VectorCopy(PickObject->Position, Position);
                        VectorCopy(PickObject->Angle, Angle);
                        Position[0] += (float)(rand() % 2000 - 1000);
                        Position[1] += (float)(rand() % 2000 - 1000);
                        Position[2] = RequestTerrainHeight(Position[0], Position[1]);
                        Angle[2] = (float)(rand() % 360);
                        CreateObject(PickObject->Type, Position, Angle, PickObject->Scale + (float)(rand() % 16 - 8) * 0.01f);
                    }
                }
                DeleteObject(PickObject, &ObjectBlock[PickObject->Block]);
                PickObject = NULL;
            }
        }
    }

    if (EditFlag == EDIT_HEIGHT)
    {
        if (MouseLButton)
        {
            AddTerrainHeight(CollisionPosition[0], CollisionPosition[1], -10.f, BrushSize + 1, BackTerrainHeight);
            CreateTerrainNormal();
            CreateTerrainLight();
        }
        if (MouseRButton)
        {
            AddTerrainHeight(CollisionPosition[0], CollisionPosition[1], 10.f, BrushSize + 1, BackTerrainHeight);
            CreateTerrainNormal();
            CreateTerrainLight();
        }
    }
    if (EditFlag == EDIT_LIGHT)
    {
        vec3_t Light;
        if (MouseLButton)
        {
            switch (SelectColor)
            {
            case 0:
                Vector(0.1f, 0.1f, 0.1f, Light);
                break;
            case 1:
                Vector(-0.1f, -0.1f, -0.1f, Light);
                break;
            case 2:
                Vector(0.05f, -0.05f, -0.05f, Light);
                break;
            case 3:
                Vector(0.05f, 0.05f, -0.05f, Light);
                break;
            case 4:
                Vector(-0.05f, 0.05f, -0.05f, Light);
                break;
            case 5:
                Vector(-0.05f, 0.05f, 0.05f, Light);
                break;
            case 6:
                Vector(-0.05f, -0.05f, 0.05f, Light);
                break;
            case 7:
                Vector(0.05f, -0.05f, 0.05f, Light);
                break;
            }
            AddTerrainLightClip(CollisionPosition[0], CollisionPosition[1], Light, BrushSize + 1, TerrainLight);
            CreateTerrainLight();
        }
        if (MouseRButton)
        {
            int mx = (int)(CollisionPosition[0] / TERRAIN_SCALE);
            int my = (int)(CollisionPosition[1] / TERRAIN_SCALE);
            for (int y = my - 2; y <= my + 2; y++)
            {
                for (int x = mx - 2; x <= mx + 2; x++)
                {
                    int Index1 = TERRAIN_INDEX_REPEAT(x, y);
                    int Index2 = TERRAIN_INDEX_REPEAT(x - 1, y);
                    int Index3 = TERRAIN_INDEX_REPEAT(x + 1, y);
                    int Index4 = TERRAIN_INDEX_REPEAT(x, y - 1);
                    int Index5 = TERRAIN_INDEX_REPEAT(x, y + 1);
                    for (int i = 0; i < 3; i++)
                    {
                        TerrainLight[Index1][i] = (TerrainLight[Index1][i] + TerrainLight[Index2][i] + TerrainLight[Index3][i] + TerrainLight[Index4][i] + TerrainLight[Index5][i]) * 0.2f;
                    }
                }
            }
            CreateTerrainLight();
        }
    }
    if (EditFlag == EDIT_MAPPING)
    {
        int sx = REFERENCE_WIDTH - 30;
        int sy = 0;
        for (int i = 0; i < 14; i++)
        {
            if (MouseX >= sx && MouseY >= sy + i * 30 && MouseX < sx + 30 && MouseY < sy + i * 30 + 30)
            {
                if (MouseLButton)
                {
                    SelectMapping = i;
                    return;
                }
            }
        }
        int x = (int)SelectXF;
        int y = (int)SelectYF;
        int Index1 = TERRAIN_INDEX_REPEAT(x, y);
        int Index2 = TERRAIN_INDEX_REPEAT(x + 1, y);
        int Index3 = TERRAIN_INDEX_REPEAT(x + 1, y + 1);
        int Index4 = TERRAIN_INDEX_REPEAT(x, y + 1);
        if (Bitmaps[BITMAP_MAPTILE + SelectMapping].Components != 4)
        {
            if (MouseLButton)
            {
                if (CurrentLayer == 0)
                {
                    for (int i = y - BrushSize; i <= y + BrushSize; i++)
                    {
                        for (int j = x - BrushSize; j <= x + BrushSize; j++)
                        {
                            TerrainMappingLayer1[TERRAIN_INDEX_REPEAT(j, i)] = SelectMapping;
                        }
                    }
                }
                if (CurrentLayer == 1)
                {
                    for (int i = y - 1; i <= y + 1; i++)
                    {
                        for (int j = x - 1; j <= x + 1; j++)
                        {
                            TerrainMappingLayer2[TERRAIN_INDEX_REPEAT(j, i)] = SelectMapping;
                        }
                    }
                    TerrainMappingAlpha[Index1] += 0.1f;
                    TerrainMappingAlpha[Index2] += 0.1f;
                    TerrainMappingAlpha[Index3] += 0.1f;
                    TerrainMappingAlpha[Index4] += 0.1f;
                    if (TerrainMappingAlpha[Index1] > 1.f) TerrainMappingAlpha[Index1] = 1.f;
                    if (TerrainMappingAlpha[Index2] > 1.f) TerrainMappingAlpha[Index2] = 1.f;
                    if (TerrainMappingAlpha[Index3] > 1.f) TerrainMappingAlpha[Index3] = 1.f;
                    if (TerrainMappingAlpha[Index4] > 1.f) TerrainMappingAlpha[Index4] = 1.f;
                }
            }
            if (MouseRButton)
            {
                if (CurrentLayer == 1)
                {
                    for (int i = y - 1; i <= y + 1; i++)
                    {
                        for (int j = x - 1; j <= x + 1; j++)
                        {
                            TerrainMappingLayer2[TERRAIN_INDEX_REPEAT(j, i)] = 255;
                        }
                    }
                    TerrainMappingAlpha[Index1] = 0.f;
                    TerrainMappingAlpha[Index2] = 0.f;
                    TerrainMappingAlpha[Index3] = 0.f;
                    TerrainMappingAlpha[Index4] = 0.f;
                }
            }
        }
    }
}
}
