#include "stdafx.h"
#include "UI/Social/PhotoViewer.h"

#include "UI/Social/SocialWindowManager.h"
#include "UI/Core/UIManager.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Text/CUIRenderText.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Core/Input/KeyState.h"
#include "Camera/CameraProjection.h"
#include "Character/CharacterManager.h"
#include "Engine/AI/GOBoid.h"
#include "Engine/AI/ZzzAI.h"
#include "Character/CSParts.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "UI/Core/WindowCommon.h"
#include "I18N/All.h"
#include <algorithm>

extern int gix, giy;
extern void MoveCharacter(CHARACTER* c, OBJECT* o);
extern void MoveCharacterVisual(CHARACTER* c, OBJECT* o);

// Matrix helpers below are copied verbatim from ZzzOpenglUtil.cpp (validated against a CPU closed
// form) -- a sign/order error here reproduces this panel's "mirrored/upside-down character" bug.
// This is the one item-preview panel with a real (non-identity) camera and no BeginBitmap/EndBitmap
// restore, so it needs its own pre-panel snapshot below instead of a fresh GL read.
static float s_PrePhotoProj[16];
static float s_PrePhotoView[16];

// Column-major float[16] (glGetFloatv layout); out = a * b, applying b first then a.
static void PhotoMat4Multiply(float* out, const float* a, const float* b)
{
    float result[16];
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            double sum = 0.0;
            for (int k = 0; k < 4; ++k)
                sum += (double)a[k * 4 + row] * (double)b[col * 4 + k];
            result[col * 4 + row] = (float)sum;
        }
    }
    memcpy(out, result, sizeof(result));
}

static void PhotoMakeRotationX(float degrees, float* out)
{
    float rad = degrees * Q_PI / 180.0f;
    float c = cosf(rad), s = sinf(rad);
    float m[16] = {1.f, 0.f, 0.f, 0.f, 0.f, c, s, 0.f, 0.f, -s, c, 0.f, 0.f, 0.f, 0.f, 1.f};
    memcpy(out, m, sizeof(m));
}

