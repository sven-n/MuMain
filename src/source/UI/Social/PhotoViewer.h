#pragma once

// The native 3D character a letter shows its sender as. It is drawn into a render target the
// letter's document shows as an image (UI::RmlBridge::RenderTarget), so it stands at that document's
// own depth -- under whatever covers the window, beneath the help drawn over it. UI::Social::
// PhotoViewerControl sizes the target, drives the gestures and mirrors this viewer's state into the
// document, because a press over a panel never reaches the legacy mouse globals.

#include "UI/Social/SocialWindowCore.h"
#include "UI/RmlBridge/RmlRenderTarget.h"
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

    // What the letter's #photo_image shows; PhotoViewerControl sizes it to that element.
    UI::RmlBridge::RenderTarget& Target() { return m_Target; }
    bool IsHelpShown() const { return m_bHelpEnable != FALSE; }
    bool IsWebzenMail() const { return m_bIsWebzenMail != FALSE; }

    // Where the well is, in the owner's own coordinates -- for the wheel, which never passes
    // through RmlUi and is still read here natively.
    void SetSlot(int iPos_x, int iPos_y, int iWidth, int iHeight);

    // Driven by UI::Social::PhotoViewerControl, which owns these gestures while the viewer stands
    // behind an RmlUi document and the native press never arrives. See its header.
    void TurnBy(float degrees);
    void ResetView();
    void ToggleHelp();

protected:
    // The target's drawer: one frame of the character, framed for `width` x `height`.
    void RenderInto(std::uint32_t width, std::uint32_t height);
    void RenderPhotoCharacter(float aspect);
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
    // Last, so it is destroyed first and its drawer never runs against a half-destroyed viewer.
    UI::RmlBridge::RenderTarget m_Target;

public:
    void SetShowType(int Stype)
    {
        m_iShowType = Stype;
    }
};
