// PhysicsManager.h: interface for the CPhysicsManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHYSICSMANAGER_H__11A9449A_CF75_4963_8F71_C8EB8EA7FE2D__INCLUDED_)
#define AFX_PHYSICSMANAGER_H__11A9449A_CF75_4963_8F71_C8EB8EA7FE2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "ZzzBmd.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "BaseCls.h"



//--------------------------------------------------------------------
//
//                          Physics Vertex
//
//--------------------------------------------------------------------

#define PVS_NORMAL		( 0x00)
#define PVS_FIXEDPOS	( 0x01)

class CPhysicsVertex
{
public:
	CPhysicsVertex();
	virtual ~CPhysicsVertex();
	void Clear( void);

	static float s_Gravity;
	static float s_fMass;
	static float s_fInvOfMass;

protected:
	vec3_t	m_vForce;
	vec3_t	m_vVel;
	vec3_t	m_vPos;
	BYTE	m_byState;

	// 기본 함수
public:
	void Init( float fXPos, float fYPos, float fZPos, BOOL bFixed = FALSE);
    void UpdateForce( unsigned int iKey, DWORD dwType = 0, float fWind=0.f);
	void AddToForce( float fXForce, float fYForce, float fZForce);
	void Move( float fTime);

	// 정보 얻기
public:
	void GetPosition( vec3_t *pPos);
	float GetDistance( CPhysicsVertex *pVertex2, vec3_t *pDistance);

	// 확장 기능
protected:
	int m_iCountOneTimeMove;
	vec3_t m_vOneTimeMove;
public:
	BOOL KeepLength( CPhysicsVertex *pVertex2, float *pfLength);
	void AddOneTimeMoveToKeepLength( CPhysicsVertex *pVertex2, float fLength);
	void DoOneTimeMove( void);
	void AddOneTimeMove( vec3_t vMove);
};


//--------------------------------------------------------------------
//
//                          Physics Collision
//
//--------------------------------------------------------------------

enum ENUM_COLLISION_TYPE
{
	CLT_DEFAULT = 0,
	CLT_SPHERE,
	NUM_CLT
};


class CPhysicsCollision
{
public:
	CPhysicsCollision();
	virtual ~CPhysicsCollision();
	void Clear( void);

protected:
	vec3_t m_vCenterBeforeTransform;
	int m_iBone;
	vec3_t m_vCenter;

public:
	virtual int GetType( void)	{ return ( CLT_DEFAULT); }

	int GetBone( void)	{ return ( m_iBone); }
	void SetPosition( float fXPos, float fYPos, float fZPos);
	void GetCenter( vec3_t vCenter);
	void GetCenterBeforeTransform( vec3_t vCenter);
	virtual void ProcessCollision( CPhysicsVertex *pVertex);
};

//--------------------------------------------------------------------
//
//                          Physics Collision Sphere
//
//--------------------------------------------------------------------

class CPhysicsColSphere : public CPhysicsCollision
{
public:
	CPhysicsColSphere();
	virtual ~CPhysicsColSphere();
	void Clear( void);

protected:
	float m_fRadius;

public:
	virtual int GetType( void)	{ return ( CLT_SPHERE); }

	void Init( float fXPos, float fYPos, float fZPos, float fRadius, int iBone);
	virtual void ProcessCollision( CPhysicsVertex *pVertex);
	float GetRadius( void)	{ return ( m_fRadius); }
};


//--------------------------------------------------------------------
//
//                          Physics Cloth
//
//--------------------------------------------------------------------


//------------------------------ 천 속성 --------------------------------------
// 생김새
#define PCT_MASK_SHAPE			( 0x00000003)
#define PCT_FLAT				( 0x00000000)	// 평평한 모양
#define PCT_CURVED				( 0x00000001)	// 윗부분이 둥글게 감싸짐
#define PCT_STICKED				( 0x00000002)	// flat 이 몸에서 20.0f 떨어져 있는데 이것을 쓰면 바짝 붙는다.
// 생김새 2
#define PCT_MASK_SHAPE_EXT		( 0x0000000C)
#define PCT_SHAPE_NORMAL		( 0x00000000)	// 보통 모양
#define PCT_SHORT_SHOULDER		( 0x00000004)	// 윗부분이 좁은 사다리꼴 형태
#define PCT_CYLINDER			( 0x00000008)	// 원통형 모양 : 칸투르 보스(나이트메어) 망토
// 생김새 3
#define PCT_MASK_SHAPE_EXT2		( 0x00000030)
#define PCT_SHAPE_HALLOWEEN		( 0x00000010)	// 할로위 이벤트에 쓰일 머플러(위치때문에)

