// SummonSystem.cpp: implementation of the CSummonSystem class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SummonSystem.h"

#include "DSPlaySound.h"
#include "zzzEffect.h"

#include "ZzzAI.h"
#include "MapManager.h"
#include "SkillManager.h"
#include "ZzzInterface.h"

CSummonSystem g_SummonSystem;

CSummonSystem::CSummonSystem()
{
}

CSummonSystem::~CSummonSystem()
{
    m_EquipEffectRandom.clear();
}

void CSummonSystem::MoveEquipEffect(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1)
{
    if (iItemType >= MODEL_BOOK_OF_SAHAMUTT && iItemType <= MODEL_STAFF + 29)
    {
        CreateEquipEffect_WristRing(pCharacter, iItemType, iItemLevel, iItemOption1);
        CreateEquipEffect_Summon(pCharacter, iItemType, iItemLevel, iItemOption1);
    }
    else
    {
        RemoveEquipEffect_WristRing(pCharacter);
        RemoveEquipEffect_Summon(pCharacter);
    }
}

void CSummonSystem::RemoveEquipEffects(CHARACTER* pCharacter)
{
    RemoveEquipEffect_WristRing(pCharacter);
    RemoveEquipEffect_Summon(pCharacter);
}

void CSummonSystem::SetPlayerSummon(CHARACTER* pCharacter, OBJECT* pObject)
{
    switch (pCharacter->Helper.Type)
    {
    case MODEL_HORN_OF_UNIRIA:
        SetAction(pObject, PLAYER_SKILL_SUMMON_UNI);
        break;
    case MODEL_HORN_OF_DINORANT:
        SetAction(pObject, PLAYER_SKILL_SUMMON_DINO);
        break;
    case MODEL_HORN_OF_FENRIR:
        SetAction(pObject, PLAYER_SKILL_SUMMON_FENRIR);
        break;
    default:
        SetAction(pObject, PLAYER_SKILL_SUMMON);
        break;
    }
}

void CSummonSystem::CastSummonSkill(int iSkill, CHARACTER* pCharacter, OBJECT* pObject, int iTargetPos_X, int iTargetPos_Y)
{
    pObject->Angle[2] = CreateAngle(pObject->Position[0], pObject->Position[1], (float)iTargetPos_X * TERRAIN_SCALE, (float)iTargetPos_Y * TERRAIN_SCALE);
    SetPlayerSummon(pCharacter, pObject);
    CreateCastingEffect(pObject->Position, pObject->Angle, iSkill);
    RemoveEquipEffect_Summon(pCharacter);
    CreateSummonObject(iSkill, pCharacter, pObject, (float)iTargetPos_X * TERRAIN_SCALE, (float)iTargetPos_Y * TERRAIN_SCALE);
}

bool CheckTarget(CHARACTER* c);
float RequestTerrainHeight(float xf, float yf);

BOOL CSummonSystem::SendRequestSummonSkill(int iSkill, CHARACTER* pCharacter, OBJECT* pObject)
{
    if (iSkill < AT_SKILL_SUMMON_EXPLOSION || iSkill > AT_SKILL_SUMMON_POLLUTION)
        return FALSE;

    extern int TargetX, TargetY;
    extern MovementSkill g_MovementSkill;
    extern int SelectedCharacter;

    CheckTarget(pCharacter);

    int iTargetKey = -1;
    if (CheckAttack())
    {
        iTargetKey = CharactersClient[SelectedCharacter].Key;
    }
    if (iTargetKey != -1)
    {
        CHARACTER* pTargetCharacter = &CharactersClient[SelectedCharacter];
        TargetX = (int)(pTargetCharacter->Object.Position[0] / TERRAIN_SCALE);
        TargetY = (int)(pTargetCharacter->Object.Position[1] / TERRAIN_SCALE);
    }

    g_MovementSkill.m_bMagic = FALSE;
    g_MovementSkill.m_iSkill = iSkill;
    g_MovementSkill.m_iTarget = SelectedCharacter;

    float fDistance = gSkillManager.GetSkillDistance(iSkill, pCharacter);

    if (CheckTile(pCharacter, pObject, fDistance))
    {
        if (iTargetKey != -1)
        {
            SendRequestMagicContinue(iSkill, (BYTE)(pCharacter->TargetPosition[0] / TERRAIN_SCALE), (BYTE)(pCharacter->TargetPosition[1] / TERRAIN_SCALE),
                (BYTE)(pObject->Angle[2] / 360.f * 256.f), 0, 0, (WORD)iTargetKey, 0);
        }
        else
        {
            SendRequestMagicContinue(iSkill, (BYTE)TargetX, (BYTE)TargetY,
                (BYTE)(pObject->Angle[2] / 360.f * 256.f), 0, 0, 0, 0);
        }
        CastSummonSkill(iSkill, pCharacter, pObject, TargetX, TargetY);
    }
    return FALSE;
}

