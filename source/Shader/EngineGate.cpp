#include "stdafx.h"

#ifdef MR0

//////////////////////////////////////////////////////////////////////////
// 
// EngineGate.cpp : 쉐이더 렌더러 내부 렌더러 머신, 메니저 초기화, 메모리 반환 처리
//
//////////////////////////////////////////////////////////////////////////

#include "EngineGate.h"
#include <io.h>

using namespace ModelManager;

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 설정 파일을 입출력 설정. // 테스트용
///////////////////////////////////////////////////////////////////////////////////////////
namespace EngineGate
{
	//렌더러 옵션값들
	bool m_bIsOn = false;
	bool m_bRenderWater = false;

	void OpenRendererConf();
	void ExpCurrentConf();

	const std::string cStrExpFn = "CurMR.txt";
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 타입 설정 파일에서 읽어들일 설정값을 적용 하도록 설정
///////////////////////////////////////////////////////////////////////////////////////////
void EngineGate::OpenRendererConf()
{
	std::string nConfig;
	char szTemp[MAX_PATH] = { 0, };
	
	GetModuleFileName(NULL, szTemp, MAX_PATH);
	nConfig = szTemp;
	nConfig = nConfig.substr(0, nConfig.rfind('\\')+1);
	nConfig += "mrconfig.cfg";

	if(_access(nConfig.c_str(), 0) == -1)
		return;

	GetPrivateProfileString("General", "Type", "", szTemp, 11, nConfig.c_str());
	SetGeneralType(atoi(szTemp));
	GetPrivateProfileString ("General", "Water", "", szTemp, 11, nConfig.c_str());
	if(atoi(szTemp) != 0) m_bRenderWater = true;
	else m_bRenderWater = false;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 타입 버전 파일로 출력 CurMR.txt 파일
///////////////////////////////////////////////////////////////////////////////////////////
void EngineGate::ExpCurrentConf()
{
	FILE* fp = fopen(cStrExpFn.c_str(), "wt");
	fprintf(fp, "Used Driver Type:%d\n", g_iUseDriverType);
	fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 스크립트 컴파일, 쉐이더 렌더 머신 초기화
///////////////////////////////////////////////////////////////////////////////////////////
void EngineGate::Init()
{
	// 1. 현재 시스템(그래픽카드)이 제공하는 PixelShader, VertexShader 버전에 따른 쉐이더 단계(g_iUseDriverType) 자동 설정.
	InitExtension();
	
	// 2. mfconfig.cfg쉐이더 설정 파일로부터 쉐이더 단계(g_iUseDriverType)를 재설정.
	OpenRendererConf();

#ifdef LDS_MR0_FORDEBUG_MESHALLWAYS_VSTYPE

	// 3. 현재 쉐이더 설정 강제 설정 - 테스트 용도.
#ifdef LDS_MR0_MOD_SET_LOWRENDERER_INIT
	//g_iUseDriverType = GPVER_LOW;
	g_iUseDriverType = GPVER_HIGH;
#else // LDS_MR0_MOD_SET_LOWRENDERER_INIT
	if( g_iUseDriverType == GPVER_HIGHEST )		// ** FragmentShader 버전 아직 미구현
	{
		g_iUseDriverType = GPVER_HIGH;
	}
#endif // LDS_MR0_MOD_LOWRENDERER_SETFORINTIALIZE

#endif // LDS_MR0_FORDEBUG_MESHALLWAYS_VSTYPE

	// 4. 쉐이더 설정 단계에 따른 (버텍스 쉐이더 머신or 프라그먼트 쉐이더 머신)렌더러 초기화
	if(g_iUseDriverType == GPVER_HIGH)			// VertexShaderRendererMachine
	{
		g_Renderer = new CVBOShaderMachine;
		m_bIsOn = true;
	}
	else if(g_iUseDriverType == GPVER_HIGHEST)	// PixelShaderRendererMachine
	{
		g_Renderer = new CVBOPixelShaderMachine;
		m_bIsOn = true;
	}
	else
	{
		m_bIsOn = false;
		ExpCurrentConf();
		return;
	}
	
	// 5. 쉐이더 스크립트 및 쉐이더 텍스쳐 컴파일.
	VPManager::Init();

	// 6. 렌더 메니져 머신 초기화.
	ModelManager::Init();

#ifdef MR0_NEWRENDERING_EFFECTS_SPRITES
	//스프라이트 매니저 초기화
	SpriteManager::Init();
#endif // MR0_NEWRENDERING_EFFECTS_SPRITES

	// 7. CurMR.txt파일로 현재 버전을 출력. // FOR DEBUG
	ExpCurrentConf();
}

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 렌더러를 켜고 끄기.
///////////////////////////////////////////////////////////////////////////////////////////
bool EngineGate::IsOn() { return m_bIsOn; }

#ifdef LDS_MR0_ADD_VBOMANAGER_SETONOFF
void EngineGate::SetOn( bool bSetOn )
{
	// 현재 쉐이더 버전이 Low 면, 이미 쉐이더 렌더러를 하지 않으므로 설정 할 필요 없음.
	if(g_iUseDriverType < GPVER_HIGH)
	{
		return;
	}

	m_bIsOn = bSetOn;
}
#endif // LDS_MR0_ADD_VBOMANAGER_SETONOFF

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 머신 메모리 반환 처리 : 렌더 메쉬 메니져 머신, 쉐이더 컴파일러 링커 메니져, 쉐이더 머신 메모리 반환.
///////////////////////////////////////////////////////////////////////////////////////////
void EngineGate::UnInit()
{
#ifdef MR0_NEWRENDERING_EFFECTS_SPRITES
	SpriteManager::UnInit();
#endif // MR0_NEWRENDERING_EFFECTS_SPRITES

	// 쉐이더 렌더 메니져 머신 메모리 반환.
	ModelManager::UnInit();

	// 컴파일된 쉐이더 링커 OpenGL 명령어를 통한 메모리 반환.
	VPManager::Uninit();

	// MR0 쉐이더 머신 메모리 반환 : 버텍스쉐이더 머신 또는 픽셀쉐이더 머신
	if(g_Renderer) 
	{
		delete g_Renderer;
		g_Renderer = NULL;
	}
}

#endif //MR0
