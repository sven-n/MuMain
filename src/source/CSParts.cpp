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
#include "ZzzInterface.h"
#include "ZzzEffect.h"
#include "ZzzTexture.h"
#include "Guild/GuildCache.h"
#include "ZzzLodTerrain.h"
#include "CSParts.h"

#include <memory>

namespace
{
constexpr float kRenderableAlphaThreshold = 0.01f;
constexpr int kDefaultBoneIndex = 20;
constexpr int kDefaultLifetimeTicks = 30;
constexpr float kDefaultVelocity = 0.5f;

bool ShouldAssignWebzenPart(CHARACTER& character)
{
    const bool hasGmBuff = g_isCharacterBuff((&character.Object), eBuff_GMEffect) != FALSE;
    const bool isOperator = (character.CtlCode == CTLCODE_20OPERATOR) || (character.CtlCode == CTLCODE_08OPERATOR);
    return hasGmBuff || isOperator;
}

bool ShouldHideCastleMarks(const CHARACTER& character)
{
    const bool isCastleMark = (character.EtcPart >= PARTS_ATTACK_TEAM_MARK) && (character.EtcPart <= PARTS_DEFENSE_KING_TEAM_MARK);
    return isCastleMark && !battleCastle::IsBattleCastleStart();
}

std::unique_ptr<CSIPartsMDL> CreatePartsByType(int etcPart)
{
    switch (etcPart)
    {
    case PARTS_WEBZEN:
        return std::make_unique<CSParts>(MODEL_WEBZEN_MARK, kDefaultBoneIndex, false, 70.f, -5.f, 0.f, 0.f, 0.f, 45.f);
    case PARTS_ATTACK_TEAM_MARK:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 0, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_ATTACK_TEAM_MARK2:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 1, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_ATTACK_TEAM_MARK3:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 2, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_ATTACK_KING_TEAM_MARK:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 3, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_ATTACK_KING_TEAM_MARK2:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 4, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_ATTACK_KING_TEAM_MARK3:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 5, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_DEFENSE_TEAM_MARK:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 6, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    case PARTS_DEFENSE_KING_TEAM_MARK:
        return std::make_unique<CSParts2D>(BITMAP_FORMATION_MARK, 7, kDefaultBoneIndex, 120.f, 0.f, 0.f);
    default:
        return nullptr;
    }
}

void InitializeCommonObjectState(OBJECT& object, int type, bool billboard)
{
    object.Type = type;
    object.Live = true;
    object.Visible = false;
    object.LightEnable = true;
    object.ContrastEnable = false;
    object.AlphaEnable = false;
    object.EnableBoneMatrix = false;
    object.Owner = nullptr;
    object.SubType = 0;
    object.HiddenMesh = -1;
    object.BlendMesh = -1;
    object.BlendMeshLight = 1.f;
    object.Scale = 1.f;
    object.LifeTime = kDefaultLifetimeTicks;
    object.Alpha = 1.f;
    object.AlphaTarget = 1.f;
    object.EnableShadow = false;
    object.CurrentAction = 0;
    object.PriorAction = 0;
    object.PriorAnimationFrame = 0.f;
    object.AnimationFrame = 0.f;
    object.Velocity = kDefaultVelocity;
    object.bBillBoard = billboard;

    g_CharacterClearBuff((&object));
    Vector(1.f, 1.f, 1.f, object.Light);
    Vector(0.f, 0.f, 0.f, object.HeadAngle);
}
} // namespace

void CreatePartsFactory(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    if (ShouldAssignWebzenPart(*c))
    {
        if (c->m_pParts != nullptr && c->EtcPart != PARTS_WEBZEN)
        {
            DeleteParts(c);
        }
        c->EtcPart = PARTS_WEBZEN;
    }

    if (c->m_pParts == nullptr && c->EtcPart != 0)
    {
        if (auto newParts = CreatePartsByType(c->EtcPart))
        {
            c->m_pParts = newParts.release();
        }
    }
}

void RenderParts(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    if (g_isCharacterBuff((&c->Object), eBuff_Cloaking))
    {
        return;
    }

    if (ShouldHideCastleMarks(*c))
    {
        return;
    }

    if (c->m_pTempParts != nullptr)
    {
        auto* tempParts = static_cast<CSIPartsMDL*>(c->m_pTempParts);
        tempParts->IRender(c);
    }

    if (c->m_pParts == nullptr)
    {
        return;
    }

    auto* parts = static_cast<CSIPartsMDL*>(c->m_pParts);
    parts->IRender(c);
}

