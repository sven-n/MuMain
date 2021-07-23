// DebugCameraDefault.h: interface for the CDebugCameraDefault class.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_PROCESS_DEBUGCAMERA

#if !defined(AFX_DEBUGCAMERADEFAULT_H__80F82360_4233_4B2F_85D6_927DCC321152__INCLUDED_)
#define AFX_DEBUGCAMERADEFAULT_H__80F82360_4233_4B2F_85D6_927DCC321152__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DebugCameraInterface.h"

class CDebugCameraDefault : public CDebugCameraInterface
{
public:
	void RequestMoveEventHorizontalRight()	{ m_bRequestEvent_HorizontalRight = FALSE; };
	void RequestMoveEventHorizontalLeft()	{ m_bRequestEvent_HorizontalLeft = FALSE; };
	void RequestMoveEventVerticalUp()		{ m_bRequestEvent_VerticalUp = FALSE; };
	void RequestMoveEventVerticalDown()		{ m_bRequestEvent_VerticalDown = FALSE; };
	void RequestMoveEventStraightFront()	{ m_bRequestEvent_StraightFront = FALSE; };
	void RequestMoveEventStraightBack()		{ m_bRequestEvent_StraightBack = FALSE; };
	
	void RequestRotateEventPointXY()		{ m_bRequestEvent_Rotate_Point_XY = FALSE; }

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
		m_bRequestReset = FALSE; 
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
	CDebugCameraDefault();
	virtual ~CDebugCameraDefault();

};

#endif // !defined(AFX_DEBUGCAMERADEFAULT_H__80F82360_4233_4B2F_85D6_927DCC321152__INCLUDED_)


#endif // DO_PROCESS_DEBUGCAMERA