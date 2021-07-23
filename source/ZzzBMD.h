#ifndef __ZZZBMD_H__
#define __ZZZBMD_H__

#include "./Shader/ModelManager.h"
#include "TextureScript.h"

#define MAX_BONES    200
#define MAX_MESH     50
#define MAX_VERTICES 15000

#define RENDER_COLOR        0x00000001
#define RENDER_TEXTURE      0x00000002
#define RENDER_CHROME       0x00000004
#define RENDER_METAL        0x00000008
#define RENDER_LIGHTMAP     0x00000010
#define RENDER_SHADOWMAP    0x00000020

#define RENDER_BRIGHT       0x00000040
#define RENDER_DARK         0x00000080

#define RENDER_EXTRA        0x00000100
#define RENDER_CHROME2      0x00000200
#define RENDER_WAVE			0x00000400
#define RENDER_CHROME3      0x00000800  //  세트 효과에 사용됨.
#define RENDER_CHROME4      0x00001000  //  +13 효과에 사용.
#define RENDER_NODEPTH      0x00002000
#define RENDER_CHROME5      0x00004000  //  은은한 빛을 받는다.
#define RENDER_OIL          0x00008000  //  기름 같은 느낌
#define RENDER_CHROME6      0x00010000
#define RENDER_CHROME7      0x00020000	// 칸투르 수정 오브젝트 크롬 효과.
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
#define RENDER_DOPPELGANGER        0x00040000	// 도플갱어 몬스터 이펙트
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
#ifdef PJH_NEW_CHROME
#define RENDER_CHROME8      0x00080000	// 럭키아이템.
#endif //PJH_NEW_CHROME



#define RENDER_WAVE_EXT		0x10000000
#define RENDER_BYSCRIPT		0x80000000	//	완전히 외부 스크립트로만 그린다. 

#define RNDEXT_WAVE			1
#define RNDEXT_OIL          2
#define RNDEXT_RISE			4

#define MAX_MONSTER_SOUND   10//5

typedef struct
{
	vec3_t Position;
	vec3_t Color;
	float  Range;
} Light_t;

typedef struct
{
	vec3_t *Position;
	vec3_t *Rotation;
	vec4_t *Quaternion;
} BoneMatrix_t;

typedef struct
{
	char         Name[32];
	short        Parent;
	char         Dummy;
	BoneMatrix_t *BoneMatrixes;
	char         BoundingBox;
	vec3_t       BoundingVertices[8];
} Bone_t;

typedef struct
{
	char FileName[32];
} Texture_t;

typedef struct
{
	unsigned char Width;
	unsigned char Height;
	unsigned char *Buffer;
} Bitmap_t;

typedef struct
{
	short  Node;
	vec3_t Position;
} Vertex_t;

typedef struct
{
	short  Node;
	vec3_t Normal;
	short  BindVertex;
} Normal_t;

typedef struct
{
	float TexCoordU;
	float TexCoordV;
} TexCoord_t;

typedef struct 
{
	BYTE m_Colors[3];	//0~255 RGB
} VertexColor_t;

typedef struct
{
	char       Polygon;
	short      VertexIndex[4];
	short      NormalIndex[4];
	short      TexCoordIndex[4];
	short      EdgeTriangleIndex[4];
	bool       Front;
	//TexCoord_t LightMapCoord[4]; //ver1.2
	//short      LightMapIndexes; //ver1.2
} Triangle_t;

typedef struct
{
	char       Polygon;
	short      VertexIndex[4];
	short      NormalIndex[4];
	short      TexCoordIndex[4];
	TexCoord_t LightMapCoord[4]; //ver1.2
	short      LightMapIndexes; //ver1.2
} Triangle_t2;


typedef struct
{
	bool          Loop;
	float         PlaySpeed;
	short         NumAnimationKeys;
	bool          LockPositions;
	vec3_t        *Positions;
} Action_t;

