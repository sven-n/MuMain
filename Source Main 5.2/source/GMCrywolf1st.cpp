#include "stdafx.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"
#include "ZzzInterface.h"
#include "DSPlaySound.h"
#include "BoneManager.h"
#include "CMVP1stDirection.h"
#include "CSChaosCastle.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "NewUISystem.h"
#include "SkillManager.h"

extern void MonsterMoveSandSmoke(OBJECT* o);
extern void MonsterDieSandSmoke(OBJECT* o);

extern bool LogOut;

BYTE m_AltarState[5] = { 2,2,2,2,2 };

#include "UIControls.h"


bool	View_Bal = false;
char	Suc_Or_Fail = -1;
char	View_Suc_Or_Fail = -1;
float Deco_Insert = 0.f;
char Message_Box = 0;
wchar_t   Box_String[2][200] = { NULL,NULL };
int  Dark_elf_Num = 0;
int Button_Down = 0;
int BackUp_Key = 0;
int Val_Hp = 100;
int m_iHour = 0, m_iMinute = 0;
DWORD m_dwSyncTime;
int Delay = 1;
int Add_Num = 10;
bool Dark_Elf_Check = false;
int iNextNotice = -1;

BYTE Rank = 0;
int Exp = 0;
BYTE Ranking[5];
CLASS_TYPE HeroClass[5];
int HeroScore[5] = { -1,-1,-1,-1,-1 };
wchar_t HeroName[5][MAX_ID_SIZE + 1];

int BackUpMin = 0;
bool TimeStart = false;
int Delay_Add_inter = 390;
bool View_End_Result = false;
int nPastTick = 0;
int BackUpTick = 0;

BYTE m_OccupationState = 0;
BYTE m_CrywolfState = 0;
int m_StatueHP = 0;



void M34CryWolf1st::CryWolfMVPInit()
{
    Deco_Insert = 0.f;
    memset(Box_String, 0, sizeof(Box_String));
    Button_Down = 0;
    BackUp_Key = 0;
    m_iHour = 0, m_iMinute = 0;
    m_dwSyncTime = -1;
    Add_Num = 10;
    TargetNpc = -1;
    Delay = 1;
    Dark_Elf_Check = false;

    iNextNotice = -1;
    View_Bal = false;
    Message_Box = 0;
    Dark_elf_Num = 0;
    Val_Hp = 100;

    m_OccupationState = 0;
    m_CrywolfState = 0;
    m_StatueHP = 0;
    SelectedNpc = -1;
}

int M34CryWolf1st::IsCryWolf1stMVPStart()
{
    return m_OccupationState;
}

bool M34CryWolf1st::IsCryWolf1stMVPStatePeace()
{
    if (gMapManager.WorldActive == WD_34CRYWOLF_1ST && m_OccupationState == CRYWOLF_OCCUPATION_STATE_PEACE)
        return true;

    return false;
}

void M34CryWolf1st::CheckCryWolf1stMVP(BYTE btOccupationState, BYTE btCrywolfState)
{
    if (m_OccupationState == btOccupationState && m_CrywolfState == btCrywolfState)
        return;

    m_CrywolfState = btCrywolfState;

    if (m_CrywolfState >= CRYWOLF_STATE_READY && m_CrywolfState < CRYWOLF_STATE_ENDCYCLE)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_CRYWOLF);
    }

    if (m_CrywolfState == CRYWOLF_STATE_START)
    {
        Dark_Elf_Check = true;

        g_pCryWolfInterface->InitTime();
    }

    if (m_CrywolfState != CRYWOLF_STATE_START)
        View_Bal = false;

    if (m_CrywolfState == CRYWOLF_STATE_END)
    {
        for (int i = 0; i < 5; i++)
            HeroScore[i] = -1;

        if (btOccupationState != CRYWOLF_OCCUPATION_STATE_WAR)
        {
            Suc_Or_Fail = 1;
            //			View_Suc_Or_Fail = 1;
            if (btOccupationState == CRYWOLF_OCCUPATION_STATE_PEACE)
            {
                Add_Num = 11;
                View_Suc_Or_Fail = 1;
            }
            else
                View_Suc_Or_Fail = -1;
        }
    }

    if (m_OccupationState == btOccupationState)
        return;

    m_OccupationState = btOccupationState;

    wchar_t FileName[64];
    wchar_t WorldName[32];

    swprintf(WorldName, L"World%d", gMapManager.WorldActive + 1);

    switch (m_OccupationState)
    {
    case CRYWOLF_OCCUPATION_STATE_PEACE:
        swprintf(FileName, L"%s\\TerrainLight.jpg", WorldName);
        break;
    case CRYWOLF_OCCUPATION_STATE_OCCUPIED:
        swprintf(FileName, L"%s\\TerrainLight1.jpg", WorldName);
        break;
    case CRYWOLF_OCCUPATION_STATE_WAR:
        swprintf(FileName, L"%s\\TerrainLight2.jpg", WorldName);
        break;
    }
    OpenTerrainLight(FileName);

    switch (m_OccupationState)
    {
    case CRYWOLF_OCCUPATION_STATE_PEACE:
        swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, gMapManager.WorldActive + 1);
        break;
    case CRYWOLF_OCCUPATION_STATE_OCCUPIED:
        swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, (gMapManager.WorldActive + 1) * 10 + 1);
        break;
    case CRYWOLF_OCCUPATION_STATE_WAR:
        swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, (gMapManager.WorldActive + 1) * 10 + 2);
        break;
    }
    OpenTerrainAttribute(FileName);
}

void M34CryWolf1st::CheckCryWolf1stMVPAltarfInfo(int StatueHP, BYTE AltarState1, BYTE AltarState2, BYTE AltarState3, BYTE AltarState4, BYTE AltarState5)
{
    m_StatueHP = StatueHP;
    m_AltarState[0] = AltarState1;
    m_AltarState[1] = AltarState2;
    m_AltarState[2] = AltarState3;
    m_AltarState[3] = AltarState4;
    m_AltarState[4] = AltarState5;
}

void M34CryWolf1st::DoTankerFireFixStartPosition(int SourceX, int SourceY, int PositionX, int PositionY)
{
    vec3_t Position, TargetPosition;

    Vector(PositionX * TERRAIN_SCALE, PositionY * TERRAIN_SCALE, 100.f, TargetPosition);

    int Type = 0;
    if (SourceX == 122 || SourceX == 116) Type = 2;
    else if (SourceX == 62)  Type = 0;
    else if (SourceX == 183)  Type = 1;

    switch (Type)
    {
    case 0:
        Vector(TargetPosition[0] - 800.0f, TargetPosition[1], 800.0f, Position);
        CreateEffect(MODEL_ARROW_TANKER_HIT, Position, TargetPosition, Hero->Object.Light, 0, &Hero->Object);
        break;
    case 1:
        Vector(TargetPosition[0] + 800.0f, TargetPosition[1], 800.0f, Position);
        CreateEffect(MODEL_ARROW_TANKER_HIT, Position, TargetPosition, Hero->Object.Light, 1, &Hero->Object);
        break;
    case 2:
        Vector(TargetPosition[0], TargetPosition[1] + 800.0f, 800.0f, Position);
        CreateEffect(MODEL_ARROW_TANKER_HIT, Position, TargetPosition, Hero->Object.Light, 2, &Hero->Object);
        break;
    }
}

void M34CryWolf1st::RenderNoticesCryWolf()
{
    if (m_CrywolfState != CRYWOLF_STATE_READY)
    {
        return;
    }

    int iTemp = 0;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0, 0, 0, 170);

    if (GetTimeCheck(10000))
    {
        iNextNotice++;
        if (iNextNotice >= 4)
            iNextNotice = 0;
    }
    iTemp = 4 * iNextNotice;

    wchar_t szText[256];
    int nText = 0;

    for (int i = 0; i < 4; i++)
    {
        if (i == 0)
        {
            g_pRenderText->SetTextColor(100, 200, 255, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(100, 150, 255, 255);
        }
        nText = 1957 + i + iTemp;
        if (1966 == nText || 1967 == nText)
        {
            swprintf(szText, GlobalText[nText]);
            g_pRenderText->RenderText(190, 63 + i * 13, szText);
        }
        else
            g_pRenderText->RenderText(190, 63 + i * 13, GlobalText[nText]);
    }
}

