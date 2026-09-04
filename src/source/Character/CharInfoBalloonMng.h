//*****************************************************************************
// File: CharInfoBalloonMng.h
//
// Desc: interface for the CCharInfoBalloonMng class.
//		 캐릭터 정보 풍선 관리 클래스.(캐릭터 선택씬에서 쓰임)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
#define AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_

#pragma once

#include <array>
#include <vector>

#include "CharInfoBalloon.h"
#include "UI/NewUI/NewUIBase.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

// RmlUi migration: unlike every other window ported so far, this isn't a CWin at all -- it's a
// small, independently-driven overlay rendering up to 5 floating name/guild/level tags, one per
// 3D character standing in the character-select scene. Each tag's screen position must track its
// character's live 3D->screen projection every single frame (not just on resize, unlike every
// fixed-position panel ported before this), so this uses RmlUi's array/data-for binding (a first
// for this migration) instead of one scalar field per balloon: one Rml::DataModel array of 5
// structs, re-synced every frame from the 5 CCharInfoBalloon members' own per-frame projection.
// Composites correctly over the character models' own 3D rendering because CNewUIManager::Render()
// (and therefore this class's per-frame sync) runs during the normal legacy-2D-content recording
// phase, strictly before RmlUiRuntime's SetPreSubmitCallback fires later the same frame -- see
// docs/rmlui-ui-system/README.md's frame-lifecycle section.
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md), Phase 3 -- was never a CWin (CUIMng
// drove it via a direct, hardcoded call rather than through any list), but registers with
// CUIMng::GetNewStyleMng() the same way every migrated CWin does, so Phase 4 can delete CUIMng
// with zero hardcoded per-window calls left. Most of the INewUIBase surface below is thin/inert
// for this class -- it has no interaction and no shown-vs-active distinction to make (see each
// override's own comment) -- registering it is about uniformity for Phase 4, not new behavior.
class CCharInfoBalloonMng : public SEASON3B::CNewUIObj
{
protected:
    static constexpr std::size_t kBalloonCount = 5;
    std::array<CCharInfoBalloon, kBalloonCount> m_charInfoBalloons{};
    bool m_isInitialized{false};

public:
    CCharInfoBalloonMng() = default;
    ~CCharInfoBalloonMng() override;

    void Release();
    void Create();
    void UpdateDisplay();

    // SEASON3B::INewUIBase
    bool Render() override;
    // No interaction of any kind -- never consumes.
    bool UpdateMouseEvent() override
    {
        return true;
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Nothing drives a per-frame update distinct from UpdateDisplay()'s event-driven refresh, and
    // this class has no shown-vs-active distinction to make (unlike CLoginWin, the one window in
    // this phase that actually needs CNewUIObj's shown/active split).
    bool Update() override
    {
        return true;
    }
    // m_isInitialized (Create()/Release()'s own gate every method already checks), not the base
    // CNewUIObj::m_bRender flag -- nothing meaningfully toggles "shown" for this manager beyond
    // init/release; the per-balloon and whole-document visibility is decided inside Render()
    // itself (see its own shouldHide comment).
    bool IsVisible() const override
    {
        return m_isInitialized;
    }
    // Below CCharSelMainWin's 15.0f -- no other migrated CHARACTER_SCENE window needs to stack
    // against this directly (Render()'s own shouldHide check, not depth, governs its visibility
    // relative to CCharMakeWin/CMsgWin/CSysMenuWin -- see Render()'s comment for why).
    float GetLayerDepth() override
    {
        return 10.0f;
    }

private:
    struct BalloonEntry
    {
        bool hidden = true;
        int screenX = 0;
        int screenY = 0;
        // Non-empty default -- an empty string here (before the first SyncRmlModel() call)
        // makes RmlUi try to parse "color: ;" for data-style-color, logging a syntax warning.
        Rml::String nameColor = "#ffffff";
        Rml::String name;
        Rml::String guild;
        Rml::String klass;
    };
    struct BalloonListModel
    {
        std::vector<BalloonEntry> balloons;
    };
    RmlModelBinder<BalloonListModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    void SyncRmlModel();
};

// Replaces CUIMng's old `CCharInfoBalloonMng m_CharInfoBalloonMng;` member, same convention as
// g_CreditWin.
extern CCharInfoBalloonMng g_CharInfoBalloonMng;

#endif // !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
