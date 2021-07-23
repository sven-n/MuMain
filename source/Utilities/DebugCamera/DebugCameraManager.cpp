// DebugCameraManager.cpp: implementation of the CDebugCameraManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"


#ifdef DO_PROCESS_DEBUGCAMERA

#include "DebugCameraManager.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDebugCameraManager::CDebugCameraManager()
{
	m_pCurrentActiveCamera = 0;
}

CDebugCameraManager::~CDebugCameraManager()
{

}

void CDebugCameraManager::Initialize(  )
{
	m_pCurrentActiveCamera = 0;

	m_iCurrentActivityCamera = 0;
}

void CDebugCameraManager::Release()
{
	for( unsigned int ui_ = 0; ui_ < m_arDebugCameras.size(); ++ui_ )
	{
		CDebugCameraInterface *pDebugCamera = m_arDebugCameras[ui_];

		delete pDebugCamera; pDebugCamera = 0;
	}

	m_arDebugCameras.clear();
}


void CDebugCameraManager::InsertNewCamera( EDEBUGCAMERA_TYPE eCameraType,
										 const vec3_t &v3Pos, 
										 const vec3_t &v3Ang, 
										 const float fCameraViewNear,
										 const float fCameraViewFar,
										 const float fCameraVelocityPerTick,
	   									 const float fRateAngularVelocityPerTick,
										 const BOOL  bFrequentlySetValues )
{	
	vec3_t v3CameraPosition, v3CameraAngle;
	float	fCameraDistanceNear = 0.0f; 
	float	fCameraDistanceFar = 0.0f;
		
	fCameraDistanceFar = fCameraViewFar;
	fCameraDistanceNear = fCameraViewNear;

	v3CameraPosition[0] = v3Pos[0];
	v3CameraPosition[1] = v3Pos[1];
	v3CameraPosition[2] = v3Pos[2];

	v3CameraAngle[0] = v3Ang[0];
	v3CameraAngle[1] = v3Ang[1];
	v3CameraAngle[2] = v3Ang[2];
	
	CDebugCameraInterface *pNewDebugCamera = 0;	
	switch( eCameraType )
	{
	case EDEBUGCAMERA_NONE:
		{
			pNewDebugCamera = new CDebugCameraDefault();
			m_arDebugCameras.insert( pNewDebugCamera );
		}
		break;
	case EDEBUGCAMERA_FREECONTROL:
		{			
			pNewDebugCamera = new CDebugCameraFreeControl();
			m_arDebugCameras.insert( pNewDebugCamera );
		}
		break;
	case EDEBUGCAMERA_TOPVIEW:
		{
			pNewDebugCamera = new CDebugCameraTopView();
			m_arDebugCameras.insert( pNewDebugCamera );
		}
		break;
	}
	
	pNewDebugCamera->Initialize( (unsigned int)eCameraType, 
								v3CameraPosition, 
								v3CameraAngle, 
								fCameraDistanceNear,
								fCameraDistanceFar, 
								fCameraVelocityPerTick,
								fRateAngularVelocityPerTick,
								bFrequentlySetValues );
}


void CDebugCameraManager::DeleteCamera( EDEBUGCAMERA_TYPE eCameraType )
{
	for( int i_ = 0; i_ < m_arDebugCameras.size(); ++i_ )
	{
		CDebugCameraInterface*	pDebugCamera = m_arDebugCameras[i_];

		if( (EDEBUGCAMERA_TYPE)pDebugCamera->key == eCameraType )
		{
			if( pDebugCamera )
			{
				delete pDebugCamera;	
				pDebugCamera = 0;
			}

			m_arDebugCameras.erase( i_ );
		}
	}
}