void M34CryWolf1st::ChangeBackGroundMusic(int World)
{
    if (World == WD_34CRYWOLF_1ST)
    {
        if (m_CrywolfState == CRYWOLF_STATE_NOTIFY_2)
        {
            StopMp3(MUSIC_BC_CRYWOLF_1ST);
            PlayMp3(MUSIC_CRYWOLF_BEFORE);

            if (IsEndMp3()) StopMp3(MUSIC_CRYWOLF_BEFORE);
        }
        else if (m_CrywolfState == CRYWOLF_STATE_READY)
        {
            StopMp3(MUSIC_CRYWOLF_BEFORE);
            PlayMp3(MUSIC_CRYWOLF_READY);

            if (IsEndMp3()) StopMp3(MUSIC_CRYWOLF_READY);
        }
        else if (m_CrywolfState == CRYWOLF_STATE_START)
        {
            StopMp3(MUSIC_CRYWOLF_READY);
            PlayMp3(MUSIC_CRYWOLF_BACK);

            if (IsEndMp3()) StopMp3(MUSIC_CRYWOLF_BACK);
        }
        else if (m_CrywolfState == CRYWOLF_STATE_END)
        {
            StopMp3(MUSIC_CRYWOLF_READY);
            StopMp3(MUSIC_CRYWOLF_BACK);

            if (Add_Num == 10)
                PlayBuffer(SOUND_CRY1ST_FAILED);
            else
                PlayBuffer(SOUND_CRY1ST_SUCCESS);
        }
        else
        {
            PlayMp3(MUSIC_BC_CRYWOLF_1ST);

            if (IsEndMp3()) StopMp3(MUSIC_BC_CRYWOLF_1ST);
        }
    }
    else
    {
        StopMp3(MUSIC_BC_CRYWOLF_1ST);
    }
}

bool M34CryWolf1st::IsCyrWolf1st()
{
    return (gMapManager.WorldActive == WD_34CRYWOLF_1ST) ? true : false;
}

//. ������Ʈ
bool M34CryWolf1st::CreateCryWolf1stObject(OBJECT* o)
{
    if (!IsCyrWolf1st())
        return false;

    return true;
}

bool M34CryWolf1st::MoveCryWolf1stObject(OBJECT* o)
{
    if (!IsCyrWolf1st())
        return false;

    float Luminosity;
    vec3_t Light;

    switch (o->Type)
    {
    case 36:
    {
    }
    break;
    case 70:
    {
    }
    break;
    case 41:
    {
        Vector(0.2f, 0.7f, 0.5f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
    }
    break;
    case 71:
    case 57:
    {
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
    }
    break;
    case 81:
        o->Alpha = 0.2f;
        break;
    case 82:
    case 83:
    case 84:
        o->HiddenMesh = -2;
        break;
    }
    return true;
}

bool M34CryWolf1st::RenderCryWolf1stObjectVisual(OBJECT* o, BMD* b)
{
    if (!IsCyrWolf1st())
        return false;

    vec3_t Light, p;
    float Scale;//,Luminosity;
    Vector(0.f, 0.f, 0.f, p);
    static int Change_weather = 0;

    Change_weather++;

    if (Change_weather > 70000)
    {
        weather = rand() % 3;
        Change_weather = 0;
    }
    switch (o->Type)
    {
    case 41:
    {
        Vector(1.f, 1.f, 1.f, Light);
        VectorCopy(o->Position, p);
        static float Ro = 0.0f;
        Ro += 1.0f;
        if (Ro >= 360.0f)
            Ro = 0.f;
        float aaa;
        if (Time_Effect > 20)
            aaa = 40 - Time_Effect;
        else
            aaa = Time_Effect;

        Scale = 1.3f;
        p[2] += (350.f + aaa);

        Vector(1.0f, 1.0f, 1.f, Light);
        CreateSprite(BITMAP_FLARE, p, Scale, Light, o);
        CreateSprite(BITMAP_FLARE, p, Scale, Light, o, Ro);
    }
    break;
    case 37:
    {
    }
    break;
    case 84:
        if (rand_fps_check(3) && m_OccupationState == CRYWOLF_OCCUPATION_STATE_OCCUPIED)
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale);
        }
        break;
    }

    return true;
}
bool M34CryWolf1st::RenderCryWolf1stObjectMesh(OBJECT* o, BMD* b, int ExtraMon)
{
    if (!IsCyrWolf1st())
    {
        return false;
    }

    float Luminosity;
    vec3_t Light;

    switch (o->Type)
    {
    case 36:
    {
        if (m_OccupationState == CRYWOLF_OCCUPATION_STATE_PEACE)
        {
            Vector(0.4f, 0.4f, 0.4f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        else
        {
            Vector(0.4f, 0.4f, 0.4f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    return true;
    case 56:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        if (m_OccupationState == CRYWOLF_OCCUPATION_STATE_PEACE)
        {
            float Color = (int)rand() % 10000 * 0.0001f;

            Vector(Color, Color, Color, Light);

            if (rand_fps_check(3))
                CreateEffect(BITMAP_MAGIC, o->Position, o->Angle, Light, 3, o, 4.0f);
        }
    }
    return true;
    case 41:
        Vector(0.2f, 0.7f, 0.f, Light);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return true;
    case 57:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, o->Position, o->Angle, Light, 5, o->Scale);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale);
        }
    }
    return true;
    case 71:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, o->Position, o->Angle, Light, 5, o->Scale);
        }
    }
    return true;
    case 72:
    {
        if (rand_fps_check(10))
        {
            vec3_t Position;
            VectorCopy(o->Position, Position);
            Position[2] += 50.0f;
            Luminosity = (float)(rand() % 4 + 3) * 0.1f;
            Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
            CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light, 33, o->Scale);
        }
    }
    return true;
    case 73:
    {
        if (weather == 0)
            ashies = true;
    }
    return true;
    case 74:
    {
        if (rand_fps_check(3) && m_OccupationState == CRYWOLF_OCCUPATION_STATE_OCCUPIED)
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale);
        }
    }
    return true;
    case 77:
    {
        if (rand_fps_check(6))
        {
            //	Vector(0.02f,0.02f,0.03f,Light);
            Vector(0.02f, 0.03f, 0.02f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 9, o->Scale, o);
        }
    }
    return true;
    case 78:
    {
        if (rand_fps_check(6))
        {
            Vector(0.03f, 0.06f, 0.05f, Light);
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 9, o->Scale, o);
        }
    }
    return true;
    case 81:
    {
        if (m_OccupationState == CRYWOLF_OCCUPATION_STATE_WAR)
        {
            o->Scale = 1.03f;

            float fTemp = 0.1f;

            if (m_StatueHP <= 10) fTemp = 1.0f;
            b->BodyLight[0] = (sinf(WorldTime * 0.004f) * 3.0f) * fTemp + 5.0f - m_StatueHP / 20.0f;
            b->BodyLight[1] = m_StatueHP / 25.0f - 1.0f;
            b->BodyLight[2] = m_StatueHP / 20.0f - 0.5f;

            b->StreamMesh = 0;
            b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->StreamMesh = -1;

            b->StreamMesh = 1;
            b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME2, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->StreamMesh = -1;
        }
    }
    return true;
    case MODEL_SKILL_FURY_STRIKE:
    {
        Vector(0.0f, 0.0f, 0.9f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    return true;
    }

    return RenderCryWolf1stMonsterObjectMesh(o, b, ExtraMon);
}

