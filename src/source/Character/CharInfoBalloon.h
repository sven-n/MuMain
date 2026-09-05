//*****************************************************************************
// File: CharInfoBalloon.h
//*****************************************************************************

#if !defined(AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_)
#define AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_

#pragma once

#include "Render/Sprites/Sprite.h"

#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"

// RmlUi migration: this class stays a CSprite purely for its geometry (GetWidth/GetHeight, the
// anchor-offset semantics CSprite::Create's trailing args establish) -- Render() no longer draws
// anything itself. It still does the per-frame CameraProjection::WorldToScreen() + SetPosition()
// math every call (matching the original exactly, since this position must track the character's
// live 3D position/camera every frame, not just once), so CCharInfoBalloonMng can read the result
// back via GetXPos()/GetYPos() and push it into the shared RmlUi balloon-array model. See
// docs/rmlui-ui-system/README.md for how a real-pixel-positioned overlay composites correctly
// over 3D content in this renderer.
class CCharInfoBalloon : public CSprite
{
protected:
    CHARACTER* m_pCharInfo;
    DWORD m_dwNameColor;
    wchar_t m_szName[64];
    wchar_t m_szGuild[64];
    wchar_t m_szClass[64];

public:
    CCharInfoBalloon();
    virtual ~CCharInfoBalloon();

    void Create(CHARACTER* pCharInfo);
    // cppcheck-suppress duplInheritedMember
    void Render();

    void SetInfo();

    // Read-only accessors for CCharInfoBalloonMng::SyncRmlModel() -- the cached text/color fields
    // SetInfo() computes, unchanged in shape from before, just no longer consumed by a direct
    // g_pRenderText call inside this class.
    const wchar_t* GetName() const { return m_szName; }
    const wchar_t* GetGuildText() const { return m_szGuild; }
    const wchar_t* GetClassText() const { return m_szClass; }
    DWORD GetNameColor() const { return m_dwNameColor; }

private:
    // Re-runs SetInfo() on locale change so the cached guild / class
    // strings displayed over the character flip to the new language
    // without waiting for the next character refresh.
    static void OnLocaleChanged(void* ctx) noexcept;
};

#endif // !defined(AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_)