// 탄성
#define PCT_MASK_ELASTIC		( 0x00000300)
#define PCT_COTTON				( 0x00000000)	// 보통 탄성
#define PCT_RUBBER				( 0x00000100)	// 탄성력 최고 - 시노비처럼
#define PCT_RUBBER2				( 0x00000200)	// 탄성력 최고 - 시노비처럼
// 무게
#define PCT_MASK_WEIGHT			( 0x00000C00)
#define PCT_NORMAL_THICKNESS	( 0x00000000)	// 보통 무게
#define PCT_HEAVY				( 0x00000400)	// 무겁게 - 무게감 생김
// 알파
#define PCT_MASK_DRAW			( 0x00003000)
#define PCT_MASK_BLIT			( 0x00000000)	// 보통 출력
#define PCT_MASK_ALPHA			( 0x00001000)	// 알파 블렌딩
#define PCT_MASK_BLEND          ( 0x00002000)   // 블랜딩효과.

// 탄성 확장
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
#define PCT_MASK_ELASTIC_EXT	( 0x0000E000)
#else //PBG_ADD_NEWCHAR_MONK_ITEM
#define PCT_MASK_ELASTIC_EXT	( 0x0000C000)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#define PCT_ELASTIC_HALLOWEEN	( 0x00004000)	// 탄성 할로윈 이벤트 머플러효과
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
#define PCT_ELASTIC_RAGE_L		( 0x00008000)
#define PCT_ELASTIC_RAGE_R		( 0x0000C000)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

// 추가 속성
#define PCT_OPT_MESHPROG		( 0x10000000)
#define PCT_OPT_CORRECTEDFORCE	( 0x20000000)	// 메쉬 사이즈가 작은 경우 이것을 넣음
#define PCT_MASK_LIGHT          ( 0x40000000)	// 빛효과.
#define PCT_OPT_HAIR            ( 0x80000000)   // 머리카락을 위한 효과.

// Link 스타일
#define PLS_NORMAL				( 0x00)	// 보통 연결
#define PLS_LOOSEDISTANCE		( 0x01)	// 일반 연결 ( 거리 유지)
#define PLS_SPRING				( 0x02)	// 탄성력 적용
#define PLS_STRICTDISTANCE		( 0x04)	// 강제 거리 유지

typedef struct
{
	short	m_nVertices[2];
	float	m_fDistance[2];
	BYTE	m_byStyle;
} St_PhysicsLink;

class CPhysicsCloth
{
public:
	CPhysicsCloth();
	virtual ~CPhysicsCloth();
	void Clear( void);

protected:
	// 기본 정보
	OBJECT *m_oOwner;
	int m_iBone;
	int m_iTexFront;
	int m_iTexBack;
	DWORD m_dwType;
	// 격자 정보
	float m_fxPos, m_fyPos, m_fzPos;
	float m_fWidth, m_fHeight;
	int m_iNumHor, m_iNumVer;
	int m_iNumVertices;
	CPhysicsVertex *m_pVertices;
	int m_iNumLink;
	St_PhysicsLink *m_pLink;

    //  윈드.
    float   m_fWind;
    BYTE    m_byWindMax;
    BYTE    m_byWindMin;

#ifdef LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING
	int		m_iReferenceModelType;
	int		m_iReferenceMeshSeq;
#endif // LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING

	// 정보
public:
	OBJECT* GetOwner( void)	{ return ( m_oOwner); }

	// 생성
protected:
	float m_fUnitWidth, m_fUnitHeight;
public:
#ifdef LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX
	virtual BOOL Create( OBJECT *o, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int TexBack, DWORD dwType = 0, int iIdxPhysiqModel = -1, int iReferenceModelType = -1);
#else // LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX
	virtual BOOL Create( OBJECT *o, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int TexBack, DWORD dwType = 0);
#endif // LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX
	virtual void Destroy( void);
protected:
	virtual void SetFixedVertices( float Matrix[3][4]);
	virtual void NotifyVertexPos( int iVertex, vec3_t vPos)	{}
	void SetLink( int iLink, int iVertex1, int iVertex2, float fDistanceSmall, float fDistanceLarge, BYTE byStyle);

