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
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

// RmlUi migration: unlike every other window ported so far, this isn't a CWin at all -- it's a
// small, independently-driven overlay (CUIMng::Render() calls Render() directly, once per frame,
// ahead of the CWin list) rendering up to 5 floating name/guild/level tags, one per 3D character
// standing in the character-select scene. Each tag's screen position must track its character's
// live 3D->screen projection every single frame (not just on resize, unlike every fixed-position
// panel ported before this), so this uses RmlUi's array/data-for binding (a first for this
// migration) instead of one scalar field per balloon: one Rml::DataModel array of 5 structs,
// re-synced every frame from the 5 CCharInfoBalloon members' own per-frame projection.
// Composites correctly over the character models' own 3D rendering because CUIMng::Render() (and
// therefore this class's per-frame sync) runs during the normal legacy-2D-content recording
// phase, strictly before RmlUiRuntime's SetPreSubmitCallback fires later the same frame -- see
// docs/rmlui-ui-system/README.md's frame-lifecycle section.
class CCharInfoBalloonMng
{
protected:
    static constexpr std::size_t kBalloonCount = 5;
    std::array<CCharInfoBalloon, kBalloonCount> m_charInfoBalloons{};
    bool m_isInitialized{false};

public:
    CCharInfoBalloonMng() = default;
    virtual ~CCharInfoBalloonMng();

    void Release();
    void Create();
    void Render();
    void UpdateDisplay();

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

#endif // !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
