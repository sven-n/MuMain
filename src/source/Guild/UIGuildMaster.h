//////////////////////////////////////////////////////////////////////////
//  UIGuildMaster.h
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_UIGUILDMASTER_H__A1F8F21B_D41E_4688_A9E8_C5B29A6F3074__INCLUDED_)
#define AFX_UIGUILDMASTER_H__A1F8F21B_D41E_4688_A9E8_C5B29A6F3074__INCLUDED_

#pragma once

#include "UIWindows.h"

class CUIManager;
class CUIGuildInfo;
class CUIGuildMaster;

enum eCurrentMode { MODE_NONE, MODE_CREATE_GUILD, MODE_EDIT_GUILDMARK };
enum eCurrentStep { STEP_MAIN, STEP_CREATE_GUILDINFO, STEP_EDIT_GUILD_MARK, STEP_CONFIRM_GUILDINFO };

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
    BOOL IsValidGuildName(const wchar_t* szName);
    BOOL IsValidGuildMark();

    void StepPrev();
    void StepNext();
    void CloseMyPopup();

    void DoCreateGuildAction();
    void RenderCreateGuild();

    void DoCreateInfoAction();
    void RenderCreateInfo();

    void DoEditGuildMarkAction();
    void RenderEditGuildMark();

    void DoGuildMasterMainAction();
    void RenderGuildMasterMain();

public:
    void ReceiveGuildRelationShip(BYTE byRelationShipType, BYTE byRequestType, BYTE  byTargetUserIndexH, BYTE byTargetUserIndexL);

    virtual BOOL DoMouseAction();
    virtual void Render();
    void Open();
    bool IsOpen();
    virtual void Close();
};

#endif // !defined(AFX_UIGUILDMASTER_H__A1F8F21B_D41E_4688_A9E8_C5B29A6F3074__INCLUDED_)