static void PhotoMakeRotationZ(float degrees, float* out)
{
    float rad = degrees * Q_PI / 180.0f;
    float c = cosf(rad), s = sinf(rad);
    float m[16] = {c, s, 0.f, 0.f, -s, c, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
    memcpy(out, m, sizeof(m));
}

static void PhotoMakeTranslation(float x, float y, float z, float* out)
{
    float m[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, x, y, z, 1.f};
    memcpy(out, m, sizeof(m));
}

void CUIPhotoViewer::RenderPhotoCharacter(float aspect)
{
    CHARACTER* c = &m_PhotoChar;
    OBJECT* o = &c->Object;
    int WorldBackup = gMapManager.WorldActive;
    gMapManager.WorldActive = WD_0LORENCIA;
    MoveCharacter(c, o);
    MoveCharacterVisual(c, o);
    MoveMount(&m_PhotoHelper, TRUE);
    gMapManager.WorldActive = WorldBackup;

    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    // The capture brings its own viewport, the whole target, so only the projection is set here:
    // the character is framed by the well it stands in at whatever size that is.
    gluPerspective2(1.f, aspect, 2000, 20000);
    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
    EnableDepthTest();
    EnableDepthMask();

    mu::GetRenderer().Rotate(-90.0f, 1.f, 0.f, 0.f);
    mu::GetRenderer().Rotate(-90.0f, 0.f, 0.f, 1.f);
    mu::GetRenderer().Translate(-10000.0f, 0.0f, -75.f);

    if (c->Helper.Type == MODEL_DARK_HORSE_ITEM)
        mu::GetRenderer().Translate(-o->Position[0], -o->Position[1], -o->Position[2] - 50.0f);
    else
        mu::GetRenderer().Translate(-o->Position[0], -o->Position[1], -o->Position[2]);

    Vector(0.0f, 0.0f, m_fCurrentAngle, o->Angle);

    mu::GetRenderer().SetAlphaTest(false);
    mu::GetRenderer().SetTexture2D(true);
    EnableDepthTest();
    EnableCullFace();
    EnableDepthMask();
    bool AlphaTestEnable = false;

    TextureEnable = true;
    DepthTestEnable = true;
    CullFaceEnable = true;
    DepthMaskEnable = true;
    mu::GetRenderer().SetDepthFunc(GL_LEQUAL);
    mu::GetRenderer().SetAlphaFunc(GL_GREATER, 0.25f);
    mu::GetRenderer().SetFogEnabled(false);
    mu::GetRenderer().ClearDepthBuffer();
    o->Scale = 0.7f * m_fCurrentZoom;
    m_PhotoHelper.Scale = m_fPhotoHelperScale * m_fCurrentZoom;
    Vector(1, 1, 1, o->Light);
    Vector(1, 1, 1, m_PhotoHelper.Light);

    c->HideShadow = true;
    if (c->Wing.Type != -1 && m_iSettingAnimation > AT_HEALING1)
        c->SafeZone = true;
    else c->SafeZone = false;
    if (c->Helper.Type == MODEL_HORN_OF_UNIRIA)
        m_PhotoHelper.Position[2] += 10;
    else if (c->Helper.Type == MODEL_HORN_OF_DINORANT)
        m_PhotoHelper.Position[2] += 25;
    RenderMount(&m_PhotoHelper, TRUE);
    if (c->Helper.Type == MODEL_HORN_OF_UNIRIA)
        m_PhotoHelper.Position[2] -= 10;
    else if (c->Helper.Type == MODEL_HORN_OF_DINORANT)
        m_PhotoHelper.Position[2] -= 25;
    RenderCharacter(c, o);

    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PopMatrix();
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PopMatrix();
}

int CUIPhotoViewer::SetPhotoPose(int iCurrentAni, int iMoveDir)
{
    if (m_PhotoHelper.Live == true &&
        (m_PhotoHelper.Type == MODEL_UNICON || m_PhotoHelper.Type == MODEL_PEGASUS || m_PhotoHelper.Type == MODEL_DARK_HORSE || (m_PhotoHelper.Type >= MODEL_FENRIR_BLACK && m_PhotoHelper.Type <= MODEL_FENRIR_GOLD)))
    {
        static const int MAX_POSE_NUM = 3;
        static int siPose[MAX_POSE_NUM] = { AT_STAND1, AT_MOVE1, AT_ATTACK1 };

        int iCurrentAniArray = 0;

        for (int i = 0; i < MAX_POSE_NUM; ++i)
        {
            iCurrentAniArray = i;
            if (iCurrentAni == siPose[i]) break;
        }

        iCurrentAniArray += iMoveDir;
        if (iCurrentAniArray < 0) iCurrentAniArray = MAX_POSE_NUM * 100 + iCurrentAniArray;
        iCurrentAniArray %= MAX_POSE_NUM;
        iCurrentAni = siPose[iCurrentAniArray];
    }
    else
    {
        static const int MAX_POSE_NUM = 24;
        static int siPose[MAX_POSE_NUM] = {
            AT_STAND1, AT_GREETING1, AT_CLAP1, AT_GESTURE1, AT_DIRECTION1, AT_AWKWARD1, AT_CRY1, AT_SEE1,
            AT_CHEER1, AT_UNKNOWN1, AT_WIN1, AT_SMILE1, AT_SLEEP1, AT_COLD1, AT_AGAIN1, AT_RESPECT1,
            AT_SALUTE1, AT_GOODBYE1, AT_MOVE1, AT_RUSH1,AT_SIT1, AT_POSE1, AT_HEALING1, AT_ATTACK1
        };

        int iCurrentAniArray = 0;
        for (int i = 0; i < MAX_POSE_NUM; ++i)
        {
            iCurrentAniArray = i;
            if (iCurrentAni == siPose[i])
                break;
        }

        iCurrentAniArray += iMoveDir;
        if (iCurrentAniArray < 0) iCurrentAniArray = MAX_POSE_NUM * 100 + iCurrentAniArray;
        iCurrentAniArray %= MAX_POSE_NUM;
        iCurrentAni = siPose[iCurrentAniArray];
    }

    CHARACTER* c = &m_PhotoChar;
    OBJECT* o = &c->Object;
    int WorldBackup = gMapManager.WorldActive;
    switch (iCurrentAni)
    {
    case AT_STAND1:
        gMapManager.WorldActive = WD_0LORENCIA;
        SetPlayerStop(c);
        gMapManager.WorldActive = WorldBackup;
        break;
    case AT_ATTACK1:
        gMapManager.WorldActive = WD_0LORENCIA;
        SetPlayerAttack(c);
        gMapManager.WorldActive = WorldBackup;
        c->AttackTime = 1;
        c->Object.AnimationFrame = 0;
        c->TargetCharacter = -1;
        break;
    case AT_MOVE1:
        gMapManager.WorldActive = WD_0LORENCIA;
        SetPlayerWalk(c);
        gMapManager.WorldActive = WorldBackup;
        break;
    case AT_SIT1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_SIT1);
        else
            SetAction(&c->Object, PLAYER_SIT_FEMALE1);
        break;
    case AT_POSE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_POSE1);
        else
            SetAction(&c->Object, PLAYER_POSE_FEMALE1);
        break;
    case AT_HEALING1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_HEALING1);
        else
            SetAction(&c->Object, PLAYER_HEALING_FEMALE1);
        break;
    case AT_GREETING1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GREETING1);
        else
            SetAction(o, PLAYER_GREETING_FEMALE1);
        break;
    case AT_GOODBYE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GOODBYE1);
        else
            SetAction(o, PLAYER_GOODBYE_FEMALE1);
        break;
    case AT_CLAP1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CLAP1);
        else
            SetAction(o, PLAYER_CLAP_FEMALE1);
        break;
    case AT_GESTURE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GESTURE1);
        else
            SetAction(o, PLAYER_GESTURE_FEMALE1);
        break;
    case AT_DIRECTION1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_DIRECTION1);
        else
            SetAction(o, PLAYER_DIRECTION_FEMALE1);
        break;
    case AT_UNKNOWN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_UNKNOWN1);
        else
            SetAction(o, PLAYER_UNKNOWN_FEMALE1);
        break;
    case AT_CRY1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CRY1);
        else
            SetAction(o, PLAYER_CRY_FEMALE1);
        break;
    case AT_AWKWARD1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_AWKWARD1);
        else
            SetAction(o, PLAYER_AWKWARD_FEMALE1);
        break;
    case AT_SEE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SEE1);
        else
            SetAction(o, PLAYER_SEE_FEMALE1);
        break;
    case AT_CHEER1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CHEER1);
        else
            SetAction(o, PLAYER_CHEER_FEMALE1);
        break;
    case AT_WIN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_WIN1);
        else
            SetAction(o, PLAYER_WIN_FEMALE1);
        break;
    case AT_SMILE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SMILE1);
        else
            SetAction(o, PLAYER_SMILE_FEMALE1);
        break;
    case AT_SLEEP1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SLEEP1);
        else
            SetAction(o, PLAYER_SLEEP_FEMALE1);
        break;
    case AT_COLD1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_COLD1);
        else
            SetAction(o, PLAYER_COLD_FEMALE1);
        break;
    case AT_AGAIN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_AGAIN1);
        else
            SetAction(o, PLAYER_AGAIN_FEMALE1);
        break;
    case AT_RESPECT1:
        SetAction(o, PLAYER_RESPECT1);
        break;
    case AT_SALUTE1:
        SetAction(o, PLAYER_SALUTE1);
        break;
    case AT_RUSH1:
        SetAction(o, PLAYER_RUSH1);
        break;
    default:
        break;
    }
    MoveCharacter(c, o);
    MoveCharacterVisual(c, o);
    m_iCurrentAnimation = iCurrentAni;
    return iCurrentAni;
}