typedef struct _Triangle_t3 : public Triangle_t
{
	short	   m_ivIndexAdditional[4];
} Triangle_t3;

//////////////////////////////////////////////////////////////////////////
/*
	Vertex buffer Object 를 위한 새 메쉬 구조체
	Idas
*/
//////////////////////////////////////////////////////////////////////////
#pragma warning( disable : 4786 )
#include <vector>

//기본 유닛
typedef struct  _VertexUnit
{
	vec4_t	m_vTex;
	vec4_t	m_vCor;
	vec3_t	m_vNorm;
	vec4_t	m_vPos;

	//GL_T4F_C4F_N3F_V4F

}VertexUnit;

//버텍스 컬러를 쓰지 않는 축약된 유닛
typedef struct _VertexUnit2
{
	vec4_t	m_vTex;
	vec3_t	m_vNorm;
	vec3_t	m_vPos;

	//GL_C4F_N3F_V3F 사용

}VertexUnit2;

//텍스쳐좌표와 위치값만을 갖는 축약된 유닛
typedef struct _VertexUnit3
{
	vec4_t	m_vTex;
	vec4_t	m_vPos;

	//GL_T4F_V4F 사용

}VertexUnit3;

//Vertex Animation 기본 구조
typedef struct _Point3
{
	float x, y, z;
	_Point3() : x(0.f), y(0.f), z(0.f) {}
}Point3;
typedef std::vector<Point3> VertexTrack_t;
typedef struct _AniTrack_t
{
	unsigned int	m_iIndex;		//몇 번 버텍스에 대한 애니메이션 트랙인가
	VertexTrack_t	m_TrackData;	//해당 버텍스에 대한 트랙

	_AniTrack_t() {
		m_iIndex = 0;
		m_TrackData.clear();
	}
} AniTrack_t;

class CVertexAnimation
{
public:
	bool			m_bLoop;			//루프되는 애니메이션인가?
	
	//Shader Only
	bool			m_bParametric;		//트랙 없이 특정 쉐이더를 대상으로 갖는 버텍스 애니메이션인가?
	unsigned int	m_iShaderID;		//쉐이더 ID

	float			m_fSpeed;			//애니메이션 속도
	unsigned int	m_iModelIdx;		//참조하는 모델의 인덱스
	std::string		m_strRefModel;		//참조하는 모델의 이름
	AniTrack_t		m_VertexTrack;		//버텍스 애니메이션
	
	CVertexAnimation() : m_bLoop(true), m_bParametric(false), m_iShaderID(0), m_fSpeed(0.f),
		m_iModelIdx(0) { m_strRefModel = ""; }

	bool			Load(const std::string& fn);
	bool			Save(const std::string& fn);
	void			PlayAnimation(bool bRev = false);
	void			Dump();								//버텍스 버퍼에 업로드
	void			SetStop();
};
typedef std::vector<CVertexAnimation> VertexAniList_t;


//Two link 임시 저장을 위한 구조체
typedef struct _TempSkinInfo_t
{
	float	m_fWeight0;
	int		m_iBone1;
	float	m_fWeight1;
}TempSkinInfo_t;
typedef std::map<int, TempSkinInfo_t> TempSkinInfoMap_t;