void CSummonSystem::CreateSummonObject(int iSkill, CHARACTER* pCharacter, OBJECT* pObject, float fTargetPos_X, float fTargetPos_Y)
{
    PART_t* pWeapon = &pCharacter->Weapon[1];
    int iSummonLevel = pWeapon->Level;
    if (iSummonLevel >= 11) iSummonLevel = 2;
    else if (iSummonLevel >= 7) iSummonLevel = 1;
    else iSummonLevel = 0;

    switch (iSkill)
    {
    case AT_SKILL_SUMMON_EXPLOSION:
    {
        vec3_t vPos;
        VectorCopy(pObject->Position, vPos);
        if (rand_fps_check(2)) vPos[0] += rand() % 300 + 150;
        else vPos[0] -= rand() % 250 + 150;
        if (rand_fps_check(2)) vPos[1] += rand() % 300 + 150;
        else vPos[1] -= rand() % 250 + 150;

        vec3_t vTargetPos;
        Vector(fTargetPos_X, fTargetPos_Y, RequestTerrainHeight(fTargetPos_X, fTargetPos_Y), vTargetPos);

        CreateEffect(MODEL_SUMMONER_SUMMON_SAHAMUTT, vPos, pObject->Angle, vTargetPos, iSummonLevel);

        PlayBuffer(SOUND_SUMMON_SAHAMUTT);
    }
    break;
    case AT_SKILL_SUMMON_REQUIEM:
    {
        float Matrix[3][4];
        vec3_t vMoveDir, vPosition;
        AngleMatrix(pObject->Angle, Matrix);
        Vector(0, -100.0f, 0, vMoveDir);
        VectorRotate(vMoveDir, Matrix, vPosition);
        VectorAdd(pObject->Position, vPosition, vPosition);

        vec3_t vTargetPos;
        Vector(fTargetPos_X, fTargetPos_Y, RequestTerrainHeight(fTargetPos_X, fTargetPos_Y), vTargetPos);

        CreateEffect(MODEL_SUMMONER_SUMMON_NEIL, vPosition, pObject->Angle, vTargetPos, iSummonLevel);

        PlayBuffer(SOUND_SUMMON_NEIL);
    }
    break;
    case AT_SKILL_SUMMON_POLLUTION:
    {
        vec3_t vTargetPos;
        Vector(fTargetPos_X, fTargetPos_Y, RequestTerrainHeight(fTargetPos_X, fTargetPos_Y), vTargetPos);
        vec3_t vLight, vAngle;

        Vector(1.0f, 1.0f, 1.0f, vLight);
        Vector(1.0f, 1.0f, 1.0f, vAngle);
        CreateEffect(MODEL_SUMMONER_SUMMON_LAGUL, vTargetPos, vAngle, vLight, 0, NULL, iSummonLevel);

        int anRargle[3] = { 1, 2, 4 };
        for (int i = 0; i < anRargle[iSummonLevel]; ++i)
        {
            Vector(0.f, 0.f, i * 90.f, vAngle);
            CreateJoint(BITMAP_JOINT_SPIRIT, vTargetPos, vTargetPos, vAngle, 24, NULL, 100.f);
            CreateJoint(BITMAP_JOINT_SPIRIT, vTargetPos, vTargetPos, vAngle, 24, NULL, 20.f);
        }
        PlayBuffer(SOUND_SUMMOM_RARGLE);
    }
    break;
    }
}