CUIPhotoViewer::CUIPhotoViewer()
    : m_Target([this](std::uint32_t width, std::uint32_t height) { RenderInto(width, height); })
{
    m_bIsInitialized = FALSE;
    m_iSettingAnimation = 0;
    m_iCurrentFrame = 0;
    m_bActionRepeatCheck = FALSE;
    m_fSettingAngle = 0;
    m_fCurrentAngle = 0;
    m_fRotateClickPos_x = 0;
    m_fSettingZoom = 1.0f;
    m_fCurrentZoom = 1.0f;
    m_bHelpEnable = FALSE;
    m_bUpdatePlayer = FALSE;
    m_fPhotoHelperScale = 0;
    m_iCurrentAnimation = 0;
    m_bIsWebzenMail = FALSE;
}

CUIPhotoViewer::~CUIPhotoViewer()
{
    g_SummonSystem.RemoveEquipEffects(&m_PhotoChar);
    DeleteCloth(&m_PhotoChar, &m_PhotoChar.Object);
}

void CUIPhotoViewer::Init(int iInitType)
{
    if (iInitType < 0)		return;

    m_PhotoHelper.Initialize();

    m_PhotoChar.Initialize();

    CreateCharacterPointer(&m_PhotoChar, MODEL_PLAYER, (Hero->PositionX), (Hero->PositionY), 0);

    // 이동
    Vector(-300, -300, -300, m_PhotoChar.Object.Position);

    m_bIsInitialized = TRUE;
}

