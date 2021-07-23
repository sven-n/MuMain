// DebugCameraInterface.h: interface for the CDebugCameraInterface class.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_PROCESS_DEBUGCAMERA

#if !defined(AFX_DEBUGCAMERAINTERFACE_H__7EED12EC_F2C6_4609_A29F_1F65C192F6C6__INCLUDED_)
#define AFX_DEBUGCAMERAINTERFACE_H__7EED12EC_F2C6_4609_A29F_1F65C192F6C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DebugCameraDefine.h"

typedef float vec_t;
typedef vec_t vec3_t[3];

class CDebugCameraInterface  
{	
public: 
	unsigned int key;

public:
	float	m_fRateAngularVelocity, m_fVelocityPerTick, m_fMultipleToVelocity, m_fMultipleToAngularVelocity;
	
	int		m_iMouseXPre, m_iMouseYPre, m_iMouseXOffset, m_iMouseYOffset;
	BOOL	m_bRequestEvent_HorizontalRight, m_bRequestEvent_HorizontalLeft,
			m_bRequestEvent_VerticalUp, m_bRequestEvent_VerticalDown,
			m_bRequestEvent_StraightFront, m_bRequestEvent_StraightBack,
			m_bRequestEvent_Rotate_Point_XY, m_bRequestReset,
			m_bRequestEvent_MultipleToVelocity, m_bRequestEvent_MultipleToAngularVelocity;

public:
	BOOL	m_bFrequentlySetValues, m_bDidSetValues_;

public:
	vec3_t m_vBasis_CameraPos;
	vec3_t m_vBasis_Angle;

	vec3_t m_vCameraPos;
	vec3_t m_vLookAt;
	vec3_t m_vRight;
	vec3_t m_vUp;
	
	vec3_t m_vBaseLookAt;
	
	vec3_t m_vAngle;

	float m_fCameraViewFar;
	float m_fCameraViewNear;

protected:
	void InitAllRequestEvents()
	{
		m_bRequestEvent_HorizontalRight = FALSE;
		m_bRequestEvent_HorizontalLeft	= FALSE;
		m_bRequestEvent_VerticalUp		= FALSE;
		m_bRequestEvent_VerticalDown	= FALSE;
		m_bRequestEvent_StraightFront	= FALSE;
		m_bRequestEvent_StraightBack	= FALSE;
		m_bRequestEvent_Rotate_Point_XY = FALSE;
		m_bRequestReset					= FALSE;

		m_bRequestEvent_MultipleToVelocity = FALSE;
		m_bRequestEvent_MultipleToAngularVelocity = FALSE;
	}

	virtual BOOL ProcessEvent_Check()
	{
		if( m_bRequestEvent_HorizontalLeft ||
			m_bRequestEvent_HorizontalRight || 
			m_bRequestEvent_VerticalUp || 
			m_bRequestEvent_VerticalDown ||
			m_bRequestEvent_StraightFront ||
			m_bRequestEvent_StraightBack ||
			m_bRequestEvent_Rotate_Point_XY || 
			m_bRequestReset ||
			(m_iMouseXOffset + m_iMouseYOffset > 0 ) )
		{
			return TRUE;	
		}
		
		return FALSE;
	}
	
	
	void ProcessEvent_BuildDirs();

	void ProcessEvent_BuildDirs_UsingQuaternion();

	virtual void ProcessEvent_Reset()					= 0;
	
	virtual void ProcessEvent_RotationEvent()			= 0;

	virtual void ProcessEvent_SetPoints_XY( int iCurMouseX, int iCurMouseY ) = 0;

public:
	virtual void RequestMoveEventHorizontalRight()	= 0;
	virtual void RequestMoveEventHorizontalLeft()	= 0;
	virtual void RequestMoveEventVerticalUp()		= 0;
	virtual void RequestMoveEventVerticalDown()		= 0;
	virtual void RequestMoveEventStraightFront()	= 0;
	virtual void RequestMoveEventStraightBack()		= 0;
	virtual void RequestRotateEventPointXY()		= 0;

	virtual void RequestValueMultipleToVelocity(float	fMultipleToVelocity = _DEFAULT_MULTIPLE_TO_VEOLOCITY ) = 0;
	virtual void RequestValueMultipleToAngularVelocity(float	fMultipleToVelocity = _DEFAULT_MULTIPLE_TO_ANGULAR_VEOLOCITY ) = 0;
	
	virtual void RequestReset(  vec3_t *v3Angle =0, vec3_t *v3Position = 0 )
													= 0;

public:
	virtual void Initialize( unsigned int iDebugCameraType,
							 const vec3_t &v3Pos, 
							 const vec3_t &v3Ang, 
							 const float fCameraViewNear,
							 const float fCameraViewFar,
							 const float fCameraVelocityPerTick = _DEFAULT_VELOCITY_PERTICK,
							 const float fRateAngularVelocityPerTick = _DEFAULT_RATE_ANGULARVELOCITY,
							 const BOOL bFrequentlySetValues = FALSE );

	virtual void Release() = 0;
	
	virtual void ProcessEvent( int iMousePointX, int iMousePointY  ) = 0;
public:
	CDebugCameraInterface();

	virtual ~CDebugCameraInterface();

};

#endif // !defined(AFX_DEBUGCAMERAINTERFACE_H__7EED12EC_F2C6_4609_A29F_1F65C192F6C6__INCLUDED_)

#endif // DO_PROCESS_DEBUGCAMERA