//텍스쳐 단위 메쉬
//(2^16 - 1) 개의 인덱스를 포함한다.
typedef std::vector<VertexUnit> UnitVector;
typedef std::vector<VertexUnit2> UnitVector2;
typedef std::vector<VertexUnit3> UnitVector3;
typedef struct  _VBOMesh
{
	bool			NoneBlendMesh;
	short			Texture;
	unsigned int	Vbo;
	unsigned int	IdxVbo;
	unsigned int	RealType;			//실제 메쉬의 버텍스 구조
	TextureScript*	m_csTScript;

	UnitVector		VBuffer;
	UShortVector	IBuffer;

	ShortVector		BoneContainer;		//본 컨테이너. m_vPos[3]에 들어가는 본 ID는 이 버퍼의 인덱스이다.
	VertexAniList_t	VertexAniList;		//버텍스 애니메이션 리스트
	EffectScripVec_t MeshEffectScript;	//Post Effect Struct		
	
#ifdef LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
	bool			RegisterBone(const float* BoneTransform);
#else // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
	bool			RegisterBone( const float* BoneTransform, 
								bool bTrans, 
								vec3_t vTrans, 
								float fScale, 
								bool bApplyRequestAbsoluteScale = false, 
								float fRequestAbsoluteScale = 0.0f );
#else // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
	bool			RegisterBone(const float* BoneTransform, bool bTrans, vec3_t vTrans, float fScale);
#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
#endif // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER

	bool			RegisterBone(float (*BoneMatrix)[3][4], bool bTrans, vec3_t vTrans, float fScale);

	_VBOMesh() : NoneBlendMesh(true), Texture(0), 
		Vbo(0), IdxVbo(0), 
		RealType(GL_T4F_C4F_N3F_V4F),
		m_csTScript(NULL) { 
		VBuffer.clear(); IBuffer.clear(); }

}VBOMesh;
typedef std::vector<VBOMesh> MeshVector;

//////////////////////////////////////////////////////////////////////////


typedef struct _Mesh_t
{
    bool          NoneBlendMesh;
	short         Texture;
	short         NumVertices;
	short         NumNormals;
	short         NumTexCoords;
	short		  NumVertexColors;	//ver1.3 이상
	short         NumTriangles;
	int           NumCommandBytes; //ver1.1
	Vertex_t      *Vertices;
	Normal_t      *Normals;
	TexCoord_t    *TexCoords;
	VertexColor_t* VertexColors;	//ver1.3 이상
	Triangle_t    *Triangles;
	unsigned char *Commands; //ver1.1

    TextureScript* m_csTScript;

	EffectScripVec_t	MeshEffectScript;			//Post Effect Struct
	TempSkinInfoMap_t	MeshAdditionalSkinInfo;		//Temp Skining Infomation

	_Mesh_t()
	{	
		Vertices = NULL;
		Normals = NULL;
		Triangles = NULL;
		Commands = NULL;
		m_csTScript= NULL;

		NumVertices = NumNormals = NumTexCoords = 
			NumVertexColors = NumTriangles = 0;
	}

} Mesh_t;

class BMD
{
public:
	char          Name[32];
	char          Version;
    short         NumBones;
	short         NumMeshs;
	short         NumActions;
	Mesh_t        *Meshs;
	Bone_t        *Bones;
	Action_t      *Actions;
	Texture_t     *Textures;
	GLuint*		  IndexTexture;

	short         NumLightMaps;  //ver1.2
	short         IndexLightMap; //ver1.2
	Bitmap_t      *LightMaps;    //ver1.2

	bool          LightEnable;
	bool          ContrastEnable;
	vec3_t        BodyLight;
	int           BoneHead;

    int           BoneFoot[4];
	float         BodyScale;
	vec3_t        BodyOrigin;
	vec3_t        BodyAngle;
	float         BodyHeight;
	char          StreamMesh;
	vec3_t        ShadowAngle;
	char          Skin;
	bool          HideSkin;
    float         Velocity;
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
	//애니메이션 255개이상추가로 인한 확장
	unsigned short CurrentAction;
	unsigned short PriorAction;
#else //PBG_ADD_NEWCHAR_MONK_ANI
	unsigned char CurrentAction;
	unsigned char PriorAction;
#endif //PBG_ADD_NEWCHAR_MONK_ANI
	float         CurrentAnimation;
	short         CurrentAnimationFrame;
	short         Sounds[MAX_MONSTER_SOUND];
    int           renderCount;
	float		  fTransformedSize;
	
//////////////////////////////////////////////////////////////////////////
//Light Object 지원
	UIntVector	  LightIDVector;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//VBO Version
	unsigned int		m_iBMDSeqID;
	MeshVector			NewMeshs;
	bool				bLightMap;
	bool				bOffLight;		//무조건 라이트를 적용시키지 않는다.
	char				iBillType;		//빌보드 타입

#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
	float				m_fRequestScale;	// 사용자 Scale 변환 변수 (Sillouette효과 등 메쉬의 크기를 임시 변환 시 사용)
#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	bool				m_bCompletedAlloc;	// 초기화가 완료 되었는지 여부판단
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	
#ifdef LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING
	bool				m_bProcessAnimationTMUsingPassedMothod;
#endif // LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING

