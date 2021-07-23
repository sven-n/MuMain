// DebugCameraTopView.cpp: implementation of the CDebugCameraTopView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#ifdef DO_PROCESS_DEBUGCAMERA

#include "DebugCameraTopView.h"

void CDebugCameraTopView::Initialize( unsigned int iDebugCameraType,
									 const vec3_t &v3Pos, 
									 const vec3_t &v3Ang, 
									 const float fCameraViewNear,
									 const float fCameraViewFar,
									 const float fCameraVelocityPerTick,
									 const float fRateAngularVelocityPerTick,
									 const BOOL bFrequentlySetValues )
{
	vec3_t	v3Pos_, v3Ang_;
	v3Pos_[0] = 0.0f;	v3Pos_[1] = 0.0f; v3Pos_[2] = 0.0f;
	v3Ang_[0] = 0.0f;	v3Ang_[1] = 0.0f; v3Ang_[2] = 0.0f;
	
	VectorCopy( v3Pos, v3Pos_ );
	v3Pos_[2] = v3Pos_[2] + fCameraViewFar;

	ProcessEvent_BuildDirs();
	
	CDebugCameraInterface::Initialize( iDebugCameraType,
		v3Pos_,
		v3Ang_,
		fCameraViewNear,
		fCameraViewFar,
		fCameraVelocityPerTick,
		fRateAngularVelocityPerTick,
		bFrequentlySetValues );
	
	return;
}

void CDebugCameraTopView::ProcessEvent_SetPoints_XY( int iCurMouseX, int iCurMouseY )
{
	m_iMouseXOffset = (iCurMouseX - m_iMouseXPre);
	m_iMouseYOffset = (iCurMouseY - m_iMouseYPre);
	
	m_iMouseXPre = iCurMouseX;
	m_iMouseYPre = iCurMouseY;
}


void CDebugCameraTopView::CalculateCameraPosition_Vertical( float fValue )
{
	m_vCameraPos[0] += m_vUp[0] * fValue;
	m_vCameraPos[1] += m_vUp[1] * fValue;
	m_vCameraPos[2] += m_vUp[2] * fValue;	
}

void CDebugCameraTopView::CalculateCameraPosition_Horizontal( float fValue )
{
	m_vCameraPos[0] += m_vRight[0] * fValue;
	m_vCameraPos[1] += m_vRight[1] * fValue;
	m_vCameraPos[2] += m_vRight[2] * fValue;
}

void CDebugCameraTopView::CalculateCameraPosition_Straight( float fValue )
{
	m_vCameraPos[0] += m_vLookAt[0] * fValue;
	m_vCameraPos[1] += m_vLookAt[1] * fValue;
	m_vCameraPos[2] += m_vLookAt[2] * fValue;
}

void CDebugCameraTopView::ProcessEvent_RotationEvent()
{	
	if( abs(m_iMouseXOffset) + abs(m_iMouseYOffset) > 0 )
	{		
		m_vAngle[2] = m_vAngle[2] + ((float)m_iMouseXOffset * m_fRateAngularVelocity);
		m_vAngle[0] = m_vAngle[0] + ((float)m_iMouseYOffset * m_fRateAngularVelocity);
	}
	else
	{
		return;
	}
}

void CDebugCameraTopView::ProcessEvent( int iMousePointX, int iMousePointY  )
{
	ProcessEvent_SetPoints_XY( iMousePointX, iMousePointY );
	
	if( TRUE == ProcessEvent_Check() )
	{
		if( TRUE == m_bRequestEvent_Rotate_Point_XY )
		{
			ProcessEvent_RotationEvent();	// 사용자 키값의 회전값을 Angle에 반영
		}

		if( TRUE == m_bRequestReset )
		{
			ProcessEvent_Reset();
		}

		float fDist = m_fVelocityPerTick;
		
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
		
		ProcessEvent_BuildDirs();	
	}
	
	InitAllRequestEvents();
}

void CDebugCameraTopView::ProcessEvent_Reset()
{
	m_vAngle[0] = 0.0f;	m_vAngle[1] = 0.0f; m_vAngle[2] = 0.0f;
	m_vAngle[0] = 270.0f;

	VectorCopy( m_vBasis_CameraPos, m_vCameraPos );
	m_vCameraPos[2] = m_vCameraPos[2] + m_fCameraViewFar;

	ProcessEvent_BuildDirs();
	
	return;
}


void CDebugCameraTopView::Release()
{
	
	return;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugCameraTopView::CDebugCameraTopView()
{
}

CDebugCameraTopView::~CDebugCameraTopView()
{

}

#endif // DO_PROCESS_DEBUGCAMERA