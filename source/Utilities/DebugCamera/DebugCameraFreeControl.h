// DebugCameraFreeControl.h: interface for the CDebugCameraFreeControl class.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_PROCESS_DEBUGCAMERA

#if !defined(AFX_DEBUGCAMERAFREECONTROL_H__19A05458_77F8_4BCF_AE37_7C46873665E8__INCLUDED_)
#define AFX_DEBUGCAMERAFREECONTROL_H__19A05458_77F8_4BCF_AE37_7C46873665E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DebugCameraInterface.h"

#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}

class CDebugCameraFreeControl : public CDebugCameraInterface
{
public:
	void RequestMoveEventHorizontalRight()	{ m_bRequestEvent_HorizontalRight = TRUE; };
	void RequestMoveEventHorizontalLeft()	{ m_bRequestEvent_HorizontalLeft = TRUE; };
	void RequestMoveEventVerticalUp()		{ m_bRequestEvent_VerticalUp = TRUE; };
	void RequestMoveEventVerticalDown()		{ m_bRequestEvent_VerticalDown = TRUE; };
	void RequestMoveEventStraightFront()	{ m_bRequestEvent_StraightFront = TRUE; };
	void RequestMoveEventStraightBack()		{ m_bRequestEvent_StraightBack = TRUE; };
	void RequestRotateEventPointXY()		{ m_bRequestEvent_Rotate_Point_XY = TRUE; }

	virtual void RequestValueMultipleToVelocity(float	fMultipleToVelocity = _DEFAULT_MULTIPLE_TO_VEOLOCITY )
	{
		m_bRequestEvent_MultipleToVelocity = TRUE;
		m_fMultipleToVelocity = fMultipleToVelocity;
	}
	
	virtual void RequestValueMultipleToAngularVelocity(float	fMultipleToAngularVelocity = _DEFAULT_MULTIPLE_TO_ANGULAR_VEOLOCITY )
	{
		m_bRequestEvent_MultipleToAngularVelocity = TRUE;
		m_fMultipleToAngularVelocity = fMultipleToAngularVelocity;
	}

	void RequestReset(  vec3_t *v3Angle =0, vec3_t *v3Position = 0 )
	{ 
		if( 0 != v3Angle )
		{
			VectorCopy( *v3Angle, m_vBasis_Angle );
		}

		if( 0 != v3Position )
		{
			VectorCopy( *v3Position, m_vBasis_CameraPos );
		}
		
		m_bRequestReset = TRUE; 
	}

	void ProcessEvent_SetPoints_XY( int iCurMouseX, int iCurMouseY );

protected:
	void CalculateCameraPosition_Vertical( float fValue );
	
	void CalculateCameraPosition_Horizontal( float fValue );
	
	void CalculateCameraPosition_Straight( float fValue );

protected:
	void ProcessEvent_RotationEvent();

	void ProcessEvent_Reset();

public:
	void Initialize( unsigned int iDebugCameraType,
					 const vec3_t &v3Pos, 
					 const vec3_t &v3Ang, 
					 const float fCameraViewNear,
					 const float fCameraViewFar,
					 const float fCameraVelocityPerTick = _DEFAULT_VELOCITY_PERTICK,
					 const float fRateAngularVelocityPerTick = _DEFAULT_RATE_ANGULARVELOCITY,
					 const BOOL bFrequentlySetValues = FALSE );

	void ProcessEvent( int iMousePointX, int iMousePointY );

	void Release();
	
public:
	CDebugCameraFreeControl();
	CDebugCameraFreeControl( EDEBUGCAMERA_TYPE eDebugCamera );
	
	virtual ~CDebugCameraFreeControl();

};

#endif // !defined(AFX_DEBUGCAMERAFREECONTROL_H__19A05458_77F8_4BCF_AE37_7C46873665E8__INCLUDED_)

#endif // DO_PROCESS_DEBUGCAMERA
