//////////////////////////////////////////////////////////////////////////
//  UIGuildInfo.h
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "UIWindows.h"

class CUIManager;
class CUIGuildInfo;
class CUIGuildMaster;

extern int s_nTargetFireMemberIndex;

enum GUILD_STATUS
{
    G_NONE = (BYTE)-1,
    G_PERSON = 0,
    G_MASTER = 128,
    G_SUB_MASTER = 64,
    G_BATTLE_MASTER = 32
};

enum GUILD_TYPE
{
    GT_NORMAL = 0x00,
    GT_ANGEL = 0x01
};

enum GUILD_RELATIONSHIP
{
    GR_NONE = 0x00,
    GR_UNION = 0x01,
    GR_UNIONMASTER = 0x04,
    GR_RIVAL = 0x02,
    GR_RIVALUNION = 0x08
};

class CUIGuildInfo : public CUIControl
{
public:
    CUIGuildInfo();
    virtual ~CUIGuildInfo();

protected:
    bool			m_bOpened;
    int				m_nCurrentTab;

    BOOL			m_bRequestUnionList;

    wchar_t			m_szRivalGuildName[MAX_GUILDNAME + 1];

    CUINewGuildMemberListBox	m_GuildMemberListBox;
    CUIGuildNoticeListBox		m_GuildNoticeListBox;
    CUIUnionGuildListBox		m_UnionListBox;

    DWORD			m_dwPopupID;
    CUIButton		m_BreakUpGuildButton;
    CUIButton		m_AppointButton;
    CUIButton		m_DisbandButton;
    CUIButton		m_FireButton;
    CUIButton		m_BreakUnionButton;
    CUIButton		m_BanUnionButton;

protected:
    BOOL IsGuildMaster() { return (Hero->GuildStatus == G_MASTER); }
    BOOL IsSubGuildMaster() { return (Hero->GuildStatus == G_SUB_MASTER); }
    BOOL IsBattleMaster() { return (Hero->GuildStatus == G_BATTLE_MASTER); }

    int GetGuildMemberIndex(wchar_t* szName);
    const wchar_t* GetGuildMasterName();
    const wchar_t* GetSubGuildMasterName();
    const wchar_t* GetBattleMasterName();

    void CloseMyPopup();

    void DoGuildInfoTabMouseAction();
    void RenderGuildInfoTab();

    void DoGuildMemberTabMouseAction();
    void RenderGuildMemberTab();

    void DoGuildUnionMouseAction();
    void RenderGuildUnionTab();

public:
    void SetRivalGuildName(wchar_t* szName);

    void AddGuildNotice(wchar_t* szText);
    void ClearGuildLog();

    void AddMemberList(GUILD_LIST_t* pInfo);
    void ClearMemberList();

    void AddUnionList(BYTE* pGuildMark, wchar_t* szGuildName, int nMemberCount);
    int GetUnionCount();
    void ClearUnionList();

    virtual BOOL DoMouseAction();
    virtual void Render();
    void Open();
    bool IsOpen();
    virtual void Close();
};

void    UseBattleMasterSkill(void);
