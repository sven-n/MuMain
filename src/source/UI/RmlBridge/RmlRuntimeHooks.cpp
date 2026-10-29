#include "stdafx.h"
#include "UI/RmlBridge/RmlRuntimeHooks.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/GameConfig/GameConfig.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Diagnostics/DiagnosticsOverlay.h"
#include "UI/RmlBridge/RmlDocumentHints.h"
#include "UI/RmlBridge/RmlMarquee.h"
#include "UI/RmlBridge/RmlNativeText.h"
#include "UI/RmlBridge/RmlNativeTextFit.h"
#include "UI/RmlBridge/RmlRenderTarget.h"
#include "UI/RmlBridge/RmlScaleInputs.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/RmlBridge/ThemeFileInterface.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "UI/RmlBridge/ThemeFonts.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Core/WindowSystem.h"
#include "UI/HUD/ChatLogWindow.h"

namespace UI::RmlBridge
{
void InstallRuntimeHooks()
{
    RmlUiRuntimeHooks hooks;

    // The panel scale, so dp and the native transforms grow and stop growing together.
    hooks.dpRatio = [](int windowWidth, int windowHeight)
    { return UI::Scaling::TypographyScale(windowWidth, windowHeight); };
    hooks.afterScale = [](Rml::Context* context)
    {
        ApplyScaleInputs(context);
        ApplyNativeTextSize(context);
    };
    hooks.afterCreate = []
    {
        SetSelectedFontFamily(StringUtils::WideToNarrow(GameConfig::GetInstance().GetFontSelection().c_str()));
        LoadActiveThemeFonts();
        UI::Diagnostics::Initialize();
    };
    hooks.beforeDestroy = []
    {
        UI::Diagnostics::Release();
        ClearThemeFontCache();
    };
    hooks.beforeUpdate = []
    {
        SuspendMainSceneDocumentsOutsideMainScene();
        ApplySceneStackingDepths();
        Tooltip::ExpireUnrefreshed();
        if (Rml::Context* context = RmlUiRuntime::Instance().GetContext())
            DocumentHints::Update(*context);
    };
    hooks.afterUpdate = [](Rml::Context* context)
    {
        FitNativeTextToBoxes(context);
        UpdateMarquees(context);
        if (auto* chat = g_pChatListBox)
            chat->AfterRmlLayout(context);
    };
    hooks.resolveTextureFile = [](const Rml::String& source) { return ResolveThemeResource(source, true); };
    hooks.createFileInterface = [] { return std::make_unique<ThemeFileInterface>(); };
    hooks.resolveTexture = [](const Rml::String& source, void*& texture, int& width, int& height)
    {
        if (!RenderTarget::IsSource(source))
            return false;
        texture = RenderTarget::Resolve(source, width, height);
        return true;
    };

    RmlUiRuntime::Instance().SetHooks(std::move(hooks));
}
} // namespace UI::RmlBridge
