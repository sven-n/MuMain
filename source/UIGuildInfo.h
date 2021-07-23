//////////////////////////////////////////////////////////////////////////
//  
//  UIGuildInfo.h
//  
//  내  용 : 길드정보 인터페이스 ( G 키 입력시 )
//  
//  날  짜 : 2004년 11월 09일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGUILDINFO_H__EC7AE059_4458_4AA3_A31E_C893AEFDECD2__INCLUDED_)
#define AFX_UIGUILDINFO_H__EC7AE059_4458_4AA3_A31E_C893AEFDECD2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "UIWindows.h"


class CUIManager;
class CUIGuildInfo;
class CUIGuildMaster;

extern int s_nTargetFireMemberIndex;

// 길드직위
enum GUILD_STATUS
{
	G_NONE			= (BYTE)-1,
	G_PERSON		= 0,
	G_MASTER		= 128,
	G_SUB_MASTER	= 64,
	G_BATTLE_MASTER	= 32
};
// 길드타입
enum GUILD_TYPE
{
	GT_NORMAL		= 0x00,		// 일반길드
	GT_ANGEL		= 0x01		// 수호천사길드
};
// 길드관계
enum GUILD_RELATIONSHIP
{
	GR_NONE			= 0x00,		// 관계엾음
	GR_UNION		= 0x01,		// 길드연합
	GR_UNIONMASTER	= 0x04,		// 길드연합마스터
	GR_RIVAL		= 0x02,		// 적대길드
	GR_RIVALUNION	= 0x08		// 적대길드길드연합
};

// 길드 정보창
class CUIGuildInfo : public CUIControl
{
public:
	CUIGuildInfo();
	virtual ~CUIGuildInfo();

protected:
	bool			m_bOpened;
	int				m_nCurrentTab;		// 정보 탭 ( 0: 길드, 1:길드원, 2:길드연합 )

	BOOL			m_bRequestUnionList;	// 연합리스트요청했는지 여부 (창닫기전에는 한번만)

	char			m_szRivalGuildName[MAX_GUILDNAME+1];

	CUINewGuildMemberListBox	m_GuildMemberListBox;
	CUIGuildNoticeListBox		m_GuildNoticeListBox;
	CUIUnionGuildListBox		m_UnionListBox;

	DWORD			m_dwPopupID;

	// 길드 - 해체/탈퇴 버튼
	CUIButton		m_BreakUpGuildButton;

	// 길드원 - 직책, 해제, 방출 버튼
	CUIButton		m_AppointButton;
	CUIButton		m_DisbandButton;
	CUIButton		m_FireButton;

	// 연합 - 해체/탈퇴
	CUIButton		m_BreakUnionButton;

	// 연합 - 방출
	CUIButton		m_BanUnionButton;
	
protected:
	BOOL IsGuildMaster()	{ return ( Hero->GuildStatus == G_MASTER ); }			// 길마인가
	BOOL IsSubGuildMaster()	{ return ( Hero->GuildStatus == G_SUB_MASTER ); }		// 부길마인가
	BOOL IsBattleMaster()	{ return ( Hero->GuildStatus == G_BATTLE_MASTER ); }	// 배틀마스터인가

	int GetGuildMemberIndex( char* szName );
	const char* GetGuildMasterName();
	const char* GetSubGuildMasterName();
	const char* GetBattleMasterName();

	// 이 인터페이스에서 띄운 팝업을 닫는다.
	void CloseMyPopup();
	
	// 길드
	void DoGuildInfoTabMouseAction();
	void RenderGuildInfoTab();

	// 길드원
	void DoGuildMemberTabMouseAction();
	void RenderGuildMemberTab();

	// 길드연합
	void DoGuildUnionMouseAction();
	void RenderGuildUnionTab();

public:
	void SetRivalGuildName( char* szName );

	void AddGuildNotice( char* szText );		// 길드공지 추가
	void ClearGuildLog();

	void AddMemberList( GUILD_LIST_t* pInfo );
	void ClearMemberList();

	void AddUnionList( BYTE* pGuildMark, char* szGuildName, int nMemberCount );
	int GetUnionCount();
	void ClearUnionList();

	virtual BOOL DoMouseAction();
	virtual void Render();
	void Open();
	bool IsOpen();
	virtual void Close();
};

void    UseBattleMasterSkill ( void );

#endif // !defined(AFX_UIGUILDINFO_H__EC7AE059_4458_4AA3_A31E_C893AEFDECD2__INCLUDED_)
