
#include "stdafx.h"

#include "UI/HUD/SlideWindow.h"
#include "UI/Core/WindowManager.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <cstdio>

mu::ui::window::CSlideWindow::CSlideWindow()
{
    m_pNewUIMng = NULL;
    m_pSlideMgr = NULL;
}

mu::ui::window::CSlideWindow::~CSlideWindow()
{
    Release();
}

bool mu::ui::window::CSlideWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SLIDEWINDOW, this);
    m_pSlideMgr = new UI::HUD::SlideTicker;
    std::wstring strFileName = L"Data\\Local\\" + g_strSelectedML + L"\\Slide_" + g_strSelectedML + L".bmd";
    m_pSlideMgr->OpenSlideTextFile(strFileName.c_str());
    BuildRmlUi();

    return true;
}

void mu::ui::window::CSlideWindow::BindRmlModel(Rml::DataModelConstructor& c, SlideNoticeRmlModel& model)
{
    c.Bind("root_scale", &model.rootScale);
    c.Bind("text_px", &model.textPx);
    c.Bind("shown", &model.shown);
    c.Bind("text_x", &model.textX);
    c.Bind("text_top", &model.textTop);
    c.Bind("band_top", &model.bandTop);
    c.Bind("band_height", &model.bandHeight);
    c.Bind("band_color", &model.bandColor);
    c.Bind("text_color", &model.textColor);
    c.Bind("text", &model.text);
}

void mu::ui::window::CSlideWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CSlideWindow::SyncRmlModel()
{
    if (!m_RmlView.Document() || !m_pSlideMgr)
        return;

    const UI::HUD::SlideDisplay d = m_pSlideMgr->Display();
    auto& model = m_RmlView.GetModel();

    // The ticker runs in the screen's W/640 x H/480 stretch, scaled by its height.
    const auto transform = UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::rootScale, "root_scale", transform.scaleY);
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::shown, "shown", d.shown);
    if (d.shown)
    {
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::textX, "text_x", d.x);
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::textTop, "text_top", static_cast<float>(d.y));
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::bandTop, "band_top", static_cast<float>(d.y - 3));
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::bandHeight, "band_height", static_cast<float>(d.bandHeight));
        // RmlUi's rgba() alpha is 0-255, not 0-1, so both colours are built whole here rather
        // than assembled in the document.
        char band[40] = {};
        std::snprintf(band, sizeof(band), "rgba(0,0,0,%u)", static_cast<unsigned>(d.alpha));
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::bandColor, "band_color", Rml::String(band));

        char rgb[48] = {};
        std::snprintf(rgb, sizeof(rgb), "rgba(%u,%u,%u,%u)", d.colorRgb & 0xFF,
                      (d.colorRgb >> 8) & 0xFF, (d.colorRgb >> 16) & 0xFF,
                      static_cast<unsigned>(d.alpha));
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::textColor, "text_color", Rml::String(rgb));
        SyncField(m_RmlView.Binder(), &SlideNoticeRmlModel::text, "text",
                  StringUtils::WideToNarrow(d.text ? d.text : L""));
    }

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), true);
}

void mu::ui::window::CSlideWindow::Release()
{
    m_RmlView.Release();
    SAFE_DELETE(m_pSlideMgr);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CSlideWindow::UpdateMouseEvent()
{
    return true;
}
bool mu::ui::window::CSlideWindow::UpdateKeyEvent()
{
    return true;
}
bool mu::ui::window::CSlideWindow::Update()
{
    m_pSlideMgr->ManageSlide();
    SyncRmlModel();

    return true;
}
bool mu::ui::window::CSlideWindow::Render()
{
    // slide_notice.rml draws the band and the text; Display() in Update() advanced them.
    return true;
}

float mu::ui::window::CSlideWindow::GetLayerDepth()
{
    return 1.91f;
}