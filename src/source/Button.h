#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include "Sprite.h"

#define	BTN_UP				0
#define	BTN_DOWN			1
#define	BTN_ACTIVE			2
#define	BTN_DISABLE			3
#define	BTN_UP_CHECK		4
#define	BTN_DOWN_CHECK		5
#define	BTN_ACTIVE_CHECK	6
#define	BTN_DISABLE_CHECK	7
#define	BTN_IMG_MAX			8

class CButton : public CSprite
{
protected:
    static	CButton* m_pBtnHeld;

    bool	m_bEnable;
    bool	m_bActive;
    bool	m_bClick;
    bool	m_bCheck;

    std::array<int, BTN_IMG_MAX> m_imageFrames{};
    std::wstring m_text;
    mutable std::vector<wchar_t> m_textBuffer;
    std::array<DWORD, BTN_IMG_MAX> m_textColors{};
    std::size_t m_textColorCount{0};
    DWORD	m_textColor{0};
    float	m_fTextAddYPos{0.0f};

public:
    CButton();
    virtual ~CButton();

    void Release();
    void Create(int nWidth, int nHeight, int nTexID, int nMaxFrame = 1, int nDownFrame = -1, int nActiveFrame = -1, int nDisableFrame = -1, int nCheckUpFrame = -1, int nCheckDownFrame = -1, int nCheckActiveFrame = -1, int nCheckDisableFrame = -1);
    void Update();
    void Render();
    void Show(bool bShow = true);
    BOOL CursorInObject();
    void SetEnable(bool bEnable = true) { m_bEnable = bEnable; }
    bool IsEnable() { return m_bEnable; }
    void SetActive(bool bActive = true) { m_bActive = bActive; }
    bool IsClick() { return m_bClick; }
    void SetCheck(bool bCheck = true) { m_bCheck = bCheck; }
    bool IsCheck() { return m_bCheck; }
    void SetText(const wchar_t* pszText, DWORD* adwColor);
    wchar_t* GetText() const;

protected:
    void ReleaseText();
    void ApplyVisualState(int frameIndex);
    void ApplyTextColorForState(int colorIndex);
    bool HasCheckVisuals() const;
    int ResolveColorIndex(int requestedIndex) const;
};