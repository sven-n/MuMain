#pragma once

// The native 3D character a letter shows its sender as -- the one thing in this family that is not
// an RmlUi document. It composites after the main context through UI::RmlBridge::OverlayRender, so
// it stands on its window rather than under it; UI::Social::PhotoViewerControl drives its gestures
// from the document, because a press over a panel never reaches the legacy mouse globals.

#include "UI/Social/SocialWindowCore.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"

const int UIPHOTOVIEWER_CANCONTROL = 1;

class CUIPhotoViewer : public CUIControl
{
public:
    CUIPhotoViewer();
    virtual ~CUIPhotoViewer();

    virtual CHARACTER* GetPhotoChar()
    {
        return &m_PhotoChar;
    }

    virtual void Init(int iInitType);
    virtual void SetClass(CLASS_TYPE byClass);
    virtual void SetEquipmentPacket(BYTE* pbyEquip);
    virtual void CopyPlayer();
    virtual void SetAngle(float fDegree);
    virtual void SetZoom(float fZoom);
    virtual void SetAutoupdatePlayer(BOOL bFlag)
    {
        m_bUpdatePlayer = bFlag;
    }

    virtual void SetAnimation(int iAnimationType);
    virtual void ChangeAnimation(int iMoveDir = 0);

    void SetID(const wchar_t* pszID);
    const wchar_t* GetID()
    {
        return m_PhotoChar.ID;
    }
    float GetCurrentAngle()
    {
        return m_fCurrentAngle;
    }
    int GetCurrentAction()
    {
        return m_iSettingAnimation;
    }
    float GetCurrentZoom()
    {
        return m_fCurrentZoom;
    }
    void SetWebzenMail(BOOL bFlag)
    {
        m_bIsWebzenMail = bFlag;
    }

    virtual BOOL DoMouseAction();
    virtual void Render();

    // The well this stands in, in the owner's own coordinates. Until its document has been placed
    // there is no well yet, and Render() draws nothing rather than at the default rect.
    void SetSlot(int iPos_x, int iPos_y, int iWidth, int iHeight);

    // Driven by UI::Social::PhotoViewerControl, which owns these gestures while the viewer stands
    // behind an RmlUi document and the native press never arrives. See its header.
    void TurnBy(float degrees);
    void ResetView();
    void ToggleHelp();

protected:
    void RenderPhotoCharacter();
    void ShowHelpText();
    int SetPhotoPose(int iCurrentAni, int iMoveDir = 0);

protected:
    CHARACTER m_PhotoChar;
    OBJECT m_PhotoHelper;
    float m_fPhotoHelperScale;
    BOOL m_bIsInitialized;
    BOOL m_bHelpEnable;
    BOOL m_bUpdatePlayer;
    BOOL m_bActionRepeatCheck;
    int m_iShowType;
    int m_iCurrentAnimation;
    int m_iSettingAnimation;
    int m_iCurrentFrame;
    float m_fSettingAngle;
    float m_fCurrentAngle;
    float m_fRotateClickPos_x;
    float m_fSettingZoom;
    float m_fCurrentZoom;
    BOOL m_bIsWebzenMail;
    bool m_bHasSlot = false;

public:
    void SetShowType(int Stype)
    {
        m_iShowType = Stype;
    }
};
