#ifndef __MODEL_MANAGER_H__
#define __MODEL_MANAGER_H__

//////////////////////////////////////////////////////////////////////////
// 
// ModelManager.h : 메쉬 컨테이너, 쉐이더 메니저, BMD에서 VBOMESH변환 기능 정의.
//
//////////////////////////////////////////////////////////////////////////

#include "GLUtils.h"
#include "StlDefinition.h"

unsigned int		GenID();		//엔진 각 요소들에 대한 ID함수

///////////////////////////////////////////////////////////////////////////////////////////
// Post Effect : 간혹 BMD::Open2 에서 Exporting 될 메쉬에 정의된 Post Effect 들 처리.
///////////////////////////////////////////////////////////////////////////////////////////
typedef struct _EffectScrip
{
	unsigned int	m_tShader;		//이펙트 쉐이더 선택
	int				m_iTex;			//텍스쳐값
	float			m_fAlpha;		//메쉬의 알파값 (알파채널이 있는 경우만 )
	vec3_t			m_vColor3f;		//메쉬 BodyLight에 곱해지는 색상
	char			m_strTexName[MAX_PATH];

	_EffectScrip() : m_tShader(0), m_iTex(0), m_fAlpha(0.f) 
	{
		Vector(0.f,0.f,0.f, m_vColor3f);
		strcpy(m_strTexName, "");
	}
	_EffectScrip(int iShader, int iTex, float fAlpha, vec3_t vColor3f) :
		m_tShader(iShader), m_iTex(iTex), m_fAlpha(fAlpha)
	{
		VectorCopy(vColor3f, m_vColor3f);
		strcpy(m_strTexName, "");
	}
	_EffectScrip(const _EffectScrip& r)
	{
		m_tShader = r.m_tShader;
		m_iTex = r.m_iTex;
		m_fAlpha = r.m_fAlpha;
		VectorCopy(r.m_vColor3f, m_vColor3f);
		strcpy(m_strTexName, r.m_strTexName);
	}
	_EffectScrip& operator=(const _EffectScrip& r)
	{
		m_tShader = r.m_tShader;
		m_iTex = r.m_iTex;
		m_fAlpha = r.m_fAlpha;
		VectorCopy(r.m_vColor3f, m_vColor3f);
		strcpy(m_strTexName, r.m_strTexName);
		return *this;
	}
} EffectScrip_t;
typedef std::vector<EffectScrip_t> EffectScripVec_t;

///////////////////////////////////////////////////////////////////////////////////////////
// 모델 타입. 쉐이더 렌더러에서 사용되는 Vertex 개체들 정의. BMD에서 VBOMesh로 변환 함수 정의. 
///////////////////////////////////////////////////////////////////////////////////////////
namespace ModelManager
{
	typedef struct _nv2
	{
		float x, y;
		_nv2() : x(0.f), y(0.f) {}
	}nv2;

	typedef struct _nv3
	{
		float x, y, z;
		_nv3() : x(0.f), y(0.f), z(0.f) {}
	}nv3;

	typedef struct _nv4
	{
		float x, y, z, w;
		_nv4() : x(0.f), y(0.f), z(0.f), w(0.f){}
	}nv4;

	typedef struct _nv34
	{
		nv4	l1, l2, l3;
		_nv34() {}
		_nv34(float (*Matrix)[4]){
			memcpy(&l1.x, Matrix, sizeof(float) * 12);
		}
		_nv34(const float* v){
			memcpy(&l1.x, v, sizeof(float) * 12);
		}
	}nv34;

	typedef struct _nv44
	{
		nv4	l1, l2, l3, l4;
		_nv44() {}
		_nv44(float (*Matrix)[4]){
			memcpy(&l1.x, Matrix, sizeof(float) * 16);
		}
		_nv44(const float* v){
			memcpy(&l1.x, v, sizeof(float) * 16);
		}
	}nv44;

	typedef std::vector<nv34> MatrixVector;
	typedef std::vector<nv4> Vector4Vec;
	typedef std::vector<nv3> Vector3Vec;
	typedef std::vector<nv2> Vector2Vec;