	// 움직임
public:
	BOOL Move2( float fTime, int iCount);
	BOOL Move( float fTime);
protected:
	virtual void InitForces( void);
	void MoveVertices( float fTime);
	BOOL PreventFromStretching( void);

	// 그리기
public:
	virtual void Render( vec3_t *pvColor = NULL, int iLevel = 0);
protected:
	void RenderFace( BOOL bFront, int iTexture, vec3_t *pvRenderPos);
	void RenderVertex( vec3_t *pvRenderPos, int xVertex, int yVertex);
	void RenderCollisions( void);
	
#ifdef LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX
public:
	void Render_MappingOption(	BMD& b, int iMesh, 
								int iRenderFlag, BOOL bFront, 
								vec3_t &v3VertexColor, int iTextureIdx = -1, 
								float fMeshAlpha = 1.0f, 
								float fBright = 1.0f,
								BOOL bVertexColorProcessMappingFunc = true );
	
protected:
	void RenderFaceMappingOptionColor( BMD& b, int iMesh, int iRenderFlag, vec3_t *pvRenderPos, BOOL bFront, vec3_t &v3VertexColor, int iTextureIdx = -1, float fMeshAlpha = 1.0f );

	void RenderVertexMappingOptionAlphaONE( vec3_t *pvRenderPos, int xVertex, int yVertex, float** arrfChromeUV,vec3_t &v3VertexColor );
	void RenderVertexMappingOptionAlpha( vec3_t *pvRenderPos, int xVertex, int yVertex, float** arrfChromeUV, vec3_t &v3VertexColor, float fVertexAlpha );
#endif // LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX

	// 충돌 정보
protected:
	CList<CPhysicsCollision*> m_lstCollision;
public:
    void SetWindMinMax ( BYTE Min, BYTE Max ) { m_byWindMin = Min; m_byWindMax = Max; }
	void AddCollisionSphere( float fXPos, float fYPos, float fZPos, float fRadius, int iBone);
	void ProcessCollision( void);
};


//--------------------------------------------------------------------
//
//                     Physics Cloth from Mesh
//
//--------------------------------------------------------------------

class CPhysicsClothMesh	: public CPhysicsCloth
{
public:
    CPhysicsClothMesh();
	virtual ~CPhysicsClothMesh();
	virtual void Clear( void);

protected:
	int m_iMesh;
	int m_iBoneForFixed;
	int m_iBMDType;

public:
	virtual BOOL Create( OBJECT *o, int iMesh, int iBone, DWORD dwType = 0, int iBMDType = -1);
	virtual BOOL Create( OBJECT *o, int iMesh, int iBone, float fxPos, float fyPos, float fzPos, int iNumHor, int iNumVer, float fWidth, float fHeight, int iTexFront, int TexBack, DWORD dwType = 0, int iBMDType = -1);
protected:
	int FindMatchVertex( Mesh_t *pMesh, int iV1, int iV2, int iV3);
	BOOL FindInLink( int iCount, int iV1, int iV2);
	virtual void SetFixedVertices( float Matrix[3][4]);
	virtual void NotifyVertexPos( int iVertex, vec3_t vPos);
	virtual void InitForces( void);
public:
	virtual void Render( vec3_t *pvColor = NULL, int iLevel = 0);
};


//--------------------------------------------------------------------
//
//                          Physics Manager
//
//--------------------------------------------------------------------

class CPhysicsManager
{
public:
	CPhysicsManager();
	virtual ~CPhysicsManager();
	void Clear( void);

	static float s_fWind;
	static vec3_t s_vWind;

public:
	void Move( float fTime);
	void Render( void);

	// 리스트
protected:
	CList<CPhysicsCloth*> m_lstCloth;
public:
	void Add( CPhysicsCloth *pCloth);
	void Remove( OBJECT *oOwner);
	void RemoveAll( void);
};


#endif // !defined(AFX_PHYSICSMANAGER_H__11A9449A_CF75_4963_8F71_C8EB8EA7FE2D__INCLUDED_)
