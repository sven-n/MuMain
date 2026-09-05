//*****************************************************************************
// File: CharInfoBalloonMng.cpp
//
// Desc: implementation of the CCharInfoBalloonMng class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "CharInfoBalloonMng.h"

#include "CharInfoBalloon.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/SysMenuWin.h"
#include "CharMakeWin.h"
#include <RmlUi/Core/ElementDocument.h>

// Replaces CUIMng's old `CCharInfoBalloonMng m_CharInfoBalloonMng;` member, same convention as
// g_CreditWin.
CCharInfoBalloonMng g_CharInfoBalloonMng;

namespace
{
    // ARGB(a,r,g,b)-packed DWORD (this engine's convention, UI/Widgets/UIBaseDef.h) -> a CSS hex
    // color string RmlUi's data-style-color can bind directly. Alpha is dropped -- every color
    // CCharInfoBalloon::GetNameColor() ever returns is fully opaque (see ResolveNameColor() in
    // CharInfoBalloon.cpp).
    Rml::String ColorToCssHex(DWORD color)
    {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x",
            (unsigned)((color >> 16) & 0xFF),
            (unsigned)((color >> 8) & 0xFF),
            (unsigned)(color & 0xFF));
        return Rml::String(buf);
    }
}

CCharInfoBalloonMng::~CCharInfoBalloonMng()
{
    Release();
}

void CCharInfoBalloonMng::Release()
{
    if (!m_isInitialized)
        return;

    m_isInitialized = false;

    // See CLoginMainWin::PreRelease()'s identical comment -- this class isn't a CWin, so it never
    // had any shared-list sweep to rely on; Release() is called explicitly at every character-scene
    // exit point instead (CSceneUICoordinator::CreateLoginScene()/CreateMainScene()/Release()),
    // which is exactly the right place to hide the document too.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 캐릭터 정보 풍선 매니저 생성.
//			   (캐릭터 선택씬에서 쓰임. 풍선 5개 생성.)
//*****************************************************************************
void CCharInfoBalloonMng::Create()
{
    for (std::size_t i = 0; i < kBalloonCount; ++i)
        m_charInfoBalloons[i].Create(&CharactersClient[i]);

    m_isInitialized = true;

    // RmlUi migration -- guarded the same way every other migrated window's Create() is
    // (re-entrant on resolution change), so the document/model/array size are set up once, ever.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "char_info_balloons",
            [](Rml::DataModelConstructor& c, BalloonListModel& model)
            {
                model.balloons.resize(kBalloonCount);

                auto entry = c.RegisterStruct<BalloonEntry>();
                entry.RegisterMember("hidden", &BalloonEntry::hidden);
                entry.RegisterMember("screen_x", &BalloonEntry::screenX);
                entry.RegisterMember("screen_y", &BalloonEntry::screenY);
                entry.RegisterMember("name_color", &BalloonEntry::nameColor);
                entry.RegisterMember("name", &BalloonEntry::name);
                entry.RegisterMember("guild", &BalloonEntry::guild);
                entry.RegisterMember("klass", &BalloonEntry::klass);
                c.RegisterArray<std::vector<BalloonEntry>>();

                c.Bind("balloons", &model.balloons);
            });

        if (modelCreated)
        {
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/char_info_balloon.rml");
            if (m_pRmlDoc)
                m_pRmlDoc->Show();
        }
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_CHAR_INFO_BALLOON, this);
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 캐릭터 정보 풍선들 렌더.
//*****************************************************************************
bool CCharInfoBalloonMng::Render()
{
    if (!m_isInitialized)
        return true;

    // Each balloon.Render() call recomputes its own live world->screen projection (position only
    // now -- see CCharInfoBalloon's header comment); SyncRmlModel() then pushes the result, plus
    // the text/color SetInfo() already cached, into the shared RmlUi array. This runs every
    // frame, during the normal legacy-2D-content recording phase -- strictly before
    // RmlUiRuntime's SetPreSubmitCallback fires later the same frame, so the position is always
    // fresh by the time RmlUi actually renders it.
    for (auto& balloon : m_charInfoBalloons)
        balloon.Render();

    // Before this migration, this manager's Render() drew the balloons as ordinary legacy 2D
    // content, in the same pass and *before* CSceneUICoordinator::Render()'s CWin list (CCharMakeWin, CMsgWin,
    // CSysMenuWin) -- so those windows' own legacy drawing correctly painted over the balloons
    // whenever they were open. RmlUi renders unconditionally last in the frame now, so that
    // relationship inverted: with nothing telling it otherwise, a balloon would paint on top of
    // the character-creation dialog, a CMsgWin confirmation prompt, or the system menu instead of
    // being covered by them, since all three are themselves drawn earlier in the frame (their own
    // panels are RmlUi too, and either way none has any relationship to *when* RmlUi's pass runs).
    // Restore the original visual hierarchy explicitly: hide the whole balloon document while any
    // of the three is shown, since all of them used to legitimately cover it. g_SysMenuWin added
    // when its own CUIMng/CNewUIManager-merger migration surfaced this exact symptom (the menu
    // painting behind the balloon) -- this check needs the same treatment for every future
    // CHARACTER_SCENE-relevant migration too, same as the IsCursorOnUI() fold-in
    // (docs/newui-legacy-merger.md). This registered adapter's own GetLayerDepth() can't replace
    // this check even now that every window involved is a CObject: that depth-sort only orders
    // *this manager's own* dispatch, not RmlUi's separate, always-last compositor pass, so a
    // permanent explicit toggle is still the only fix (same reasoning as CLoginWin's own
    // credits/sysmenu render-side gates -- see its Render()'s comment).
    const bool shouldHide = g_CharMakeWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible();
    if (m_pRmlDoc)
    {
        if (shouldHide) m_pRmlDoc->Hide();
        else            m_pRmlDoc->Show();
    }
    if (shouldHide)
        return true;

    SyncRmlModel();
    return true;
}

//*****************************************************************************
// 함수 이름 : UpdateDisplay()
// 함수 설명 : 캐릭터 정보를 업데이트.
//*****************************************************************************
void CCharInfoBalloonMng::UpdateDisplay()
{
    if (!m_isInitialized)
        return;

    for (auto& balloon : m_charInfoBalloons)
        balloon.SetInfo();
}

void CCharInfoBalloonMng::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& balloons = m_RmlBinder.GetModel().balloons;
    for (std::size_t i = 0; i < kBalloonCount; ++i)
    {
        CCharInfoBalloon& balloon = m_charInfoBalloons[i];
        BalloonEntry& entry = balloons[i];

        entry.hidden = !balloon.IsShow();
        // CSprite::SetPosition() already subtracts the (59, 54) anchor offset internally when
        // computing what GetXPos()/GetYPos() return (Sprite.cpp: m_aScrCoord[LT].fX = nXCoord -
        // m_fDatumX) -- GetXPos()/GetYPos() already ARE the anchor-adjusted top-left corner, the
        // same thing an RmlUi element's left/top needs. Subtracting the offset again here (an
        // earlier version of this code did) double-applies it, shifting the balloon uniformly
        // off to the upper-left of every character instead of centered above it.
        entry.screenX = balloon.GetXPos();
        entry.screenY = balloon.GetYPos();
        entry.nameColor = ColorToCssHex(balloon.GetNameColor());
        entry.name = StringUtils::WideToNarrow(balloon.GetName());
        entry.guild = StringUtils::WideToNarrow(balloon.GetGuildText());
        entry.klass = StringUtils::WideToNarrow(balloon.GetClassText());
    }
    m_RmlBinder.MarkDirty("balloons");
}