	void				Init();			// 메쉬 머신을 초기화 합니다.
	void				UnInit();		// 메쉬 머신을 메모리반환 합니다.

#ifdef LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
										// BMD에서 읽은 메쉬를 VBO메쉬로 변환 합니다. ID생성 및 Upload관리
	void				Convert(unsigned int iModel, bool bDelOldMesh = true, bool bValidationCheckForDebug = false);	
#else // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
	void				Convert(unsigned int iModel, bool bDelOldMesh = true);		
#endif // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
										// 맵 에디터용 함수 - 모든 메쉬에 기본 이펙트 (텍스쳐 ) 를 더합니다.
	void				AttachDefaultEffect(unsigned int iModel);
										// 맵 에디터용 함수 - 메쉬에 이펙트 (텍스쳐 )갯수를 가져옵니다.			
	unsigned int		GetEffectCount(unsigned int ModelID, unsigned int MeshID);	
										
										// 모델(BMD Models[MAX_MODEL])의 하나개체에 이름을 지정 합니다. 
	void				SetModelName(unsigned int i, const std::string& n);			

										// 모델(BMD Models[MAX_MODEL])의 하나개체의 에니메이션 갯수를 가져옵니다.
	int					GetActionCount(unsigned int i);	

										// 모델(BMD Models[MAX_MODEL])의 하나개체의 메쉬 갯수를 가져옵니다.
	int					GetMeshCount(unsigned int i);

										// 모델(BMD Models[MAX_MODEL])의 하나개체의 버전을 가져옵니다.
	char				GetModelVersion(unsigned int i);							
} // namespace ModelManager

///////////////////////////////////////////////////////////////////////////////////////////
// Immediate Mesh 렌더러 정의 (렌더 지오메트리, 렌더 머신) (여기서 Immediate Mesh는 실제 렌더될 메쉬 인스턴스를 지칭)
///////////////////////////////////////////////////////////////////////////////////////////
class BMD;
class CRenderMachine;
namespace ModelManager
{
	// 하나의 메쉬 렌더링 정보. (모델은 텍스쳐로 나뉘어진 여러개의 메쉬로 구성.)
	typedef struct _MeshRender_t
	{
		BMD*		m_pSourceBMD;		// Immediate Render메쉬가 참조하는 실제 모델.
		int			m_iMesh;			// Immediate Render메쉬의 인덱스 번호.
		int			m_iTex;				// Immediate Render메쉬의 Texture 번호.
		int			m_iFlag;			// Texture에게 부여된 RenderState 정의. RENDERBRIGHT와 같은. (ex. rNew.m_iFlag&RENDER_BRIGHT )
		int			m_iNumConst;		// Immediate 상황에 현재 Shader에게 넘길 Parameter Index (ex. Local Parameter Index 또는 Environment Parameter Index로 각각의 쉐이더에게 보낼 인자는 쉐이더 스크립트에 미리 정의된 파라미터의 인덱스에 맞추어야 합니다.)
		bool		m_bUseFixedVP;		// Immediate 상황의 현재 메쉬가 특별한 쉐이더를 이용할 것인지 여부. (현재 특별한 쉐이더는 ShadowMap을 이용한 실그림자 효과 입니다.)
		bool		m_bLight;			// Immediate 상황의 현재 메쉬가 빛효과 여부.
		int			m_iVProgram;		// 현재 선택된 버텍스 프로세스. (ex. 쉐도우 연산인지, 버텍스 렌더링 연산인지 등.)
		int			m_iPProgram;		// 현재 선택된 픽셀 프로세스. m_iVProgram와 동일하며, 추후 구현 요망.
		float		m_fAlpha;			// Immediate 상황의 현재 메쉬의 알파 값.
		nv3			m_vColor;			// Immediate 상황의 현재 메쉬의 Color 값.
		Vector4Vec	m_matConstants;		// 쉐이더로 넘길 모든 파라미터 값들.
										// 1. 
										// 1. Local+World+View Mat (Vec4*3 STL::Vector) : 현재 Animation된 최종 Matrix로 쉐이더에게 버텍스에게 곱하도록 요청.

#ifdef LDS_MR0_ADD_WHENSHUTDOWN_UNINIT_MODELMANAGER
public:
		void UnInit()
		{
			m_matConstants.clear();		// Shader Parameter Table 초기화.
		}
#endif // LDS_MR0_ADD_WHENSHUTDOWN_UNINIT_MODELMANAGER

public:	
		_MeshRender_t()
		{
			m_pSourceBMD = NULL;
			m_iMesh = m_iTex = m_iFlag = -1;
			m_iNumConst = 0;
			m_bUseFixedVP = false;
			m_bLight = false;
			m_iVProgram = m_iPProgram = -1;
			m_fAlpha = 1.f;
			m_vColor.x = m_vColor.y = m_vColor.z = 1.f;
			m_matConstants.reserve(10);					// 쉐이더에게 넘겨줄 파라미터 테이블은 10개 예약해둡니다.
		}
	}MeshRender_t;
	typedef std::vector<MeshRender_t> MeshRenderVector_t;

///////////////////////////////////////////////////////////////////////////////////////////
// Immediate Mode로 그릴 모든 RenderMesh 메니저 클래스.
///////////////////////////////////////////////////////////////////////////////////////////
	class CMeshMachine
	{
	private:
		MeshRenderVector_t	m_Data;					// MeshRender_t 렌더 메쉬 컨테이너
		bool				m_bLock;				// 현재 그려질 단계를 Locking.
		bool				m_bOn;					// 렌더 메니저 머신의 ON OFF 여부.
		bool				m_bTransop;				// Transform함수 단계에서 BodyOrigin반영 여부.
		vec3_t				m_vLightPosOrg;			// 월드 공간 상의 Light좌표.
		vec3_t				m_vLightDirOrg;			// 월드 공간 상의 Light방향.
		vec3_t				m_vLightPos;			// 객체에게 적용될 Light 위치.
		vec3_t				m_vLightDir;			// 객체에게 적용될 Light 방향.
		