CHARACTER* M34CryWolf1st::CreateCryWolf1stMonster(int iType, int PosX, int PosY, int Key)
{
    if (!IsCyrWolf1st() && !(gMapManager.InDevilSquare()))
        return NULL;

    CHARACTER* c = NULL;

    switch (iType)
    {
    case MONSTER_HAMMER_SCOUT:
    {
        OpenMonsterModel(MONSTER_MODEL_SCOUT);
        c = CreateCharacter(Key, MODEL_SCOUT, PosX, PosY);
        c->Object.Scale = 1.2f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_LANCE_SCOUT:
    {
        OpenMonsterModel(MONSTER_MODEL_SOLAM);
        c = CreateCharacter(Key, MODEL_SOLAM, PosX, PosY);
        c->Object.Scale = 1.2f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_BOW_SCOUT:
    {
        OpenMonsterModel(MONSTER_MODEL_VALAM);
        c = CreateCharacter(Key, MODEL_VALAM, PosX, PosY);
        c->Object.Scale = 1.2f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;

        BoneManager::RegisterBone(c, L"Monster96_Top", 27);
        BoneManager::RegisterBone(c, L"Monster96_Center", 28);
        BoneManager::RegisterBone(c, L"Monster96_Bottom", 29);
    }
    break;
    case MONSTER_WEREWOLF:
    {
        OpenMonsterModel(MONSTER_MODEL_WEREWOLF_HERO);
        c = CreateCharacter(Key, MODEL_WEREWOLF_HERO, PosX, PosY);
        c->Object.Scale = 1.25f;
        c->Object.SubType = 0;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;

        BoneManager::RegisterBone(c, L"Monster95_Head", 6);
    }
    break;
    case MONSTER_SCOUTHERO:
    {
        OpenMonsterModel(MONSTER_MODEL_SCOUT);
        c = CreateCharacter(Key, MODEL_SCOUT, PosX, PosY);
        c->Object.Scale = 1.6f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_WEREWOLFHERO:
    {
        OpenMonsterModel(MONSTER_MODEL_WEREWOLF_HERO);
        c = CreateCharacter(Key, MODEL_WEREWOLF_HERO, PosX, PosY);
        c->Object.Scale = 1.65f;
        c->Object.SubType = 1;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;

        BoneManager::RegisterBone(c, L"Monster95_Head", 6);
    }
    break;
    case MONSTER_VALAM:
    case MONSTER_BALRAM:
    {
        OpenMonsterModel(MONSTER_MODEL_BALRAM);
        c = CreateCharacter(Key, MODEL_BALRAM, PosX, PosY);
        c->Object.Scale = 1.25f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_SOLAM:
    case MONSTER_SORAM:
    {
        OpenMonsterModel(MONSTER_MODEL_SORAM);
        c = CreateCharacter(Key, MODEL_SORAM, PosX, PosY);
        c->Object.Scale = 1.3f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
    }
    break;
    case MONSTER_BALGASS:
    {
        OpenMonsterModel(MONSTER_MODEL_BALGASS);
        c = CreateCharacter(Key, MODEL_BALGASS, PosX, PosY);
        c->Object.Scale = 2.f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        CreateJoint(BITMAP_JOINT_ENERGY, c->Object.Position, c->Object.Position, c->Object.Angle, 2, &c->Object, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, c->Object.Position, c->Object.Position, c->Object.Angle, 3, &c->Object, 30.f);
    }
    break;
    case MONSTER_DEATH_SPIRIT:
    {
        OpenMonsterModel(MONSTER_MODEL_DEATH_SPIRIT);
        c = CreateCharacter(Key, MODEL_DEATH_SPIRIT, PosX, PosY);
        c->Object.Scale = 1.25f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;

        BoneManager::RegisterBone(c, L"Monster94_zx", 27);
        BoneManager::RegisterBone(c, L"Monster94_zx01", 28);
    }
    break;
    case MONSTER_DARK_ELF:
    case MONSTER_DARKELF:
    {
        OpenMonsterModel(MONSTER_MODEL_DARK_ELF_1);
        c = CreateCharacter(Key, MODEL_DARK_ELF_1, PosX, PosY);
        c->Object.Scale = 1.5f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        BoneManager::RegisterBone(c, L"Left_Hand", 17);
    }
    break;
    case MONSTER_BALLISTA:
    {
        OpenMonsterModel(MONSTER_MODEL_BALLISTA);
        c = CreateCharacter(Key, MODEL_BALLISTA, PosX, PosY, 180);
        c->Object.Scale = 1.0f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;

        if (PosY == 90)
        {
        }
        if (PosX == 62)
            c->Object.Angle[2] = 90.0f;
        if (PosX == 183)
            c->Object.Angle[2] = 90.0f;
    }
    break;
    }

    return c;
}

bool M34CryWolf1st::MoveCryWolf1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_SCOUT:
    {
        float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
        float fAnimationFrame = o->AnimationFrame - fActionSpeed;
        b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
        vec3_t Light;
        if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t EndPos, EndPos1, EndRelative;
            if (o->AnimationFrame >= 5.5f && o->AnimationFrame < (5.5f + fActionSpeed))
            {
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[24], EndRelative, EndPos, false);
                b->TransformPosition(BoneTransform[16], EndRelative, EndPos1, false);
                Vector(1.f, 1.f, 1.f, Light);

                for (int iu = 0; iu < 6; iu++)
                {
                    CreateEffectFpsChecked(MODEL_STONE2, EndPos, o->Angle, o->Light);
                    CreateEffectFpsChecked(MODEL_STONE2, EndPos1, o->Angle, o->Light);
                }
            }
        }
        else
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
                }
            }
    }
    break;
    case MODEL_SORAM:
    {
        float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
        vec3_t Light;
        vec3_t EndPos, EndRelative;
        Vector(1.f, 1.f, 1.f, Light);

        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->AnimationFrame >= 6.5f && o->AnimationFrame < (6.5f + fActionSpeed) && rand_fps_check(1))
            {
                Vector(0.0f, 0.0f, 0.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[27], EndRelative, EndPos, true);
                CreateEffect(BITMAP_CRATER, EndPos, o->Angle, o->Light, 2);
                CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
                CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
                Vector(1.0f, 0.6f, 0.4f, Light);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
                CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);

                for (int iu = 0; iu < 4; iu++)
                {
                    CreateEffect(MODEL_BIG_STONE1, EndPos, o->Angle, o->Light, 10);
                    CreateEffect(MODEL_STONE2, EndPos, o->Angle, o->Light);
                }
            }
        }
        else
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
                }
            }
    }
    break;
    case MODEL_BALGASS:
    {
        MoveEye(o, b, 9, 10);
        float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
        float fAnimationFrame = o->AnimationFrame - fActionSpeed;
        b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
        vec3_t Light;

        if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t EndPos, EndRelative;
            if (o->AnimationFrame >= 7.5f && o->AnimationFrame < (7.5f + fActionSpeed) && rand_fps_check(1))
            {
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[33], EndRelative, EndPos, false);
                Vector(1.f, 1.f, 1.f, Light);
                CreateEffect(BITMAP_CRATER, EndPos, o->Angle, o->Light, 2);
                CreateParticle(BITMAP_EXPLOTION, EndPos, o->Angle, Light, 2);
                for (int iu = 0; iu < 6; iu++)
                    CreateEffect(MODEL_BIG_STONE1, EndPos, o->Angle, o->Light, 10);
                //						CreateEffect ( MODEL_STONE2,EndPos,o->Angle,o->Light);
            }
        }
        else
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
                }
            }
    }
    break;
    case MODEL_BALRAM:
    {
        vec3_t Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10)) {
                CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
            }
        }
    }
    break;
    case MODEL_DARK_ELF_1:
    {
        if (m_CrywolfState == CRYWOLF_STATE_START && Dark_Elf_Check)
        {
            Dark_Elf_Check = false;

            vec3_t Light;
            vec3_t EndPos, EndRelative;

            Vector(1.f, 0.2f, 0.2f, Light);
            Vector(0.0f, 0.0f, 0.0f, EndRelative);
            b->TransformPosition(o->BoneTransform[27], EndRelative, EndPos, true);
            CreateEffect(BITMAP_CRATER, EndPos, o->Angle, o->Light, 2);
            CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
            CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 1);
            Vector(1.0f, 0.2f, 0.2f, Light);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
            CreateEffect(BITMAP_SHOCK_WAVE, EndPos, o->Angle, Light, 8);
        }
    }
    break;
    case MODEL_DEATH_SPIRIT:
    {
        vec3_t Light;
        Vector(0.9f, 0.2f, 0.1f, Light);
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10)) {
                CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
            }
        }
    }
    break;
    case MODEL_WEREWOLF_HERO:
    {
        //CreateEffect ( MODEL_SKILL_FURY_STRIKE, o->Position, o->Angle, o->Light, 1, o, -1, 0, 1 );
        vec3_t Position, Light;

        if (o->CurrentAction != MONSTER01_DIE) {
            Vector(0.9f, 0.2f, 0.1f, Light);
            BoneManager::GetBonePosition(o, L"Monster95_Head", Position);
            CreateSprite(BITMAP_LIGHT, Position, 3.5f, Light, o);
        }

        float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
        vec3_t EndPos, EndRelative;
        Vector(1.f, 1.f, 1.f, Light);

        //			CreateParticle(BITMAP_FIRE,EndPos,o->Angle,Light);
        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->AnimationFrame >= 2.5f && o->AnimationFrame < (2.5f + fActionSpeed) && rand_fps_check(1))
            {
                Vector(0.0f, 0.0f, 100.0f, EndRelative);
                b->TransformPosition(o->BoneTransform[20], EndRelative, EndPos, true);
                if (o->Scale > 1.25f)
                {
                    //						Vector ( 0.f, 0.f, 1.f, Light );
                    CreateEffect(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 1);
                }
            }
        }
        else
            //			Vector ( 0.9f, 0.2f, 0.1f, Light );
                        //. Walking & Running Scene Processing
            if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
            {
                if (rand_fps_check(10)) {
                    CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
                }
            }
    }
    break;
    case MODEL_SOLAM:
    {
        vec3_t Position, Light;

        if (o->CurrentAction == MONSTER01_ATTACK2 && o->AnimationFrame >= 4.0f && o->AnimationFrame <= 6.0f)
        {
            float Matrix[3][4];
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);

            Vector(1.f, 0.0f, 0.5f, Light);
            CreateEffectFpsChecked(MODEL_PIERCING2, o->Position, o->Angle, Light, 1);
        }
    }
    break;
    case MODEL_VALAM:
    {
        vec3_t Position, Light;

        auto Rotation = (float)(rand() % 360);
        float Luminosity = sinf(WorldTime * 0.0012f) * 0.8f + 1.3f;

        float fScalePercent = 1.f;
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
            fScalePercent = .5f;

        BoneManager::GetBonePosition(o, L"Monster96_Center", Position);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 0.1f, Light);
        CreateSprite(BITMAP_LIGHT, Position, fScalePercent, Light, o);

        Vector(0.5f, 0.5f, 0.5f, Light);

        BoneManager::GetBonePosition(o, L"Monster96_Top", Position);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, 360.f - Rotation);

        BoneManager::GetBonePosition(o, L"Monster96_Bottom", Position);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, 360.f - Rotation);
    }
    break;
    case MODEL_BALLISTA:
    {
        if (m_CrywolfState == CRYWOLF_STATE_START)
        {
            if (rand_fps_check(100))
            {
                SetAction(o, MONSTER01_ATTACK1);
                o->Timer = 1;
            }

            if (o->CurrentAction == MONSTER01_ATTACK1 && o->Timer == 1 && o->AnimationFrame >= 5.0f)
            {
                CreateEffect(MODEL_ARROW_TANKER, o->Position, o->Angle, o->Light, 1, o, o->PKKey);
                o->Timer = 0;
                return true;
            }
        }
    }
    break;
    }
    return false;
}

