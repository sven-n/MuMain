//////////////////////////////////////////////////////////////////////////
//  CSParts.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "zzzInterface.h"
#include "zzzEffect.h"
#include "zzztexture.h"
#include "GuildCache.h"
#include "zzzlodterrain.h"
#include "CSParts.h"

void CreatePartsFactory(CHARACTER* c)
{
    if (g_isCharacterBuff((&c->Object), eBuff_GMEffect) || ((c->CtlCode == CTLCODE_20OPERATOR) || (c->CtlCode == CTLCODE_08OPERATOR)))
    {
        if (c->m_pParts != NULL && c->EtcPart != PARTS_WEBZEN)
        {
            DeleteParts(c);
        }
        c->EtcPart = PARTS_WEBZEN;
    }

    if (c->m_pParts == NULL && c->EtcPart != 0)
    {
        switch (c->EtcPart)
        {
        case PARTS_WEBZEN:
        {
            c->m_pParts = (CSIPartsMDL*)new CSParts(MODEL_WEBZEN_MARK, 20, false, 70.f, -5.f, 0.f, 0.f, 0.f, 45.f);
        }
        break;
        case PARTS_ATTACK_TEAM_MARK:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 0, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_ATTACK_TEAM_MARK2:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 1, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_ATTACK_TEAM_MARK3:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 2, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_ATTACK_KING_TEAM_MARK:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 3, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_ATTACK_KING_TEAM_MARK2:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 4, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_ATTACK_KING_TEAM_MARK3:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 5, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_DEFENSE_TEAM_MARK:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 6, 20, 120.f, 0.f, 0.f);
            break;

        case PARTS_DEFENSE_KING_TEAM_MARK:
            c->m_pParts = (CSIPartsMDL*)new CSParts2D(BITMAP_FORMATION_MARK, 7, 20, 120.f, 0.f, 0.f);
            break;
        default:
            break;
        }
    }
}

void RenderParts(CHARACTER* c)
{
    if (g_isCharacterBuff((&c->Object), eBuff_Cloaking))
    {
        return;
    }

    if (c->EtcPart >= PARTS_ATTACK_TEAM_MARK && c->EtcPart <= PARTS_DEFENSE_KING_TEAM_MARK && battleCastle::IsBattleCastleStart() == false)
    {
        return;
    }

    if (c->m_pTempParts != NULL)
    {
        auto* pTempParts = (CSIPartsMDL*)c->m_pTempParts;
        pTempParts->IRender(c);
    }

    if (c->m_pParts == NULL)
    {
        return;
    }

    auto* pParts = (CSIPartsMDL*)c->m_pParts;

    pParts->IRender(c);
}

void DeleteParts(CHARACTER* c)
{
    if (c->m_pParts != NULL)
    {
        SAFE_DELETE(c->m_pParts);
        c->EtcPart = 0;
    }

    SAFE_DELETE(c->m_pTempParts);
}

CSParts::CSParts(int Type, int BoneNumber, bool bBillBoard, float x, float y, float z, float ax, float ay, float az)
{
    m_iBoneNumber = BoneNumber;

    m_vOffset[0] = x;
    m_vOffset[1] = y;
    m_vOffset[2] = z;

    m_pObj.Type = Type;
    m_pObj.Live = true;
    m_pObj.Visible = false;
    m_pObj.LightEnable = true;
    m_pObj.ContrastEnable = false;
    m_pObj.AlphaEnable = false;
    m_pObj.EnableBoneMatrix = false;
    m_pObj.Owner = NULL;
    m_pObj.SubType = 0;
    m_pObj.HiddenMesh = -1;
    m_pObj.BlendMesh = -1;
    m_pObj.BlendMeshLight = 1.f;
    m_pObj.Scale = 1.f;
    m_pObj.LifeTime = 30;
    m_pObj.Alpha = 1.f;
    m_pObj.AlphaTarget = 1.f;

    g_CharacterClearBuff((&m_pObj));

    Vector(1.f, 1.f, 1.f, m_pObj.Light);
    Vector(0.f, 0.f, 0.f, m_pObj.HeadAngle);

    m_pObj.EnableShadow = false;
    m_pObj.CurrentAction = 0;
    m_pObj.PriorAction = 0;
    m_pObj.PriorAnimationFrame = 0.f;
    m_pObj.AnimationFrame = 0.f;
    m_pObj.Velocity = 0.5f;

    m_pObj.bBillBoard = bBillBoard;
    Vector(ax, ay, az, m_pObj.Angle);
}

void CSParts::IRender(CHARACTER* c)
{
    if (c == NULL) return;
    if (m_pObj.Alpha < 0.01f) return;

    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    vec3_t  Position;

    b->TransformPosition(o->BoneTransform[m_iBoneNumber], m_vOffset, Position, true);
    VectorCopy(Position, m_pObj.Position);
    if (m_pObj.bBillBoard == false)
    {
        VectorCopy(o->Angle, m_pObj.Angle);
    }

    if (m_pObj.Type == MODEL_XMAS_EVENT_EARRING)
    {
        m_pObj.Angle[2] -= o->HeadAngle[0];
    }

    b = &Models[m_pObj.Type];
    b->CurrentAction = m_pObj.CurrentAction;

    float fSpeed = m_pObj.Velocity;
    b->PlayAnimation(&m_pObj.AnimationFrame, &m_pObj.PriorAnimationFrame, &m_pObj.PriorAction, fSpeed, m_pObj.Position, m_pObj.Angle);

    Vector(1.f, 1.f, 1.f, b->BodyLight);
    RenderObject(&m_pObj, true);
}

