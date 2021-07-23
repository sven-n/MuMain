// DebugCameraManager.h: interface for the CDebugCameraManager class.
//
//////////////////////////////////////////////////////////////////////

#ifdef DO_PROCESS_DEBUGCAMERA

#if !defined(AFX_DEBUGCAMERAMANAGER_H__4140B1CA_99EC_47E8_8C85_CDF1ECA855E1__INCLUDED_)
#define AFX_DEBUGCAMERAMANAGER_H__4140B1CA_99EC_47E8_8C85_CDF1ECA855E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "./Utilities/Debug/debugSingleton_.h"

#include "./Utilities/Debug/DebugArray_.h"

#include "DebugCameraInterface.h"

#include "DebugCameraDefault.h"

#include "DebugCameraFreeControl.h"

#include "DebugCameraTopView.h"

#include "./Utilities/DebugProfiling/Profiler_util_.h"

class CDebugCameraManager  : public CdebugSingleton_< CDebugCameraManager >
{
private:
	int m_iCurrentActivityCamera;
	
	EDEBUGCAMERA_TYPE GetSequencialActivity()
	{
		m_iCurrentActivityCamera = (m_iCurrentActivityCamera+1) % (int)EDEBUGCAMERA_END;
		return (EDEBUGCAMERA_TYPE)m_iCurrentActivityCamera;
	}
public:
	EDEBUGCAMERA_TYPE GetCurrentActivity()
	{
		return (EDEBUGCAMERA_TYPE)m_iCurrentActivityCamera;
	}

private:
	CDebugCameraInterface						*m_pCurrentActiveCamera;
	util_array_::array_< CDebugCameraInterface* >	m_arDebugCameras;
	void (CDebugCameraManager::*ProcessKeyEvent_Active)( void );
	
public:
	void Initialize();
	void Release();

private:
	float Convert32To1( unsigned int uiCo )
	{
		return (float)uiCo / (float)0xff;
	}
	
	// Draw3DLine
	void DRAWLINE3D( const vec3_t &start, const vec3_t &end, const DWORD color,float flinewidth )
	{
		::glLineWidth(flinewidth);
		::glColor3f( Convert32To1(PickR(color)), Convert32To1(PickG(color)), Convert32To1(PickB(color)) );
		::glBegin(GL_LINES);
		::glVertex3f(start[0], start[1], start[2] );
		::glVertex3f(end[0], end[1], end[2] );
		::glEnd();
	}

	void DrawDimensionDebugLine( vec3_t &BasePosition, vec3_t &v3LookAt, vec3_t &v3Up, vec3_t &v3Right );

public:
	void DrawOriginLine( vec3_t &v3Pos )
	{
		vec3_t		v3Y, v3X, v3Z;

		v3Y[0] = 0.0f; v3Y[1] = 1.0f; v3Y[2] = 0.0f; 
		v3X[0] = 1.0f; v3X[1] = 0.0f; v3X[2] = 0.0f; 
		v3Z[0] = 0.0f; v3Z[1] = 0.0f; v3Z[2] = 1.0f; 

		DrawDimensionDebugLine( v3Pos, v3Y, v3Z, v3X );
	}

	void DrawCameraDebugLine(vec3_t	&v3Pos)
	{
		if( !m_pCurrentActiveCamera || 0 == m_iCurrentActivityCamera )
		{
			return;
		}

		/*
		DrawDimensionDebugLine( m_pCurrentActiveCamera->m_vCameraPos, 
								m_pCurrentActiveCamera->m_vLookAt, 
								m_pCurrentActiveCamera->m_vUp, 
								m_pCurrentActiveCamera->m_vRight );
								*/
		DrawDimensionDebugLine( v3Pos, 
								m_pCurrentActiveCamera->m_vLookAt, 
								m_pCurrentActiveCamera->m_vUp, 
								m_pCurrentActiveCamera->m_vRight );
	}
	
public:
	EDEBUGCAMERA_TYPE GetActiveCameraMode()
	{
		return (EDEBUGCAMERA_TYPE)m_iCurrentActivityCamera;
	}

	void RequestMoveEventStraightFront()
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestMoveEventStraightFront();
	}

	void RequestMoveEventStraightBack()
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestMoveEventStraightBack();
	}

	void RequestMoveEventVerticalUp()
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestMoveEventVerticalUp();
	}

	void RequestMoveEventVerticalDown()
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestMoveEventVerticalDown();
	}

	void RequestMoveEventHorizontalLeft()
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestMoveEventHorizontalLeft();
	}

	void RequestMoveEventHorizontalRight()
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestMoveEventHorizontalRight();
	}

	void RequestRotateEventPointXY()
	{
		if( !m_pCurrentActiveCamera )
			return;
		
		m_pCurrentActiveCamera->RequestRotateEventPointXY();
	}
	
	void RequestSetValueMultipleVelocity( float fMultipleToVelocity = _DEFAULT_MULTIPLE_TO_VEOLOCITY )
	{
		if( !m_pCurrentActiveCamera )
			return;
		
		m_pCurrentActiveCamera->RequestValueMultipleToVelocity(fMultipleToVelocity);
	}

	void RequestSetValueMultipleAngularVelocity( float fMultipleToAngularVelocity = _DEFAULT_MULTIPLE_TO_ANGULAR_VEOLOCITY )
	{
		if( !m_pCurrentActiveCamera )
			return;
		
		m_pCurrentActiveCamera->RequestValueMultipleToAngularVelocity(fMultipleToAngularVelocity);
	}

	void RequestCameraReset( vec3_t *v3RequestAngle, vec3_t *v3RequestPosition )
	{
		if( !m_pCurrentActiveCamera )
			return;

		m_pCurrentActiveCamera->RequestReset( v3RequestAngle, v3RequestPosition );
	}

	void InsertNewCamera( EDEBUGCAMERA_TYPE eCameraType,
						  const vec3_t &v3Pos, 
						  const vec3_t &v3Ang, 
						  const float fCameraViewNear = _DEFAULT_CAMERADISTANCE_NEAR_,
						  const float fCameraViewFar =  _DEFAULT_CAMERADISTANCE_FAR_,
						  const float fCameraVelocityPerTick = _DEFAULT_VELOCITY_PERTICK,
						  const float fRateAngularVelocityPerTick = _DEFAULT_RATE_ANGULARVELOCITY,
						  const BOOL  bFrequentlySetValues = FALSE );

	void DeleteCamera( EDEBUGCAMERA_TYPE eCameraType );

	BOOL SetActiveCameraMode( EDEBUGCAMERA_TYPE eCurrentType,
								const vec3_t &v3Pos, 
								const vec3_t &v3Ang );

	BOOL SetActiveCameraMode_Sequencial( const vec3_t &v3Pos,
										 const vec3_t &v3Ang )
	{
		return SetActiveCameraMode( GetSequencialActivity(), v3Pos, v3Ang );
	}

	CDebugCameraInterface *GetActiveDebugCamera( )
	{ 
		return m_pCurrentActiveCamera;
	}

	void ProcessCamera( int iMousePointX, int iMousePointY );
public:
	CDebugCameraManager();
	virtual ~CDebugCameraManager();
	
};

#endif // !defined(AFX_DEBUGCAMERAMANAGER_H__4140B1CA_99EC_47E8_8C85_CDF1ECA855E1__INCLUDED_)


#endif DO_PROCESS_DEBUGCAMERA