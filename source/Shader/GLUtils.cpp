#include "stdafx.h"

#ifdef MR0

//////////////////////////////////////////////////////////////////////////
// 
// GLUtils.cpp : 쉐이더 렌더러에서 사용하는 Opengl Extension 연산 함수들 
//
//////////////////////////////////////////////////////////////////////////

#include "GLUtils.h"


BYTE g_iEnumDriverType = GPVER_LOW;						// Driver가 제공하는 쉐이더 타입 버전.
BYTE g_iUseDriverType = GPVER_LOW;						// 외부에서 설정한 쉐이더 타입 버전.
float g_posLight[3] = { 5000.f, -5000.f, 10000.f };		// 라이트 위치
float g_vAngleLight[3] = { 0.f, 0.f, 0.f };				// 라이트 각도값


///////////////////////////////////////////////////////////////////////////////////////////
// OpenGL로 현재 Driver가 제공되는 GL 기능들에서 외부에서 찾고자 하는 기능이 있는지 찾습니다.
///////////////////////////////////////////////////////////////////////////////////////////
bool	InStr(char* searchStr, char* str)
{
	char* extension = NULL;
	char* endofStr = NULL;
	int idx = 0;
	endofStr = str + strlen(str);
	while(str<endofStr)
	{
		idx = strcspn(str, " ");
		if( ((int)strlen(searchStr) == idx) && (strncmp(searchStr, str, idx) == 0) )
		{
			return true;
		}
		str += (idx+1);
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 현재 Driver가 제공하는 Shader Type Version에 따란 쉐이더 타입을 설정 합니다.
///////////////////////////////////////////////////////////////////////////////////////////
void	InitExtension()
{
	char* str_extension = (char* )glGetString(GL_EXTENSIONS);
	if(!str_extension) return;
	if(InStr("GL_ARB_vertex_buffer_object", str_extension) &&
		InStr("GL_ARB_vertex_program",str_extension))
	{
		if(InStr("GL_ARB_fragment_program",str_extension))
			g_iEnumDriverType = GPVER_HIGHEST;
		else
			g_iEnumDriverType = GPVER_HIGH;
	}
	else if(InStr("GL_EXT_compiled_vertex_array", str_extension) &&
		InStr("GL_ARB_vertex_blend", str_extension) &&
		InStr("GL_ARB_multitexture", str_extension) &&
		InStr("GL_ARB_texture_env_combine", str_extension))
	{
		g_iEnumDriverType = GPVER_MIDDLE;
	}
	else
		g_iEnumDriverType = GPVER_LOW;
	
	g_iUseDriverType = g_iEnumDriverType;
}

///////////////////////////////////////////////////////////////////////////////////////////
// 현재 쉐이더 타입 버전 여부 반환.
///////////////////////////////////////////////////////////////////////////////////////////
bool IsHighVersion()
{
	return (g_iUseDriverType >= GPVER_HIGH); 
}
bool IsLowVersion()
{ 
	return (g_iUseDriverType == GPVER_LOW); 
}
bool IsHighestVersion()
{
	return (g_iUseDriverType == GPVER_HIGHEST);
}

///////////////////////////////////////////////////////////////////////////////////////////
// 현재 쉐이더 타입 버전을 강제 설정.
///////////////////////////////////////////////////////////////////////////////////////////
void SetGeneralType(unsigned char t)
{
	if(t > g_iEnumDriverType)
	{
		g_iUseDriverType = g_iEnumDriverType;
		return;
	}

	g_iUseDriverType = t;
}


#endif //MR0