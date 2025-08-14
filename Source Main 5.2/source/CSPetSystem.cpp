//////////////////////////////////////////////////////////////////////////
//  CSPetSystem.
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSPetSystem.h"
#include "zzzlodterrain.h"
#include "zzzAi.h"
#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzzObject.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzscene.h"
#include "zzzInterface.h"
#include "zzzinventory.h"
#include "zzzeffect.h"
#include "dsplaysound.h"

#include "UIManager.h"
#include "DuelMgr.h"
#include "MapManager.h"

extern bool g_PetEnableDuel;

extern  double   WorldTime;
extern	wchar_t    TextList[50][100];
extern	int     TextListColor[50];
extern	int     TextBold[50];

CSPetSystem::~CSPetSystem()
{
    SAFE_DELETE_ARRAY(m_PetCharacter.Object.BoneTransform);
}

void CSPetSystem::CreatePetPointer(int Type, unsigned char PositionX, unsigned char PositionY, float Rotation)
{
    CHARACTER* c = &m_PetCharacter;
    OBJECT* o = &c->Object;

    m_PetTarget = NULL;
    m_byCommand = PET_CMD_DEFAULT;
    m_pPetInfo = NULL;

    o->Initialize();
    c->PositionX = PositionX;
    c->PositionY = PositionY;
    c->TargetX = PositionX;
    c->TargetY = PositionY;

    c->byExtensionSkill = 0;
    c->m_pPet = NULL;

    int Index = TERRAIN_INDEX_REPEAT(c->PositionX, c->PositionY);
    if ((TerrainWall[Index] & TW_SAFEZONE) == TW_SAFEZONE)
        c->SafeZone = true;
    else
        c->SafeZone = false;

    c->Path.PathNum = 0;
    c->Path.CurrentPath = 0;
    c->Movement = false;
    o->Live = true;
    o->Visible = false;
    o->AlphaEnable = true;
    o->LightEnable = true;
    o->ContrastEnable = false;
    o->EnableBoneMatrix = true;
    o->EnableShadow = false;
    c->Dead = 0;
    c->Blood = false;
    c->GuildTeam = 0;
    c->Run = 0;
    c->GuildMarkIndex = -1;
    c->PK = PVP_NEUTRAL;
    o->Type = Type;
    o->Scale = 0.7f;
    o->Timer = 0.f;
    o->Alpha = 1.f;
    o->AlphaTarget = 1.f;
    o->Velocity = 0.f;
    o->ShadowScale = 0.f;
    o->m_byHurtByDeathstab = 0;
    o->AI = 0;
    o->Velocity = 1.f;
    o->Gravity = 13;
    c->ExtendState = 0;
    c->ExtendStateTime = 0;

    c->GuildStatus = -1;
    c->GuildType = 0;
    c->ProtectGuildMarkWorldTime = 0.0f;

    c->m_byDieType = 0;
    o->m_bActionStart = false;
    o->m_bySkillCount = 0;

    c->Class = CLASS_WIZARD;
    o->PriorAction = 0;
    o->CurrentAction = 0;
    o->AnimationFrame = 0.f;
    o->PriorAnimationFrame = 0;
    c->JumpTime = 0;
    o->HiddenMesh = -1;
    c->MoveSpeed = 10;

    g_CharacterClearBuff(o);

    o->Teleport = TELEPORT_NONE;
    o->Kind = KIND_PET;
    c->Change = false;
    o->SubType = 0;
    c->MonsterIndex = MONSTER_UNDEFINED;
    o->BlendMeshTexCoordU = 0.f;
    o->BlendMeshTexCoordV = 0.f;
    o->Position[0] = (float)(PositionX * TERRAIN_SCALE) + 0.5f * TERRAIN_SCALE;
    o->Position[1] = (float)(PositionY * TERRAIN_SCALE) + 0.5f * TERRAIN_SCALE;

    o->InitialSceneTime = WorldTime;
    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);

    Vector(0.f, 0.f, Rotation, o->Angle);
    Vector(0.5f, 0.5f, 0.5f, o->Light);
    Vector(-60.f, -60.f, 0.f, o->BoundingBoxMin);
    switch (Type)
    {
    case MODEL_DARK_SPIRIT:
        Vector(50.f, 50.f, 150.f, o->BoundingBoxMax);
        break;
    default:
        Vector(50.f, 50.f, 150.f, o->BoundingBoxMax);
        break;
    }

    if (o->BoneTransform != NULL)
    {
        delete[] o->BoneTransform;
        o->BoneTransform = NULL;
    }
    o->BoneTransform = new vec34_t[Models[Type].NumBones];

    int i;
    for (i = 0; i < 2; i++)
    {
        c->Weapon[i].Type = -1;
        c->Weapon[i].Level = 0;
        c->Weapon[i].ExcellentFlags = 0;
    }
    for (i = 0; i < MAX_BODYPART; i++)
    {
        c->BodyPart[i].Type = -1;
        c->BodyPart[i].Level = 0;
        c->BodyPart[i].ExcellentFlags = 0;
        c->BodyPart[i].AncientDiscriminator = 0;
    }

    c->Wing.Type = -1;
    c->Helper.Type = -1;
    c->Flag.Type = -1;

    c->LongRangeAttack = -1;
    c->CollisionTime = 0;
    o->CollisionRange = 200.f;
    c->Rot = 0.f;
    c->Level = 0;
    c->Item = -1;

    o->BlendMesh = -1;
    o->BlendMeshLight = 1.f;

    switch (Type)
    {
    case MODEL_DARK_SPIRIT:
        c->Weapon[0].LinkBone = 0;
        c->Weapon[1].LinkBone = 0;
        break;
    default:
        c->Weapon[0].LinkBone = 0;
        c->Weapon[1].LinkBone = 0;
        break;
    }
    m_byCommand = PET_CMD_DEFAULT;
}