CSAnimationParts::CSAnimationParts(int Type, int BoneNumber, bool bBillBoard, float x, float y, float z, float ax, float ay, float az)
{
    m_iBoneNumber = BoneNumber;

    m_vOffset[0] = x;
    m_vOffset[1] = y;
    m_vOffset[2] = z;

    m_pObj.Type = Type;
    m_pObj.Live = true;
    m_pObj.Visible = false;
    m_pObj.LightEnable = true;
    m_pObj.ContrastEnable = false;
    m_pObj.AlphaEnable = false;
    m_pObj.EnableBoneMatrix = false;
    m_pObj.Owner = NULL;
    m_pObj.SubType = 0;
    m_pObj.HiddenMesh = -1;
    m_pObj.BlendMesh = -1;
    m_pObj.BlendMeshLight = 1.f;
    m_pObj.Scale = 1.f;
    m_pObj.LifeTime = 30;
    m_pObj.Alpha = 1.f;
    m_pObj.AlphaTarget = 1.f;

    g_CharacterClearBuff((&m_pObj));

    Vector(1.f, 1.f, 1.f, m_pObj.Light);
    Vector(0.f, 0.f, 0.f, m_pObj.HeadAngle);

    m_pObj.EnableShadow = false;
    m_pObj.CurrentAction = 0;
    m_pObj.PriorAction = 0;
    m_pObj.PriorAnimationFrame = 0.f;
    m_pObj.AnimationFrame = 0.f;
    m_pObj.Velocity = 0.5f;

    m_pObj.bBillBoard = bBillBoard;
    Vector(ax, ay, az, m_pObj.Angle);
}

void CSAnimationParts::Animation(CHARACTER* c)
{
    if (c == NULL)	return;

    BMD* b = &Models[m_pObj.Type];
    b->CurrentAction = m_pObj.CurrentAction;

    float fSpeed = m_pObj.Velocity;
    b->PlayAnimation(&m_pObj.AnimationFrame, &m_pObj.PriorAnimationFrame, &m_pObj.PriorAction, fSpeed, m_pObj.Position, m_pObj.Angle);
}

void CSAnimationParts::IRender(CHARACTER* c)
{
    if (c == NULL) return;
    if (m_pObj.Alpha < 0.01f) return;

    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    vec3_t  Position;

    b->TransformPosition(o->BoneTransform[m_iBoneNumber], m_vOffset, Position, true);
    VectorAdd(o->Position, Position, m_pObj.Position);
    if (m_pObj.bBillBoard == false)
    {
        VectorCopy(o->Angle, m_pObj.Angle);
    }

    Animation(c);
    RenderObject(&m_pObj, true);
}

CSParts2D::CSParts2D(int Type, int SubType, int BoneNumber, float x, float y, float z)
{
    m_iBoneNumber = BoneNumber;

    m_vOffset[0] = x;
    m_vOffset[1] = y;
    m_vOffset[2] = z;

    m_pObj.Type = Type;
    m_pObj.SubType = SubType;
    Vector(1.f, 1.f, 1.f, m_pObj.Light);
}

void CSParts2D::IRender(CHARACTER* c)
{
    if (c == NULL) return;

    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    vec3_t  Position;
    BYTE    bSubType = m_pObj.SubType;

    b->TransformPosition(o->BoneTransform[m_iBoneNumber], m_vOffset, Position, true);
    VectorCopy(o->Position, m_pObj.Position);
    m_pObj.Position[2] = Position[2];

    if ((c->EtcPart == PARTS_ATTACK_TEAM_MARK
        || c->EtcPart == PARTS_ATTACK_TEAM_MARK2
        || c->EtcPart == PARTS_ATTACK_TEAM_MARK3
        || c->EtcPart == PARTS_DEFENSE_TEAM_MARK
        || c->EtcPart == PARTS_ATTACK_TEAM_MARK3
        || c->EtcPart == PARTS_ATTACK_TEAM_MARK3
        )
        && c->GuildStatus == G_MASTER)
    {
        if (wcscmp(GuildMark[c->GuildMarkIndex].UnionName, GuildMark[c->GuildMarkIndex].GuildName) == NULL || wcscmp(GuildMark[c->GuildMarkIndex].UnionName, L"") == NULL)
        {
            if (c->EtcPart == PARTS_DEFENSE_TEAM_MARK)
                bSubType += 1;
            else
                bSubType += 3;
        }
    }
    CreateSprite(m_pObj.Type, m_pObj.Position, 1.f, m_pObj.Light, NULL, 0, bSubType);
}