void CSummonSystem::CreateCastingEffect(vec3_t vPosition, vec3_t vAngle, int iSubType)
{
    vec3_t vLight;
    Vector(1.0f, 1.0f, 1.0f, vLight);
    CreateEffect(BITMAP_MAGIC, vPosition, vAngle, vLight, 10);
    switch (iSubType)
    {
    case AT_SKILL_SUMMON_EXPLOSION:	Vector(1.0f, 0.6f, 0.4f, vLight); break;
    case AT_SKILL_SUMMON_REQUIEM:	Vector(0.7f, 0.7f, 1.0f, vLight); break;
    case AT_SKILL_SUMMON_POLLUTION:	Vector(0.6f, 0.6f, 0.9f, vLight); break;
    default: Vector(0.7f, 0.7f, 1.0f, vLight); break;
    }
    CreateEffect(BITMAP_MAGIC, vPosition, vAngle, vLight, 9);
    switch (iSubType)
    {
    case AT_SKILL_SUMMON_EXPLOSION:	Vector(1.0f, 0.5f, 0.0f, vLight); break;
    case AT_SKILL_SUMMON_REQUIEM:	Vector(0.0f, 0.7f, 1.0f, vLight); break;
    case AT_SKILL_SUMMON_POLLUTION:	Vector(0.6f, 0.3f, 0.9f, vLight); break;
    default: Vector(0.0f, 0.7f, 1.0f, vLight); break;
    }
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT1, vPosition, vAngle, vLight);
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT11, vPosition, vAngle, vLight);
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT111, vPosition, vAngle, vLight);
    switch (iSubType)
    {
    case AT_SKILL_SUMMON_EXPLOSION:	Vector(1.0f, 0.5f, 0.0f, vLight); break;
    case AT_SKILL_SUMMON_REQUIEM:	Vector(0.0f, 0.0f, 1.0f, vLight); break;
    case AT_SKILL_SUMMON_POLLUTION:	Vector(0.8f, 0.1f, 0.6f, vLight); break;
    default: Vector(0.0f, 0.0f, 1.0f, vLight); break;
    }
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT2, vPosition, vAngle, vLight);
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT22, vPosition, vAngle, vLight);
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT222, vPosition, vAngle, vLight);
    switch (iSubType)
    {
    case AT_SKILL_SUMMON_EXPLOSION:	Vector(1.0f, 0.5f, 0.8f, vLight); break;
    case AT_SKILL_SUMMON_REQUIEM:	Vector(0.8f, 0.5f, 1.0f, vLight); break;
    case AT_SKILL_SUMMON_POLLUTION:	Vector(0.9f, 0.1f, 1.0f, vLight); break;
    default: Vector(0.8f, 0.5f, 1.0f, vLight); break;
    }
    CreateEffect(MODEL_SUMMONER_CASTING_EFFECT4, vPosition, vAngle, vLight);
}

void CSummonSystem::CreateEquipEffect_WristRing(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1)
{
    OBJECT* pObject = &pCharacter->Object;
    BMD* pModel = &Models[pObject->Type];

    vec3_t vPos, vRelative, vLight;
    Vector(0.0f, 0.0f, 0.0f, vRelative);
    pModel->TransformPosition(pObject->BoneTransform[37], vRelative, vPos, true);

    switch (iItemLevel)
    {
    case 0: case 1: case 2:		Vector(1.0f, 1.0f, 0.0f, vLight);	break;
    case 3: case 4:				Vector(0.5f, 1.0f, 0.5f, vLight);	break;
    case 5: case 6:				Vector(0.5f, 0.1f, 1.0f, vLight);	break;
    case 7: case 8:				Vector(1.0f, 0.5f, 0.0f, vLight);	break;
    case 9: case 10:			Vector(1.0f, 0.2f, 0.2f, vLight);	break;
    case 11: case 12: case 13:	Vector(0.3f, 0.5f, 1.0f, vLight);	break;
    }

    if (!SearchJoint(MODEL_SPEARSKILL, pObject, 14))
    {
        for (int i = 0; i < 4; ++i)
        {
            CreateJoint(MODEL_SPEARSKILL, vPos, vPos, pObject->Angle, 14, pObject, 18.0f, -1, 0, 0,
                pCharacter->TargetCharacter, vLight);
        }

        if (iItemLevel >= 12)
        {
            CreateEffect(MODEL_SUMMONER_WRISTRING_EFFECT, vPos, pObject->Angle, vLight, 0, pObject);
        }
    }

    if (iItemLevel >= 5)
    {
        CreateSprite(BITMAP_FLARE, vPos, 0.7f, vLight, pObject);
    }
    if (iItemLevel >= 7)
    {
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.8f, vLight, pObject);
    }
    if (iItemLevel >= 9)
    {
        CreateSprite(BITMAP_SHINY + 1, vPos, 1.0f, vLight, pObject, (float)((int)(WorldTime / 20) % 360));
    }
    if (iItemLevel >= 13)
    {
        if (rand() % 4)
        {
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, pObject->Angle, vLight, 23, 0.5f, pObject);
        }
    }
}