bool CSPetSystem::PlayAnimation(OBJECT* o)
{
    BMD* b = &Models[o->Type];
    float   playSpeed = 0.1f;

    switch (m_PetType)
    {
    case PET_TYPE_DARK_SPIRIT:
        playSpeed = 0.4f;
        break;
    }

    b->CurrentAction = o->CurrentAction;
    return b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, playSpeed, o->Position, o->Angle);
}

void CSPetSystem::SetAI(int AI)
{
    m_PetCharacter.Object.AI = AI;
    m_PetCharacter.Object.LifeTime = 0;
}

void CSPetSystem::SetCommand(int Key, BYTE cmd)
{
    m_byCommand = cmd;
    if (m_PetCharacter.Object.AI != PET_ATTACK && m_PetCharacter.Object.AI != PET_ATTACK_MAGIC)
    {
        m_PetCharacter.Object.m_bActionStart = false;
    }
    if (cmd == PET_CMD_TARGET)
    {
        int Index = FindCharacterIndex(Key);
        m_PetTarget = &CharactersClient[Index];

        m_PetCharacter.Object.m_bActionStart = true;
    }
}

void CSPetSystem::SetAttack(int Key, int attackType)
{
    int Index = FindCharacterIndex(Key);
    m_PetTarget = &CharactersClient[Index];
    OBJECT* Owner = &m_PetOwner->Object;

    if (g_isCharacterBuff(Owner, eDeBuff_Stun))
    {
        return;
    }
    else if (g_isCharacterBuff(Owner, eBuff_Cloaking))
    {
        m_PetCharacter.TargetCharacter = Index;
        SetAI(PET_ATTACK + attackType);
        return;
    }

    m_PetCharacter.TargetCharacter = Index;
    m_PetCharacter.AttackTime = 0;
    m_PetCharacter.LastAttackEffectTime = -1;
    SetAI(PET_ATTACK + attackType);

    if (m_PetCharacter.Object.AI == PET_ATTACK)
    {
        OBJECT* o = &m_PetCharacter.Object;

        o->m_bActionStart = true;
        o->Velocity = rand() % 10 + 20.f;
        o->Gravity = 0.5f;

        PlayBuffer(SOUND_DSPIRIT_RUSH);
    }
    else if (m_PetCharacter.Object.AI == PET_ATTACK_MAGIC)
    {
        PlayBuffer(SOUND_DSPIRIT_MISSILE);
    }
}

void CSPetSystem::MoveInventory(void)
{
}

CSPetDarkSpirit::CSPetDarkSpirit(CHARACTER* c)
{
    m_PetType = PET_TYPE_DARK_SPIRIT;
    m_PetOwner = c;

    m_PetCharacter.Object.BoneTransform = NULL;
    CreatePetPointer(MODEL_DARK_SPIRIT, (c->PositionX), (c->PositionY), 0.f);

    m_PetCharacter.Object.Position[2] += 300.f;
    m_PetCharacter.Object.CurrentAction = 0;
}

