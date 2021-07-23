#ifndef __VPMANAGER_H__
#define __VPMANAGER_H__

//////////////////////////////////////////////////////////////////////////
// 
// VPManager.h : 쉐이더 소스 Parsing, 컴파일 및 관리
//
//////////////////////////////////////////////////////////////////////////

#include "StlDefinition.h"
//Texture-> 본소스의 OpenTexture함수에서 파일이름으로 키로 하여 읽어들인다.
//모델뷰어의 경우 AttachTexture에서 수행한다. 
#define MAX_ENV_PARAMS	196	//쉐이더 전역 공간

#include <map>

#define UNSIGNEDINT_NOFIND		65535*32

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 프로그램 컨테이너 : OpenGL로 컴파일된 Shader Script의 
///////////////////////////////////////////////////////////////////////////////////////////
typedef std::map<unsigned int, unsigned int>	mapRegisterGenValues;		// First Key Index, Second Key Value

extern mapRegisterGenValues	m_mapRegisterGen_VPS;			// 컴파일된 쉐이더 컨테이너 : 버텍스 쉐이더
extern mapRegisterGenValues	m_mapRegisterGen_FPS;			// 컴파일된 쉐이더 컨테이너 : 픽셀 쉐이더
extern mapRegisterGenValues	m_mapRegisterGen_VPSP;			// 컴파일된 쉐이더 컨테이너 : 특수한 상황에 사용하는 쉐이더(현재는 ShadowMap으로 그림자 렌더 시 사용.)


///////////////////////////////////////////////////////////////////////////////////////////
// OpenGL함수 사용 이후 OpenGL에서 에러 발생하는 모든 에러를 감지 합니다. 
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
inline int CheckGLError( const char *file, int line)
{
    GLenum glErr;
    int retCode = 0, iLimitGLError = 0; 
    while ( ((glErr=glGetError()) != GL_NO_ERROR) && (iLimitGLError < 2) )
    {
		char	szOutputError[512];

        switch(glErr)
        {
        case GL_INVALID_ENUM:		// 허용치 범위 밖의 값 사용시.
            sprintf(szOutputError, "GL_INVALID_ENUM error in File %s at line: %d\n", file, line);
            break;
        case GL_INVALID_VALUE:		// 
            sprintf(szOutputError, "GL_INVALID_VALUE error in File %s at line: %d\n", file, line);
            break;
        case GL_INVALID_OPERATION:	// 잘못된 연산을 넘길시
            sprintf(szOutputError, "GL_INVALID_OPERATION error in File %s at line: %d\n", file, line);
            break;	
        case GL_STACK_OVERFLOW:		// GLPushMatrix(), GLPopMatrix()와 같이 OpenGL내부 Stack에 값이 넘칠때.
            sprintf(szOutputError, "GL_STACK_OVERFLOW error in File %s at line: %d\n", file, line);
            break;
        case GL_STACK_UNDERFLOW:	// GLPushMatrix(), GLPopMatrix()와 같이 OpenGL내부 Stack에 값이 모자랄때.
            sprintf(szOutputError, "GL_STACK_UNDERFLOW error in File %s at line: %d\n", file, line);
            break;
        case GL_OUT_OF_MEMORY:		// GL 치명적인 범위 밖 메모리 접근 에러.
            sprintf(szOutputError, "GL_OUT_OF_MEMORY error in File %s at line: %d\n", file, line);
            break;
        default:
            sprintf(szOutputError, "UNKNOWN ERROR in File %s at line: %d\n", file, line);
        }
		++iLimitGLError;

#ifdef CONSOLE_DEBUG
 		g_ConsoleDebug->Write( MCD_NORMAL, szOutputError );
#endif // CONSOLE_DEBUG
		
 		g_ErrorReport.Write( szOutputError );
#ifdef KWAK_ADD_TRACE_FUNC
		__TraceF(TEXT("%s"), szOutputError);
#endif // KWAK_ADD_TRACE_FUNC

//		MessageBox( NULL, szOutputError, "Critical Error", MB_OK );
    }
    return retCode;
}

#endif // _DEBUG

#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT

///////////////////////////////////////////////////////////////////////////////////////////
// 문자열을 특정 문자열을 기준으로 Split 하여주는 클래스.
///////////////////////////////////////////////////////////////////////////////////////////
#include <string>

class CStringSplitor
{	
private:
	string m_strSplitor;
	
private:
	BOOL ExtractSpotLeftBuffer_Ascending__( const char *szFullStr,		// Param1:szFullStr 문자열 좌측 끝을 기준으로 순차적으로 Param3:szFineValues를 찾아나가 찾게 되면 그 앞의 문자열을 반환.
		char *szReturnBuffer,									
		const char *szFindValues,
		const int iSkipDegree,
		BOOL bExceptionSpot = TRUE );
	
public:
	
	bool		SplitString( string& strSrc, vector<string>	&vecStr );	// Param1:strSrc을 SplitStr을 조건으로 Split하여 Param2:vecStr 로 적재 하여 반환.
	void		SetStrSplit( string strSplitStr )						// Split 지정자를 설정.
	{
		m_strSplitor = strSplitStr;
	}
	
public:
	CStringSplitor( string strSplitor )
	{
		m_strSplitor = strSplitor;
	}
	