BOOL CDebugCameraManager::SetActiveCameraMode(	EDEBUGCAMERA_TYPE eCurrentType,
												const vec3_t &v3Pos, 
												const vec3_t &v3Ang )
{
	CDebugCameraInterface*	pDebugCamera = 0;

	for( int i_ = 0; i_ < m_arDebugCameras.size(); ++i_ )
	{
		pDebugCamera = m_arDebugCameras[i_];
		
		if( (EDEBUGCAMERA_TYPE)pDebugCamera->key == eCurrentType )
		{
			if( pDebugCamera )
			{
				m_pCurrentActiveCamera = pDebugCamera;
				break;
			}
			else
			{
				return FALSE;
			}
		}
	}

	if( 0 == pDebugCamera )
	{
		return FALSE;
	}

	switch( eCurrentType )
	{
	case EDEBUGCAMERA_FREECONTROL:
	case EDEBUGCAMERA_TOPVIEW:	
		{
			if( TRUE == m_pCurrentActiveCamera->m_bFrequentlySetValues ||
				FALSE == m_pCurrentActiveCamera->m_bDidSetValues_ )
			{				
				m_pCurrentActiveCamera->Initialize( eCurrentType,
					v3Pos, 
					v3Ang, 
					m_pCurrentActiveCamera->m_fCameraViewNear,
					m_pCurrentActiveCamera->m_fCameraViewFar,
					m_pCurrentActiveCamera->m_fVelocityPerTick,
					m_pCurrentActiveCamera->m_fRateAngularVelocity,
					m_pCurrentActiveCamera->m_bFrequentlySetValues );

				m_pCurrentActiveCamera->m_bDidSetValues_ = TRUE;
			}
		}
		break;
	}
	
	return TRUE;
}

void CDebugCameraManager::ProcessCamera( int iMousePointX, int iMousePointY )
{
	if( m_pCurrentActiveCamera )
	{
		m_pCurrentActiveCamera->ProcessEvent( iMousePointX, iMousePointY );
	}
}

void CDebugCameraManager::DrawDimensionDebugLine( vec3_t &BasePosition, 
												 vec3_t &v3LookAt, 
												 vec3_t &v3Up, 
												 vec3_t &v3Right )
{
	float fLineLenBasis = 200.0f, fLineLen = 0.0f;
	
	//DWORD dwColorLookAt = 0xff0000ff, dwColorRight = 0x00ff00ff, dwColorUp = 0x0000ffff;
	DWORD dwColorLookAt = 0xffffffff, dwColorRight = 0xffffffff, dwColorUp = 0xffffffff;
	vec3_t v3LookAt_End, v3Right_End, v3Up_End;
	
	fLineLen = fLineLenBasis * 1.0f;

	v3LookAt_End[0] = BasePosition[0] + ( v3LookAt[0] * fLineLen );
	v3LookAt_End[1] = BasePosition[1] + ( v3LookAt[1] * fLineLen );
	v3LookAt_End[2] = BasePosition[2] + ( v3LookAt[2] * fLineLen );

	fLineLen = fLineLenBasis * 0.5f;

	v3Right_End[0] = BasePosition[0] + ( v3Right[0] * fLineLen );
	v3Right_End[1] = BasePosition[1] + ( v3Right[1] * fLineLen );
	v3Right_End[2] = BasePosition[2] + ( v3Right[2] * fLineLen );

	fLineLen = fLineLenBasis * 0.25f;

	v3Up_End[0] = BasePosition[0] + ( v3Up[0] * fLineLen );
	v3Up_End[1] = BasePosition[1] + ( v3Up[1] * fLineLen );
	v3Up_End[2] = BasePosition[2] + ( v3Up[2] * fLineLen );
	
	DRAWLINE3D( BasePosition, v3LookAt_End, dwColorLookAt, 4.0f );
	DRAWLINE3D( BasePosition, v3Up_End, dwColorUp, 4.0f );
	DRAWLINE3D( BasePosition, v3Right_End, dwColorRight, 4.0f );
}

#endif // DO_PROCESS_DEBUGCAMERA