CSPetDarkSpirit::~CSPetDarkSpirit(void)
{
    SAFE_DELETE_ARRAY(m_PetCharacter.Object.BoneTransform);
}

void CSPetDarkSpirit::MovePet(void)
{
    bool    Play;
    CHARACTER* c = &m_PetCharacter;
    OBJECT* o = &c->Object;
    OBJECT* Owner = &m_PetOwner->Object;

    if (g_isCharacterBuff(Owner, eDeBuff_Stun))
        return;

    BMD* b = &Models[Owner->Type];
    o->WeaponLevel = (BYTE)c->Level;

    if (!g_DuelMgr.IsPetDuelEnabled())
    {
        m_byCommand = 0;
        auto* pPet = (CSPetSystem*)m_PetOwner->m_pPet;
        SocketClient->ToGameServer()->SendPetCommandRequest(pPet->GetPetType(), AT_PET_COMMAND_DEFAULT, 0xFFFF);
        g_DuelMgr.EnablePetDuel(TRUE);
    }

    if (c->TargetCharacter >= 0 && c->TargetCharacter < MAX_CHARACTERS_CLIENT)
    {
        CHARACTER* tc = &CharactersClient[c->TargetCharacter];

        if ((g_isCharacterBuff((&tc->Object), eBuff_Cloaking) || g_isCharacterBuff(Owner, eBuff_Cloaking))
            && (o->AI == PET_ATTACK || o->AI == PET_ESCAPE || o->AI == PET_ATTACK_MAGIC))
        {
            auto* pPet = (CSPetSystem*)m_PetOwner->m_pPet;

            float dx = o->Position[0] - Owner->Position[0];
            float dy = o->Position[1] - Owner->Position[1];
            float Distance = sqrtf(dx * dx + dy * dy);

            c->TargetCharacter = -1;
            SocketClient->ToGameServer()->SendPetCommandRequest(pPet->GetPetType(), AT_PET_COMMAND_DEFAULT, 0xFFFF);
            SetAI(PET_STAND);
            if (Distance > 50 || (o->AI != PET_STAND_START && o->AI >= PET_FLYING && o->AI <= PET_STAND))
            {
                SetAI(PET_STAND_START);
                o->Velocity = 3.f;
            }
        }
    }
    else if ((g_isCharacterBuff(Owner, eBuff_Cloaking)) && (o->AI == PET_ATTACK || o->AI == PET_ESCAPE || o->AI == PET_ATTACK_MAGIC))
    {
        auto* pPet = (CSPetSystem*)m_PetOwner->m_pPet;

        float dx = o->Position[0] - Owner->Position[0];
        float dy = o->Position[1] - Owner->Position[1];
        float Distance = sqrtf(dx * dx + dy * dy);

        c->TargetCharacter = -1;
        SocketClient->ToGameServer()->SendPetCommandRequest(pPet->GetPetType(), AT_PET_COMMAND_DEFAULT, 0xFFFF);
        SetAI(PET_STAND);
        if (Distance > 50 || (o->AI != PET_STAND_START && o->AI >= PET_FLYING && o->AI <= PET_STAND))
        {
            SetAI(PET_STAND_START);
            o->Velocity = 3.f;
        }
    }

    if (m_PetOwner->SafeZone == true)
    {
        if (o->AI != PET_STAND && o->AI != PET_STAND_START)
        {
            float dx = o->Position[0] - Owner->Position[0];
            float dy = o->Position[1] - Owner->Position[1];
            float Distance = sqrtf(dx * dx + dy * dy);

            SetAI(PET_STAND);
            if (Distance > 50 || (o->AI != PET_STAND_START && o->AI >= PET_FLYING && o->AI <= PET_STAND))
            {
                SetAI(PET_STAND_START);
                o->Velocity = 3.f;
            }
        }
    }
    else if (o->AI == PET_STAND || o->AI == PET_STAND_START)
    {
        SetAI(PET_FLYING);
    }

    Play = PlayAnimation(o);
    if (Play == false)
    {
        switch (o->AI)
        {
        case PET_FLY:           SetAction(o, 0); break;
        case PET_FLYING:        SetAction(o, 1); break;
        case PET_STAND:         SetAction(o, 2); break;
        case PET_STAND_START:   SetAction(o, 1); break;
        case PET_ATTACK:
        case PET_ESCAPE:        SetAction(o, 3); break;
        default:               SetAction(o, 0); break;
        }
    }

    vec3_t  Range, TargetPosition;
    float   FlyRange = 150.f;

    if (o->m_bActionStart == true)
    {
        OBJECT* to = &m_PetTarget->Object;

        VectorCopy(to->Position, TargetPosition);
        VectorSubtract(TargetPosition, o->Position, Range);
    }
    else
    {
        VectorCopy(Owner->Position, TargetPosition);
        VectorSubtract(TargetPosition, o->Position, Range);
    }

    if (o->AI == PET_FLY || o->AI == PET_FLYING)
    {
        o->m_bActionStart = false;

        vec3_t p, Pos, Light;
        Vector(0.3f, 0.4f, 0.7f, Light);
        Vector(0.f, 0.f, 0.f, p);

        if (!g_isCharacterBuff(Owner, eBuff_Cloaking))
        {
            for (int i = 0; i < 1; ++i)
            {
                b->TransformPosition(o->BoneTransform[rand() % 66], p, Pos);

                CreateParticleFpsChecked(BITMAP_SPARK + 1, Pos, o->Angle, Light, 5, 0.8f);
            }
        }

        float Distance = Range[0] * Range[0] + Range[1] * Range[1];
        if (Distance >= FlyRange * FlyRange)
        {
            float Angle = CreateAngle2D(o->Position, TargetPosition);
            o->Angle[2] = TurnAngle2(o->Angle[2], Angle, rand() % 15 + 5.f);
        }
        AngleMatrix(o->Angle, o->Matrix);

        vec3_t Direction;
        VectorRotate(o->Direction, o->Matrix, Direction);
        VectorAdd(o->Position, Direction, o->Position);

        int speedRandom = 28;
        int CharacterHeight = 250;

        if (m_PetOwner->Helper.Type == ITEM_DARK_HORSE_ITEM || gMapManager.WorldActive == WD_55LOGINSCENE)
        {
            CharacterHeight = 350;
        }

        float Height = TargetPosition[2] + CharacterHeight;
        if (o->Position[2] < Height)
        {
            speedRandom = 10;
            o->Angle[0] -= 2.f;
            if (o->Angle[0] < -15.f) o->Angle[0] = -15.f;
        }
        else if (o->Position[2] > Height + 100)
        {
            speedRandom = 20;
            o->Angle[0] += 2.f;
            if (o->Angle[0] > 15.f) o->Angle[0] = 15.f;
        }

        float Speed = 0;
        if (rand_fps_check(speedRandom))
        {
            if (Distance >= FlyRange * FlyRange)
            {
                Speed = -(float)(rand() % 64 + 128) * 0.1f;
            }
            else
            {
                Speed = -(float)(rand() % 8 + 32) * 0.1f;
                o->Angle[2] += (float)(rand() % 60) * FPS_ANIMATION_FACTOR;
            }

            Speed += o->Direction[1] * FPS_ANIMATION_FACTOR;
            Speed = Speed / 2.f;

            o->Direction[0] = 0.f;
            o->Direction[1] = Speed;
            o->Direction[2] = (float)(rand() % 64 - 32) * 0.1f;
        }

        if (o->Direction[1] < -12.f)
        {
            if (o->AI != PET_FLYING)
            {
                SetAI(PET_FLYING);
            }
        }
        else if (o->AI != PET_FLY)
        {
            SetAI(PET_FLY);
        }
    }
    else if (o->AI == PET_ATTACK || o->AI == PET_ESCAPE)
    {
        vec3_t p, Pos;

        AngleMatrix(o->Angle, o->Matrix);
        Vector(0.f, -o->Velocity, 0.f, p);
        VectorRotate(p, o->Matrix, Pos);
        VectorAddScaled(o->Position, Pos, o->Position, FPS_ANIMATION_FACTOR);

        if (o->AI == PET_ATTACK)
        {
            TargetPosition[2] += 50.f * FPS_ANIMATION_FACTOR;
            float Distance = MoveHumming(o->Position, o->Angle, TargetPosition, o->Velocity);
            if (Distance < 20 || o->LifeTime>20)
            {
                SetAI(PET_ESCAPE);
                o->Angle[0] = -45.f;
                if (m_byCommand != PET_CMD_TARGET)
                {
                    o->m_bActionStart = false;
                }
            }
            o->Velocity += o->Gravity * FPS_ANIMATION_FACTOR;
            o->Gravity += 0.2f * FPS_ANIMATION_FACTOR;
            o->LifeTime+= FPS_ANIMATION_FACTOR;
        }
        else if (o->AI == PET_ESCAPE)
        {
            float Distance = Range[0] * Range[0] + Range[1] * Range[1];
            if (Distance >= (FlyRange + 100) * (FlyRange + 100))
            {
                SetAI(PET_FLYING);
            }
            o->Velocity -= 1.f * FPS_ANIMATION_FACTOR;
        }
        SetAction(o, 3);
    }
    else if (o->AI == PET_ATTACK_MAGIC)
    {
        if (c->TargetCharacter != -1)
        {
            CHARACTER* tc = &CharactersClient[c->TargetCharacter];
            OBJECT* to = &tc->Object;

            float Angle = CreateAngle2D(o->Position, to->Position);
            o->Angle[2] = TurnAngle2(o->Angle[2], Angle, 40.f);
        }
    }
    else if (o->AI == PET_STAND)
    {
        vec3_t p;

        Vector(-10.f, 0.f, 10.f, p);
        b->TransformPosition(Owner->BoneTransform[37], p, o->Position, true);
        VectorCopy(Owner->Angle, o->Angle);
        o->Angle[2] -= 120.f;
    }
    else if (o->AI == PET_STAND_START)
    {
        vec3_t p, Pos;

        AngleMatrix(o->Angle, o->Matrix);
        if (o->Velocity != 0.0f)
        {
            Vector(0.f, -o->Velocity, 0.f, p);
            VectorRotate(p, o->Matrix, Pos);
            VectorAddScaled(o->Position, Pos, o->Position, FPS_ANIMATION_FACTOR);
        }
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(Owner->BoneTransform[42], p, Pos, true);

        float Distance = MoveHumming(o->Position, o->Angle, Pos, o->Velocity);
        o->Velocity++;
        if (Distance < 50)
        {
            SetAI(PET_STAND);
        }
    }
    if (o->AI >= PET_ATTACK && o->AI <= PET_ATTACK_MAGIC)
    {
        c->AttackTime += FPS_ANIMATION_FACTOR;
        if (c->AttackTime >= 15)
        {
            c->AttackTime = 15;
        }
    }
    if (rand_fps_check(100) && rand_fps_check(60))
    {
        PlayBuffer(SOUND_DSPIRIT_SHOUT, o);
    }

    VectorCopy(m_PetOwner->Object.Position, TargetPosition);
    VectorSubtract(TargetPosition, o->Position, Range);
    float Distance = Range[0] * Range[0] + Range[1] * Range[1];
    if (o->Position[2] < (TargetPosition[2] - 200.f) || Distance>409600.f)
    {
        o->LifeTime += FPS_ANIMATION_FACTOR;
    }
    if (o->LifeTime > 90)
    {
        o->LifeTime = 0;
        VectorCopy(TargetPosition, o->Position);
        o->Position[2] += 250.f * FPS_ANIMATION_FACTOR;
    }
}