bool M34CryWolf1st::AttackEffectCryWolf1stMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    //CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 6, o, 20.f, 40 );
    OBJECT* to = NULL;
    /*
        if ( c->TargetCharacter >= 0 && c->TargetCharacter<MAX_CHARACTERS_CLIENT )
        {
             tc = &CharactersClient[c->TargetCharacter];
             to = &tc->Object;
             o->Angle[2] = CreateAngle2D(o->Position, to->Position);
        }
    */
    if (!IsCyrWolf1st() && !(gMapManager.InDevilSquare()))
        return false;

    switch (o->Type)
    {
    case MODEL_VALAM:
    {
        if (c->CheckAttackTime(14))
        {
            CreateEffect(MODEL_ARROW_NATURE, o->Position, o->Angle, o->Light, 1, o, o->PKKey);
            c->SetLastAttackEffectTime();
            return true;
        }
    }
    break;
    case MODEL_BALRAM:
    {
        if (c->CheckAttackTime(14))
        {
            CreateEffect(MODEL_ARROW_HOLY, o->Position, o->Angle, o->Light, 1, o, o->PKKey);
            c->SetLastAttackEffectTime();
            return true;
        }
    }
    break;
    case MODEL_TANTALLOS:
    {
        vec3_t Angle;
        if (c->CheckAttackTime(1))
        {
            CreateInferno(o->Position);
            c->SetLastAttackEffectTime();
        }
        if (c->CheckAttackTime(14))
        {
            if (c->MonsterIndex == MONSTER_ZAIKAN)
            {
                if ((c->Skill) == AT_SKILL_BOSS)
                {
                    for (int i = 0; i < 18; i++)
                    {
                        VectorCopy(o->Angle, Angle);
                        Angle[2] += i * 20.f;
                        CreateEffect(MODEL_STAFF_OF_DESTRUCTION, o->Position, Angle, o->Light);
                    }
                }
            }

            c->SetLastAttackEffectTime();
        }
    }
    break;
    case MODEL_BEAM_KNIGHT:
    {
        vec3_t p, Position, Angle;
        for (int i = 0; i < 6; i++)
        {
            int Hand = 0;
            if (i >= 3) Hand = 1;
            b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
            Vector(0.f, 0.f, (float)(rand() % 360), Angle);

            if (to != NULL)
            {
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 50.f);
                CreateJointFpsChecked(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 10.f);
            }
        }
        if (c->CheckAttackTime(1))
        {
            PlayBuffer(SOUND_EVIL);
            c->SetLastAttackEffectTime();
        }

        for (int i = 0; i < 4; i++)
        {
            int Hand = 0;
            if (i >= 2) Hand = 1;
            b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
            Vector(0.f, 0.f, (float)(rand() % 360), Angle);
            if (to != NULL)
                CreateJointFpsChecked(BITMAP_JOINT_LASER + 1, Position, to->Position, Angle, 0, to, 50.f);
            CreateParticleFpsChecked(BITMAP_FIRE, Position, o->Angle, o->Light);
        }
    }
    break;
    case MODEL_BALLISTA:
    {
        if (c->CheckAttackTime(15))
        {
            CreateEffect(MODEL_ARROW_TANKER, o->Position, o->Angle, o->Light, 1, o, o->PKKey);
            c->SetLastAttackEffectTime();
            return true;
        }
    }
    break;
    }
    return false;
}

