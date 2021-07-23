// DebugCameraTopView.h: interface for the CDebugCameraTopView class.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_PROCESS_DEBUGCAMERA

#if !defined(AFX_DEBUGCAMERATOPVIEW_H__7B99CD9B_2C55_4A76_8B8E_1B8DFF80D99B__INCLUDED_)
#define AFX_DEBUGCAMERATOPVIEW_H__7B99CD9B_2C55_4A76_8B8E_1B8DFF80D99B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DebugCameraInterface.h"

class CDebugCameraTopView  : public CDebugCameraInterface
{
public:
	void RequestMoveEventHorizontalRight()	{ m_bRequestEvent_HorizontalRight = TRUE; };
	void RequestMoveEventHorizontalLeft()	{ m_bRequestEvent_HorizontalLeft = TRUE; };
	void RequestMoveEventVerticalUp()		{ m_bRequestEvent_VerticalUp = FALSE; };
	void RequestMoveEventVerticalDown()		{ m_bRequestEvent_VerticalDown = FALSE; };
	void RequestMoveEventStraightFront()	{ m_bRequestEvent_VerticalUp = TRUE; };
	void RequestMoveEventStraightBack()		{ m_bRequestEvent_VerticalDown = TRUE; };

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
	void Initialize( unsigned int iDebugCameraType,
					 const vec3_t &v3Pos, 
					 const vec3_t &v3Ang, 
					 const float fCameraViewNear,
					 const float fCameraViewFar,
					 const float fCameraVelocityPerTick = _DEFAULT_VELOCITY_PERTICK,
					 const float fRateAngularVelocityPerTick = _DEFAULT_RATE_ANGULARVELOCITY,
					 const BOOL bFrequentlySetValues = FALSE );
	
	void ProcessEvent_RotationEvent();

	void ProcessEvent( int iMousePointX, int iMousePointY  );

	void ProcessEvent_Reset();


	void Release();

public:
	CDebugCameraTopView();
	CDebugCameraTopView( EDEBUGCAMERA_TYPE eDebugCamera );	
	
	virtual ~CDebugCameraTopView();

};

#endif // !defined(AFX_DEBUGCAMERATOPVIEW_H__7B99CD9B_2C55_4A76_8B8E_1B8DFF80D99B__INCLUDED_)

#endif // DO_PROCESS_DEBUGCAMERA