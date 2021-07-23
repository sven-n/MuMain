#ifndef __GLUTILS_H__
#define __GLUTILS_H__


///////////////////////////////////////////////////////////////////////////////////////////
// GLUtils.h : 쉐이더 타입 버전.
///////////////////////////////////////////////////////////////////////////////////////////

void	InitExtension();				// Driver가 제공하는 ShaderType 설정
bool	IsHighVersion();				// VertexShader
bool	IsHighestVersion();				// FragmentShader
bool	IsLowVersion();					// No Shader
void	SetGeneralType(unsigned char t);// Shader Render Type 설정

///////////////////////////////////////////////////////////////////////////////////////////
// 쉐이더 타입 버전.
///////////////////////////////////////////////////////////////////////////////////////////
enum _GP_DRIVER_INFO
{
	GPVER_LOW,							// 1.1
	GPVER_MIDDLE,						// 1.1 이상, Vertex Blend, Complied Array
	GPVER_HIGH,							// VBO, vertex prog
	GPVER_HIGHEST,						// VBO, vertex, fragment prog					
};
extern unsigned char g_iEnumDriverType;	// Driver가 제공하는 Shader Type 
extern unsigned char g_iUseDriverType;	// 최종 VertexShader 또는 PixelShader 선택

extern float g_posLight[3];				// 전역 광원 위치
extern float g_vAngleLight[3];			// 전역 광원 방향

//WANI_COMMON 정리가 필요함...
// 부스트 도입
//#include "WinMain.h"
//#include <gl/gl.h>
//#include <gl/glu.h>
//#include "glext.h"
//#include "wglext.h"
//#include "glprocs.h"


#endif __GLUTILS_H__