void M34CryWolf1st::MoveCryWolf1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    //CreateEffect ( MODEL_STONE2,o->Position,o->Angle,o->Light);
    vec3_t Angle, Position;
    float Matrix[3][4];
    vec3_t  p, p2, EndPos;
    vec3_t  TempAngle;
    switch (o->Type)
    {
    case MODEL_SORAM:
    {
        vec3_t  Light;
        Vector(1.0f, 1.0f, 1.0f, Light);

        /*
        Vector(0.0f,  0.0f, 0.0f, p);
        b->TransformPosition(o->BoneTransform[27],p,EndPos,true);
        CreateParticle(BITMAP_FIRE,EndPos,o->Angle,Light,0,2.0f);
*/

        if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            //				vec3_t StartPos;//, StartRelative;
            vec3_t EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 100.f, -150.f, EndRelative);

                b->TransformPosition(BoneTransform[16], EndRelative, EndPos, false);

                if (o->AnimationFrame > 5.0f && o->AnimationFrame < 7.0f)
                {
                    CreateParticle(BITMAP_FIRE, EndPos, o->Angle, Light);
                    //						CreateParticle(BITMAP_BLUE_BLUR,EndPos, o->Angle, Light,1);
                }

                Vector(0.f, -150.f, 0.f, p);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(Position, o->Position, p2);

                o->Angle[2] -= 18;//

                Vector((float)(rand() % 60 + 60 - 90), 0.f, (float)(rand() % 30 + 90), Angle);
                VectorAdd(Angle, o->Angle, Angle);
                VectorCopy(p2, Position);
                //					Position[0] += rand()%20-10;
                //					Position[1] += rand()%20-10;

                                    /*
                                    b->TransformPosition(BoneTransform[39],StartRelative,StartPos,false);
                                    b->TransformPosition(BoneTransform[41],EndRelative,EndPos,false);
                                    CreateBlur(c,StartPos,EndPos,Light,3,true,24);

                                    b->TransformPosition(BoneTransform[43],StartRelative,StartPos,false);
                                    b->TransformPosition(BoneTransform[45],EndRelative,EndPos,false);
                                    CreateBlur(c,StartPos,EndPos,Light,3,true,25);
                */
                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);
        }
    }
    break;
    case MODEL_SCOUT:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            vec3_t  Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                //					Vector(0.f, 0.f, -60.f, StartRelative);
                //					Vector(0.f, 0.f, -150.f, EndRelative);
                Vector(20.f, 0.f, 0.f, StartRelative);
                Vector(60.f, 0.f, 0.f, EndRelative);
                //					Vector(20.f, 0.f, 0.f, StartRelative);
                //					Vector(60.f, 0.f, 0.f, EndRelative);

                b->TransformPosition(BoneTransform[12], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[16], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 23);

                b->TransformPosition(BoneTransform[20], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[24], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 24);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_BALGASS:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            vec3_t  Light;
            Vector(1.f, 1.f, 1.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;

            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, -60.f, StartRelative);
                Vector(0.f, 0.f, -150.f, EndRelative);
                b->TransformPosition(BoneTransform[c->Weapon[0].LinkBone], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[c->Weapon[0].LinkBone], EndRelative, EndPos, false);

                if (o->AnimationFrame > 3.5f && o->AnimationFrame < 4.5f)
                {
                    CreateParticle(BITMAP_BLUE_BLUR, EndPos, o->Angle, Light, 0);
                }
                Vector(0.f, 0.f, 5.f, Light);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);

                /*
                                    Vector(0.f,-150.f,0.f,p);
                                    AngleMatrix(o->Angle,Matrix);
                                    VectorRotate(EndPos,Matrix,Position);
                                    VectorAdd(Position,o->Position,p2);
                //					o->Angle[2] -= 18;//
                                    Vector((float)(rand()%60+60-90),0.f,(float)(rand()%30+90),Angle);
                                    VectorAdd(Angle,o->Angle,Angle);
                                    VectorCopy(p2,Position);
                                    Position[0] += rand()%20-10;
                                    Position[1] += rand()%20-10;
                                    if(o->AnimationFrame > 3.5f && o->AnimationFrame < 4.5f)
                                    CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
                */
                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);
        }
    }
    break;

    case MODEL_DARK_ELF_1:
    {
        if ((o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(1.f, 1.f, 1.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;

            if (o->CurrentAction == MONSTER01_ATTACK2 && (o->AnimationFrame > 4.5f && o->AnimationFrame < 5.0f))
                CreateEffect(MODEL_DARK_ELF_SKILL, o->Position, o->Angle, o->Light, 2, o);

            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, -60.f, StartRelative);
                Vector(0.f, 0.f, -150.f, EndRelative);
                b->TransformPosition(BoneTransform[c->Weapon[0].LinkBone], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[c->Weapon[0].LinkBone], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);

                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            vec3_t Position, Light;
            BoneManager::GetBonePosition(o, L"Left_Hand", Position);

            Vector(0.2f, 0.2f, 0.7f, Light);
            CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light, 27, 1.0f);
            Vector(0.3f, 0.3f, 0.4f, Light);
            CreateParticle(BITMAP_LIGHT + 1, Position, o->Angle, Light, 2, 0.8f);
            CreateParticle(BITMAP_LIGHT + 1, Position, o->Angle, Light, 2, 0.6f);
        }
    }
    break;
    case MODEL_DEATH_SPIRIT:
    {
        if ((o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  Light;
            Vector(0.2f, 1.f, 0.4f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;

            VectorCopy(o->Angle, TempAngle);
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 250.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[27], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);

                fAnimationFrame += fSpeedPerFrame;
            }
            VectorCopy(TempAngle, o->Angle);

            if (o->AnimationFrame > 4.5f && o->AnimationFrame < 5.0f)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;
                vec3_t Angle = { 0.f, 0.f, o->Angle[2] };
                vec3_t Pos = { 0.f, 0.f, (to->BoundingBoxMax[2] / 1.f) };

                Vector(80.f, 0.f, 20.f, p);
                b->TransformPosition(o->BoneTransform[0], p, Position, true);
                Position[2] += 50.0f;
                Angle[2] = o->Angle[2] + 90;
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 0, to);
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 1, to);
                Angle[2] = o->Angle[2] - 90;
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 0, to);
                CreateEffect(MODEL_DEATH_SPI_SKILL, Position, Angle, Pos, 1, to);
            }
        }
    }
    break;
    case MODEL_WEREWOLF_HERO:
    {
        if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            vec3_t  Light;
            Vector(1.f, 1.f, 1.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, -90.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[80], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[80], EndRelative, EndPos, false);

                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);

                Vector(0.f, 0.f, 90.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[82], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[82], EndRelative, EndPos, false);

                CreateBlur(c, StartPos, EndPos, Light, 3, true, 84);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_SOLAM:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            vec3_t  Light;
            Vector(1.f, 1.f, 1.f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++) {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                Vector(0.f, 0.f, 120.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                b->TransformPosition(BoneTransform[25], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);

                CreateBlur(c, StartPos, EndPos, Light, 3, true, 25);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}

bool M34CryWolf1st::RenderCryWolf1stMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_SORAM:
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->BeginRender(o->Alpha);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
        b->EndRender();
        return true;
    }
    break;
    case MODEL_BALGASS:
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->BeginRender(o->Alpha);
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(i, RENDER_CHROME2 | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME3);
        }
        b->EndRender();
        return true;
    }
    break;
    case MODEL_BALRAM:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
        return true;
    }
    break;
    case MODEL_DEATH_SPIRIT:
    {
        b->BeginRender(o->Alpha);
        static float aaa = 0.f;
        aaa += 0.1f;
        if (aaa > 1.0f)
            aaa = 0.f;
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            if (i == 2)
            {
                if (o->Alpha > 0.3f)
                    o->Alpha = 0.3f;

                o->BlendMesh = -2;
                b->EndRender();
                b->BeginRender(o->Alpha);
                Models[o->Type].StreamMesh = i;
            }
            else
                o->BlendMesh = -1;

            o->BlendMeshTexCoordU = (int)WorldTime % 10000 * 0.0005f;
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        b->EndRender();
        o->Alpha = 1.0f;
        return true;
    }
    break;
    case MODEL_SCOUT:
    {
        if (ExtraMon)
        {
            Vector(0.5f, 0.5f, 0.6f, b->BodyLight);
            b->BeginRender(o->Alpha);
            for (int i = 0; i < Models[o->Type].NumMeshs; i++)
            {
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
            }
            b->EndRender();
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
        return true;
    }
    break;
    case MODEL_WEREWOLF_HERO:
    {
        if (ExtraMon)
        {
            Vector(0.5f, 0.5f, 0.8f, b->BodyLight);
            b->BeginRender(o->Alpha);
            for (int i = 0; i < Models[o->Type].NumMeshs; i++)
            {
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
            }
            b->EndRender();
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
        return true;
    }
    break;
    case MODEL_TANTALLOS:
    {
        Vector(0.6f, 0.8f, 0.6f, b->BodyLight);
        b->BeginRender(o->Alpha);
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        b->EndRender();
        return true;
    }
    break;
    case MODEL_BLOODY_WOLF:
    {
        Vector(0.6f, 0.8f, 0.6f, b->BodyLight);
        b->BeginRender(o->Alpha);
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        b->EndRender();
        return true;
    }
    break;
    case MODEL_BEAM_KNIGHT:	//318
    {
        Vector(0.6f, 0.8f, 0.6f, b->BodyLight);
        b->BeginRender(o->Alpha);
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        b->EndRender();
        return true;
    }
    break;
    case MODEL_DRAGON_:	//319
    {
        Vector(0.6f, 0.8f, 0.6f, b->BodyLight);
        b->BeginRender(o->Alpha);
        for (int i = 0; i < Models[o->Type].NumMeshs; i++)
        {
            b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        b->EndRender();
        return true;
    }
    break;
    }
    return false;
}

bool M34CryWolf1st::RenderCryWolf1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!IsCyrWolf1st() && !(gMapManager.InDevilSquare()))
        return false;

    switch (o->Type)
    {
    case MODEL_CRYWOLF_ALTAR1:
    case MODEL_CRYWOLF_ALTAR2:
    case MODEL_CRYWOLF_ALTAR3:
    case MODEL_CRYWOLF_ALTAR4:
    case MODEL_CRYWOLF_ALTAR5:
    {
        vec3_t Light;
        float fRotation1, fRotation2;
        float Luminosity = sinf(WorldTime * 0.002f) * 0.1f + 0.28f;
        float Luminosity2 = sinf(WorldTime * 0.002f) * 0.04f + 0.2f;

        EnableAlphaBlend();

        if (g_isCharacterBuff(o, eBuff_CrywolfAltarContracted))
        {
            Vector(Luminosity2, Luminosity2, 0.05f, Light);
            fRotation1 = WorldTime * 0.01f;
            fRotation2 = -WorldTime * 0.01f;
            Vector(0.09f, 0.09f, 0.04f, o->Light);

            CreateParticleFpsChecked(BITMAP_EFFECT, o->Position, o->Angle, o->Light);
            CreateParticleFpsChecked(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 1);
        }
        if (g_isCharacterBuff(o, eBuff_CrywolfAltarOccufied))
        {
            Vector(Luminosity, 0.1f, 0.1f, Light);
        }
        if (g_isCharacterBuff(o, eBuff_CrywolfAltarDisable))
        {
            Vector(Luminosity, 0.1f, 0.1f, Light);
        }
        if (g_isCharacterBuff(o, eBuff_CrywolfAltarEnable) || g_isCharacterBuff(o, eBuff_CrywolfAltarAttempt))
        {
            fRotation1 = WorldTime * 0.01f;
            Vector(0.15f, 0.15f, Luminosity, Light);
        }

        Vector(Light[0] * 2.0f, Light[1] * 2.0f, Light[2] * 2.0f, Light);

        RenderTerrainAlphaBitmap(BITMAP_MAGIC_CIRCLE, o->Position[0], o->Position[1], 2.8f, 2.8f, Light, fRotation1);
        RenderTerrainAlphaBitmap(BITMAP_MAGIC_CIRCLE, o->Position[0], o->Position[1], 3.6f, 3.6f, Light, fRotation2);
        DisableAlphaBlend();

        if (fRotation1 >= 360.0f) fRotation1 = 0.0f;
        if (fRotation2 >= 360.0f) fRotation2 = 0.0f;
    }
    break;
    case MODEL_BALGASS:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_BALGAS_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALGAS_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALGAS_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALGAS_SKILL1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK4)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALGAS_SKILL2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALGAS_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    break;
    case MODEL_DARK_ELF_1:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_DARKELF_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_SKILL1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK4)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_SKILL2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DARKELF_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    break;
    case MODEL_BALLISTA:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_TANKER_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_TANKER_DIE);
            }
        }

        if (!(o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_DIE))
            o->SubType = FALSE;
    }
    break;
    case MODEL_DEATH_SPIRIT:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_DEATHSPIRIT_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DEATHSPIRIT_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DEATHSPIRIT_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_DEATHSPIRIT_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;

        vec3_t Position, Light;
        int dummy = rand() % 14;
        auto Data = (float)((float)dummy / (float)100);
        auto Rot = (float)(rand() % 360);
        Vector(1.0f, 1.0f, 1.0f, Light);
        BoneManager::GetBonePosition(o, L"Monster94_zx", Position);
        CreateSprite(BITMAP_DS_EFFECT, Position, 1.5f, Light, o);
        Vector(0.3f, 0.3f, 0.7f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 3.5f + (Data * 5), Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 3.5f + (Data * 5), Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 3.5f + (Data * 5), Light, o);
        CreateSprite(BITMAP_DS_SHOCK, Position, 0.8f + Data, Light, o, Rot);

        BoneManager::GetBonePosition(o, L"Monster94_zx01", Position);
        Vector(0.1f, 0.0f, 0.6f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.8f, Light, o, Rot);
        if (rand_fps_check(2))
        {
            Vector(0.7f, 0.7f, 1.0f, Light);
            CreateSprite(BITMAP_SHINY + 1, Position, 0.8f, Light, o, 360.f - Rot);
        }
        Vector(0.3f, 0.3f, 0.7f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
    }
    return true;
    case MODEL_BALRAM:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_BALRAM_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALRAM_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALRAM_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_BALRAM_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    return true;
    case MODEL_SORAM:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_SORAM_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SORAM_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SORAM_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SORAM_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    return true;
    case MODEL_WEREWOLF_HERO:
    {
        vec3_t Position, Light;

        if (o->CurrentAction != MONSTER01_DIE) {
            Vector(0.9f, 0.2f, 0.1f, Light);
            BoneManager::GetBonePosition(o, L"Monster95_Head", Position);
            CreateSprite(BITMAP_LIGHT, Position, 3.5f, Light, o);
        }

        Vector(0.9f, 0.2f, 0.1f, Light);
        //. Walking & Running Scene Processing
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(10)) {
                CreateParticle(BITMAP_SMOKE + 1, o->Position, o->Angle, Light);
            }
        }

        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_WWOLF_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_WWOLF_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_WWOLF_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_WWOLF_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    return true;
    case MODEL_SOLAM:
    {
        vec3_t Position, Light;

        if (o->CurrentAction == MONSTER01_ATTACK2 && o->AnimationFrame >= 4.0f && o->AnimationFrame <= 6.0f)
        {
            float Matrix[3][4];
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(o->Direction, Matrix, Position);

            Vector(1.f, 0.0f, 0.5f, Light);
            CreateEffect(MODEL_PIERCING2, o->Position, o->Angle, Light, 1);
        }
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_SCOUT2_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT2_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT2_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT2_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    return true;
    case MODEL_VALAM:
    {
        vec3_t Position, Light;

        auto Rotation = (float)(rand() % 360);
        float Luminosity = sinf(WorldTime * 0.0012f) * 0.8f + 1.3f;

        float fScalePercent = 1.f;
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
            fScalePercent = .5f;

        BoneManager::GetBonePosition(o, L"Monster96_Center", Position);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 0.1f, Light);
        CreateSprite(BITMAP_LIGHT, Position, fScalePercent, Light, o);

        Vector(0.5f, 0.5f, 0.5f, Light);

        BoneManager::GetBonePosition(o, L"Monster96_Top", Position);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, 360.f - Rotation);

        BoneManager::GetBonePosition(o, L"Monster96_Bottom", Position);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f * fScalePercent, Light, o, 360.f - Rotation);

        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_SCOUT3_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT3_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT3_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT3_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    return true;
    case MODEL_SCOUT:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_CRY1ST_SCOUT1_MOVE1 + rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT1_ATTACK1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT1_ATTACK2);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_CRY1ST_SCOUT1_DIE);
            }
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    return true;
    }
    return false;
}