void CSPetDarkSpirit::CalcPetInformation(const PET_INFO& Petinfo)
{
}

void    CSPetDarkSpirit::RenderPetInventory(void)
{
    RenderCmdType();
}

void    CSPetDarkSpirit::RenderPet(int PetState)
{
    CHARACTER* c = &m_PetCharacter;
    OBJECT* o = &c->Object;

    if (c == nullptr || o == nullptr)
    {
        return;
    }

    if (m_pPetInfo)
        o->WeaponLevel = static_cast<BYTE>(m_pPetInfo->m_wLevel & 0xFF);

    if (o->Live)
    {
        o->Visible = TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -20.f);
        if (o->Visible)
        {
            if (m_PetOwner->Object.Type != MODEL_PLAYER && o->Type != MODEL_DARK_SPIRIT)
                return;

            RenderObject(o, false, 0, PetState);
            AttackEffect(c, o);
        }
        else
        {
            o->Velocity = 3.f;
            if (o->AI != PET_STAND_START)
            {
                o->AI = PET_FLYING;
            }
        }
    }
}

void    CSPetDarkSpirit::Eff_LevelUp(void)
{
    OBJECT* o = &m_PetCharacter.Object;

    vec3_t Angle = { 0.f, 0.f, 0.f };
    vec3_t Position = { o->Position[0], o->Position[1], o->Position[2] };

    for (int i = 0; i < 5; ++i)
    {
        CreateJoint(BITMAP_FLARE, Position, Position, Angle, 0, o, 40, 2);
    }
}