	// MR0:VBOMesh로 변환 이후 ValidationCheck정보를 화면상에 출력 여부 인자를 추가.
#ifdef LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
	void				ConvertMesh(bool bOutputRealtimeInfo = false);
#else // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
	void				ConvertMesh();
#endif // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH

	void				UploadVBO();		//VBO에 업로드
	void				UnloadVBO();		//VBO로부터 로컬 언로드
	void				DelOldMesh();		//이전 버전 메쉬 구조체를 날린다. 

	//수정사항이 너무 많아서 함수단으로 분기한 BMD 1.2버전의 함수들
	void				RenderBodyOld(int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int HiddenMesh=-1,int Texture=-1);
	void				RenderBodyShadowOld(int BlendMesh=-1,int HiddenMesh=-1, int StartMeshNumber=-1, int EndMeshNumber=-1);
	
#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	bool				Open2Old(char *DirName,char *FileName, bool bReAlloc = true);
#else // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	bool				Open2Old(char *DirName,char *FileName);
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL


	bool				Save2Old(char *DirName,char *FileName);

	BMD() : NumBones(0), NumActions(0), NumMeshs(0), 
		Meshs(NULL), Bones(NULL), Actions(NULL), Textures(NULL), IndexTexture(NULL)
	{
		LightEnable = false;
		ContrastEnable = false;
		HideSkin = false;
		bLightMap = false;
		iBillType = -1;
		bOffLight = false;

#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
		m_fRequestScale = 0.0f;			// 사용자 Scale 변환 변수 초기화 (Sillouette효과 등 메쉬의 크기를 임시 변환 시 사용)
#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		m_bCompletedAlloc = false;		// 초기화 대응 변수
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	}

#ifdef LDS_FIX_MEMORYLEAK_DESTRUCTORFORBMD
~BMD();		// 메모리 릭 발생으로 소멸자 추가.
#endif // LDS_FIX_MEMORYLEAK_DESTRUCTORFORBMD
//////////////////////////////////////////////////////////////////////////

    //utility
    void Init(bool Dummy);
	bool Open(char *DirName,char *FileName);
	bool Save(char *DirName,char *FileName);

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	bool Open2(char *DirName,char *FileName, bool bReAlloc = true);
#else // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	bool Open2(char *DirName,char *FileName);
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL

	bool Save2(char *DirName,char *FileName);	// 암호화
	void Release();
    void CreateBoundingBox();

