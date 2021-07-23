#include "stdafx.h"

#ifdef MR0

//////////////////////////////////////////////////////////////////////////
// 
// VPManager.cpp : 쉐이더 소스 Parsing, 컴파일 및 관리
//
//////////////////////////////////////////////////////////////////////////

#include "GLUtils.h"
#include "ZzzBMD.h"
#include "VPManager.h"
#include "ZzzTexture.h"
#include "./Utilities/Log/ErrorReport.h"
#ifdef CONSOLE_DEBUG
#include "./Utilities/Log/muConsoleDebug.h"
#endif // CONSOLE_DEBUG

namespace EngineGate
{
	extern bool m_bIsOn;
}

#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
///////////////////////////////////////////////////////////////////////////////////////////
// Split String (VBO Shader Script Parser를 위한 사항)
///////////////////////////////////////////////////////////////////////////////////////////
BOOL CStringSplitor::ExtractSpotLeftBuffer_Ascending__( const char	*szFullStr,
	char			*szReturnBuffer,
	const char		*szFindValues,
	const int		iSkipDegree,
	BOOL			bExceptionSpot )
{
	int		iSpot, 
		iLen, 
		iStrLen = 0, 
		iStrCurLen = 0, 
		iFindValueIdx = 0, 
		iSkipDegree_ = iSkipDegree;
	BOOL	bFind = FALSE;
	BOOL	bFindValue_OneTimeMore = FALSE;
	
	iLen = strlen( szFullStr );
	
	int i = 0;
	
	for( i = 0; i <= iLen; ++i )
	{
		bFind = FALSE;
		if( FALSE == bFindValue_OneTimeMore )
		{
			iStrCurLen = strlen( szFindValues );
			if( (strncmp( &szFullStr[i], szFindValues, iStrCurLen ) == 0) && iStrCurLen > iStrLen )
			{
				iStrLen = iStrCurLen;
				bFind = TRUE;
				bFindValue_OneTimeMore = TRUE;
			}
		}
		else
		{
			if( (strncmp( &szFullStr[i], szFindValues,iStrLen ) == 0) )
			{
				bFind = TRUE;
			}
		}
		
		if( TRUE == bFind)
		{
			if( 1 > iSkipDegree_-- )	
			{
				break;
			}
		}
	}
	
	if( i < 0 ) 
	{
		return FALSE;
	}
	
	if( bFind == FALSE )
	{
		return FALSE;
	}
	
	iSpot = (TRUE==bExceptionSpot)? i : i + iStrLen;
	
	for( i = 0; i < iSpot; ++i )
	{
		szReturnBuffer[i] = szFullStr[i];
	}
	
	szReturnBuffer[iSpot] = '\0';
	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 문자열을 SplitStr문자를 기준으로 Split하여 반환.
///////////////////////////////////////////////////////////////////////////////////////////
bool		CStringSplitor::SplitString( string& strSrc,vector<string>	&vecStr )
{
	bool bDidSeperate = true;
	char *szPartialStr = new char[strSrc.length()+ 1];
	char *szStrSrc = new char[strSrc.length() + 1];
	char *szStrCurrent = new char[strSrc.length()+1];
	memset(szPartialStr, 0, sizeof(char) * strSrc.length()+1);
	memset(szStrCurrent, 0, sizeof(char) * strSrc.length()+1);
	memset(szStrSrc, 0, sizeof(char) * strSrc.length()+1);
	
	memcpy(szStrSrc, strSrc.c_str(), sizeof(char) * strSrc.length());
	
	int iCurrentBuf = 0, iLen = 0;
	
	while(bDidSeperate == true)
	{
		memset(szPartialStr, 0, sizeof(char) * strSrc.length());
		char *szCurrentBuf = &szStrSrc[iCurrentBuf];
		
		bDidSeperate = ExtractSpotLeftBuffer_Ascending__( szCurrentBuf, szPartialStr, m_strSplitor.c_str(), 0, TRUE );
		
		if( bDidSeperate == false && iCurrentBuf + m_strSplitor.length() < strSrc.length() )
		{
			szCurrentBuf = &szStrSrc[iCurrentBuf + m_strSplitor.length()];
			
			int iStrLenCurrent = strlen(szCurrentBuf);
			
			if(iStrLenCurrent > 0)
			{
				vecStr.push_back( szCurrentBuf );
			}
		}
		
		iLen = strlen(szPartialStr);
		
		if(iLen > 0)
		{
			vecStr.push_back( szPartialStr );
			iCurrentBuf = iCurrentBuf + iLen + m_strSplitor.length();
		}
	}

	delete [] szPartialStr;
	delete [] szStrSrc;
	delete [] szStrCurrent;
	
	return true;
}


#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	

#define MIXVPPATH "Data\\MixShader\\Debug\\VP%d.txt"
///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 소스 빌드 함수들 // 컴파일 된 쉐이더는 조합완성되어 \data\mixshader\Debug\ 경로로 출력 됩니다.
///////////////////////////////////////////////////////////////////////////////////////////
namespace VPManager
{
	std::vector<unsigned int> m_data;
#ifdef LDS_MR0_FIXED_RIGHT_SHADER_MAXPARAM_VALUE
	const unsigned int m_maxParam	= MR0_MAX_PARAMETERS;
	unsigned int m_maxEnvParam		= MR0_MAX_ENV_PARAMETERS;
	unsigned int m_maxLocalParam	= MR0_MAX_LOCAL_PARAMETERS;
	
#else // LDS_MR0_FIXED_RIGHT_SHADER_MAXPARAM_VALUE
	const unsigned int m_maxParam = GL_MAX_PROGRAM_PARAMETERS_ARB;
	unsigned int m_maxEnvParam = GL_MAX_PROGRAM_ENV_PARAMETERS_ARB;
	unsigned int m_maxLocalParam = GL_MAX_PROGRAM_LOCAL_PARAMETERS_ARB;
#endif // LDS_MR0_FIXED_RIGHT_SHADER_MAXPARAM_VALUE

	unsigned int m_iProgramType = GL_VERTEX_PROGRAM_ARB;

	StrVector	m_vecSourceVPS;			// 컴파일될 쉐이더 소스 문자열. - 버텍스 쉐이더 버전
	UIntVector	m_vecVPS;				// 컴파일된 쉐이더 LINK - 버텍스 쉐이더 버전
	StrVector	m_vecSourceFPS;			// 컴파일될 쉐이더 소스 문자열. - 픽셀 쉐이더 버전
	UIntVector	m_vecFPS;				// 컴파일된 쉐이더 LINK - 픽셀 쉐이더 버전

	UIntVector	m_vecVPSP;				// 컴파일된 쉐이더 - 특수 쉐이더 (현재 쉐도우 맵을 이용한 그림자 연산 쉐이더만 존재) //물, 몸 그림자 등과 같은 특수 쉐이더 ( 후에 범용 쉐이더의 범주로 들어가도록 해야 한다.)

	unsigned int g_iCacheVP = 0;		//버텍스 쉐이더 캐싱

	char*	CreateProgram(unsigned int idx, const std::string& pStr);			// OpenGL로 쉐이더를 컴파일 및 Gen.
	char*	CreateProgramSPVP(unsigned int idx, const std::string& pStr);		// OpenGL로 쉐이더를 컴파일 및 Gen. // 쉐도우맵(그림자효과사용)

	void	PrintVProgram(int idx = -1);										// 컴파일된 해당 쉐이더의 소스를 출력.	
	bool	GetProgramPartsFromFile(const std::string& rFn, StrVector& rVec);	// 특정 지정자를 조건으로 쉐이더 통합 문자열에서 쉐이더 소스를 추출

#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
///////////////////////////////////////////////////////////////////////////////////////////
// Binary 및 Encrypt 된 쉐이더 소스를 Decrypt 하여 추출.
///////////////////////////////////////////////////////////////////////////////////////////
	bool	GetProgramPartsFromEncryptionFile(const std::string& rFn, StrVector& rVec);		// 1. 조합형 쉐이더 읽기.
	bool	GetStringFromEncryptFile(const std::string& fFn, std::string& o);				// 2. 단일형 쉐이더 파일 읽기.

	bool	FileSettoList(FILE* file, std::list<std::string>& listLineBuffer);				// 파일로 저장된 스크립트를 라인별로 리스트적재 후 반환
	bool	DecryptionLineBuffers(std::list<string>& listLineBuffer);						// ResourceConverter를 통해 Encrypt된 LineBuffer를 Decrypt
	bool	ScriptBufferToScriptVector(std::list<string>& listLineBuffer, StrVector& rVec);	// 각 라인별 리스트를 통합 벡터로 반환
	void	BuxConvert(BYTE *Buffer,int Size);												// 복호화 함수
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT

	void	CreateFromParts();			// 모든 조합 쉐이더를 생성한다. debug모드에서 생성된 쉐이더스크립트는 \data\mixshader\Debug\ 폴더로 출력 됩니다.
	void	RegisterTextureEffectAll();	// 쉐이더 이펙트별 실제 사용될 텍스쳐를 등록한다. (크롬효과 추가시 여기에 등록해야 함)
}

///////////////////////////////////////////////////////////////////////////////////////////
// Effect별 사용될 텍스쳐 저장되는 컨테이너
///////////////////////////////////////////////////////////////////////////////////////////
namespace VPManager
{
	typedef struct _EffType_t
	{
		int m_iTexID;
		int m_iShaderID;
		_EffType_t() : m_iTexID(-1), m_iShaderID(MIXSHADER_TT_NONE) {}
		_EffType_t(int iTex, int iShd) : m_iTexID(iTex), m_iShaderID(iShd) {}

	}EffType_t;
	typedef std::map<int, EffType_t> TexEffCon_t;

	TexEffCon_t m_EffectsContainer;				// Effect Texture Container.
	IntVector	m_EffectsNameContainer;			// Effect ENUM Container. 

	int	GetTexFlagOnly(int);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 텍스쳐를 적용될 효과 타입에 대응되도록 적재
///////////////////////////////////////////////////////////////////////////////////////////
bool	VPManager::RegisterTextureEffect(int iFlag, int iTex, int iProgram)
{
	if(iTex < 0 || iTex >= BITMAP_NONAMED_TEXTURES_END) return false;
	if(iProgram < 0 || iProgram >= MIXSHADER_TT_ALL) return false;

	m_EffectsNameContainer.push_back(iFlag);
	m_EffectsContainer.insert(TexEffCon_t::value_type(iFlag, EffType_t(iTex, iProgram)));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 텍스쳐를 적용될 효과 타입에 대응되도록 적재
///////////////////////////////////////////////////////////////////////////////////////////
int		VPManager::GetTexFlagOnly(int iFlag)
{
	for(unsigned int i = 0; m_EffectsNameContainer.size(); ++i)
		if(m_EffectsNameContainer[i]&iFlag) return m_EffectsNameContainer[i];
	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////////////////
void	VPManager::GetTextureEffect(int iFlag, int& outTex, int& outProgram)
{
	int iEff = GetTexFlagOnly(iFlag);
	if(iEff == -1)
	{
		outTex = -1;
		outProgram = MIXSHADER_TT_NONE;
		return;
	}
	
	EffType_t& rOut = m_EffectsContainer[iEff];
	outTex = rOut.m_iTexID; outProgram = rOut.m_iShaderID;
}

bool	GetStringFromFile(const std::string& n, std::string& o)
{
	int	size = 0;
	char* pEnd = NULL;
	FILE* fp = fopen(n.c_str(), "rb");
	char* tmp = NULL;
	if(!fp) return false;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
//	tmp = (char* )malloc(size+100);
	tmp = new char[size + 100];
	fread(tmp, size, 1, fp);
	//END를 찾아 그 뒤는 자른다.
	pEnd = strstr(tmp, "END");
	if(!pEnd)
	{
		//free(tmp);
		delete [] tmp;
		
		o = "";
		fclose(fp);
		return false;
	}
	*(pEnd+3) = NULL;
	size = pEnd - tmp + 3;
	o.reserve(size);
	o = tmp;
	
	//free(tmp);
	delete [] tmp;

	fclose(fp);
	return true;
}

//에러 보고 함수
#define ERRORFILENAME "ShaderError.txt"
void	ReportError(const std::string& err)
{
	//현재는 파일로 쓴다. 
	FILE* fp = NULL;
	fp = fopen(ERRORFILENAME, "wt");
	if(fp == NULL) return;

	fprintf(fp, "Shader Error: \n%s \n", err.c_str());

#ifdef CONSOLE_DEBUG
	char		szDebugOutput[256];
	sprintf( szDebugOutput, "Shader Error: \n%s \n", err.c_str() );
	g_ConsoleDebug->Write( MCD_NORMAL, szDebugOutput );
#endif // CONSOLE_DEBUG

	fclose(fp);
}

void VPManager::Disable()
{
	g_iCacheVP = 0;
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, 0);
	glDisable(GL_VERTEX_PROGRAM_ARB);

#ifdef LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
	n_bEnabled = false;
#endif // LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
}

void VPManager::DisableFP()
{
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, 0);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	m_iProgramType = GL_VERTEX_PROGRAM_ARB;
}

void VPManager::Enable()
{
	if(!EngineGate::m_bIsOn) 
	{
#ifdef LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
		n_bEnabled = false;
#endif // LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
		return;
	}

	g_iCacheVP = 0;
	glEnable(GL_VERTEX_PROGRAM_ARB);

#ifdef LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
	n_bEnabled = true;
#endif // LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
}

void VPManager::EnableFP()
{
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
}

void VPManager::SetShaderCategory(bool isVertex)
{
	if(!isVertex)
		m_iProgramType = GL_VERTEX_PROGRAM_ARB;
	else 
		m_iProgramType = GL_FRAGMENT_PROGRAM_ARB;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 1. 쉐이더 스크립트를 모두 불러와 GL 함수로 컴파일 합니다. 
// 2. 쉐이더렌더에서 사용할 모든 이펙트 텍스쳐를 등록 합니다.
///////////////////////////////////////////////////////////////////////////////////////////
void VPManager::Init()
{
#ifdef CONSOLE_DEBUG
	char	szOutput[256];

	sprintf( szOutput, "Call VPManager::Init()" );
	g_ConsoleDebug->Write( MCD_NORMAL, szOutput );
#endif // CONSOLE_DEBUG

	// 쉐이더 스크립트를 모두 불러와 GL Compile
	CreateFromParts();

	// 효과별 텍스쳐를 모두 찾기 쉽도록 Container 적재
	RegisterTextureEffectAll();

	//  MR0 Renderer가 Enabled 설정되었는지 여부를 반환 합니다.
	//  간혹 현재 렌더가 뭔가 제대로 출력이 되지 않는 이유가 될 수 있기에
	//	현재 실시간적 여부를 알 필요가 있습니다.
#ifdef LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
	n_bEnabled = false;
#endif // LDS_MR0_FORDEBUG_USERRESPOND_BEINGVPMANAGERENABLE
}

void VPManager::RegisterTextureEffectAll()
{
	//렌더플래그, 텍스쳐ID, 쉐이더 ID순으로 등록한다. 
	RegisterTextureEffect(RENDER_CHROME, BITMAP_CHROME, MIXSHADER_TT_CHROME1);
	RegisterTextureEffect(RENDER_CHROME2, BITMAP_CHROME2, MIXSHADER_TT_CHROME2);
	RegisterTextureEffect(RENDER_CHROME3, BITMAP_CHROME2, MIXSHADER_TT_CHROME3);
	RegisterTextureEffect(RENDER_CHROME4, BITMAP_CHROME2, MIXSHADER_TT_CHROME4);
	RegisterTextureEffect(RENDER_CHROME5, -1, MIXSHADER_TT_CHROME5);
	RegisterTextureEffect(RENDER_CHROME6, BITMAP_CHROME6, MIXSHADER_TT_CHROME6);
	RegisterTextureEffect(RENDER_METAL, BITMAP_SHINY, MIXSHADER_TT_METAL);
	RegisterTextureEffect(RENDER_OIL, -1, MIXSHADER_TT_OIL);
	RegisterTextureEffect(RENDER_WAVE_EXT, -1, MIXSHADER_TT_WAVE);
}

void VPManager::Uninit()
{
	glDeleteProgramsARB(m_vecVPS.size(), &m_vecVPS[0]);
	m_vecVPS.clear();
	m_vecSourceVPS.clear();
	
	m_vecFPS.clear();
	m_vecSourceFPS.clear();
}

//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
// Local Parameter, Environment Parameter 저장
///////////////////////////////////////////////////////////////////////////////////////////

bool VPManager::SetLocalParam4f(unsigned int idx, float x, float y, float z, float w)
{
	if(idx < 0 || idx >= m_maxLocalParam)
		return false;

	glProgramLocalParameter4fARB(m_iProgramType, idx, x, y, z, w);
	return true;
}

bool VPManager::SetLocalParam4fv(unsigned int idx, const float* v)
{
	if(idx < 0 || idx >= m_maxLocalParam)
		return false;

	glProgramLocalParameter4fvARB(m_iProgramType, idx, v);
	return true;
}

//행렬 등록 함수
bool VPManager::SetLocalParamMatrix(unsigned int idx, float (*Matrix)[4])
{
	bool ret = true;
	for(int i = 0; i < 3; ++i)
	{
		ret = SetLocalParam4fv(idx+i, Matrix[i]);
		if(!ret) return ret;
	}
	return ret;
}

bool VPManager::SetEnvParamMatrix(unsigned int idx, float (*Matrix)[4])
{
	bool ret = true;
	for(int i = 0; i < 3; ++i)
	{
		ret = SetEnvParam4fv(idx+i, Matrix[i]);
		if(!ret) return ret;
	}
	return ret;
}

bool VPManager::SetEnvParam4f(unsigned int idx, float x, float y, float z, float w)
{
	if(idx < 0 || idx >= m_maxEnvParam)
		return false;

	glProgramEnvParameter4fARB(m_iProgramType, idx, x, y, z, w);
	return true;
}

bool VPManager::SetEnvParam4fv(unsigned int idx, const float* v)
{
	if(idx < 0 || idx >= m_maxEnvParam)
		return false;

	glProgramEnvParameter4fvARB(m_iProgramType, idx, v);
	return true;
}

bool VPManager::GetLocalParam4fv(unsigned int idx, float* v)
{
	if(idx < 0 || idx >= m_maxLocalParam)
		return false;
	
	glGetProgramLocalParameterfvARB(m_iProgramType, idx, v);
	return true;
}

bool VPManager::GetEnvParam4fv(unsigned int idx, float* v)
{
	if(idx < 0 || idx >= m_maxEnvParam)
		return false;
	
	glGetProgramEnvParameterfvARB(m_iProgramType, idx, v);
	return true;
}

unsigned int VPManager::GetMaxLocalParams()
{
	return m_maxLocalParam;
}

unsigned int VPManager::GetMaxEnvParams()
{
	return m_maxEnvParam;
}

void VPManager::SetRatioEnvLocal(unsigned int maxEnv)
{
	if(maxEnv <= 0 || maxEnv >= m_maxParam)
		return;
	
	m_maxEnvParam = maxEnv;
	m_maxLocalParam = m_maxParam - maxEnv;
}

//////////////////////////////////////////////////////////////////////////


void VPManager::PrintVProgram(int idx)
{
	char strFn[MAX_PATH] = { 0, };
	if(idx == -1)
	{
		for(unsigned int i = 0; i < m_vecSourceVPS.size(); ++i)
		{
			sprintf(strFn, MIXVPPATH, i);
			FILE* fp = fopen(strFn, "wt");
			fwrite(m_vecSourceVPS[i].c_str(), m_vecSourceVPS[i].length(), 1, fp);
			fclose(fp);
		}
	}
	else
	{
		sprintf(strFn, MIXVPPATH, idx);
		FILE* fp = fopen(strFn, "wt");
		fwrite(m_vecSourceVPS[idx].c_str(), m_vecSourceVPS[idx].length(), 1, fp);
		fclose(fp);
	}
}


#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
///////////////////////////////////////////////////////////////////////////////////////////
// 암호화된 쉐이더 소스파일을 복호화 및 쉐이더 프로그램 단위로 rVec로 받아 반환.
///////////////////////////////////////////////////////////////////////////////////////////
bool VPManager::GetProgramPartsFromFile(const std::string& rFn, StrVector& rVec)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	::_splitpath(rFn.c_str(), drive, dir, fname, ext);
	
	std::string strPath = drive; strPath += dir;
	std::string strFileName = strPath + fname + ".bss";

	bool bResultGetShaderScript = GetProgramPartsFromEncryptionFile(strFileName, rVec);
	
	return bResultGetShaderScript;
}
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
///////////////////////////////////////////////////////////////////////////////////////////
// 스크립트 파싱 기존버전은 암호화가 안되어있음.
///////////////////////////////////////////////////////////////////////////////////////////
bool VPManager::GetProgramPartsFromFile(const std::string& rFn, StrVector& rVec)
{
	FILE* fp = fopen(rFn.c_str(), "rt");
	if(!fp) return false;

	char strLine[100] = {0, };
	std::string strPart;
	bool bGet = false;

	while(1)
	{
		if(fgets(strLine, 100, fp) == NULL) break;

		if(strstr(strLine, "#!Start")) bGet = true;
		if(strstr(strLine, "#!End"))
		{
			if(bGet)
			{
				rVec.push_back(strPart);
				strPart = "";
				bGet = false;			
			}
			else
			{
				//파싱 오류다.
				fclose(fp);
				return false;
			}
		}

		//내용을 얻기 시작했으면+
		if(bGet)
		{
			if(!strstr(strLine, "#")) strPart += strLine;
		}
	}
	fclose(fp);
	if(bGet) return false;
	return true;
}
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT

#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
///////////////////////////////////////////////////////////////////////////////////////////
// 암호화된 파일내 전체 문자를 쉐이더 프로그램 단위로 끊어 rVec 로 저장후 반환.
///////////////////////////////////////////////////////////////////////////////////////////
bool	VPManager::GetProgramPartsFromEncryptionFile(const std::string& rFn, StrVector& rVec)
{
	FILE* fp = fopen(rFn.c_str(), "rt");
	if(!fp)
	{
		return false;
	}

	// 방식3. Splitor
	string strBuffer, strResult, strSplitor;
	char *szBuffer___;
	int iSizeBuffer = 0;
	
	fseek(fp, 0, SEEK_END);
	iSizeBuffer = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	szBuffer___ = new char[iSizeBuffer + 1];
	memset(szBuffer___, 0, sizeof(char) * iSizeBuffer+1);
	fread(szBuffer___, iSizeBuffer, 1, fp);

	BuxConvert( (BYTE*)szBuffer___, iSizeBuffer );		// 1. Decryption

	strBuffer = szBuffer___;

	CStringSplitor		StringSplitor( "@#$%" );		// 2. 구분자.
	StringSplitor.SplitString( strBuffer, rVec );			// 3. Split 문자열 쪼개기.

	fclose(fp);

	delete [] szBuffer___;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 문자열 복호화
///////////////////////////////////////////////////////////////////////////////////////////
bool VPManager::GetStringFromEncryptFile(const std::string& rFn, std::string& o)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char extConvert[_MAX_EXT];
	
	::_splitpath(rFn.c_str(), drive, dir, fname, ext);
	
	std::string strPath = drive; strPath += dir;

	if( _stricmp(ext, ".vsh") != 0 && _stricmp(ext, ".fsh") != 0 )
	{
		assert(0);
		return false;
	}

	std::string strFileName;

	memset(extConvert, 0, sizeof(char) * _MAX_EXT);
	if( _stricmp(ext, ".vsh") == 0 )
	{
		strcpy( extConvert, TEXT(".bvsh") );
	}
	else
	{
		strcpy( extConvert, TEXT(".bfsh") );
	}

	strFileName = strPath + fname + extConvert;

	int	size = 0;
	char* pEnd = NULL;
	FILE* fp = fopen(strFileName.c_str(), "rb");
	char* tmp = NULL;
	if(!fp) return false;
	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	tmp = new char[size + 100];
	fread(tmp, size, 1, fp);

	BuxConvert( (BYTE*)tmp, size );		// Decryption

	//END를 찾아 그 뒤는 자른다.
	pEnd = strstr(tmp, "END");
	if(!pEnd)
	{
		//free(tmp);
		delete [] tmp;
		
		o = "";
		fclose(fp);
		return false;
	}
	*(pEnd+3) = NULL;
	size = pEnd - tmp + 3;
	o.reserve(size);
	o = tmp;
	
	//free(tmp);
	delete [] tmp;
	
	fclose(fp);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 문자열을 라인별로 끊음.
///////////////////////////////////////////////////////////////////////////////////////////
bool	VPManager::FileSettoList(FILE* file, std::list<std::string>& listLineBuffer)
{
	char strLine[100] = {0, };
	std::string strPart;
	bool bGet = false;

	while(!feof(file))
	{
		memset( strLine, 0, sizeof(char) * 100 );

		if(fgets(strLine, 100, file) == NULL) break;

		listLineBuffer.push_back( strLine );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 모든 라인 문자 버퍼들을 복호화
///////////////////////////////////////////////////////////////////////////////////////////
bool	VPManager::DecryptionLineBuffers(std::list<string>& listLineBuffer)
{
	std::list<string>::iterator iter_;

	for(iter_ = listLineBuffer.begin(); iter_ != listLineBuffer.end(); iter_++)
	{
		string&	currentStr = (*iter_);

		BuxConvert( (BYTE*)(currentStr.c_str()), currentStr.length() );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 모든 라인버퍼들을 쉐이더 프로그램 단위로 끊어 rVec으로 담아 반환.
///////////////////////////////////////////////////////////////////////////////////////////
bool	VPManager::ScriptBufferToScriptVector(std::list<string>& listLineBuffer, StrVector& rVec)
{
	std::list<string>::iterator iter_;
	string strLine, strPart;
	bool bGet = false;
	
	for(iter_ = listLineBuffer.begin(); iter_ != listLineBuffer.end(); iter_++)
	{
		strLine = (*iter_);
		
		if(strstr(strLine.c_str(), "#!Start"))
		{
			bGet = true;
		}

		if(strstr(strLine.c_str(), "#!End"))
		{
			if(bGet)
			{
				rVec.push_back(strPart);
				strPart = "";
				bGet = false;			
			}
			else
			{
				//파싱 오류다.
				return false;
			}
		}
		
		//내용을 얻기 시작했으면+
		if(bGet)
		{
			if(!strstr(strLine.c_str(), "#")) strPart += strLine;
		}
	}

	return true;
}

void VPManager::BuxConvert(BYTE *Buffer,int Size)
{
	BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
	
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 컴파일 : 소스 문자열로 프로그램화
///////////////////////////////////////////////////////////////////////////////////////////
char* VPManager::CreateProgram(unsigned int idx, const std::string& pStr)
{
	glEnable(m_iProgramType);

	if(m_iProgramType == GL_VERTEX_PROGRAM_ARB)
	{
		glGenProgramsARB(1, &m_vecVPS[idx]);
		glBindProgramARB(m_iProgramType, m_vecVPS[idx]);
		glProgramStringARB(m_iProgramType, GL_PROGRAM_FORMAT_ASCII_ARB, pStr.size(), pStr.c_str());
	}
	else
	{
		glGenProgramsARB(1, &m_vecFPS[idx]);
		glBindProgramARB(m_iProgramType, m_vecFPS[idx]);
		glProgramStringARB(m_iProgramType, GL_PROGRAM_FORMAT_ASCII_ARB, pStr.size(), pStr.c_str());		
	}
	return (char* )glGetString(GL_PROGRAM_ERROR_STRING_ARB);	
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 컴파일 : 특수 쉐이더
///////////////////////////////////////////////////////////////////////////////////////////
char* VPManager::CreateProgramSPVP(unsigned int idx, const std::string& pStr)
{
	glEnable(m_iProgramType);
	glGenProgramsARB(1, &m_vecVPSP[idx]);
	glBindProgramARB(m_iProgramType, m_vecVPSP[idx]);
	glProgramStringARB(m_iProgramType, GL_PROGRAM_FORMAT_ASCII_ARB, pStr.size(), pStr.c_str());	
	return (char* )glGetString(GL_PROGRAM_ERROR_STRING_ARB);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 바인딩 : 특수 쉐이더 프로그램
///////////////////////////////////////////////////////////////////////////////////////////
void VPManager::SelectVPSProgram(int iProgram)
{
	g_iCacheVP = 0;

// 	unsigned int uiSize = m_vecVPSP.size();
// 	if( iProgram >= uiSize || iProgram < 0 )
// 	{
// 		char		szDebugOutput[512];
// 		
// 		sprintf( szDebugOutput, "SelectVPSProgram에서 접근인덱스(%d)가 m_vecVPS범위(0-%d)를 초과.", 
// 			iProgram, uiSize );
// #ifdef CONSOLE_DEBUG
// 		g_ConsoleDebug->Write(MCD_NORMAL, szDebugOutput);
// #endif // CONSOLE_DEBUG
// 		
// 		return;
// 	}
	
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, m_vecVPSP[iProgram]);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 바인딩 : 버텍스 쉐이더
///////////////////////////////////////////////////////////////////////////////////////////
void VPManager::SelectVProgram(int iProgram)
{
//	unsigned int uiSize = m_vecVPS.size();
// 	if( iProgram >= uiSize || iProgram < 0 )
// 	{
// 		char		szDebugOutput[512];
// 
// 		sprintf( szDebugOutput, "SelectVProgram에서 접근인덱스(%d)가 m_vecVPS범위(0-%d)를 초과.", 
// 			iProgram, uiSize );
// #ifdef CONSOLE_DEBUG
// 		g_ConsoleDebug->Write(MCD_NORMAL, szDebugOutput);
// #endif // CONSOLE_DEBUG
// 
// 		return;
// 	}

	if(m_vecVPS[iProgram] != g_iCacheVP)
	{
		g_iCacheVP = m_vecVPS[iProgram];
		glBindProgramARB(GL_VERTEX_PROGRAM_ARB, g_iCacheVP);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 바인딩 : 프라그먼트 쉐이더
///////////////////////////////////////////////////////////////////////////////////////////
void VPManager::SelectFProgram(int iProgram)
{
// 	unsigned int uiSize = m_vecFPS.size();
// 	if( iProgram >= uiSize || iProgram < 0 )
// 	{
// 		char		szDebugOutput[512];
// 		
// 		sprintf( szDebugOutput, "SelectFProgram 접근인덱스(%d)가 m_vecFPS(0-%d)를 초과.", 
// 			iProgram, uiSize );
// #ifdef CONSOLE_DEBUG
// 		g_ConsoleDebug->Write(MCD_NORMAL, szDebugOutput);
// #endif // CONSOLE_DEBUG
// 		
// 		return;
// 	}

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, m_vecFPS[iProgram]);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 모든 쉐이더 소스 코드 컴파일
///////////////////////////////////////////////////////////////////////////////////////////
void VPManager::CreateFromParts()
{
	int i, j, k, l, m = 0;

	//임시 조각 쉐이더 버퍼
	StrVector strVT[3];
	StrVector strNT[3];
	StrVector strCT[3];
	StrVector TTDefs, TTTrans;
	StrVector Defs;

	// 1. 각 소스를 Parsing 문자열로만 변환. 
	//	- 구현부 // Vertex Transform 연산부 // 위치값만
	if(!GetProgramPartsFromFile("Data\\MixShader\\VertexTransforms\\NoBoneVertexTransforms.txt", strVT[0])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\VertexTransforms\\NoBoneVertexTransforms.txt)");
		return;
	}

	//	- 구현부 // Vertex Transform 연산부 // 한개 TM연산만
	if(!GetProgramPartsFromFile("Data\\MixShader\\VertexTransforms\\OneLinkVertexTransforms.txt", strVT[1])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\VertexTransforms\\OneLinkVertexTransforms.txt)");
		return;
	}

	//	- 구현부 // Vertex Transform 연산부 // 두개 TM연산
	if(!GetProgramPartsFromFile("Data\\MixShader\\VertexTransforms\\TwoLinkVertexTransforms.txt", strVT[2])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\VertexTransforms\\TwoLinkVertexTransforms.txt)");
		return;
	}

	//	- 구현부 // Normal Transform 연산부 // 위치값만
	if(!GetProgramPartsFromFile("Data\\MixShader\\NormTransforms\\NoBoneNormTransforms.txt", strNT[0])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\NormTransforms\\NoBoneNormTransforms.txt)");
		return;
	}
	
	//	- 구현부 // Normal Transform 연산부 // 한개 TM연산만
	if(!GetProgramPartsFromFile("Data\\MixShader\\NormTransforms\\OneLinkNormTransforms.txt", strNT[1])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\NormTransforms\\OneLinkNormTransforms.txt)");
		return;
	}

	//	- 구현부 // Normal Transform 연산부 // 두개 TM연산
	if(!GetProgramPartsFromFile("Data\\MixShader\\NormTransforms\\TwoLinkNormTransforms.txt", strNT[2])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\NormTransforms\\TwoLinkNormTransforms.txt)");
		return;
	}

	//	- 구현부 // 라이트 연산을 포함한 버텍스 컬러 연산부 // 위치값만
	if(!GetProgramPartsFromFile("Data\\MixShader\\ColorTransforms\\NoBoneColorTransforms.txt", strCT[0])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\ColorTransforms\\NoBoneColorTransforms.txt)");
		return;
	}

	//	- 구현부 // 라이트 연산을 포함한 버텍스 컬러 연산부 // 한개 TM연산만
	if(!GetProgramPartsFromFile("Data\\MixShader\\ColorTransforms\\OneLinkColorTransforms.txt", strCT[1])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\ColorTransforms\\OneLinkColorTransforms.txt)");
		return;
	}

	//	- 구현부 // 라이트 연산을 포함한 버텍스 컬러 연산부 // 두개 TM연산
	if(!GetProgramPartsFromFile("Data\\MixShader\\ColorTransforms\\TwoLinkColorTransforms.txt", strCT[2])) 
	{
		ReportError("Not Exist File(Data\\MixShader\\ColorTransforms\\TwoLinkColorTransforms.txt)");
		return;
	}

	//	- 선언부 // 텍스쳐 매핑 효과 연산 
	if(!GetProgramPartsFromFile("Data\\MixShader\\TexTransforms\\TexDefTransforms.txt", TTDefs)) 
	{
		ReportError("Not Exist File(Data\\MixShader\\TexTransforms\\TexDefTransforms.txt)");
		return;
	}

	//	- 구현부 // 텍스쳐 매핑 효과 연산
	if(!GetProgramPartsFromFile("Data\\MixShader\\TexTransforms\\TexTransforms.txt", TTTrans)) 
	{
		ReportError("Not Exist File(Data\\MixShader\\TexTransforms\\TexTransforms.txt)");
		return;
	}

	//	- 선언부만 정의 // 모든 쉐이더 프로그램
	if(!GetProgramPartsFromFile("Data\\MixShader\\Definitions.txt", Defs)) 
	{
		ReportError("Not Exist File(Data\\MixShader\\Definitions.txt)");
		return;
	}

	unsigned int iCnt = (MIXSHADER_VT_NOBONE_ALL + MIXSHADER_VT_ONELINK_ALL + MIXSHADER_VT_TWOLINK_ALL)
		* MIXSHADER_NT_ALL * MIXSHADER_CT_ALL * MIXSHADER_TT_DEF_ALL * MIXSHADER_TT_ALL;
	unsigned int iOffset1 = (MIXSHADER_VT_NOBONE_ALL)
		* MIXSHADER_NT_ALL * MIXSHADER_CT_ALL * MIXSHADER_TT_DEF_ALL * MIXSHADER_TT_ALL;
	unsigned int iOffset2 = (MIXSHADER_VT_NOBONE_ALL + MIXSHADER_VT_ONELINK_ALL)
		* MIXSHADER_NT_ALL * MIXSHADER_CT_ALL * MIXSHADER_TT_DEF_ALL * MIXSHADER_TT_ALL;

	m_vecSourceVPS.reserve(iCnt);		// 컴파일된 쉐이더 적재 컨테이너 : 특수 쉐이더용
	m_vecVPS.reserve(iCnt);				// 컴파일된 쉐이더 적재 컨테이너 : 버텍스 쉐이더용
	for(i = 0; i < iCnt; ++i)
	{
		m_vecSourceVPS.push_back("");
		m_vecVPS.push_back(0);
	}

	// 2. Compile

	//	- TM 없는 연산들 모두
	for(i = 0; i < MIXSHADER_VT_NOBONE_ALL; ++i)
	{
		for(j = 0; j < MIXSHADER_NT_ALL; ++j)
		{
			for(k = 0; k < MIXSHADER_CT_ALL; ++k)
			{
				for(l = 0; l < MIXSHADER_TT_DEF_ALL; ++l)
				{
					for(m = 0; m < MIXSHADER_TT_ALL; ++m)
					{
						int iIndex = 
							i*(MIXSHADER_NT_ALL*MIXSHADER_CT_ALL*MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							j*(MIXSHADER_CT_ALL*MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							k*(MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							l*(MIXSHADER_TT_ALL) + m;

						m_vecSourceVPS[iIndex] += Defs[l];			// 선언부 : 쉐이더 변수 선언부
						m_vecSourceVPS[iIndex] += strVT[0][i];		// 구현부 : 버텍스 연산 
						m_vecSourceVPS[iIndex] += strNT[0][j];		// 구현부 : 노멀 연산
						m_vecSourceVPS[iIndex] += strCT[0][k];		// 구현부 : 빛 조명 및 컬러 연산
						
						if(l == MIXSHADER_TT_DEF_SINGLE_TEXTURE)
						{
							//어쩔 수 없이 발생하는 중복 표현의 여지를 제거.
							if(m == 0)
								m_vecSourceVPS[iIndex] += TTDefs[l];
						}
						else
							m_vecSourceVPS[iIndex] += TTDefs[l];

						m_vecSourceVPS[iIndex] += TTTrans[m];		// 구현부 : 텍스쳐 매핑효과
						m_vecSourceVPS[iIndex] += "END";			// 구현부 : 종결

						// ******************************************* 이 쉐이더를 컴파일한다. 
						std::string err = CreateProgram(iIndex, m_vecSourceVPS[iIndex]);

						if(err != "")
						{
							ReportError(err);
							PrintVProgram(iIndex);
							return;
						}
					}
				}
			}
		}
	}
	
	//	- TM 한개 연산들 모두
	for(i = 0; i < MIXSHADER_VT_ONELINK_ALL; ++i)
	{
		for(j = 0; j < MIXSHADER_NT_ALL; ++j)
		{
			for(k = 0; k < MIXSHADER_CT_ALL; ++k)
			{
				for(l = 0; l < MIXSHADER_TT_DEF_ALL; ++l)
				{
					for(m = 0; m < MIXSHADER_TT_ALL; ++m)
					{
						int iIndex = 
							i*(MIXSHADER_NT_ALL*MIXSHADER_CT_ALL*MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							j*(MIXSHADER_CT_ALL*MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							k*(MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							l*(MIXSHADER_TT_ALL) + m + iOffset1;

						m_vecSourceVPS[iIndex] += Defs[l];
						m_vecSourceVPS[iIndex] += strVT[1][i];
						m_vecSourceVPS[iIndex] += strNT[1][j];
						m_vecSourceVPS[iIndex] += strCT[1][k];
						
						if(l == MIXSHADER_TT_DEF_SINGLE_TEXTURE)
						{
							//어쩔 수 없이 발생하는 중복 표현의 여지를 제거.
							if(m == 0)
								m_vecSourceVPS[iIndex] += TTDefs[l];
						}
						else
							m_vecSourceVPS[iIndex] += TTDefs[l];

						m_vecSourceVPS[iIndex] += TTTrans[m];
						m_vecSourceVPS[iIndex] += "END";

						//이 쉐이더를 컴파일한다. 
						std::string err = CreateProgram(iIndex, m_vecSourceVPS[iIndex]);
						if(err != "")
						{
							ReportError(err);
							PrintVProgram(iIndex);
							return;
						}
					}
				}
			}
		}
	}

	//	- TM 두개 연산들 모두
	for(i = 0; i < MIXSHADER_VT_TWOLINK_ALL; ++i)
	{
		for(j = 0; j < MIXSHADER_NT_ALL; ++j)
		{
			for(k = 0; k < MIXSHADER_CT_ALL; ++k)
			{
				for(l = 0; l < MIXSHADER_TT_DEF_ALL; ++l)
				{
					for(m = 0; m < MIXSHADER_TT_ALL; ++m)
					{
						int iIndex = 
							i*(MIXSHADER_NT_ALL*MIXSHADER_CT_ALL*MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							j*(MIXSHADER_CT_ALL*MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							k*(MIXSHADER_TT_DEF_ALL*MIXSHADER_TT_ALL) + 
							l*(MIXSHADER_TT_ALL) + m + iOffset2;

						m_vecSourceVPS[iIndex] += Defs[l];
						m_vecSourceVPS[iIndex] += strVT[2][i];
						m_vecSourceVPS[iIndex] += strNT[2][j];
						m_vecSourceVPS[iIndex] += strCT[2][k];
						
						if(l == MIXSHADER_TT_DEF_SINGLE_TEXTURE)
						{
							//어쩔 수 없이 발생하는 중복 표현의 여지를 제거.
							if(m == 0)
								m_vecSourceVPS[iIndex] += TTDefs[l];
						}
						else
							m_vecSourceVPS[iIndex] += TTDefs[l];

						m_vecSourceVPS[iIndex] += TTTrans[m];
						m_vecSourceVPS[iIndex] += "END";

						//이 쉐이더를 컴파일한다. 
						std::string err = CreateProgram(iIndex, m_vecSourceVPS[iIndex]);

						if(err != "")
						{
							ReportError(err);
							PrintVProgram(iIndex);
							return;
						}
					}
				}
			}
		}
	}
	PrintVProgram(-1);

	// 3. 픽셀 프로그램 읽어들이기 // 추후 구현 
	m_vecSourceFPS.reserve(FRAGMENT_ALL);
	for(i = 0 ; i < FRAGMENT_ALL; ++i) m_vecFPS.push_back(0);


	// 4. 단독 쉐이더 프로그램들 컴파일
	std::string fpSource;
	std::string fpPath = "Data\\Shader\\";
	std::string fpfn;
	std::string fpErr;

	m_iProgramType = GL_FRAGMENT_PROGRAM_ARB;

	fpfn = fpPath + "SingleDefault.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}
	
	fpErr = CreateProgram(FRAGMENT_ST_DEFAULT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "SingleDefaultNoFog.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	
	fpErr = CreateProgram(FRAGMENT_ST_DEFAULT_NOFOG, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "SingleDefaultNL.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_ST_NONLIGHT_DEFAULT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "SingleDefaultNLNoFog.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_ST_NONLIGHT_NOFOG_DEFAULT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "Default.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_DEFAULT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "Bright.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_BRIGHT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "Dark.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_DARK, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "LightMap.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_LIGHTMAP, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "Replace.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_REPLACE, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);


	//No Light 계열
	fpfn = fpPath + "DefaultNL.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_NONLIGHT_DEFAULT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "BrightNL.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_NONLIGHT_BRIGHT, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "DarkNL.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_NONLIGHT_DARK, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	fpfn = fpPath + "LightMapNL.fsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgram(FRAGMENT_MT_NONLIGHT_LIGHTMAP, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	m_vecSourceFPS.push_back(fpSource);

	//특수 쉐이더 읽어들이기
	m_iProgramType = GL_VERTEX_PROGRAM_ARB;
	for(i = 0 ; i < SPVP_ALL; ++i) 
	{
		m_vecVPSP.push_back(0);
	}
	
	fpfn = fpPath + "shadowmap.vsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgramSPVP(SPVP_BODYSHADOW, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}

	//SPVP_SPRITE
	fpfn = fpPath + "Sprite.vsh";
#ifdef LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromEncryptFile(fpfn, fpSource))
#else // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	if(!GetStringFromFile(fpfn, fpSource))
#endif // LDS_MR0_ADD_ENCRYPTION_ALLSHADERSCRIPT
	{
		ReportError( fpfn );
		return;
	}

	fpErr = CreateProgramSPVP(SPVP_SPRITE, fpSource);
	if(fpErr != "")
	{
		ReportError(fpErr);
		return;
	}
	
	Disable();
	DisableFP();

#ifdef CONSOLE_DEBUG
	{
		char	szOutput[256];
		
		sprintf( szOutput, "Completed VPManager::CreateFromParts()!" );
		g_ConsoleDebug->Write( MCD_NORMAL, szOutput );
	}
#endif // CONSOLE_DEBUG
}



#endif //MR0