void    CSPetDarkSpirit::Eff_LevelDown(void)
{
    OBJECT* o = &m_PetCharacter.Object;

    vec3_t Position = { o->Position[0], o->Position[1], o->Position[2] };

    for (int i = 0; i < 15; ++i)
    {
        CreateJoint(BITMAP_FLARE, Position, o->Position, o->Angle, 0, o, 40, 2);
    }
}

void    CSPetDarkSpirit::RenderCmdType(void)
{
    float   x, y, Width, Height;
    float   PartyWidth = 0.f;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 128);

    if (PartyNumber > 0)
    {
        PartyWidth = 50.f;
    }
    if ((Hero->Helper.Type >= MODEL_GUARDIAN_ANGEL && Hero->Helper.Type <= MODEL_DARK_HORSE_ITEM) || Hero->Helper.Type == MODEL_HORN_OF_FENRIR)
    {
        PartyWidth += 60.f;
    }

    int Dur = 255;
    Width = 50; Height = 2; x = GetScreenWidth() - Width - PartyWidth - 15; y = 4;
    int Life = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability * (int)Width / Dur;

    EnableAlphaTest();

    g_pRenderText->RenderText((int)x + 50, (int)y, GlobalText[1214], 0, 0, RT3_WRITE_RIGHT_TO_LEFT);

    RenderBar(x, y + 12, Width, Height, (float)Life);

    glColor3f(1.f, 1.f, 1.f);

    Width = 20.f; Height = 28.f; x = GetScreenWidth() - Width - PartyWidth - 65.f; y = 5.f;
    RenderBitmap(BITMAP_SKILL_INTERFACE + 2, (float)x, (float)y, (float)Width - 4, (float)Height - 8, (((m_byCommand) % 8) * 32 + 6.f) / 256.f, (((m_byCommand) / 8) * Height + 3.f) / 256.f, Width / 256.f, (Height - 1.f) / 256.f);

    Width -= 8.f; Height -= 8.f;

    if (MouseX >= x && MouseX <= x + Width && MouseY >= y && MouseY <= y + Height)
    {
        RenderTipText((int)x, (int)(y + Height), GlobalText[1219 + m_byCommand]);
    }
}