	~CStringSplitor()
	{
	}
};
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT

// MR0:MODIFY
#ifdef LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
namespace VPManager
{
	static bool	n_bEnabled;
	inline bool	beingEnable()
	{
		return n_bEnabled;
	}
}
#endif // LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE


///////////////////////////////////////////////////////////////////////////////////////////
// 모든 쉐이더 값들을 넘기기 위한 함수들 모음. Shader Parameter Process
///////////////////////////////////////////////////////////////////////////////////////////
namespace VPManager
{
	// 컴파일된 쉐이더를 미리 컴파일된 컴파일러에서 찾아 반환. 버텍스 쉐이더 버전.
	inline unsigned int	GetGlGenValue_VPS( unsigned int uiIndex )
	{
		mapRegisterGenValues::iterator	iter = m_mapRegisterGen_VPS.find( uiIndex );

		if( iter != m_mapRegisterGen_VPS.end() )
		{
			return (unsigned int)iter->second;
		}
		
		return UNSIGNEDINT_NOFIND;
	}

	// 컴파일된 쉐이더를 미리 컴파일된 컴파일러에서 찾아 반환. 픽셀 쉐이더 버전.
	inline unsigned int	GetGlGenValue_FPS( unsigned int uiIndex )
	{
		mapRegisterGenValues::iterator	iter = m_mapRegisterGen_FPS.find( uiIndex );
		
		if( iter != m_mapRegisterGen_FPS.end() )
		{
			return (unsigned int)iter->second;
		}
		
		return UNSIGNEDINT_NOFIND;
	}

