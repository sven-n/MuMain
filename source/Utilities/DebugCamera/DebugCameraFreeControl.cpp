// DebugCameraFreeControl.cpp: implementation of the CDebugCameraFreeControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef DO_PROCESS_DEBUGCAMERA

#include "DebugCameraFreeControl.h"

void CDebugCameraFreeControl::Initialize( unsigned int iDebugCameraType,
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

void CDebugCameraFreeControl::ProcessEvent_SetPoints_XY( int iCurMouseX, int iCurMouseY )
{
	m_iMouseXOffset = (iCurMouseX - m_iMouseXPre);
	m_iMouseYOffset = (iCurMouseY - m_iMouseYPre);
	
	m_iMouseXPre = iCurMouseX;
	m_iMouseYPre = iCurMouseY;
}


void CDebugCameraFreeControl::CalculateCameraPosition_Vertical( float fValue )
{
	m_vCameraPos[0] += m_vUp[0] * fValue;
	m_vCameraPos[1] += m_vUp[1] * fValue;
	m_vCameraPos[2] += m_vUp[2] * fValue;	
}

void CDebugCameraFreeControl::CalculateCameraPosition_Horizontal( float fValue )
{
	m_vCameraPos[0] += m_vRight[0] * fValue;
	m_vCameraPos[1] += m_vRight[1] * fValue;
	m_vCameraPos[2] += m_vRight[2] * fValue;
}

void CDebugCameraFreeControl::CalculateCameraPosition_Straight( float fValue )
{
	m_vCameraPos[0] += m_vLookAt[0] * fValue;
	m_vCameraPos[1] += m_vLookAt[1] * fValue;
	m_vCameraPos[2] += m_vLookAt[2] * fValue;
}

void CDebugCameraFreeControl::ProcessEvent_RotationEvent()
{	
	if( abs(m_iMouseXOffset) + abs(m_iMouseYOffset) > 0 )
	{		
		m_vAngle[2] = m_vAngle[2] + ((float)m_iMouseXOffset * m_fRateAngularVelocity * m_fMultipleToAngularVelocity);
		m_vAngle[0] = m_vAngle[0] + ((float)m_iMouseYOffset * m_fRateAngularVelocity * m_fMultipleToAngularVelocity);
	}
	else
	{
		return;
	}
}

void CDebugCameraFreeControl::ProcessEvent( int iMousePointX, int iMousePointY )
{
	ProcessEvent_SetPoints_XY( iMousePointX, iMousePointY );
	if( TRUE == ProcessEvent_Check() )
	{
		if( TRUE == m_bRequestEvent_Rotate_Point_XY )
		{
			ProcessEvent_RotationEvent();	// 사용자 키값의 회전값을 Angle에 반영
		}
		ProcessEvent_BuildDirs();		// Direction Vector들을 구성.

		if( TRUE == m_bRequestReset )
		{
			ProcessEvent_Reset();
		}
		
		float fDist = m_fVelocityPerTick * m_fMultipleToVelocity;
		
		if( TRUE == m_bRequestEvent_VerticalUp )
		{
			CalculateCameraPosition_Vertical( fDist );
		}
		else if( TRUE == m_bRequestEvent_VerticalDown )
		{
			CalculateCameraPosition_Vertical( -fDist );
		}
		else if( TRUE == m_bRequestEvent_HorizontalLeft )
		{
			CalculateCameraPosition_Horizontal( -fDist );
		}
		else if( TRUE == m_bRequestEvent_HorizontalRight )
		{
			CalculateCameraPosition_Horizontal( fDist );
		}
		else if( TRUE == m_bRequestEvent_StraightFront )
		{
			CalculateCameraPosition_Straight( fDist );
		}
		else if( TRUE == m_bRequestEvent_StraightBack )
		{
			CalculateCameraPosition_Straight( -fDist );
		}	
	}

	InitAllRequestEvents();
}

void CDebugCameraFreeControl::ProcessEvent_Reset()
{
	VectorCopy( m_vBasis_Angle, m_vAngle );
	VectorCopy( m_vBasis_CameraPos, m_vCameraPos );

	return;
}

void CDebugCameraFreeControl::Release()
{
	
	return;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugCameraFreeControl::CDebugCameraFreeControl()
{
}

CDebugCameraFreeControl::~CDebugCameraFreeControl()
{

}

#endif // DO_PROCESS_DEBUGCAMERA
