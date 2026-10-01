
#include "stdafx.h"
#include "UI/Dialogs/HelpWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/HelpPages.h"
#include "Audio/DSPlaySound.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// RenderTipTextList(1, 1, ...): the box's left edge is clamped to logical 0 and its top is 1.
constexpr float kBoxLeft = 0.f;
constexpr float kBoxTop = 1.f;
// Its text box is the widest line plus 2 units, inside 1 unit of padding and a 1-unit frame.
constexpr float kTextBoxSlackUnits = 2.f;
constexpr float kPaddingUnits = 1.f;
constexpr float kBorderUnits = 1.f;
// Each row advances 1.1 text heights; a "\n" row half of that.
constexpr float kRowAdvance = 1.1f;
constexpr float kHalfSpacerFraction = 0.5f;

bool SameTransform(const UI::Scaling::Transform& a, const UI::Scaling::Transform& b)
{
    return a.scaleX == b.scaleX && a.scaleY == b.scaleY && a.offsetX == b.offsetX && a.offsetY == b.offsetY &&
           a.typographyScale == b.typographyScale;
}

float MeasureLogicalWidth(const std::wstring& text, bool bold)
{
    g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
    return static_cast<float>(g_pRenderText->MeasureText(text.c_str(), static_cast<int>(text.size())).cx);
}
} // namespace

mu::ui::window::CHelpWindow::CHelpWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;

    m_iIndex = 0;
}

mu::ui::window::CHelpWindow::~CHelpWindow()
{
    Release();
}

bool mu::ui::window::CHelpWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_HELP, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void mu::ui::window::CHelpWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CHelpWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CHelpWindow::UpdateMouseEvent()
{
    return true;
}

bool mu::ui::window::CHelpWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_HELP))
    {
        if (IsPress(VK_F1) == true)
        {
            if (++m_iIndex >= UI::Help::PageCount)
            {
                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_HELP);
                PlayBuffer(SOUND_CLICK01);
            }

            return false;
        }

        if (IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_HELP);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CHelpWindow::Update()
{
    SyncRmlModel();
    return true;
}

bool mu::ui::window::CHelpWindow::Render()
{
    // Nothing native left: the page is an RmlUi document. Kept because CObject requires it.
    return true;
}

void mu::ui::window::CHelpWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "help_window",
                                                 [](Rml::DataModelConstructor& c, HelpWindowRmlModel& model)
                                                 {
                                                     c.Bind("panel_x", &model.panelX);
                                                     c.Bind("panel_y", &model.panelY);
                                                     c.Bind("content_width", &model.contentWidth);
                                                     c.Bind("padding_px", &model.paddingPx);
                                                     c.Bind("border_px", &model.borderPx);
                                                     c.Bind("text_px", &model.textPx);
                                                     c.Bind("bold_text_px", &model.boldTextPx);

                                                     auto line = c.RegisterStruct<HelpLineEntry>();
                                                     line.RegisterMember("text", &HelpLineEntry::text);
                                                     line.RegisterMember("heading", &HelpLineEntry::heading);
                                                     line.RegisterMember("half_spacer", &HelpLineEntry::halfSpacer);
                                                     line.RegisterMember("height_px", &HelpLineEntry::heightPx);
                                                     line.RegisterMember("gap_px", &HelpLineEntry::gapPx);
                                                     c.RegisterArray<std::vector<HelpLineEntry>>();
                                                     c.Bind("lines", &model.lines);
                                                 });

    if (modelCreated)
    {
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                      "Data/Interface/RmlUi/help_window.rml");
    }
}

void mu::ui::window::CHelpWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;
    m_BuiltPage = -1;

    BuildRmlUi();
}

void mu::ui::window::CHelpWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // The original drew this page over the location bar and the chat and system logs.
    const bool visible = IsVisible();
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, visible);
    if (!visible)
        return;

    RebuildPageModel(UI::Scaling::GetActiveTransform());
}

void mu::ui::window::CHelpWindow::RebuildPageModel(const UI::Scaling::Transform& transform)
{
    if (m_BuiltPage == m_iIndex && SameTransform(m_BuiltTransform, transform))
        return;

    m_BuiltPage = m_iIndex;
    m_BuiltTransform = transform;

    const std::vector<UI::Help::PageLine> page = UI::Help::BuildPage(m_iIndex);
    const float normalHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal));
    const float boldHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold));

    HelpWindowRmlModel& model = m_RmlBinder.GetModel();
    model.lines.clear();
    model.lines.reserve(page.size());

    float widestLine = 0.f;
    for (const UI::Help::PageLine& pageLine : page)
    {
        HelpLineEntry entry;
        entry.heading = pageLine.heading;
        entry.halfSpacer = pageLine.halfSpacer;

        const float rowHeight = (pageLine.heading ? boldHeight : normalHeight) * transform.scaleY;
        const float advance = rowHeight * kRowAdvance;
        if (pageLine.halfSpacer)
        {
            entry.heightPx = advance * kHalfSpacerFraction;
        }
        else
        {
            entry.text = StringUtils::WideToNarrow(pageLine.text.c_str());
            entry.heightPx = rowHeight;
            entry.gapPx = advance - rowHeight;
            widestLine = std::max(widestLine, MeasureLogicalWidth(pageLine.text, pageLine.heading));
        }
        model.lines.push_back(std::move(entry));
    }

    model.borderPx = kBorderUnits * transform.scaleX;
    model.paddingPx = kPaddingUnits * transform.scaleX;
    model.contentWidth = (widestLine + kTextBoxSlackUnits) * transform.scaleX;
    model.panelX = UI::Scaling::PositionX(transform, kBoxLeft) - model.borderPx;
    model.panelY = UI::Scaling::PositionY(transform, kBoxTop) - model.borderPx;
    model.textPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform);
    model.boldTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);

    for (const char* field :
         {"lines", "panel_x", "panel_y", "content_width", "padding_px", "border_px", "text_px", "bold_text_px"})
        m_RmlBinder.MarkDirty(field);
}

float mu::ui::window::CHelpWindow::GetLayerDepth()
{
    return 8.3f;
}

float mu::ui::window::CHelpWindow::GetKeyEventOrder()
{
    return 10.f;
}

void mu::ui::window::CHelpWindow::OpenningProcess()
{
    m_iIndex = UI::Help::KeyFunctionPage;
}

void mu::ui::window::CHelpWindow::ClosingProcess() {}

void mu::ui::window::CHelpWindow::AutoUpdateIndex()
{
    if (++m_iIndex >= UI::Help::PageCount)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_HELP);
    }
}