		CMeshMachine(const CMeshMachine& );
		CMeshMachine& operator=(const CMeshMachine& );

		bool IsAlphaNoUseType(int iType);			// 렌더브라이트,크롬3,4,5,7의경우는 알파 미사용.

													// 쉐이더 파라미터 고정 테이블 mat 1~10 구성.
		void MakeConst(int iTexType, int iLightType, bool bAlphaNoUse,	
			float BlendMeshTexCoordU, float BlendMeshTexCoordV, MeshRender_t& r);	

	public:
		CMeshMachine();
		~CMeshMachine();

		void ProcessLight(bool bHighLight = false, bool bBattleCastle = false);		// 렌더시 빛의 방향을 설정. (현재 하드코딩 설정)
		inline bool bEnabled() { return m_bOn; }									// 렌더 메니져 머신 사용여부.
		inline void Toggle(bool b) { m_bOn = b; }									// 렌더 메니져 머신 사용여부 설정.
		inline void Lock(bool b) { m_bLock = b; }									// 렌더 메니져 그려지는 시점에 Locking기능 이나 현재는 특별한 기능은 없음.
		inline void	SetTransOp(bool b) { m_bTransop = b; }							// Transform연산을 쉐이더 할지, CPU에서 할지 여부. 
		inline bool GetTransOp() { return m_bTransop;}								// 반환 :         " 
		inline void SetLightPos(vec3_t vPos, vec3_t vDir) { VectorCopy(vPos, m_vLightPos); VectorCopy(vDir, m_vLightDir); }	// 라이트 위치와 방향 설정이나 현재 미사용.
		void AddRender(BMD* pSource, int i,int RenderFlag,
						float Alpha,int BlendMesh, float BlendMeshLight,			// 그려질 메쉬를 렌더 메니져 머신 컨테이너에 추가.
						float BlendMeshTexCoordU,float BlendMeshTexCoordV,int MeshTexture);	
		void Flush();																// 메쉬를 실제로 GPU로 넘겨 렌더링 요청.
#ifdef LDS_MR0_ADD_WHENSHUTDOWN_UNINIT_MODELMANAGER
		void UnInit();
#endif // LDS_MR0_ADD_WHENSHUTDOWN_UNINIT_MODELMANAGER
	};	
} // namespace ModelManager


