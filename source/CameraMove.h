// CameraMove.h: interface for the CCameraMove class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAMERAMOVE_H__B3723780_481C_43B7_B72C_5D1782D8324C__INCLUDED_)
#define AFX_CAMERAMOVE_H__B3723780_481C_43B7_B72C_5D1782D8324C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <string>

class CCameraMove  
{
#pragma pack(push, 1)
	struct WAYPOINT {
		int iIndex;
		
		float fCameraX;
		float fCameraY;
		float fCameraZ;

		int iDelay;
		float fCameraMoveAccel;
		float fCameraDistanceLevel;
	};
#pragma pack(pop)

	typedef std::vector<WAYPOINT*>	t_WayPointList;
	t_WayPointList m_listWayPoint;

	float m_CameraStartPos[3];
	float m_fCameraStartDistanceLevel;
	int m_iDelayCount;
	
	DWORD m_dwCameraWalkState;
	float m_CurrentCameraPos[3];
	float m_fCurrentDistanceLevel;

	DWORD	m_dwCurrentIndex;
	int		m_iSelectedTile;

	void Init();
public:
	enum {
		CAMERAWALK_STATE_READY = 0,
		CAMERAWALK_STATE_MOVE,
		CAMERAWALK_STATE_DONE,
	};
	~CCameraMove();

	bool LoadCameraWalkScript(const std::string& filename);
	void UnLoadCameraWalkScript();	//. 맵만 언로드
	bool SaveCameraWalkScript(const std::string& filename);

	void AddWayPoint(int iGridX, int iGridY, float fCameraMoveAccel, float fCameraDistanceLevel, int iDelay);
	void RemoveWayPoint(int iGridX, int iGridY);

	void SetCameraMoveAccel(int iTileIndex, float fCameraMoveAccel);
	void SetCameraDistanceLevel(int iTileIndex, float fCameraDistanceLevel);
	void SetDelay(int iTileIndex, int iDelay);
	float GetCameraMoveAccel(int iTileIndex);
	float GetCameraDistanceLevel(int iTileIndex);
	int GetDelay(int iTileIndex);

	bool IsCameraMove() const;
	void UpdateWayPoint();
	void GetCurrentCameraPos(float CameraPos[3]);
	float GetCurrentCameraDistanceLevel() const;

	void PlayCameraWalk(float StartPos[3], float fStartDistanceLevel);
	void StopCameraWalk(bool bDone);
	void UpdateCameraStartPos(float StartPos[3]);	//. 자연스러운 카메라 마무리 처리를 위해

	void SetCameraWalkState(DWORD dwCameraWalkState);
	DWORD GetCameraWalkState() const;

	void RenderWayPoint();
	void SetSelectedTile(int iTileIndex);
	DWORD GetSelectedTile() const;

	static CCameraMove* GetInstancePtr();

public:
	CCameraMove();		//. ban create instance

// tourmode
private:
	BOOL m_bTourMode;
	BOOL m_bTourPause;
	float m_fForceSpeed;
	float m_fTourCameraAngle;
	float m_fTargetTourCameraAngle;
	float m_vTourCameraPos[3];
#ifdef PJH_NEW_SERVER_SELECT_MAP
	float m_fCameraAngle;
	float m_fFrustumAngle;
#endif //PJH_NEW_SERVER_SELECT_MAP
public:
#ifdef PJH_NEW_SERVER_SELECT_MAP
	// 디폴트 모드일경우 시작점을 설정한다 3번째 인자 디폴트 0
	BOOL SetTourMode(BOOL bFlag, BOOL bRandomStart = FALSE, int _index = 0);
#else //PJH_NEW_SERVER_SELECT_MAP
	BOOL SetTourMode(BOOL bFlag, BOOL bRandomStart = FALSE);
#endif //PJH_NEW_SERVER_SELECT_MAP
	BOOL IsTourMode() { return m_bTourMode; }
	BOOL IsTourPaused() { return m_bTourPause; }
 	void PauseTour(BOOL bFlag);
 	void ForwardTour(float fSpeed);
 	void BackwardTour(float fSpeed);
	void UpdateTourWayPoint();
#ifdef PJH_NEW_SERVER_SELECT_MAP
	// 서버 선택화면에서 프러스텀의 값을 카메라앵글을 갖어와서 사용한다
	// 프러스텀은 나두고 앵글만 올리기 위함으로 사용한다
	void SetAngleFrustum(float _Value);		// 카메라 앵글값설정
	const float GetAngleFrustum() const { return m_fCameraAngle; };
	void SetFrustumAngle(float _Value);		// 프러스텀에 사용할 앵글값
	float GetFrustumAngle();
#endif //PJH_NEW_SERVER_SELECT_MAP
	
	float GetCameraAngle() { return m_fTourCameraAngle; }
};

#endif // !defined(AFX_CAMERAMOVE_H__B3723780_481C_43B7_B72C_5D1782D8324C__INCLUDED_)