bool M34CryWolf1st::CreateMist(PARTICLE* pParticleObj)
{
    if (!IsCyrWolf1st())
        return false;

    if (rand_fps_check(30)) {
        vec3_t Light;
        Vector(0.07f, 0.07f, 0.07f, Light);
        int ff = 200.0f;

        vec3_t TargetPosition = { 0.f, 0.f, 400.f }, TargetAngle = { 0.f, 0.f, 0.f };
        switch (rand() % 8)
        {
        case 0:
            TargetPosition[0] = Hero->Object.Position[0] + (300 - ff + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] + (300 - ff + rand() % 250);
            break;
        case 1:
            TargetPosition[0] = Hero->Object.Position[0] + (250 - ff + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] - (250 - ff + rand() % 250);
            break;
        case 2:
            TargetPosition[0] = Hero->Object.Position[0] - (200 - ff + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] + (200 - ff + rand() % 250);
            break;
        case 3:
            TargetPosition[0] = Hero->Object.Position[0] - (300 - ff + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1] - (300 - ff + rand() % 250);
            break;
        case 4:
            TargetPosition[0] = Hero->Object.Position[0] + (400 - ff + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1];
            break;
        case 5:
            TargetPosition[0] = Hero->Object.Position[0] - (400 - ff + rand() % 250);
            TargetPosition[1] = Hero->Object.Position[1];
            break;
        case 6:
            TargetPosition[0] = Hero->Object.Position[0];
            TargetPosition[1] = Hero->Object.Position[1] + (400 - ff + rand() % 250);
            break;
        case 7:
            TargetPosition[0] = Hero->Object.Position[0];
            TargetPosition[1] = Hero->Object.Position[1] - (400 - ff + rand() % 250);
            break;
        }

        if (Hero->Movement)
        {
            float Matrix[3][4];
            AngleMatrix(Hero->Object.Angle, Matrix);
            vec3_t Velocity, Direction;
            Vector(0.f, -45.f * CharacterMoveSpeed(Hero), 0.f, Velocity);
            VectorRotate(Velocity, Matrix, Direction);
            VectorAddScaled(TargetPosition, Direction, TargetPosition, FPS_ANIMATION_FACTOR);
        }
        if (Hero->Movement || (rand_fps_check(2)))
        {
            CreateParticle(BITMAP_CLOUD, TargetPosition, TargetAngle, Light, 8, 0.4f);
        }
        pParticleObj->Live = false;

        return true;
    }

    return false;
}

void M34CryWolf1st::RenderBaseSmoke(void)
{
    EnableAlphaTest();

    glColor3f(0.4f, 0.4f, 0.45f);
    float WindX2 = (float)((int)WorldTime % 100000) * 0.0005f;
    RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, 0.f, 3.f, 2.f);
    EnableAlphaBlend();
    float WindX = (float)((int)WorldTime % 100000) * 0.0002f;
    RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX, 0.f, 0.3f, 0.3f);
}

bool M34CryWolf1st::Get_State()
{
    if (m_CrywolfState == CRYWOLF_STATE_START)
        return true;
    return false;
}

bool M34CryWolf1st::Get_State_Only_Elf()
{
    if (m_CrywolfState == CRYWOLF_STATE_START || m_CrywolfState == CRYWOLF_STATE_READY)
        return true;
    return false;
}

void M34CryWolf1st::SetTime(BYTE byHour, BYTE byMinute)
{
    if (M34CryWolf1st::Get_State() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return;
    m_iHour = byHour;
    m_iMinute = byMinute;
    m_dwSyncTime = GetTickCount();

    if (TimeStart == false)
        TimeStart = true;
}

void M34CryWolf1st::Set_BossMonster(int Val_Hp, int Dl_Num)
{
    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return;
    Dark_elf_Num = Dl_Num;

    if (Val_Hp > 0 && View_Bal == false)
        View_Bal = true;
    else
        if (Val_Hp <= 0 && View_Bal == true)
            View_Bal = false;

    Set_Val_Hp(Val_Hp);
}

void M34CryWolf1st::Check_AltarState(int Num, int State)
{
    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return;
    m_AltarState[Num - 1] = State;
}

void M34CryWolf1st::Set_Message_Box(int Str, int Num, int Key, int ObjNum)
{
    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false || LogOut)
        return;

    if (Str == 56)
    {
        BYTE State = (m_AltarState[ObjNum] & 0x0f);
        swprintf(Box_String[Num], GlobalText[1950 + Str], State);
    }
    else
        wcscpy(Box_String[Num], GlobalText[1950 + Str]);
    if (Str == 56 || Str == 57)
        Message_Box = 1;
    else
        Message_Box = 2;
    if (Num == 0)
        Box_String[Num + 1][0] = NULL;

    BackUp_Key = Key;
}

void M34CryWolf1st::MoveMvp_Interface()
{
    int Yes = 250;
    int No = 330;

    if (M34CryWolf1st::IsCyrWolf1st() == false)
        return;
    /*
    int AltarContract_Loc[5][2] = {{125,27},{126,35},{120,38},{115,35},{117,27}};

    for(int i = 0; i < 5; i++)
    {
        int Key = 205+i;
        if((Hero->PositionX) == AltarContract_Loc[i][0] && (Hero->PositionY) == AltarContract_Loc[i][1] && Key != BackUp_Key)
        {
            Set_Message_Box(0,0,Key);
            break;
        }
    }
*/
    if (View_End_Result == true)
    {
        if (MouseX > 300 && MouseX < 300 + 54 && MouseY > 300 && MouseY < 300 + 30)
        {
            if (MouseLButton == true)
            {
                CryWolfMVPInit();
                Button_Down = 5;
            }
        }
        if (MouseLButton == true)
        {
            MouseLButton = false;
            MouseLButtonPush = false;
        }
    }

    if (Message_Box == 1)
    {
        if (MouseX > No && MouseX < No + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (MouseLButton == true)
            {
                Button_Down = 1;
                Set_Message_Box(2, 0, 0);
            }
        }
        if (MouseX > Yes && MouseX < Yes + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (MouseLButton == true)
            {
                if (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA
                    || Hero->Helper.Type == MODEL_HORN_OF_DINORANT
                    || Hero->Helper.Type == MODEL_HORN_OF_FENRIR
                    )
                {
                    Set_Message_Box(6, 0, 0);
                }
                else
                {
                    Button_Down = 2;
                    SocketClient->ToGameServer()->SendCrywolfContractRequest(BackUp_Key);
                }
            }
        }
        if (MouseLButton == true)
        {
            MouseLButton = false;
            MouseLButtonPush = false;
        }
    }
    else
        if (Message_Box == 2)
        {
            if (MouseX > 290 && MouseX < 290 + 54 && MouseY > 250 && MouseY < 250 + 30)
            {
                if (MouseLButton == true)
                {
                    MouseLButton = false;
                    MouseLButtonPush = false;
                    Button_Down = 3;
                }
            }
        }
}