//////////////////////////////////////////////////////////////////////////
// 렌더링 메니져 머신 글로벌 접근 개체
//////////////////////////////////////////////////////////////////////////
extern ModelManager::CMeshMachine* g_pMeshMachine;									

//////////////////////////////////////////////////////////////////////////

#ifdef MR0_NEWRENDERING_EFFECTS_SPRITES
//////////////////////////////////////////////////////////////////////////
// Sprite Sorting 해서 그리는 함수
//////////////////////////////////////////////////////////////////////////
namespace SpriteManager
{
	typedef struct _SpriteV
	{
		float u,v;
		unsigned char r,g,b,a;
		float x,y,z;
		
		_SpriteV() : u(0.f), v(0.f), 
			r(255), g(255), b(255), a(255), 
			x(0.f), y(0.f), z(0.f) {}
	}SpriteVertex;

	//스프라이트 렌더모드의 옵션들
	enum RenderEnvEnum_t
	{
		ENABLE_TEXTURE =	0x00000001,
		ENABLE_ALPHATEST =	0x00000002,
		ENABLE_CULL =		0x00000004,
		ENABLE_DEPTHTEST =	0x00000008,
		ENABLE_DEPTHMASK =	0x00000010,
		ENABLE_FOG =		0x00000020,
		ENABLE_ALPHAMODE =	0x00000080,

		ENABLE_ALPHAMODE1 =	0x00000100,
		ENABLE_ALPHAMODE2 =	0x00000200,
		ENABLE_ALPHAMODE3 =	0x00000400,
		ENABLE_ALPHAMODE4 =	0x00000800,
		ENABLE_ALPHAMODE5 =	0x00001000,
		ENABLE_ALPHAMODE6 =	0x00002000,

		ENABLE_ALPHAMODEMASK = 0xFFFFFF00,
	};

	//점들의 집합
	typedef std::vector<SpriteVertex> SprVertexData_t;
	
	//정렬 맵 (정수형 - 인덱스집합)
	typedef std::map<int, UShortVector> IntSortMap_t;

	//정렬 맵(텍스쳐 - 블렌드타입 - 인덱스)
	typedef std::map<int, IntSortMap_t> SprSortMap_t;

	//스프라이트 컨테이너 타입
	typedef struct _SprPartition
	{
		SprVertexData_t m_vData;
		SprSortMap_t m_sortData;

		unsigned int m_iVBO;
		unsigned int m_iIBO;

		_SprPartition() { m_iIBO = 0; m_iVBO = 0;}
	}SprPartition;

	typedef std::vector<SprPartition>  SprContainer;

	//스프라이트 시스템 초기화
	void Init();

	//스프라이트 시스템 해제
	void UnInit();

	//스프라이트 시스템에 기존 스프라이트를 등록할 것인가?
	void Toggle(bool);

	//현재 스프라이트 시스템이 켜져 있는가?
	bool IsOn();

	//텍스쳐-버텍스 쌍 추가
	bool RegisterQuad(int itex, vec3_t vPos, float fWidth, float fHeight, 
		vec3_t vLight, float fRot, 
		float u, float v, float uWidth, float vHeight);

	//컨테이너 초기화
	void Clear();

	//스테이지가 변했다던가 했을 때 호출
	void ClearSub();

	//Partition 수를 조정한다.
	void SetRenderPartition(unsigned int);
	
	//블렌더 세팅
	void SetBlender(int);
	void AddBlenderOption(int);
	void DelBlenderOption(int);

	//스프라이트 렌더링 전인가?
	bool IsPreDraw();

	//스프라이트 쌓는 중이다.
	bool IsWorking();

	//블렌더 타입별로 블렌드 설정
	void DoBlendByType(int);

	//파티션별로 전부다시 그린다.
	void RenderAll();
}
#endif // MR0_NEWRENDERING_EFFECTS_SPRITES


#endif //__MODEL_MANAGER_H__