    //transform
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
	bool PlayAnimation(float *AnimationFrame,float *PriorAnimationFrame,unsigned short *PriorAction,float Speed,vec3_t Origin,vec3_t Angle);
    void Animation(float (*BoneTransform)[3][4],float AnimationFrame,float PriorAnimationFrame,unsigned short PriorAction,vec3_t Angle,vec3_t HeadAngle,bool Parent=false,bool Translate=true);
#else //PBG_ADD_NEWCHAR_MONK_ANI
    bool PlayAnimation(float *AnimationFrame,float *PriorAnimationFrame,unsigned char *PriorAction,float Speed,vec3_t Origin,vec3_t Angle);
    void Animation(float (*BoneTransform)[3][4],float AnimationFrame,float PriorAnimationFrame,unsigned char PriorAction,vec3_t Angle,vec3_t HeadAngle,bool Parent=false,bool Translate=true);
#endif //PBG_ADD_NEWCHAR_MONK_ANI
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	void InterpolationTrans(float (*Mat1)[4], float (*TransMat2)[4], float _Scale);
	void Transform(float (*BoneMatrix)[3][4],vec3_t BoundingBoxMin,vec3_t BoundingBoxMax,OBB_t *OBB,bool Translate=false, float _Scale = 0.0f);
#else //PBG_ADD_NEWCHAR_MONK_ITEM
	void Transform(float (*BoneMatrix)[3][4],vec3_t BoundingBoxMin,vec3_t BoundingBoxMax,OBB_t *OBB,bool Translate=false);
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
	void TransformtoVertices(vec3_t (*Out_VertexTransform__)[MAX_VERTICES], float (*BoneMatrix)[3][4], bool Translate=false);
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO

	void TransformByObjectBone(vec3_t vResultPosition, OBJECT * pObject, int iBoneNumber, vec3_t vRelativePosition = NULL);
	// 해당 오브젝트의 본의 월드좌표를 얻는다. (vResultPosition = (pObject->BoneTransform[iBoneNumber] * vRelativePosition) + pObject->Position)
	void TransformByBoneMatrix(vec3_t vResultPosition, float (*BoneMatrix)[4], vec3_t vWorldPosition = NULL, vec3_t vRelativePosition = NULL);
	// 해당 본 행렬을 이용해 본의 월드좌표를 얻는다. (vResultPosition = (BoneMatrix * vRelativePosition) + vWorldPosition)
    void TransformPosition(float (*Matrix)[4],vec3_t Position,vec3_t WorldPosition,bool Translate=false);
    void RotationPosition(float (*Matrix)[4],vec3_t Position,vec3_t WorldPosition);

#ifdef LDS_ADD_MODEL_ATTACH_SPECIFIC_NODE_
	public:
	//typedef vector<vec3_t>		VECVEC3_TS;	// 로컬내 각 노드의 현재 위치점만 반환 하기 위한 변수.

#ifdef LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM
	void AnimationTransformWithAttachHighModel_usingGlobalTM( 
				OBJECT* oHighHierarchyModel, 
				BMD* bmdHighHierarchyModel, 
				int iBoneNumberHighHierarchyModel,
				vec3_t &vOutPosHighHiearachyModelBone,
				vec3_t *arrOutSetfAllBonePositions,
				bool bApplyTMtoVertices);
#endif // LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM

	// 자신을 oHighHierarchyModel,b의 n번째 노드위치에 애니메이션 하여 ATTACH 합니다.
	void AnimationTransformWithAttachHighModel( 
				OBJECT* oHighHierarchyModel, 
				BMD* bmdHighHierarchyModel, 
				int iBoneNumberHighHierarchyModel,
				vec3_t &vOutPosHighHiearachyModelBone,
				vec3_t *arrOutSetfAllBonePositions);

	void AnimationTransformOnlySelf( vec3_t *arrOutSetfAllBonePositions, 
									 const OBJECT* oSelf );
		
	void AnimationTransformOnlySelf( vec3_t *arrOutSetfAllBonePositions, 
									 const vec3_t &v3Angle, 
									 const vec3_t &v3Position,
									 const float &fScale,
									 OBJECT* oRefAnimation = NULL,
									 const float fFrameArea = -1.0f,
									 const float fWeight = -1.0f );	
#endif // LDS_ADD_MODEL_ATTACH_SPECIFIC_NODE_

#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
	void AnimationTransformOutAllVertices(vec3_t (*outVertexTransform__)[MAX_VERTICES], const OBJECT& oSelf );
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO

    void LinkBone(int Node);
    void TransformScale(float Scale,bool Link=false);
    void TransformShadow();
    void Lighting(float *,Light_t *,vec3_t,vec3_t);
    void Chrome(float *,int,vec3_t);
    