	// 컴파일된 쉐이더를 미리 컴파일된 컴파일러에서 찾아 반환. 특수 쉐이더 버전.
	inline unsigned int	GetGlGenValue_VPSP( unsigned int uiIndex )
	{
		mapRegisterGenValues::iterator	iter = m_mapRegisterGen_VPSP.find( uiIndex );
		
		if( iter != m_mapRegisterGen_VPSP.end() )
		{
			return (unsigned int)iter->second;
		}
		
		return UNSIGNEDINT_NOFIND;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// 1. 쉐이더 스크립트를 모두 불러와 GL 함수로 컴파일 합니다. 
	// 2. 쉐이더렌더에서 사용할 모든 이펙트 텍스쳐를 등록 합니다.
	///////////////////////////////////////////////////////////////////////////////////////////
	void Init();		// 쉐이더 스크립트 Parsing & GL Compile & ShaderContainer 등록. 
	void Uninit();
	void Disable();
	void Enable();

	void DisableFP();	// 프래그먼트 프로그램을 비 활성화시킨다.
	void EnableFP();	// 프래그먼트 프로그램을 활성화시킨다.

	void SetShaderCategory(bool isVertex);	//0 : Vertex Program, 1 : Fragment Program
	
	//상수 관리
	bool SetLocalParam4f(unsigned int idx, float x, float y, float z, float w = 0.f);
	bool SetLocalParam4fv(unsigned int idx, const float* v);
	bool SetLocalParamMatrix(unsigned int idx, float (*Matrix)[4]);
	bool SetEnvParam4f(unsigned int idx, float x, float y, float z, float w = 0.f);
	bool SetEnvParam4fv(unsigned int idx, const float* v);
	bool SetEnvParamMatrix(unsigned int idx, float (*Matrix)[4]);
	bool GetLocalParam4fv(unsigned int idx, float* v);
	bool GetEnvParam4fv(unsigned int idx, float* v);

	// GPU로 넘기는 파라미터 상수 최대 용량 관리 (Local Param, Env Param)
	unsigned int  GetMaxLocalParams();
	unsigned int  GetMaxEnvParams();
	void SetRatioEnvLocal(unsigned int maxEnv);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 소스 조합기. 쉐이더 효과 타입 정의.
///////////////////////////////////////////////////////////////////////////////////////////
namespace VPManager
{
	//기본  Vertex Transform 쉐이더 타입들
	typedef enum _MixShaderVT_NOBONE_t
	{
		MIXSHADER_VT_NOBONE_DEFAULT = 0,
		MIXSHADER_VT_NOBONE_ALL

	}MixShaderVT_NOBONE_t;
	
	typedef enum _MixShaderVT_ONELINK_t
	{
		MIXSHADER_VT_ONELINK_DEFAULT = 0,
		MIXSHADER_VT_ONELINK_ALL

	}MixShaderVT_ONELINK_t;

	typedef enum _MixShaderVT_TWOLINK_t
	{
		MIXSHADER_VT_TWOLINK_DEFAULT = 0,
		MIXSHADER_VT_TWOLINK_ALL

	}MixShaderVT_TWOLINK_t;
	
	typedef enum _MixShaderNT_t
	{
		MIXSHADER_NT_DEFAULT = 0,				//텅 빈 내용
		MIXSHADER_NT_TRANSFORM,					//애니메이션을 적용시킨다.
		MIXSHADER_NT_ALL

	}MixShaderNT_t;

	typedef enum _MixShaderCT_t
	{
		MIXSHADER_CT_DEFAULT = 0,				//컬러만 출력
		MIXSHADER_CT_AMBIENT,					//엠비언트와 결합
		MIXSHADER_CT_DIFFUSE,					//디퓨즈 라이트 출력
		MIXSHADER_CT_DIRECTIONAL,				//방향성 광원 출력
		MIXSHADER_CT_DOUBLE,					//두 방향의 광원에 의한 출력

		MIXSHADER_CT_ALL

	}MixShaderCT_t;

	typedef enum _MixShaderTT_DEF_t
	{
		MIXSHADER_TT_DEF_SINGLE_TEXTURE = 0,	//싱글 텍스쳐일 때, 기본 텍스쳐 출력
		MIXSHADER_TT_DEF_TWO_TEXTURE,			//2-Texture일 때 기본 텍스쳐 출력

		MIXSHADER_TT_DEF_ALL
		
	}MixShaderTT_DEF_t;

	// m_matConstants[1] ~ m_matConstants[10] : 
	//	1:	MIXSHADER_TT_BLENDMESH		// (Param1:BlendTextureUV값)
	//	2:	MIXSHADER_TT_CHROME1		// (Param1:WorldTime, Param2:연산시 가중치 및 승수)
	//	3:	MIXSHADER_TT_CHROME2		// (Param1:WorldTime, Param2:연산시 가중치 및 승수)
	//	4:	MIXSHADER_TT_CHROME3		// (Param1:연산시 가중치 및 승수)
	//	5:	MIXSHADER_TT_CHROME4		// (Param1:WorldTime, Param2:BlendTextureUV, Param3:연산시 가중치 및 승수)
	//	6:	MIXSHADER_TT_CHROME5		// (Param1:WorldTime, Param2:연산시 가중치 및 승수)
	//	7:	MIXSHADER_TT_METAL			// (Param1:연산시 가중치 및 승수), Texture = BITMAP_SHINY
	//	8:	MIXSHADER_TT_OIL			// (Param1:BlendTextureUV)
	//	9:	MIXSHADER_TT_CHROME6		// (Param1:WorldTime)
	//	10: MIXSHADER_TT_WAVE			// (Param1:BlendTextureUV)		// 미사용
	typedef enum _MixShaderTT_t
	{
		MIXSHADER_TT_NONE	 = 0,
		MIXSHADER_TT_BLENDMESH,
		MIXSHADER_TT_CHROME1,				//각각 텍스쳐 이펙트
		MIXSHADER_TT_CHROME2,
		MIXSHADER_TT_CHROME3,
		MIXSHADER_TT_CHROME4,
		MIXSHADER_TT_CHROME5,
		MIXSHADER_TT_METAL,
		MIXSHADER_TT_OIL,
		MIXSHADER_TT_CHROME6,
		MIXSHADER_TT_WAVE,					// 10

		MIXSHADER_TT_ALL
	}MixShaderTT_t;

	typedef enum _SpecialVP_t
	{
		SPVP_BODYSHADOW = 0,	//그림자 쉐이더
		SPVP_SPRITE,			//스프라이트 전용
		SPVP_WATER,				//플랜을 물로 렌더링

		SPVP_ALL
	}SpecialVP_t;


	// Fragment Shader 의 Program 정의.
	typedef enum _FragmentShader_t
	{
		FRAGMENT_ST_DEFAULT = 0,
		FRAGMENT_ST_DEFAULT_NOFOG,
		FRAGMENT_ST_NONLIGHT_DEFAULT,
		FRAGMENT_ST_NONLIGHT_NOFOG_DEFAULT,

		FRAGMENT_MT_DEFAULT,
		FRAGMENT_MT_BRIGHT,
		FRAGMENT_MT_DARK,
		FRAGMENT_MT_LIGHTMAP,

		FRAGMENT_MT_NONLIGHT_DEFAULT,
		FRAGMENT_MT_NONLIGHT_BRIGHT,
		FRAGMENT_MT_NONLIGHT_DARK,
		FRAGMENT_MT_NONLIGHT_LIGHTMAP,

		FRAGMENT_MT_REPLACE,

		FRAGMENT_ALL
	}FragmentShader_t;

	
	void	SelectVProgram(int iProgram);		//버텍스 프로그램(쉐이더) 를 선택한다.
	void	SelectVPSProgram(int iProgram);		//특수한 버텍스 프로그램을 선택한다. 
	void	SelectFProgram(int iProgram);		//프래그먼트 프로그램을 선택한다.

	//Chrome류의 이펙트 등록, 검색
	bool	RegisterTextureEffect(int iflag, int iTex, int iProgram);	// 쉐이더로 사용할 각 효과 별로 텍스쳐를 입력. (ex. RENDER_CHROME, RENDER_CHROME1, RENDER_CHROME2,,,)
	void	GetTextureEffect(int iFlag, int& outTex, int& outProgram);	// 효과가 사용하는 텍스쳐와 쉐이더 프로그램 을 얻음.
}

#endif // __VPMANAGER_H__