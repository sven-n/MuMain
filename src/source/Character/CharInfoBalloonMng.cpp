
#include "stdafx.h"
#include "CharInfoBalloonMng.h"

#include "CharInfoBalloon.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Core/SceneUICoordinator.h"
#include <RmlUi/Core/ElementDocument.h>

// Replaces CUIMng's old `CCharInfoBalloonMng m_CharInfoBalloonMng;` member, same convention as
// g_CreditWin.
CCharInfoBalloonMng g_CharInfoBalloonMng;

namespace
{
// Key char_info_balloon.rml compares against; the theme's .rcss styles each (.name-<key>).
const char* NameStatusKey(BalloonNameStatus status)
{
    switch (status)
    {
    case BalloonNameStatus::BlockedCharacter:
        return "blocked-character";
    case BalloonNameStatus::BlockedItems:
        return "blocked-items";
    case BalloonNameStatus::Operator:
        return "operator";
    case BalloonNameStatus::Normal:
        break;
    }
    return "normal";
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
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_CHAR_INFO_BALLOON, this);
}

void CCharInfoBalloonMng::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "char_info_balloons",
        [](Rml::DataModelConstructor& c, BalloonListModel& model)
        {
            model.balloons.resize(kBalloonCount);

            // See CCharMakeWin::BuildRmlUi()'s comment on why this must re-run in full every
            // call, including from ReloadRmlTheme() -- no guard here.
            auto entry = c.RegisterStruct<BalloonEntry>();
            entry.RegisterMember("hidden", &BalloonEntry::hidden);
            entry.RegisterMember("screen_x", &BalloonEntry::screenX);
            entry.RegisterMember("screen_y", &BalloonEntry::screenY);
            entry.RegisterMember("name_status", &BalloonEntry::nameStatus);
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

void CCharInfoBalloonMng::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    // m_pRmlDoc's own visibility, not m_isInitialized -- Release() hides it directly without
    // going through m_isInitialized, and Render()'s own shouldHide correction bails out before
    // reaching that logic whenever !m_isInitialized, so neither can be trusted here.
    const bool wasVisible = m_pRmlDoc->IsVisible();

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi(); // shows unconditionally, same as Create() -- corrected back below if that's wrong
    if (!wasVisible && m_pRmlDoc)
        m_pRmlDoc->Hide();
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

    // The original drew the balloons before the scene's windows (CUIMng::Render()), so the
    // character-creation dialog, a message window and the system menu covered them while the
    // balloons stayed visible around them. The stacking table (RmlStackingOrder.cpp) keeps that
    // order: the balloon document sits under the scene windows' documents.
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, true);

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
        entry.nameStatus = NameStatusKey(balloon.GetNameStatus());
        entry.name = StringUtils::WideToNarrow(balloon.GetName());
        entry.guild = StringUtils::WideToNarrow(balloon.GetGuildText());
        entry.klass = StringUtils::WideToNarrow(balloon.GetClassText());
    }
    m_RmlBinder.MarkDirty("balloons");
}