    //render
	void RenderBone(float (*BoneTransform)[3][4]);
    void RenderObjectBoundingBox();
	void BeginRender(float);
	void EndRender();

#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
    void RenderMeshEffect ( int i, int iType, int iSubType=0, vec3_t Angle=0, VOID* obj=NULL, const OBJECT* objSelf = NULL);
#else // LDS_ADD_RENDERMESHEFFECT_FOR_VBO
	void RenderMeshEffect ( int i, int iType, int iSubType=0, vec3_t Angle=0, VOID* obj=NULL );
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO

    void RenderMesh(int i,int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int Texture=-1);
	void RenderMeshAlternative( int iRndExtFlag, int iParam, int i,int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int Texture=-1);
	void RenderBody(int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int HiddenMesh=-1,int Texture=-1);
	void RenderBodyAlternative(int iRndExtFlag, int iParam,int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int HiddenMesh=-1,int Texture=-1);
    void RenderMeshTranslate(int i,int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int Texture=-1);
    void RenderBodyTranslate(int RenderFlag,float Alpha=1.f,int BlendMesh=-1,float BlendMeshLight=1.f,float BlendMeshTexCoordU=0.f,float BlendMeshTexCoordV=0.f,int HiddenMesh=-1,int Texture=-1);
    void RenderBodyShadow(int BlendMesh=-1,int HiddenMesh=-1, int StartMeshNumber=-1, int EndMeshNumber=-1);

	void SetBodyLight( vec3_t right ) { VectorCopy( right, BodyLight ); }

	//lightmap(ver1.2)
	bool LightMapEnable;
    bool CollisionDetectLineToMesh(vec3_t,vec3_t,bool Collision=true,int Mesh=-1,int Triangle=-1);
    void CreateLightMapSurface(Light_t *,Mesh_t *,int,int,int,int,int,int,vec3_t,vec3_t,int);
	void CreateLightMaps();
    void BindLightMaps();
	void ReleaseLightMaps();

//#ifdef USE_SHADOWVOLUME
	void FindNearTriangle( void);

#ifdef LDS_OPTIMIZE_FORLOADING
	void FindTriangleForEdge( Mesh_t* m, int iTriBasis );
#else // LDS_OPTIMIZE_FORLOADING
	void FindTriangleForEdge( int iMesh, int iTri, int iIndex11);
#endif // LDS_OPTIMIZE_FORLOADING

//#endif //USE_SHADOWVOLUME
private:
	//복사 불가
	BMD(const BMD& b);
	BMD& operator=(const BMD& b);

	//버텍스 확장에 쓰이는 함수
	void ExtendVertice(Mesh_t* oM, VBOMesh* nM);

	// MR0:VBOMesh로 변환 이후 ValidationCheck정보를 화면상에 출력.
#ifdef LDS_MR0_FORDEBUG_VERIFYCONVERTMESH

	enum UNVALIDATETYPE	{
		UT_SUCCESS = 0,
		UT_IB = 1,
		UT_VB = 2, 
		UT_TRANSFORM = 4,
		UT_END = 8,
	};
	
	void ProcessVerification(MeshVector& CurrentVBOMeshContainer);
	void UnValidateTypeText( unsigned int uiUnvalidateType, char* szOutText );
	unsigned int VerifyTotalVBOMESHS( MeshVector& CurrentVBOMeshContainer );
	unsigned int VerifyCurrentVBOMeshValidate( VBOMesh& CurrentVBOMesh );
#endif // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
};

//extern BMD   Models[];
extern BMD   *Models;
extern BMD *ModelsDump;
extern float BoneTransform[MAX_BONES][3][4];
extern vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t LightTransform[MAX_MESH][MAX_VERTICES];
extern float  IntensityTransform[MAX_MESH][MAX_VERTICES];
extern float g_chrome[MAX_VERTICES][2];


#endif
