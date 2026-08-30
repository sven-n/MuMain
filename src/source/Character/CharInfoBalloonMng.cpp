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
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include <RmlUi/Core/ElementDocument.h>

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

    // See CLoginMainWin::PreRelease()'s identical comment -- this class isn't a CWin, so it has
    // no CUIMng::RemoveWinList() sweep to rely on; Release() is called explicitly at every
    // character-scene exit point instead (CUIMng::CreateLoginScene()/CUIMng::Release()), which is
    // exactly the right place to hide the document too.
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
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 캐릭터 정보 풍선들 렌더.
//*****************************************************************************
void CCharInfoBalloonMng::Render()
{
    if (!m_isInitialized)
        return;

    // Each balloon.Render() call recomputes its own live world->screen projection (position only
    // now -- see CCharInfoBalloon's header comment); SyncRmlModel() then pushes the result, plus
    // the text/color SetInfo() already cached, into the shared RmlUi array. This runs every
    // frame, during the normal legacy-2D-content recording phase -- strictly before
    // RmlUiRuntime's SetPreSubmitCallback fires later the same frame, so the position is always
    // fresh by the time RmlUi actually renders it.
    for (auto& balloon : m_charInfoBalloons)
        balloon.Render();

    SyncRmlModel();
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
        // Reproduces CSprite::Create's (59, 54) anchor offset -- the balloon's real art/text box
        // is bottom-center-anchored at the projected point (see CCharInfoBalloon::Render()'s own
        // comment), so the RmlUi element's top-left (real screen pixels, matching this engine's
        // Rml::Context sizing) needs that same offset subtracted.
        entry.screenX = balloon.GetXPos() - 59;
        entry.screenY = balloon.GetYPos() - 54;
        entry.nameColor = ColorToCssHex(balloon.GetNameColor());
        entry.name = StringUtils::WideToNarrow(balloon.GetName());
        entry.guild = StringUtils::WideToNarrow(balloon.GetGuildText());
        entry.klass = StringUtils::WideToNarrow(balloon.GetClassText());
    }
    m_RmlBinder.MarkDirty("balloons");
}