void M34CryWolf1st::Sub_Interface()
{
    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return;

    int Yes = 250;
    int No = 330;

    g_pRenderText->SetTextColor(255, 148, 21, 255);
    g_pRenderText->SetBgColor(0x00000000);

    if (Message_Box == 1)
    {
        g_pCryWolfInterface->Render(212, 206, 209, 80, 0.f, 0.f, 207.f / 256.f, 77.f / 128.f, 22);
        if (MouseX > No && MouseX < No + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (Button_Down == 1)
            {
                g_pCryWolfInterface->Render(No, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 15);
                Message_Box = 0;
                Button_Down = 0;
            }
            else
            {
                g_pCryWolfInterface->Render(No, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 14);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(No, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 13);
        }

        if (MouseX > Yes && MouseX < Yes + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (Button_Down == 2)
            {
                g_pCryWolfInterface->Render(Yes, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 21);
                Message_Box = 0;
                Button_Down = 0;
            }
            else
            {
                g_pCryWolfInterface->Render(Yes, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 20);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(Yes, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 19);
        }

        if (Box_String[0][0] != NULL)
        {
            int Y_loc = 239;
            if (Box_String[1][0] != NULL)
                Y_loc = 227;
            g_pRenderText->RenderText(317, Y_loc, Box_String[0], 0, 0, RT3_WRITE_CENTER);
        }
        if (Box_String[1][0] != NULL)
            g_pRenderText->RenderText(317, 238, Box_String[1], 0, 0, RT3_WRITE_CENTER);
    }
    else if (Message_Box == 2)
    {
        g_pCryWolfInterface->Render(212, 206, 209, 80, 0.f, 0.f, 207.f / 256.f, 77.f / 128.f, 22);

        if (MouseX > 290 && MouseX < 290 + 54 && MouseY > 250 && MouseY < 250 + 30)
        {
            if (Button_Down == 3)
            {
                g_pCryWolfInterface->Render(290, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 18);

                Message_Box = 0;
                Button_Down = 0;
            }
            else
            {
                g_pCryWolfInterface->Render(290, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 17);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(290, 250, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 16);
        }

        if (Box_String[0][0] != NULL)
        {
            int Y_loc = 239;
            if (Box_String[1][0] != NULL)
                Y_loc = 227;
            g_pRenderText->RenderText(317, Y_loc, Box_String[0], 0, 0, RT3_WRITE_CENTER);
        }
        if (Box_String[1][0] != NULL)
            g_pRenderText->RenderText(317, 238, Box_String[1], 0, 0, RT3_WRITE_CENTER);
    }
}

void M34CryWolf1st::Set_Hp(int State)
{
    m_StatueHP = State;
}

void M34CryWolf1st::Set_Val_Hp(int State)
{
    Val_Hp = State;
}

bool M34CryWolf1st::Get_AltarState_State(int Num)
{
    BYTE Use = (m_AltarState[Num] & 0xf0) >> 4;
    //	BYTE State = (m_AltarState[Num] & 0x0f);
    if (Use == CRYWOLF_ALTAR_STATE_CONTRACTED)
        return true;
    else
        return false;
}

bool M34CryWolf1st::Render_Mvp_Interface()
{
    if (M34CryWolf1st::IsCyrWolf1st() == false)
        return false;

    EnableAlphaTest();

    wchar_t Text[300];

    float Main[] = { 518.f,278.f,122.f,119.f,120.f / 128.f,118.f / 128.f };
    float Number[5][6] = { {565.f,280.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {582.f,282.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {598.f,286.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {613.f,294.f,13.f,13.f,12.f / 16.f,12.f / 16.f},
                            {625.f,306.f,13.f,13.f,12.f / 16.f,12.f / 16.f} };
    float bar[] = { 0.f,98.f,97.f,21.f,97.f / 128.f,21.f / 32.f };
    float Dark_Elf_Icon[] = { 623.f,358.f,15.f,15.f,14.f / 16.f,14.f / 16.f };
    float Val_Icon[] = { 4.f,101.f,15.f,15.f,14.f / 16.f,14.f / 16.f };

    int TotDelay = 400;

    if (View_End_Result == true)
    {
        g_pCryWolfInterface->Render(200, 110, 252, 240, 0.f, 0.f, 252.f / 256.f, 323.f / 512.f, 44);
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(255, 148, 21, 255);
        g_pRenderText->SetBgColor(0x00000000);

        swprintf(Text, L"%s", GlobalText[680]);
        g_pRenderText->RenderText(240, 160, Text, 0, 0, RT3_WRITE_CENTER);
        swprintf(Text, L"%s", GlobalText[681]);
        g_pRenderText->RenderText(285, 160, Text, 0, 0, RT3_WRITE_CENTER);
        swprintf(Text, L"%s", GlobalText[1973]);
        g_pRenderText->RenderText(333, 160, Text, 0, 0, RT3_WRITE_CENTER);
        swprintf(Text, L"%s", GlobalText[1977]);
        g_pRenderText->RenderText(387, 160, Text, 0, 0, RT3_WRITE_CENTER);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(0x00000000);

        int icntIndex = 0;
        for (int i = 0; i < 5; i++)
        {
            icntIndex = i;
            if (HeroScore[i] == -1)
                continue;

            swprintf(Text, L"%d", i + 1);
            g_pRenderText->RenderText(240, 175 + i * 15, Text, 0, 0, RT3_WRITE_CENTER);
            swprintf(Text, L"%s", HeroName[i]);
            g_pRenderText->RenderText(285, 175 + i * 15, Text, 0, 0, RT3_WRITE_CENTER);

            swprintf(Text, L"%s", gCharacterManager.GetCharacterClassText(HeroClass[i]));

            g_pRenderText->RenderText(335, 175 + i * 15, Text, 0, 0, RT3_WRITE_CENTER);
            swprintf(Text, L"%d", HeroScore[i]);
            g_pRenderText->RenderText(385, 175 + i * 15, Text, 0, 0, RT3_WRITE_CENTER);
        }

        g_pRenderText->SetTextColor(255, 0, 255, 255);
        g_pRenderText->SetBgColor(0);

        if (View_Suc_Or_Fail == 1)
        {
            g_pRenderText->RenderText(330, 175 + icntIndex * 17, GlobalText[2000], 0, 0, RT3_WRITE_CENTER); icntIndex++;
            g_pRenderText->RenderText(328, 175 + icntIndex * 17, GlobalText[2001], 0, 0, RT3_WRITE_CENTER);
        }
        else
        {
            g_pRenderText->RenderText(330, 175 + icntIndex * 17, GlobalText[2009], 0, 0, RT3_WRITE_CENTER);
        }

        if (MouseX > 300 && MouseX < 300 + 54 && MouseY > 300 && MouseY < 300 + 30)
        {
            if (Button_Down == 5)
            {
                g_pCryWolfInterface->Render(300, 300, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 21);
                Message_Box = 0;
                Button_Down = 0;
                View_End_Result = false;
            }
            else
            {
                g_pCryWolfInterface->Render(300, 300, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 20);
            }
        }
        else
        {
            g_pCryWolfInterface->Render(300, 300, 54, 30, 0.f, 0.f, 54.f / 64.f, 30.f / 32.f, 19);
        }
    }

    if (Suc_Or_Fail == 1)
    {
        Delay++;
        if (Delay >= TotDelay)
        {
            Delay = 0;
            Suc_Or_Fail = 0;
        }
    }

    if (Suc_Or_Fail >= 0)
    {
        float A_Value = 0.f;
        int aa = (Delay * 2) % 140;

        if (aa > 70)
            A_Value = 1.f - ((aa - 70) * 0.01f);
        else
            A_Value = 0.3f + (aa * 0.01f);

        if (Delay_Add_inter <= 0)
            Delay_Add_inter = 0;
        else
            Delay_Add_inter -= 15;

        if ((Delay * 15) > 479)
        {
            g_pCryWolfInterface->Render(150, 50, 329, 94, 0.f, 0.f, 328.f / 512.f, 93.f / 128.f, Add_Num, false, false, A_Value);
        }
        else if (Suc_Or_Fail == 0)
        {
            g_pCryWolfInterface->Render(150 + (Delay * 15), 50, 329, 94, 0.f, 0.f, 329.f / 512.f, 94.f / 128.f, Add_Num, false, false, A_Value);

            Delay++;
            if ((Delay * 15) > 479)
            {
                Delay = 0;
                Suc_Or_Fail = -1;

                Delay_Add_inter = 390;
                View_End_Result = true;
            }
        }
        else
        {
            g_pCryWolfInterface->Render(-329 + (Delay * 15), 50, 329, 94, 0.f, 0.f, 329.f / 512.f, 94.f / 128.f, Add_Num, false, false, A_Value);
        }

        g_pCryWolfInterface->Render(230, 150, 196, 141, 0.f, 0.f, 195.f / 256.f, 140.f / 256.f, 26);
        g_pCryWolfInterface->Render(250 + Delay_Add_inter, 188, 110, 28, 0.f, 0.f, 110.f / 128.f, 27.f / 32.f, 27);

        if (Delay_Add_inter == 0)
        {
            int Exp_val[9] = { 0,0,0,0,0,0,0,0,0 }, Exp_Dummy = 0, Val = 0;

            g_pCryWolfInterface->Render(250 + 120, 188, 29, 28, 0.f, 0.f, 29.f / 32.f, 27.f / 32.f, 28 + Rank);

            for (int i = 0; i < 9; i++)
            {
                if (Exp >= Val)
                {
                    if (Val > 0)
                    {
                        Exp_Dummy = Exp / Val;
                        Exp_val[8 - i] = Exp_Dummy % 10;
                    }
                    else
                    {
                        Exp_val[8 - i] = Exp % 10;
                        Val = 1;
                    }
                }
                else
                    break;

                Val *= 10;
            }
            int Move_X = 29;
            g_pCryWolfInterface->Render(200 + Move_X, 235, 60, 19, 0.f, 0.f, 60.f / 64.f, 19.f / 32.f, 43);
            g_pCryWolfInterface->Render(250 + 130 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[8]);
            g_pCryWolfInterface->Render(250 + 115 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[7]);
            g_pCryWolfInterface->Render(250 + 100 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[6]);
            g_pCryWolfInterface->Render(250 + 85 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[5]);
            g_pCryWolfInterface->Render(250 + 70 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[4]);
            g_pCryWolfInterface->Render(250 + 55 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[3]);
            g_pCryWolfInterface->Render(250 + 40 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[2]);
            g_pCryWolfInterface->Render(250 + 25 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[1]);
            g_pCryWolfInterface->Render(250 + 10 + Move_X, 235, 15, 19, 0.f, 0.f, 15.f / 16.f, 19.f / 32.f, 33 + Exp_val[0]);
        }
    }

    if (M34CryWolf1st::Get_State_Only_Elf() == false || M34CryWolf1st::IsCyrWolf1st() == false)
        return false;

    g_pCryWolfInterface->Render(Main[0], Main[1], Main[2], Main[3], 0.f, 0.f, Main[4], Main[5], 3);

    for (int ia = 0; ia < 5; ia++)
    {
        BYTE Use = (m_AltarState[ia] & 0xf0) >> 4;
        BYTE State = (m_AltarState[ia] & 0x0f);
        if (Use == CRYWOLF_ALTAR_STATE_CONTRACTED)
        {
            if (State == 1)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 23);
            }
            else if (State == 2)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 24);
            }
            else
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 25);
            }
        }
        else
        {
            if (State == 1)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 7);
            }
            else if (State == 2)
            {
                g_pCryWolfInterface->Render(Number[ia][0], Number[ia][1], Number[ia][2], Number[ia][3], 0.f, 0.f, Number[ia][4], Number[ia][5], 8);
            }
        }
    }

    if (Dark_elf_Num == 0)
    {
        g_pCryWolfInterface->Render(Dark_Elf_Icon[0], Dark_Elf_Icon[1], Dark_Elf_Icon[2], Dark_Elf_Icon[3], 0.f, 0.f, Dark_Elf_Icon[4], Dark_Elf_Icon[5], 6);
    }
    else
    {
        g_pCryWolfInterface->Render(Dark_Elf_Icon[0], Dark_Elf_Icon[1], Dark_Elf_Icon[2], Dark_Elf_Icon[3], 0.f, 0.f, Dark_Elf_Icon[4], Dark_Elf_Icon[5], 5);
    }

    g_pCryWolfInterface->Render(538, 392, 104, 37, 0.f, 0.f, 104.f / 128.f, 36.f / 64.f, 12);
    glColor3f(1.f, 0.6f, 0.3f);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 148, 21, 255);
    g_pRenderText->SetBgColor(0);
    swprintf(Text, GlobalText[1948], Dark_elf_Num);
    g_pRenderText->RenderText(582, 359, Text, 0, 0, RT3_WRITE_CENTER);

    if (View_Bal == true)
    {
        if (Deco_Insert < 21.f)
            Deco_Insert += 1.f;
        else
        {
            g_pCryWolfInterface->Render(bar[0], bar[1], bar[2], bar[3], 0.f, 0.f, bar[4], bar[5], 0);
            g_pCryWolfInterface->Render(Val_Icon[0], Val_Icon[1], Val_Icon[2], Val_Icon[3], 0.f, 0.f, Val_Icon[4], Val_Icon[5], 4);
            swprintf(Text, GlobalText[1949]);
            g_pRenderText->RenderText(38, 101, Text, 0, 0, RT3_WRITE_CENTER);

            float Hp = ((67.f / 100.f) * (float)Val_Hp);
            float nx = ((68.f / 100.f) * (float)Val_Hp);
            g_pCryWolfInterface->Render(25, 109, nx, 8, 0.f, 0.f, Hp / 128.f, 8.f / 8.f, 1);
        }
    }
    if (View_Bal == false)
    {
        if (Deco_Insert > 0.f)
            Deco_Insert -= 1.f;
    }

    if (TimeStart = true)
    {
        int nPastHour = m_iHour;
        int nPastMinute;

        if (m_iMinute > 0)
        {
            int Time_Add = 0;
            float Tim = GetTickCount() - m_dwSyncTime;
            nPastMinute = m_iMinute - (GetTickCount() - m_dwSyncTime) / 60000;

            Time_Add = (Tim / 60000) * 60;

            if (BackUpTick != Time_Add)
            {
                BackUpTick = Time_Add;
                nPastTick++;
                if (nPastTick >= 60)
                    nPastTick = 0;
            }
        }
        else nPastMinute = 0;

        if (nPastMinute < 0)
        {
            nPastMinute = nPastMinute + 60;
            --nPastHour;
        }

        if (BackUpMin != nPastMinute)
        {
            BackUpMin = nPastMinute;
            nPastTick = 0;
        }
        if (View_Bal == false)
            glColor3f(1.f, 1.0f, 1.0f);
        else
            glColor3f(1.f, 0.3f, 0.3f);
        RenderNumber2D(510 + 60, 384 + 18, nPastMinute, 14, 14);
        if (nPastTick == 0)
            RenderNumber2D(520 + 87, 384 + 18, nPastTick, 14, 14);
        else
            RenderNumber2D(520 + 87, 384 + 18, 60 - nPastTick, 14, 14);
    }

    float Hp = ((88.f / 100.f) * (float)m_StatueHP);
    float nx = ((89.f / 100.f) * (float)m_StatueHP);

    g_pCryWolfInterface->Render(548 + (89.f - nx), 323, nx, 30, (Hp / 128.f), 0.f, Hp / 128.f, 29.f / 32.f, 9);

    RenderNoticesCryWolf();
    DisableAlphaBlend();
    return true;
}

bool M34CryWolf1st::SetCurrentActionCrywolfMonster(CHARACTER* c, OBJECT* o)
{
    if (!IsCyrWolf1st() && !(gMapManager.InDevilSquare()))
        return false;

    switch (c->MonsterIndex)
    {
    case MONSTER_DARK_ELF:
    case MONSTER_DARKELF:
    case MONSTER_BALRAM:
    case MONSTER_DEATH_SPIRIT:
    case MONSTER_SORAM:
    case MONSTER_BALGASS:
        return CheckMonsterSkill(c, o);
    }
    return false;
}

void M34CryWolf1st::Set_MyRank(BYTE MyRank, int GettingExp)
{
    Rank = MyRank;
    Exp = GettingExp;
}

void M34CryWolf1st::Set_WorldRank(BYTE Rank, CLASS_TYPE Class, int Score, wchar_t* szHeroName)
{
    HeroScore[Rank] = Score;
    HeroClass[Rank] = Class;
    memcpy(HeroName[Rank], szHeroName, MAX_ID_SIZE);
    HeroName[Rank][MAX_ID_SIZE] = NULL;
}