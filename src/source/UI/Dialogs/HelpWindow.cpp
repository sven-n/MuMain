
#include "stdafx.h"
#include "UI/Dialogs/HelpWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/HelpPages.h"
#include "Audio/DSPlaySound.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// RenderTipTextList()'s text box is the widest line plus 2 units.
constexpr float kTextBoxSlackUnits = 2.f;

float MeasureLogicalWidth(const std::wstring& text, bool bold)
{
    g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
    return static_cast<float>(g_pRenderText->MeasureText(text.c_str(), static_cast<int>(text.size())).cx);
}
} // namespace

mu::ui::window::CHelpWindow::CHelpWindow()
{
    m_pNewUIMng = NULL;

    m_iIndex = 0;
}

mu::ui::window::CHelpWindow::~CHelpWindow()
{
    Release();
}

bool mu::ui::window::CHelpWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_HELP, this);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CHelpWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
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

void mu::ui::window::CHelpWindow::BindRmlModel(Rml::DataModelConstructor& c, HelpWindowRmlModel& model)
{
    c.Bind("content_width", &model.contentWidth);
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("line_height", &model.lineHeight);
    c.Bind("bold_line_height", &model.boldLineHeight);

    auto line = c.RegisterStruct<HelpLineEntry>();
    line.RegisterMember("text", &HelpLineEntry::text);
    line.RegisterMember("heading", &HelpLineEntry::heading);
    line.RegisterMember("half_spacer", &HelpLineEntry::halfSpacer);
    c.RegisterArray<std::vector<HelpLineEntry>>();
    c.Bind("lines", &model.lines);
}

void mu::ui::window::CHelpWindow::OnRmlReloaded()
{
    m_BuiltPage = -1;
}

void mu::ui::window::CHelpWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CHelpWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // The original drew this page over the location bar and the chat and system logs.
    const bool visible = IsVisible();
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), visible);
    if (!visible)
        return;

    RebuildPageModel();
}

void mu::ui::window::CHelpWindow::RebuildPageModel()
{
    const float textPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal);
    if (m_BuiltPage == m_iIndex && m_BuiltTextPx == textPx)
        return;

    m_BuiltPage = m_iIndex;
    m_BuiltTextPx = textPx;

    const std::vector<UI::Help::PageLine> page = UI::Help::BuildPage(m_iIndex);

    HelpWindowRmlModel& model = m_RmlView.GetModel();
    model.lines.clear();
    model.lines.reserve(page.size());

    float widestLine = 0.f;
    for (const UI::Help::PageLine& pageLine : page)
    {
        HelpLineEntry entry;
        entry.heading = pageLine.heading;
        entry.halfSpacer = pageLine.halfSpacer;
        if (!pageLine.halfSpacer)
        {
            entry.text = StringUtils::WideToNarrow(pageLine.text.c_str());
            widestLine = std::max(widestLine, MeasureLogicalWidth(pageLine.text, pageLine.heading));
        }
        model.lines.push_back(std::move(entry));
    }

    model.contentWidth = widestLine + kTextBoxSlackUnits;
    model.textPx = textPx;
    model.boldTextPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    model.lineHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal));
    model.boldLineHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold));

    for (const char* field : {"lines", "content_width", "text_px", "bold_text_px", "line_height", "bold_line_height"})
        m_RmlView.MarkDirty(field);
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
