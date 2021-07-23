#ifndef _ENGINE_GATE_H_
#define _ENGINE_GATE_H_

#ifdef MR0

//모든 새로 추가되는 헤더파일 이곳에 추가
#include "GLUtils.h"
#include "RenderMachine.h"
#include "glprocs.h"

//////////////////////////////////////////////////////////////////////////
// 
// EngineGate.cpp : 쉐이더 렌더러 내부 렌더러 머신, 메니저 초기화, 메모리 반환 처리
//
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 설정 파일을 입출력 설정. // 테스트용
///////////////////////////////////////////////////////////////////////////////////////////
namespace EngineGate
{
	void			Init();				// 쉐이더 스크립트 컴파일, 쉐이더 렌더 머신 초기화
	void			UnInit();			// 쉐이더 머신 메모리 반환 처리 : 렌더 메쉬 메니져 머신, 쉐이더 컴파일러 링커 메니져, 쉐이더 머신 메모리 반환.
	bool			IsOn();				// 쉐이더 렌더러를 켜졌는지 반환
#ifdef LDS_MR0_ADD_VBOMANAGER_SETONOFF
	void			SetOn(bool bSetOn);	// 쉐이더 렌더러를 켜고 끄기
#endif // LDS_MR0_ADD_VBOMANAGER_SETONOFF
}

#endif // MR0

#endif //_ENGINE_GATE_H_