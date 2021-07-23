// DebugCameraInterface.cpp: implementation of the CDebugCameraInterface class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef DO_PROCESS_DEBUGCAMERA

#include "DebugCameraInterface.h"

void CDebugCameraInterface::Initialize( unsigned int iDebugCameraType,
										const vec3_t &v3Pos, 
										const vec3_t &v3Ang, 
										const float fCameraViewNear,
										const float fCameraViewFar,
										const float fCameraVelocityPerTick,
										const float fRateAngularVelocityPerTick,
										const BOOL bFrequentlySetValues )
{
	// 0. 1:1값 대입 시킨 뒤
	m_iMouseXOffset = 0;
	m_iMouseYOffset = 0;
	
	key = iDebugCameraType;
	
	VectorCopy( v3Pos, m_vCameraPos );
	VectorCopy( v3Ang, m_vAngle );

	VectorCopy( v3Pos, m_vBasis_CameraPos );
	VectorCopy( v3Ang, m_vBasis_Angle );
	
	m_fCameraViewFar = fCameraViewFar;
	m_fCameraViewNear = fCameraViewNear;

	m_fRateAngularVelocity = fRateAngularVelocityPerTick;
	m_fVelocityPerTick = fCameraVelocityPerTick;

	m_bFrequentlySetValues = bFrequentlySetValues;

	m_bDidSetValues_ = FALSE;
	
	// 1. Right, Up, Lookat Vector 구성
	ProcessEvent_BuildDirs();
	//ProcessEvent_BuildDirs_UsingQuaternion();

	// 2. Intiailize All Events
	InitAllRequestEvents();
}

void CDebugCameraInterface::ProcessEvent_BuildDirs_UsingQuaternion()
{
	vec3_t		v3Dir_, v3Up_, v3Right_;
	float		matRotation[3][4];

	v3Dir_[0] = 0.0f;		v3Dir_[1] = 1.0f;	v3Dir_[2] = 0.0f;
	v3Up_[0] = 0.0f;		v3Up_[1] = 0.0f;	v3Up_[2] = 1.0f;
	v3Right_[0] = 1.0f;	v3Right_[1] = 0.0f; v3Right_[2] = 0.0f;	

	vec4_t	quatRotation;

	AngleQuaternion( m_vAngle, quatRotation );
	QuaternionMatrix( quatRotation, matRotation );

	VectorRotate( v3Dir_, matRotation, m_vLookAt );	
	VectorRotate( v3Up_, matRotation, m_vUp );	
	VectorRotate( v3Right_, matRotation, m_vRight );
}

void CDebugCameraInterface::ProcessEvent_BuildDirs()
{
	vec3_t		v3Dir_, v3Up_, v3Right_, v3Ang_;
	float		matRotation[3][4];	

	v3Dir_[0] = 0.0f;		v3Dir_[1] = 0.0f;	v3Dir_[2] = -1.0f;
	v3Up_[0] = 0.0f;		v3Up_[1] = 1.0f;	v3Up_[2] = 0.0f;
	v3Right_[0] = 1.0f;		v3Right_[1] = 0.0f; v3Right_[2] = 0.0f;
	
	// Angle To Convert Angle
	v3Ang_[0] = -m_vAngle[0]; v3Ang_[1] = -m_vAngle[1]; v3Ang_[2] = -m_vAngle[2];

	// Rotation Matrix By Scalar
	AngleMatrix( v3Ang_, matRotation );
	
	VectorRotate( v3Dir_, matRotation, m_vLookAt );	
	VectorRotate( v3Up_, matRotation, m_vUp );	
	VectorRotate( v3Right_, matRotation, m_vRight );
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDebugCameraInterface::CDebugCameraInterface()
{

}

CDebugCameraInterface::~CDebugCameraInterface()
{

}

#endif // DO_PROCESS_DEBUGCAMERA