void CSPetDarkSpirit::AttackEffect(CHARACTER* c, OBJECT* o)
{
    BMD* b = &Models[o->Type];
    vec3_t p, Pos, Light;

    switch (o->AI)
    {
    case PET_ATTACK:
        if (c->AttackTime >= 0 && c->AttackTime <= 2 && m_PetTarget != NULL)
        {
            for (int i = 0; i < 10; i++)
            {
                CreateJoint(BITMAP_LIGHT, o->Position, o->Position, o->Angle, 1, NULL, (float)(rand() % 40 + 20));
            }

            if (c->CheckAttackTime(1))
            {
                vec3_t Angle, Light;

                Vector(45.f, (float)(rand() % 180) - 90, 0.f, Angle);
                Vector(1.f, 0.8f, 0.6f, Light);
                CreateEffect(MODEL_DARKLORD_SKILL, o->Position, Angle, Light, 3);
                c->SetLastAttackEffectTime();
            }
        }
        if (c->AttackTime > 3 && (int)c->AttackTime % 2 && rand_fps_check(1))
        {
            if (o->Position[2] > (m_PetOwner->Object.Position[2] + 100.f))
            {
                Vector(50.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[6], p, Pos);

                CreateEffect(MODEL_AIR_FORCE, Pos, o->Angle, o->Light, 0, o);
            }
        }
        break;

    case PET_ATTACK_MAGIC:
        if (c->AttackTime < 15)
        {
            if (o->BoneTransform != NULL)
            {
                Vector(1.f, 0.6f, 0.4f, Light);
                Vector(0.f, 0.f, 0.f, p);
                for (int i = rand() % 2; i < 66; i += 2)
                {
                    if (!b->Bones[i].Dummy && i < b->NumBones)
                    {
                        b->TransformPosition(o->BoneTransform[i], p, Pos);
                        CreateParticleFpsChecked(BITMAP_LIGHT, Pos, o->Angle, Light, 6, 1.3f);
                    }
                }
            }
        }
        else if (c->AttackTime >= 15)
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                if (to != NULL)
                {
                    b->RenderMeshEffect(1, BITMAP_LIGHT, 1, o->Angle, to);
                }

                SetAI(PET_FLYING);
            }
            c->AttackTime = 0;
            c->LastAttackEffectTime = -1;
        }
        break;

    default:
        break;
    }
}