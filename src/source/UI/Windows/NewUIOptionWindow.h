// NewUIOptionWindow.h: interface for the CNewUIOptionWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_)
#define AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_

#pragma once

#include "NewUIManager.h"
#include "NewUIMyInventory.h"

namespace SEASON3B
{
class CNewUIOptionWindow : public CNewUIObj
{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
public:
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
    enum IMAGE_LIST
    {
        IMAGE_OPTION_FRAME_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
        IMAGE_OPTION_BTN_CLOSE = CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_CLOSE,
        IMAGE_OPTION_FRAME_DOWN = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,

        IMAGE_OPTION_FRAME_UP = BITMAP_OPTION_BEGIN,
        IMAGE_OPTION_FRAME_LEFT,
        IMAGE_OPTION_FRAME_RIGHT,
        IMAGE_OPTION_LINE,
        IMAGE_OPTION_POINT,
        IMAGE_OPTION_BTN_CHECK,
        IMAGE_OPTION_EFFECT_BACK,
        IMAGE_OPTION_EFFECT_COLOR,
        IMAGE_OPTION_VOLUME_BACK,
        IMAGE_OPTION_VOLUME_COLOR,
    };

public:
    CNewUIOptionWindow();
    virtual ~CNewUIOptionWindow();

    bool Create(CNewUIManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 10.5f
    float GetKeyEventOrder(); // 10.f;

    void OpenningProcess();
    void ClosingProcess();

    void SetAutoAttack(bool bAuto);
    bool IsAutoAttack();
    void SetWhisperSound(bool bSound);
    bool IsWhisperSound();
    void SetSlideHelp(bool bHelp);
    bool IsSlideHelp();
    void SetVolumeLevel(int iVolume);
    int GetVolumeLevel();
    // Story 5.4.1: BGM/SFX volume accessors (programmatic — UI slider deferred to future story)
    void SetBGMVolumeLevel(int iVolume);
    int GetBGMVolumeLevel();
    void SetSFXVolumeLevel(int iVolume);
    int GetSFXVolumeLevel();
    void SetRenderLevel(int iRender);
    int GetRenderLevel();
    void SetRenderAllEffects(bool bRenderAllEffects);
    bool GetRenderAllEffects();

private:
    void LoadImages();
    void UnloadImages();

    void SetButtonInfo();

    void RenderFrame();
    void RenderContents();
    void RenderButtons();

private:
    CNewUIManager* m_pNewUIMng;
    POINT m_Pos;

    CNewUIButton m_BtnClose;

    bool m_bAutoAttack;    // �ڵ� ����
    bool m_bWhisperSound;  // �Ӹ� �˸���
    bool m_bSlideHelp;     // �����̵� ����
    int m_iVolumeLevel;    // SFX volume level (0-10)
    int m_iBGMVolumeLevel; // BGM volume level (0-10) — Story 5.4.1
    int m_iRenderLevel;    // effect render level
    bool m_bRenderAllEffects;
};
} // namespace SEASON3B

#endif // !defined(AFX_NEWUIOPTIONWINDOW_H__1469FA1D_7C15_4AFE_AD6E_59C303E72BC0__INCLUDED_)