BOOL CompareItemEqual(const PART_t* item1, const PART_t* item2)
{
    return (item1->Type == item2->Type &&
        item1->Level == item2->Level &&
        item1->ExcellentFlags == item2->ExcellentFlags);
}

BOOL CompareItemEqual(const PART_t* item1, const ITEM* item2, int iDefaultValue)
{
    if (item2->Type == -1)
    {
        return (item1->Type == iDefaultValue &&
            item1->Level == item2->Level &&
            item1->ExcellentFlags == item2->ExcellentFlags);
    }
    else
    {
        return (item1->Type == item2->Type + MODEL_ITEM &&
            item1->Level == item2->Level &&
            item1->ExcellentFlags == item2->ExcellentFlags);
    }
}

void SetItemToPhoto(PART_t* itemDest, const ITEM* itemSrc, int iDefaultValue)
{
    if (itemSrc->Type == -1)
    {
        itemDest->Type = iDefaultValue;
        itemDest->Level = itemSrc->Level;
        itemDest->ExcellentFlags = itemSrc->ExcellentFlags;
    }
    else
    {
        itemDest->Type = itemSrc->Type + MODEL_ITEM;
        itemDest->Level = itemSrc->Level;
        itemDest->ExcellentFlags = itemSrc->ExcellentFlags;
    }
}

