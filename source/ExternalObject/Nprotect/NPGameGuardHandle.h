// NPGameGuardHandle.h: interface for the CNPGameGuardHandle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NPGAMEGUARDHANDLE_H__B48EB0DA_BD1E_4DF7_82BC_62989273759B__INCLUDED_)
#define AFX_NPGAMEGUARDHANDLE_H__B48EB0DA_BD1E_4DF7_82BC_62989273759B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef PBG_MOD_GAMEGUARD_HANDLE
class CNPGameGuardHandle  
{
	enum {
		MAX_TEXT = 50,
	};
public:
	CNPGameGuardHandle();
	virtual ~CNPGameGuardHandle();

	static CNPGameGuardHandle* GetInstance();

	// 캐릭터 정보관리
	void SetCharacterInfo(char* _Name, char* _Hack);
	char* GetCharacterName();
	char* GetCharacterHack();

	// 오토 사용여부
	void SetNPHack(BOOL _IsHack);
	BOOL IsNPHack();

	// 타임체크
	bool CheckTime();
	// 핵발견이후(캐릭선택창이후) 종료시점계산
	bool CheckDestroyWin();
	// 메시지 박스관련
	void SetDestroyMsg(bool _bDestroyMsg);
	BOOL GetDestroyMsg();
	// 게임가드 에러메시지 인덱스 관리
	void SetErrorMsgIndex(DWORD _nMsgIndex);
	DWORD GetErrorMsgIndex();
private:
	const int m_iTime;
	int m_nRandTime;
	DWORD m_dwNPGameCurrTime;
	BOOL m_bHack;
	BOOL m_bDestroyMsg;
	int m_nStandardTime;
	DWORD m_dwErrorMsgIndex;

	char m_szCharName[MAX_TEXT];
	char m_szHackName[MAX_TEXT];

	void Init();
	void Destroy();
};

#define g_NPGameGuardHandle	CNPGameGuardHandle::GetInstance()

#endif //PBG_MOD_GAMEGUARD_HANDLE
#endif // !defined(AFX_NPGAMEGUARDHANDLE_H__B48EB0DA_BD1E_4DF7_82BC_62989273759B__INCLUDED_)
