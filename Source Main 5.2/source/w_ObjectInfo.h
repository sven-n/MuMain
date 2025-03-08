// w_ObjectInfo.h: interface for the ObjectInfo class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>
#include "./Math/ZzzMathLib.h"

class	CInterpolateContainer
{
public:
    struct INTERPOLATE_FACTOR
    {
        float	fRateStart = 0;
        float	fRateEnd = 0;

        vec3_t	v3Start = {};
        vec3_t	v3End = {};
        INTERPOLATE_FACTOR(const float fRateStart_, const float fRateEnd_,
                           const vec3_t& v3Start_, const vec3_t& v3End_)
        {
            fRateStart = fRateStart_;
            fRateEnd = fRateEnd_;

            VectorCopy(v3Start_, v3Start);
            VectorCopy(v3End_, v3End);
        }

        INTERPOLATE_FACTOR() = default;
    };

    struct INTERPOLATE_FACTOR_F
    {
        float	fRateStart;
        float	fRateEnd;

        float	fStart;
        float	fEnd;
        INTERPOLATE_FACTOR_F(const float fRateStart_, const float fRateEnd_,
                             const float& fStart_, const float& fEnd_)
        {
            fRateStart = fRateStart_;
            fRateEnd = fRateEnd_;

            fStart = fStart_; 
            fEnd = fEnd_;
        };

        INTERPOLATE_FACTOR_F() = default;
    };

public:
    typedef std::vector<INTERPOLATE_FACTOR>		VEC_INTERPOLATES;
    typedef std::vector<INTERPOLATE_FACTOR_F>	VEC_INTERPOLATES_F;
    VEC_INTERPOLATES						m_vecInterpolatesAngle;
    VEC_INTERPOLATES						m_vecInterpolatesPos;
    VEC_INTERPOLATES_F						m_vecInterpolatesScale;
    VEC_INTERPOLATES_F						m_vecInterpolatesAlpha;

public:
    void GetCurrentValue(vec3_t& v3Out, float fCurrentRate, VEC_INTERPOLATES& vecInterpolates);
    void GetAngleCurrent(vec3_t& v3Out, float fCurrentRate)
    {
        GetCurrentValue(v3Out, fCurrentRate, m_vecInterpolatesAngle);
    }
    void GetPosCurrent(vec3_t& v3Out, float fCurrentRate)
    {
        GetCurrentValue(v3Out, fCurrentRate, m_vecInterpolatesPos);
    }

    void GetCurrentValueF(float& fOut, float fCurrentRate, VEC_INTERPOLATES_F& vecInterpolates);
    void GetScaleCurrent(float& fOut, float fCurrentRate)
    {
        GetCurrentValueF(fOut, fCurrentRate, m_vecInterpolatesScale);
    }

    void GetAlphaCurrent(float& fOut, float fCurrentRate)
    {
        GetCurrentValueF(fOut, fCurrentRate, m_vecInterpolatesAlpha);
    }

public:
    void ClearContainer();

    CInterpolateContainer() = default;
    ~CInterpolateContainer() = default;
};

typedef struct tagMU_POINTF
{
    float x;
    float y;
} MU_POINTF;

typedef struct tagSIZEF
{
    float cx;
    float cy;
} SIZEF;

typedef struct
{
    vec3_t StartPos;
    vec3_t XAxis;
    vec3_t YAxis;
    vec3_t ZAxis;
} OBB_t;

class OBJECT
{
public:
    OBJECT();
    virtual ~OBJECT();

public:
    void Initialize();
    void Destroy();

public:
    bool          Live;
    bool          bBillBoard;
    bool          m_bCollisionCheck;
    bool          m_bRenderShadow;
    bool          EnableShadow;
    bool		  LightEnable;
    bool		  m_bActionStart;
    bool		  m_bRenderAfterCharacter;
    bool	      Visible;
    bool	      AlphaEnable;
    bool          EnableBoneMatrix;
    bool		  ContrastEnable;
    bool          ChromeEnable;

public:
    unsigned char AI;
    unsigned short CurrentAction;
    unsigned short PriorAction;

public:
    BYTE          ExtState;
    BYTE          Teleport;
    BYTE          Kind;
    WORD		Skill;
    BYTE		  m_byNumCloth;
    float		  m_byHurtByDeathstab;
    BYTE          WeaponLevel;
    BYTE          DamageTime;
    BYTE          m_byBuildTime;
    BYTE		  m_bySkillCount;
    BYTE		m_bySkillSerialNum;
    BYTE		  Block;
    void* m_pCloth;

public:
    short         ScreenX;
    short         ScreenY;
    short         Weapon;

public:
    int			  Type;
    int           SubType;
    int			  m_iAnimation;
    int           HiddenMesh;
    float           LifeTime;
    int           BlendMesh;
    int           AttackPoint[2];
    int           RenderType;
    int			  InitialSceneTime;
    int           LinkBone;

public:
    DWORD			m_dwTime;

public:
    float         Scale;
    float         BlendMeshLight;
    float         BlendMeshTexCoordU;
    float         BlendMeshTexCoordV;
    float         Timer;
    float         m_fEdgeScale;
    float         Velocity;
    float		  CollisionRange;
    float         ShadowScale;
    float         Gravity;
    float         Distance;
    float         AnimationFrame;
    float         PriorAnimationFrame;
    float	      AlphaTarget;
    float         Alpha;


    float       LastHorseWaveEffect;
    float         PKKey;

public:
    vec3_t        Light;
    vec3_t        Direction;
    vec3_t		  m_vPosSword;
    vec3_t		  StartPosition;
    vec3_t        BoundingBoxMin;
    vec3_t        BoundingBoxMax;
    vec3_t		  m_vDownAngle;
    vec3_t		  m_vDeadPosition;
    vec3_t        Position;
    vec3_t	 	  Angle;
    vec3_t	 	  HeadAngle;
    vec3_t	   	  HeadTargetAngle;
    vec3_t  	  EyeLeft;
    vec3_t  	  EyeRight;
    vec3_t		  EyeLeft2;
    vec3_t		  EyeRight2;
    vec3_t		  EyeLeft3;
    vec3_t		  EyeRight3;
public:
    vec34_t	 	  Matrix;
    vec34_t* BoneTransform;

public:
    OBB_t		  OBB;

public:
    OBJECT* Owner;
    OBJECT* Prior;
    OBJECT* Next;

public:
    Buff		  m_BuffMap;

public:
    short int		m_sTargetIndex;

public:
    BOOL		m_bpcroom;
    vec3_t		m_v3PrePos1;
    vec3_t		m_v3PrePos2;

    CInterpolateContainer	m_Interpolates;
};
