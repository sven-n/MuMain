#include "stdafx.h"
#include "Core/Text/TextLineWrap.h"
#include "UI/HUD/Notices.h"

#include "App/Platform/Windows/Winmain.h"    // g_hFontBold
#include "Render/Textures/ZzzOpenglUtil.h" // EnableAlphaTest
#include "UI/Core/WindowSystem.h"        // g_pNewUISystem
#include "Engine/AI/ZzzAI.h"             // FPS_ANIMATION_FACTOR
#include "Engine/Object/ZzzInterface.h"  // CutText
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/HUD/NoticesRmlModel.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

namespace
{
    constexpr int MAX_NOTICE = 6;
    constexpr int NOTICE_LIFETIME = 300;
    constexpr int NOTICE_TEXT_MAX = 256;

    struct Notice
    {
        wchar_t Text[NOTICE_TEXT_MAX];
        int     LifeTime;
        BYTE    Color;
    };

    int    s_count = 0;
    // float, not int -- FPS_ANIMATION_FACTOR is a sub-1.0 delta at high FPS; int truncation would
    // swallow it (same root cause as CCreditWin's alpha-fade bug).
    float  s_time = NOTICE_LIFETIME;
    float  s_blinkPhase = 0.f;
    Notice s_notices[MAX_NOTICE];

    void BindModel(Rml::DataModelConstructor& c, UI::Notices::NoticesRmlModel& model)
    {
        c.Bind("row_width", &model.rowWidth);
        c.Bind("text_px", &model.textPx);
        c.Bind("line_height_px", &model.lineHeightPx);
        auto line = c.RegisterStruct<UI::Notices::NoticeLineEntry>();
        line.RegisterMember("text", &UI::Notices::NoticeLineEntry::text);
        line.RegisterMember("kind", &UI::Notices::NoticeLineEntry::kind);
        line.RegisterMember("top", &UI::Notices::NoticeLineEntry::top);
        c.RegisterArray<std::vector<UI::Notices::NoticeLineEntry>>();
        c.Bind("lines", &model.lines);
    }

    // The notices in RmlUi (notices.rml): one main-context document above every other document but
    // the tooltip (the original drew the notices after every window). Render() fills it; Move(),
    // which runs once per frame before the scene draws, hides it when the scene stopped calling
    // Render() (the loading scene draws no notices).
    UI::RmlBridge::ThemedView<UI::Notices::NoticesRmlModel> s_view{"notices", BindModel, {{"Data/Interface/RmlUi/notices.rml"}}};
    bool s_renderedThisFrame = false;

    // Shift the buffer up by one when it is full so the newest line fits.
    void Scroll()
    {
        if (s_count > MAX_NOTICE - 1)
        {
            s_count = MAX_NOTICE - 1;
            for (int i = 1; i < MAX_NOTICE; i++)
            {
                s_notices[i - 1].Color = s_notices[i].Color;
                wcscpy(s_notices[i - 1].Text, s_notices[i].Text);
            }
        }
    }
}

namespace UI::Notices
{
    void CopyText(wchar_t (&destination)[NOTICE_TEXT_MAX], const wchar_t* source)
    {
        wcsncpy_s(destination, source, _TRUNCATE);
    }

    void Clear()
    {
        memset(s_notices, 0, sizeof(s_notices));
    }

    void Create(const wchar_t* text, int color)
    {
        g_pRenderText->SetFont(g_hFontBold);
        const SIZE size = g_pRenderText->MeasureText(text, lstrlen(text));

        Scroll();
        s_notices[s_count].Color = color;
        if (size.cx < NOTICE_TEXT_MAX)
        {
            CopyText(s_notices[s_count++].Text, text);
        }
        else
        {
            wchar_t topText[NOTICE_TEXT_MAX] = { 0 };
            wchar_t bottomText[NOTICE_TEXT_MAX] = { 0 };
            CutText(text, topText, bottomText, NOTICE_TEXT_MAX);
            CopyText(s_notices[s_count++].Text, topText);
            Scroll();
            s_notices[s_count].Color = color;
            CopyText(s_notices[s_count++].Text, bottomText);
        }
        s_time = NOTICE_LIFETIME;
    }

    void Move()
    {
        if (!s_renderedThisFrame)
            UI::RmlBridge::SyncDocumentVisibility(s_view.Document(), false);
        s_renderedThisFrame = false;

        s_time -= FPS_ANIMATION_FACTOR;
        if (s_time <= 0)
        {
            s_time = NOTICE_LIFETIME;
            Create(L"", 0);
        }
    }

    namespace
    {

    // The original's per-line draw: RenderText(320, 300 + i * 13) centred, bold, on a
    // half-transparent black box sized to the text; empty lines draw nothing.
    void SyncView(bool visible)
    {
        UI::RmlBridge::SyncDocumentVisibility(s_view.Document(), visible);
        if (!visible)
            return;

        const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
        g_pRenderText->SetFont(g_hFontBold);
        const SIZE lineSize = g_pRenderText->MeasureText(L"Q", 1);
        SyncField(s_view.Binder(), &NoticesRmlModel::rowWidth, "row_width", 2.f * UI::Scaling::PositionX(transform, 320.f));
        SyncField(s_view.Binder(), &NoticesRmlModel::textPx, "text_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
        SyncField(s_view.Binder(), &NoticesRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineSize.cy) * transform.scaleY);

        std::vector<NoticeLineEntry> lines;
        for (int i = 0; i < MAX_NOTICE; i++)
        {
            const Notice& n = s_notices[i];
            if (n.Text[0] == L'\0')
                continue;
            NoticeLineEntry line;
            line.text = StringUtils::WideToNarrow(n.Text);
            if (n.Color == 0)
                line.kind = (int)s_blinkPhase % 10 < 5 ? "gold-dim" : "gold";
            else
                line.kind = "green";
            line.top = UI::Scaling::PositionY(transform, static_cast<float>(300 + i * 13));
            lines.push_back(std::move(line));
        }
        SyncField(s_view.Binder(), &NoticesRmlModel::lines, "lines", std::move(lines));
    }

    void RenderNative()
    {
        EnableAlphaTest();

        g_pRenderText->SetFont(g_hFontBold);

        for (int i = 0; i < MAX_NOTICE; i++)
        {
            Notice* n = &s_notices[i];
            if (n->Color == 0)
            {
                g_pRenderText->SetBgColor(0, 0, 0, 128);
                if ((int)s_blinkPhase % 10 < 5)
                {
                    g_pRenderText->SetTextColor(255, 200, 80, 128);
                }
                else
                {
                    g_pRenderText->SetTextColor(255, 200, 80, 255);
                }
            }
            else
            {
                g_pRenderText->SetTextColor(100, 255, 200, 255);
                g_pRenderText->SetBgColor(0, 0, 0, 128);
            }

            g_pRenderText->RenderText(320, 300 + i * 13, n->Text, 0, 0, RT3_WRITE_CENTER);
        }
    }
    } // namespace

    void Render()
    {
        s_renderedThisFrame = true;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP) == true)
        {
            UI::RmlBridge::SyncDocumentVisibility(s_view.Document(), false);
            return;
        }
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

        s_view.Ensure();
        if (s_view.Document() != nullptr)
            SyncView(true);
        else
            RenderNative();

        s_blinkPhase += FPS_ANIMATION_FACTOR;
    }
}