void DeleteParts(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    if (c->m_pParts != nullptr)
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

    InitializeCommonObjectState(m_pObj, Type, bBillBoard);
    Vector(ax, ay, az, m_pObj.Angle);
}

void CSParts::IRender(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    if (m_pObj.Alpha < kRenderableAlphaThreshold)
    {
        return;
    }

    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    vec3_t  Position;

    b->TransformPosition(o->BoneTransform[m_iBoneNumber], m_vOffset, Position, true);
    VectorCopy(Position, m_pObj.Position);
    if (!m_pObj.bBillBoard)
    {
        VectorCopy(o->Angle, m_pObj.Angle);
    }

    if (m_pObj.Type == MODEL_XMAS_EVENT_EARRING)
    {
        m_pObj.Angle[2] -= o->HeadAngle[0];
    }

    b = &Models[m_pObj.Type];
    b->CurrentAction = m_pObj.CurrentAction;

    const float speed = m_pObj.Velocity;
    b->PlayAnimation(&m_pObj.AnimationFrame, &m_pObj.PriorAnimationFrame, &m_pObj.PriorAction, speed, m_pObj.Position, m_pObj.Angle);

    Vector(1.f, 1.f, 1.f, b->BodyLight);
    RenderObject(&m_pObj, true);
}

CSAnimationParts::CSAnimationParts(int Type, int BoneNumber, bool bBillBoard, float x, float y, float z, float ax, float ay, float az)
{
    m_iBoneNumber = BoneNumber;

    m_vOffset[0] = x;
    m_vOffset[1] = y;
    m_vOffset[2] = z;

    InitializeCommonObjectState(m_pObj, Type, bBillBoard);
    Vector(ax, ay, az, m_pObj.Angle);
}

void CSAnimationParts::Animation(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    BMD* b = &Models[m_pObj.Type];
    b->CurrentAction = m_pObj.CurrentAction;

    float fSpeed = m_pObj.Velocity;
    b->PlayAnimation(&m_pObj.AnimationFrame, &m_pObj.PriorAnimationFrame, &m_pObj.PriorAction, fSpeed, m_pObj.Position, m_pObj.Angle);
}

void CSAnimationParts::IRender(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    if (m_pObj.Alpha < kRenderableAlphaThreshold)
    {
        return;
    }

    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    vec3_t  Position;

    b->TransformPosition(o->BoneTransform[m_iBoneNumber], m_vOffset, Position, true);
    VectorAdd(o->Position, Position, m_pObj.Position);
    if (!m_pObj.bBillBoard)
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

    InitializeCommonObjectState(m_pObj, Type, true);
    m_pObj.SubType = SubType;
}

void CSParts2D::IRender(CHARACTER* c)
{
    if (c == nullptr)
    {
        return;
    }

    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    vec3_t  Position;
    BYTE    bSubType = m_pObj.SubType;

    b->TransformPosition(o->BoneTransform[m_iBoneNumber], m_vOffset, Position, true);
    VectorCopy(o->Position, m_pObj.Position);
    m_pObj.Position[2] = Position[2];

    const bool isMarkType =
        (c->EtcPart == PARTS_ATTACK_TEAM_MARK ||
         c->EtcPart == PARTS_ATTACK_TEAM_MARK2 ||
         c->EtcPart == PARTS_ATTACK_TEAM_MARK3 ||
         c->EtcPart == PARTS_DEFENSE_TEAM_MARK);

    if (isMarkType && c->GuildStatus == G_MASTER)
    {
        const bool sameUnionAndGuild = (wcscmp(GuildMark[c->GuildMarkIndex].UnionName, GuildMark[c->GuildMarkIndex].GuildName) == 0);
        const bool emptyUnionName = (wcscmp(GuildMark[c->GuildMarkIndex].UnionName, L"") == 0);
        if (sameUnionAndGuild || emptyUnionName)
        {
            if (c->EtcPart == PARTS_DEFENSE_TEAM_MARK)
            {
                bSubType += 1;
            }
            else
            {
                bSubType += 3;
            }
        }
    }

    CreateSprite(m_pObj.Type, m_pObj.Position, 1.f, m_pObj.Light, nullptr, 0, bSubType);
}