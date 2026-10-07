#include "stdafx.h"
#include "UI/RmlBridge/RmlRuntimeHooks.h"

#include "Data/GameConfig/GameConfig.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Diagnostics/DiagnosticsOverlay.h"
#include "UI/RmlBridge/RmlDocumentHints.h"
#include "UI/RmlBridge/RmlNativeText.h"
#include "UI/RmlBridge/RmlNativeTextFit.h"
#include "UI/RmlBridge/RmlRenderTarget.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/RmlBridge/ThemeFileInterface.h"
#include "UI/Scaling/UITransform.h"

namespace UI::RmlBridge
{
void InstallRuntimeHooks()
{
    RmlUiRuntimeHooks hooks;

    // The player's UI scale times the fit to the window, composed as UI::Scaling's native
    // transforms compose them. ViewportFitScale() already folds in the OS display scale; multiplying
    // GetWindowContentScale() in again would count it twice.
    hooks.dpRatio = [](int windowWidth, int windowHeight)
    {
        const float percent = static_cast<float>(GameConfig::GetInstance().GetUIScalePercent());
        return percent / 100.f * UI::Scaling::ViewportFitScale(windowWidth, windowHeight, UI::Scaling::MaximumPanelScale);
    };
    hooks.afterScale = [](Rml::Context* context) { ApplyNativeTextSize(context); };
    hooks.afterCreate = [] { UI::Diagnostics::Initialize(); };
    hooks.beforeDestroy = [] { UI::Diagnostics::Release(); };
    hooks.beforeUpdate = []
    {
        SuspendMainSceneDocumentsOutsideMainScene();
        Tooltip::ExpireUnrefreshed();
        if (Rml::Context* context = RmlUiRuntime::Instance().GetContext())
            DocumentHints::Update(*context);
    };
    hooks.afterUpdate = [](Rml::Context* context) { FitNativeTextToBoxes(context); };
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
