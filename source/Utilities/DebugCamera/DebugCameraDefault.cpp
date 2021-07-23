// DebugCameraDefault.cpp: implementation of the CDebugCameraDefault class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef DO_PROCESS_DEBUGCAMERA

#include "DebugCameraDefault.h"


void CDebugCameraDefault::Initialize( unsigned int iDebugCameraType,
										const vec3_t &v3Pos, 
										const vec3_t &v3Ang, 
										const float fCameraViewNear,
										const float fCameraViewFar,
										const float fCameraVelocityPerTick,
										const float fRateAngularVelocityPerTick,
										const BOOL bFrequentlySetValues )
{
	CDebugCameraInterface::Initialize( iDebugCameraType,
		v3Pos,
		v3Ang,
		fCameraViewNear,
		fCameraViewFar,
		fCameraVelocityPerTick,
		fRateAngularVelocityPerTick,
		bFrequentlySetValues );
	
	return;
}

void CDebugCameraDefault::CalculateCameraPosition_Vertical( float fValue )
{
	return;
}

void CDebugCameraDefault::CalculateCameraPosition_Horizontal( float fValue )
{
	return;
}

void CDebugCameraDefault::CalculateCameraPosition_Straight( float fValue )
{
	m_vCameraPos[0] += m_vLookAt[0] * fValue;
	m_vCameraPos[1] += m_vLookAt[1] * fValue;
	m_vCameraPos[2] += m_vLookAt[2] * fValue;
}

void CDebugCameraDefault::ProcessEvent_SetPoints_XY( int iCurMouseX, int iCurMouseY )
{
	return;
}

void CDebugCameraDefault::ProcessEvent_RotationEvent()
{	
	return;
}

void CDebugCameraDefault::ProcessEvent( int iMousePointX, int iMousePointY  )
{
	return;		
}

void CDebugCameraDefault::ProcessEvent_Reset()
{
	
	return;
}

void CDebugCameraDefault::Release()
{

	return;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugCameraDefault::CDebugCameraDefault()
{

}

CDebugCameraDefault::~CDebugCameraDefault()
{

}

#endif // DO_PROCESS_DEBUGCAMERA