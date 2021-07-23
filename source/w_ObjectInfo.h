// w_ObjectInfo.h: interface for the ObjectInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_OBJECT_H__4FAC8EFA_AAC5_490A_B4A1_FD61801D8AE0__INCLUDED_)
#define AFX_W_OBJECT_H__4FAC8EFA_AAC5_490A_B4A1_FD61801D8AE0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
#include <vector>
#include "./Math/ZzzMathLib.h"

class	CInterpolateContainer
{
public:
	struct INTERPOLATE_FACTOR
	{
		float	fRateStart;
		float	fRateEnd;
		
		vec3_t	v3Start;
		vec3_t	v3End;
		INTERPOLATE_FACTOR( float fRateStart_, float fRateEnd_,
			vec3_t& v3Start_, vec3_t& v3End_ )
		{
			fRateStart = fRateStart_;
			fRateEnd = fRateEnd_;
			
			VectorCopy(v3Start_, v3Start);
			VectorCopy(v3End_, v3End);
		};
		
		INTERPOLATE_FACTOR()
		{
			
		};
	};
	
	struct INTERPOLATE_FACTOR_F
	{
		float	fRateStart;
		float	fRateEnd;
		
		float	fStart;
		float	fEnd;
		INTERPOLATE_FACTOR_F( float fRateStart_, float fRateEnd_,
			float& fStart_, float& fEnd_ )
		{
			fRateStart = fRateStart_;
			fRateEnd = fRateEnd_;
			
			fStart = fStart_;
			fEnd = fEnd_;
		};
		
		INTERPOLATE_FACTOR_F()
		{
			
		};
	};
	

public:
	typedef vector<INTERPOLATE_FACTOR>		VEC_INTERPOLATES;
	typedef vector<INTERPOLATE_FACTOR_F>	VEC_INTERPOLATES_F;
	VEC_INTERPOLATES						m_vecInterpolatesAngle;
	VEC_INTERPOLATES						m_vecInterpolatesPos;
	VEC_INTERPOLATES_F						m_vecInterpolatesScale;
	VEC_INTERPOLATES_F						m_vecInterpolatesAlpha;	
		
	public:
		void GetCurrentValue(vec3_t& v3Out, float fCurrentRate, VEC_INTERPOLATES& vecInterpolates );
		void GetAngleCurrent(vec3_t& v3Out, float fCurrentRate )
		{
			GetCurrentValue( v3Out, fCurrentRate, m_vecInterpolatesAngle );
		}
		void GetPosCurrent(vec3_t& v3Out, float fCurrentRate )
		{
			GetCurrentValue( v3Out, fCurrentRate, m_vecInterpolatesPos );
		}

		void GetCurrentValueF(float& fOut, float fCurrentRate, VEC_INTERPOLATES_F& vecInterpolates );
		void GetScaleCurrent(float& fOut, float fCurrentRate )
		{
			GetCurrentValueF( fOut, fCurrentRate, m_vecInterpolatesScale );
		}

		void GetAlphaCurrent(float& fOut, float fCurrentRate )
		{
			GetCurrentValueF( fOut, fCurrentRate, m_vecInterpolatesAlpha );
		}
		
	public:
		void ClearContainer();
		
		CInterpolateContainer(){};
		~CInterpolateContainer(){};
};



#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN


#ifdef _VS2008PORTING
typedef struct tagMU_POINTF
{
	float x;
	float y;
} MU_POINTF;
#else // _VS2008PORTING
typedef struct tagPOINTF
{
	float x;
	float y;
} POINTF;
#endif // _VS2008PORTING

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
	bool          m_bCollisionCheck;		//  충돌 체크 할까?
	bool          m_bRenderShadow;			//  그림자를 찍을까? 말까?
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
#ifdef PBG_ADD_NEWCHAR_MONK_ANI	
	//애니메이션255개 이상 추가로인한 확장
	unsigned short CurrentAction;
	unsigned short PriorAction;
#else //PBG_ADD_NEWCHAR_MONK_ANI
	unsigned char CurrentAction;
	unsigned char PriorAction;
#endif //PBG_ADD_NEWCHAR_MONK_ANI

public:
	BYTE          ExtState;
	BYTE          Teleport;
	BYTE          Kind;
	WORD		Skill;
	BYTE		  m_byNumCloth;				// 천 개수
	BYTE		  m_byHurtByOneToOne;
	BYTE          WeaponLevel;
	BYTE          DamageTime;				//  공격을 받은 충격 시간.
	BYTE          m_byBuildTime;
	BYTE		  m_bySkillCount;
	BYTE		m_bySkillSerialNum;
	BYTE		  Block;
	void*		  m_pCloth;	// 천 붙이기

public:
	short         ScreenX;
	short         ScreenY;
	short         PKKey;
	short         Weapon;

public:
	int			  Type;
	int           SubType;
	int			  m_iAnimation;				// 펜릴 발바닥 관련 에니메이션 키값
	int           HiddenMesh;
	int           LifeTime;
	int           BlendMesh;
	int           AttackPoint[2];
	int           RenderType;
	int			  InitialSceneFrame;
	int           LinkBone;

public:
	DWORD			m_dwTime;				// 펜릴 관련 시간재는 변수로 사용
	
public:
	float         Scale;
	float         BlendMeshLight;
	float         BlendMeshTexCoordU;
	float         BlendMeshTexCoordV;
	float         Timer;
	float         m_fEdgeScale;				//  외곽선 스케일.	
	float         Velocity;
	float		  CollisionRange;
	float         ShadowScale;
	float         Gravity;
	float         Distance;
	float         AnimationFrame;
	float         PriorAnimationFrame;
	float	      AlphaTarget;
	float         Alpha;
	
public:
	vec3_t        Light;
	vec3_t        Direction;
	vec3_t		  m_vPosSword;				// 칼끝 위치
	vec3_t		  StartPosition;			// bullet
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
	vec34_t       *BoneTransform;

public:
	OBB_t		  OBB;
	
public:
	OBJECT        *Owner;
	OBJECT		  *Prior;
	OBJECT		  *Next;

public:
	Buff		  m_BuffMap;

public:
	short int		m_sTargetIndex;			// 체인라이트닝 스킬에서, 다음 타겟 오브젝트의 인덱스를 저장 (Effect 에서 사용)

#if defined PCROOM_EVENT
public:
	BOOL		m_bpcroom;		// 피씨방 관련 이벤트 전용 NPC 체크 변수
#endif //PCROOM_EVENT

#if defined MR0 || defined OBJECT_ID
public:
	unsigned int  m_iID;
#endif //MR0

	vec3_t		m_v3PrePos1;
	vec3_t		m_v3PrePos2;

#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
	CInterpolateContainer	m_Interpolates;
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
};

#endif // !defined(AFX_W_OBJECT_H__4FAC8EFA_AAC5_490A_B4A1_FD61801D8AE0__INCLUDED_)