void CSummonSystem::RemoveEquipEffect_WristRing(CHARACTER* pCharacter)
{
    OBJECT* pObject = &pCharacter->Object;
    DeleteJoint(MODEL_SPEARSKILL, pObject, 14);
    DeleteEffect(MODEL_SUMMONER_WRISTRING_EFFECT, pObject);
}

void CSummonSystem::CreateEquipEffect_Summon(CHARACTER* pCharacter, int iItemType, int iItemLevel, int iItemOption1)
{
    OBJECT* pObject = &pCharacter->Object;

    if (
        gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE
        ) return;

    if (pObject->CurrentAction == PLAYER_SKILL_SUMMON || pObject->CurrentAction == PLAYER_SKILL_SUMMON_UNI
        || pObject->CurrentAction == PLAYER_SKILL_SUMMON_DINO || pObject->CurrentAction == PLAYER_SKILL_SUMMON_FENRIR)
    {
        return;
    }

    vec3_t vLight;
    Vector(1.0f, 1.0f, 1.0f, vLight);

    BYTE byRandom;
    auto iter = m_EquipEffectRandom.find(pCharacter->Key);
    if (iter == m_EquipEffectRandom.end())
    {
        byRandom = rand() % 256;
        m_EquipEffectRandom.insert(std::pair<SHORT, BYTE>(pCharacter->Key, byRandom));
    }
    else
    {
        byRandom = iter->second;
    }

    switch (iItemType)
    {
    case MODEL_BOOK_OF_SAHAMUTT:
        if (!SearchEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, pObject, 0) && !pCharacter->SafeZone && sinf(WorldTime * 0.0004f + byRandom * 0.024f) > 0.3f)
        {
            CreateEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, pObject->Position, pObject->Angle, vLight, 0, pObject, -1, byRandom);
        }
        break;
    case MODEL_BOOK_OF_NEIL:
        if (!SearchEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, pObject, 0) && !pCharacter->SafeZone && sinf(WorldTime * 0.0004f + byRandom * 0.024f) > 0.3f)
        {
            CreateEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, pObject->Position, pObject->Angle, vLight, 0, pObject, -1, byRandom);
        }
        break;
    case MODEL_BOOK_OF_LAGLE:
        if (!SearchEffect(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, pObject, 0) && !pCharacter->SafeZone && sinf(WorldTime * 0.0004f + byRandom * 0.024f) > 0.3f)
        {
            CreateEffect(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, pObject->Position, pObject->Angle, vLight, 0, pObject, -1, byRandom);
        }
        break;
    default:
        break;
    }
}

void CSummonSystem::RemoveEquipEffect_Summon(CHARACTER* pCharacter)
{
    OBJECT* pObject = &pCharacter->Object;
    DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, pObject, 0);
    DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, pObject, 0);
    DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, pObject, 0);
}

void CSummonSystem::CreateDamageOfTimeEffect(int iSkill, OBJECT* pObject)
{
    switch (iSkill)
    {
    case AT_SKILL_SUMMON_EXPLOSION:
        if (!SearchEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, pObject, 1))
        {
            CreateEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, pObject->Position, pObject->Angle, pObject->Light, 1, pObject, -1);
        }
        break;
    case AT_SKILL_SUMMON_REQUIEM:
        if (!SearchEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, pObject, 1))
        {
            CreateEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, pObject->Position, pObject->Angle, pObject->Light, 1, pObject, -1);
        }
        break;
    }
}

void CSummonSystem::RemoveDamageOfTimeEffect(int iSkill, OBJECT* pObject)
{
    switch (iSkill)
    {
    case AT_SKILL_SUMMON_EXPLOSION:
        DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, pObject, 1);
        break;
    case AT_SKILL_SUMMON_REQUIEM:
        DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, pObject, 1);
        break;
    }
}

void CSummonSystem::RemoveAllDamageOfTimeEffect(OBJECT* pObject)
{
    RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, pObject);
    RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, pObject);
}