void CUIPhotoViewer::CopyPlayer()
{
    if (m_bIsInitialized == FALSE) return;

    if (m_PhotoChar.Class != Hero->Class)
    {
        m_PhotoChar.Class = Hero->Class;
        SetChangeClass(&m_PhotoChar);
    }

    int i;
    int maxClass = MAX_CLASS;

    BOOL bChangeArmor = FALSE;
    BOOL bChangeWeapon = FALSE;
    BOOL bChangeWing = FALSE;
    BOOL bChangeHelper = FALSE;
    if (Hero->Change == FALSE)
    {
        for (i = 0; i < MAX_BODYPART; ++i)
        {
            if (CompareItemEqual(&m_PhotoChar.BodyPart[i], &Hero->BodyPart[i]) == FALSE)
            {
                bChangeArmor = TRUE;
                break;
            }
        }
        for (i = 0; i < 2; ++i)
        {
            if (CompareItemEqual(&m_PhotoChar.Weapon[i], &Hero->Weapon[i]) == FALSE)
            {
                bChangeWeapon = TRUE;
                break;
            }
        }
        if (CompareItemEqual(&m_PhotoChar.Wing, &Hero->Wing) == FALSE)
            bChangeWing = TRUE;
        if (CompareItemEqual(&m_PhotoChar.Helper, &Hero->Helper) == FALSE)
            bChangeHelper = TRUE;
    }
    else	// 변신 상태
    {
        if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_HELM], &CharacterMachine->Equipment[EQUIPMENT_HELM],
            static_cast<int>(MODEL_BODY_HELM) + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_ARMOR], &CharacterMachine->Equipment[EQUIPMENT_ARMOR],
            static_cast<int>(MODEL_BODY_ARMOR) + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_PANTS], &CharacterMachine->Equipment[EQUIPMENT_PANTS],
            static_cast<int>(MODEL_BODY_PANTS) + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_GLOVES], &CharacterMachine->Equipment[EQUIPMENT_GLOVES],
            static_cast<int>(MODEL_BODY_GLOVES) + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;
        else if (CompareItemEqual(&m_PhotoChar.BodyPart[BODYPART_BOOTS], &CharacterMachine->Equipment[EQUIPMENT_BOOTS],
            static_cast<int>(MODEL_BODY_BOOTS) + Hero->SkinIndex) == FALSE) bChangeArmor = TRUE;

        for (i = 0; i < 2; ++i)
        {
            if (CompareItemEqual(&m_PhotoChar.Weapon[i], &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT + i], -1) == FALSE)
            {
                bChangeWeapon = TRUE;
                break;
            }
        }
        if (CompareItemEqual(&m_PhotoChar.Wing, &CharacterMachine->Equipment[EQUIPMENT_WING], -1) == FALSE)
            bChangeWing = TRUE;
        if (CompareItemEqual(&m_PhotoChar.Helper, &CharacterMachine->Equipment[EQUIPMENT_HELPER], -1) == FALSE)
            bChangeHelper = TRUE;
    }

    if (bChangeArmor == FALSE && bChangeWeapon == FALSE && bChangeWing == FALSE && bChangeHelper == FALSE)
        return;

    if (Hero->Change == FALSE)
    {
        if (bChangeArmor == TRUE)
        {
            DeleteCloth(&m_PhotoChar, NULL, NULL);
            memcpy(&m_PhotoChar.BodyPart, &Hero->BodyPart, sizeof(PART_t) * MAX_BODYPART);
            for (i = 0; i < MAX_BODYPART; ++i)
            {
                m_PhotoChar.BodyPart[i].m_pCloth[0] = NULL;
                m_PhotoChar.BodyPart[i].m_pCloth[1] = NULL;
                m_PhotoChar.BodyPart[i].m_byNumCloth = 0;
            }
        }
        if (bChangeWeapon == TRUE)
        {
            memcpy(&m_PhotoChar.Weapon, &Hero->Weapon, sizeof(PART_t) * 2);
        }
        if (bChangeWing == TRUE)
        {
            memcpy(&m_PhotoChar.Wing, &Hero->Wing, sizeof(PART_t));
            DeleteCloth(NULL, &m_PhotoChar.Object, NULL);
        }
        if (bChangeHelper == TRUE)
        {
            memcpy(&m_PhotoChar.Helper, &Hero->Helper, sizeof(PART_t));
        }
    }
    else	// 변신 상태
    {
        if (bChangeArmor == TRUE)
        {
            DeleteCloth(&m_PhotoChar, NULL, NULL);

            m_PhotoChar.BodyPart[BODYPART_HEAD].Type = static_cast<int>(MODEL_BODY_HELM) + Hero->SkinIndex;
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_HELM], &CharacterMachine->Equipment[EQUIPMENT_HELM],
                static_cast<int>(MODEL_BODY_HELM) + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_ARMOR], &CharacterMachine->Equipment[EQUIPMENT_ARMOR],
                static_cast<int>(MODEL_BODY_ARMOR) + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_PANTS], &CharacterMachine->Equipment[EQUIPMENT_PANTS],
                static_cast<int>(MODEL_BODY_PANTS) + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_GLOVES], &CharacterMachine->Equipment[EQUIPMENT_GLOVES],
                static_cast<int>(MODEL_BODY_GLOVES) + Hero->SkinIndex);
            SetItemToPhoto(&m_PhotoChar.BodyPart[BODYPART_BOOTS], &CharacterMachine->Equipment[EQUIPMENT_BOOTS],
                static_cast<int>(MODEL_BODY_BOOTS) + Hero->SkinIndex);

            for (i = 0; i < MAX_BODYPART; ++i)
            {
                m_PhotoChar.BodyPart[i].m_pCloth[0] = NULL;
                m_PhotoChar.BodyPart[i].m_pCloth[1] = NULL;
                m_PhotoChar.BodyPart[i].m_byNumCloth = 0;
            }
        }
        if (bChangeWeapon == TRUE)
        {
            for (i = 0; i < 2; ++i)
            {
                SetItemToPhoto(&m_PhotoChar.Weapon[i], &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT + i], -1);
            }
        }
        if (bChangeWing == TRUE)
        {
            SetItemToPhoto(&m_PhotoChar.Wing, &CharacterMachine->Equipment[EQUIPMENT_WING], -1);
        }
        if (bChangeHelper == TRUE)
        {
            SetItemToPhoto(&m_PhotoChar.Helper, &CharacterMachine->Equipment[EQUIPMENT_HELPER], -1);
        }
    }

    if (bChangeHelper == TRUE)
    {
        SetPhotoPose(AT_STAND1);
        m_iSettingAnimation = AT_STAND1;
    }
    else
    {
        SetPhotoPose(m_iCurrentAnimation);
    }

    if (bChangeHelper == TRUE || bChangeWeapon == TRUE)
    {
        m_PhotoHelper.Live = false;
        switch (m_PhotoChar.Helper.Type - MODEL_HELPER)
        {
        case 0:CreateMountSub(MODEL_HELPER, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 2:CreateMountSub(MODEL_UNICON, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 3:CreateMountSub(MODEL_PEGASUS, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 4:CreateMountSub(MODEL_DARK_HORSE, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper); break;
        case 37:	//^ 펜릴 편지 관련
            if (m_PhotoChar.Helper.ExcellentFlags == 0x01)
            {
                CreateMountSub(MODEL_FENRIR_BLACK, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            else if (m_PhotoChar.Helper.ExcellentFlags == 0x02)
            {
                CreateMountSub(MODEL_FENRIR_BLUE, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            else if (m_PhotoChar.Helper.ExcellentFlags == 0x04)
            {
                CreateMountSub(MODEL_FENRIR_GOLD, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            else
            {
                CreateMountSub(MODEL_FENRIR_RED, m_PhotoChar.Object.Position, &m_PhotoChar.Object, &m_PhotoHelper);
            }
            break;
        }
        m_PhotoHelper.Alpha = 0;
        m_fPhotoHelperScale = m_PhotoHelper.Scale * 0.7f / m_PhotoChar.Object.Scale;
    }
}

void CUIPhotoViewer::SetClass(CLASS_TYPE byClass)
{
    if (m_bIsInitialized == FALSE) return;
    CHARACTER* c = CharactersClient;
    CharactersClient = &m_PhotoChar;
    CharactersClient->Class = byClass;
    SetChangeClass(CharactersClient);
    CharactersClient = c;
}

void CUIPhotoViewer::SetEquipmentPacket(BYTE* pbyEquip)
{
    if (m_bIsInitialized == FALSE) return;
    //CHARACTER *c = CharactersClient;
    //CharactersClient = &m_PhotoChar;

    ReadEquipmentExtended(0, 0, pbyEquip, &m_PhotoChar, &m_PhotoHelper);

    m_fPhotoHelperScale = m_PhotoHelper.Scale * 0.7f / Hero->Object.Scale;

    if (m_PhotoChar.Wing.Type != -1 && m_iSettingAnimation > AT_HEALING1)
        m_PhotoChar.SafeZone = true;
    else
        m_PhotoChar.SafeZone = false;
}

void CUIPhotoViewer::SetAngle(float fDegree)
{
    if (m_bIsInitialized == FALSE) return;
    OBJECT* o = &m_PhotoChar.Object;
    Vector(-20.f, 5.f, 60.f, o->Angle);

    m_fSettingAngle = fDegree;
    m_fCurrentAngle = fDegree;
}

void CUIPhotoViewer::SetZoom(float fZoom)
{
    m_fSettingZoom = fZoom;
    m_fCurrentZoom = fZoom;
}

void CUIPhotoViewer::SetAnimation(int iAnimationType)
{
    m_iSettingAnimation = iAnimationType;
    SetPhotoPose(m_iSettingAnimation);
}

void CUIPhotoViewer::ChangeAnimation(int iMoveDir)
{
    m_iSettingAnimation = SetPhotoPose(m_iSettingAnimation, iMoveDir);
    m_iCurrentFrame = 0;
    m_bActionRepeatCheck = TRUE;
}

void CUIPhotoViewer::SetID(const wchar_t* pszID)
{
    if (pszID == NULL) return;
    mu_swprintf(m_PhotoChar.ID, pszID);
}

extern bool EquipmentSuccess;

void CUIPhotoViewer::DoAction(BOOL bMessageOnly)
{
    if (bMessageOnly)
        return;

    if (m_bIsWebzenMail == TRUE)
    {
        if (m_bPointerOver)
        {
            MouseOnWindow = true;
        }
        return;
    }

    if (m_bUpdatePlayer == TRUE && EquipmentSuccess == true)
    {
        CopyPlayer();
    }

    m_PhotoChar.EtcPart = Hero->EtcPart;

    if (m_bCanControl)
    {
        // Only the wheel is still read here. Every press-driven control -- turning, the reset and
        // the "?" toggle -- moved to UI::Social::PhotoViewerControl, because a press over the
        // letter's own document never sets MouseLButtonPush at all; see that header.
        if (m_bPointerOver)
        {
            MouseOnWindow = true;
            if (MouseWheel != 0)
            {
                m_bHelpEnable = FALSE;
                m_fCurrentZoom += MouseWheel / 50.0f;
                if (m_fCurrentZoom > 1.1f) m_fCurrentZoom = 1.1f;
                else if (m_fCurrentZoom < 0.8f) m_fCurrentZoom = 0.8f;
                MouseWheel = 0;
            }
        }
    }
    else
    {
        if (m_bPointerOver)
        {
            MouseOnWindow = true;
            if (MouseLButtonPush)
            {
                MouseLButtonPush = FALSE;
                MouseLButton = FALSE;
            }
        }
    }
}

void CUIPhotoViewer::SetPointerOver(bool pointerOver)
{
    m_bPointerOver = pointerOver;
}

void CUIPhotoViewer::TurnBy(float degrees)
{
    m_bHelpEnable = FALSE;
    m_fCurrentAngle += degrees;
}

void CUIPhotoViewer::ResetView()
{
    m_bHelpEnable = FALSE;
    m_fCurrentAngle = m_fSettingAngle;
    m_fCurrentZoom = m_fSettingZoom;
}

void CUIPhotoViewer::ToggleHelp()
{
    m_bHelpEnable = (m_bHelpEnable + 1) % 2;
}

// The target's drawer, from the renderer's offscreen seam. Only the character: the "?" and its help
// are the document's own now, drawn above this image rather than inside it.
void CUIPhotoViewer::RenderInto(std::uint32_t width, std::uint32_t height)
{
    if (m_bIsWebzenMail == TRUE || height == 0)
        return;

    CHARACTER* c = &m_PhotoChar;
    OBJECT* o = &c->Object;

    if (o->AnimationFrame < m_iCurrentFrame)
    {
        if (m_bActionRepeatCheck == FALSE && rand_fps_check(4))
        {
            m_bActionRepeatCheck = TRUE;
            SetPhotoPose(m_iSettingAnimation);
        }
        else
        {
            if (m_iSettingAnimation >= AT_STAND1 && m_iSettingAnimation <= AT_HEALING1);
            else
            {
                m_bActionRepeatCheck = FALSE;
                SetPhotoPose(AT_STAND1);
            }
        }
        m_iCurrentFrame = 0;
    }
    else m_iCurrentFrame = o->AnimationFrame;

    if (c->EtcPart < PARTS_LION)
    {
        DeleteParts(&m_PhotoChar);
    }
    RenderPhotoCharacter(static_cast<float>(width) / static_cast<float>(height));
}
