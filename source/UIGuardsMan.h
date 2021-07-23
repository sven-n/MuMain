//////////////////////////////////////////////////////////////////////////
//  
//  UIGuardsMan.h
//  
//  내  용 : 근위병 인터페이스
//  
//  날  짜 : 2004년 11월 12일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGUARDSMAN_H__076768BF_F97F_4B80_B84C_ADFFD37F4E39__INCLUDED_)
#define AFX_UIGUARDSMAN_H__076768BF_F97F_4B80_B84C_ADFFD37F4E39__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUIGuardsMan  
{
public:
	CUIGuardsMan();
	virtual ~CUIGuardsMan();

protected:
	enum
	{
		BC_REQ_LEVEL = 200,			// 공성전 선포하기 위한 최소 레벨
		BC_REQ_MEMBERCOUNT = 20		// 공성전 선포하기 위한 최소 길드인원
	};
	enum REG_STATUS
	{
		REG_STATUS_NONE = 0,
		REG_STATUS_REG = 1
	};
	REG_STATUS	m_eRegStatus;		// 등록상태
	DWORD		m_dwRegMark;

public:
	bool IsSufficentDeclareLevel();	// 참가 신청 가능 레벨제한 검사
	bool HasRegistered() { return (m_eRegStatus == REG_STATUS_REG); }	// 참가 신청 상태인가
	void SetRegStatus( BYTE byStatus ) { m_eRegStatus = (REG_STATUS)byStatus; }	// 참가 신청 상태 업데이트 (서버로부터)

	DWORD GetRegMarkCount() { return m_dwRegMark; }	// 등록된 표식의 수
	void SetMarkCount( DWORD dwMarkCount ) { m_dwRegMark = dwMarkCount; }	// 등록된 표식의 수 업데이트 (서버로부터)

	DWORD GetMyMarkCount();		// 소지하고 있는 표식의 수
	int GetMyMarkSlotIndex();	// 소지하고 있는 첫번째 표식의 인벤토리 인덱스
};

extern CUIGuardsMan g_GuardsMan;

#endif // !defined(AFX_UIGUARDSMAN_H__076768BF_F97F_4B80_B84C_ADFFD37F4E39__INCLUDED_)
