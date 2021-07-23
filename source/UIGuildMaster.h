//////////////////////////////////////////////////////////////////////////
//  
//  UIGuildMaster.h
//  
//  내  용 : NPC 길드마스터 클릭시 보여주는 인터페이스
//  
//  날  짜 : 2004년 11월 09일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGUILDMASTER_H__A1F8F21B_D41E_4688_A9E8_C5B29A6F3074__INCLUDED_)
#define AFX_UIGUILDMASTER_H__A1F8F21B_D41E_4688_A9E8_C5B29A6F3074__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIWindows.h"

class CUIManager;
class CUIGuildInfo;
class CUIGuildMaster;

enum eCurrentMode	{ MODE_NONE, MODE_CREATE_GUILD, MODE_EDIT_GUILDMARK };
enum eCurrentStep	{ STEP_MAIN, STEP_CREATE_GUILDINFO, STEP_EDIT_GUILD_MARK, STEP_CONFIRM_GUILDINFO };


// 길드 마스터 인터페이스
class CUIGuildMaster : public CUIControl
{
public:
	CUIGuildMaster();
	virtual ~CUIGuildMaster();

protected:
	bool			m_bOpened;
	// STEP_MAIN
	CUIButton		m_CreateGuildButton;
	CUIButton		m_EditGuildMarkButton;

	CUIButton		m_NextButton;
	CUIButton		m_PreviousButton;

	DWORD			m_dwEditGuildMarkConfirmPopup;
	DWORD			m_dwGuildRelationShipReplyPopup;

protected:
	BOOL IsValidGuildName( const char* szName );		// 유효한 길드명인지 검사
	BOOL IsValidGuildMark();							// 유효한 길드마크인지 검사

	// 이전, 다음 버튼 눌렀을때의 스텝 변경
	void StepPrev();
	void StepNext();

	// 이 인터페이스에서 띄운 팝업을 닫는다.
	void CloseMyPopup();

// 길드 만들기		STEP_MAIN -> STEP_CREATE_GUILDINFO -> STEP_CONFIRM_GUILDINFO
	// 길드 만들기 - 길드명, 마크 입력 ( STEP_CREATE_GUILDINFO )
	void DoCreateGuildAction();
	void RenderCreateGuild();

	// 길드 만들기 - 생성정보 확인 ( STEP_CONFIRM_GUILDINFO )
	void DoCreateInfoAction();
	void RenderCreateInfo();


// 길드마크 변경	STEP_MAIN -> STEP_EDIT_GUILD_MARK -> STEP_CONFIRM_GUILDINFO
	// 길드마크 변경 ( STEP_EDIT_GUILD_MARK )
	void DoEditGuildMarkAction();
	void RenderEditGuildMark();


// 길드마스터 초기화면 ( STEP_MAIN )
	void DoGuildMasterMainAction();
	void RenderGuildMasterMain();

public:
	// 길드관계맺기 요청 받음
	void ReceiveGuildRelationShip( BYTE byRelationShipType, BYTE byRequestType, BYTE  byTargetUserIndexH, BYTE byTargetUserIndexL );

	virtual BOOL DoMouseAction();
	virtual void Render();
	void Open();
	bool IsOpen();
	virtual void Close();
};

#endif // !defined(AFX_UIGUILDMASTER_H__A1F8F21B_D41E_4688_A9E8_C5B29A6F3074__INCLUDED_)
