#pragma once

namespace CfgSections
{
    inline constexpr wchar_t CfgSectionWindow[]     = L"Window";
    inline constexpr wchar_t CfgSectionGraphics[]   = L"Graphics";
    inline constexpr wchar_t CfgSectionAudio[]      = L"Audio";
    inline constexpr wchar_t CfgSectionUI[]         = L"UI";
    inline constexpr wchar_t CfgSectionLogin[]      = L"LOGIN";
    inline constexpr wchar_t CfgSectionConnectionSettings[] = L"CONNECTION SETTINGS";
    inline constexpr wchar_t CfgSectionCamera[] = L"Camera";
    inline constexpr wchar_t CfgSectionRender[] = L"Render";
}

namespace CfgKeys
{
    // Window
    inline constexpr wchar_t CfgKeyWidth[]      = L"Width";
    inline constexpr wchar_t CfgKeyHeight[]     = L"Height";
    inline constexpr wchar_t CfgKeyWindowed[]   = L"Windowed";

    // Audio — volume 0 = off, >0 = on (no separate Enabled flag).
    inline constexpr wchar_t CfgKeySoundVolume[]  = L"SoundVolume";
    inline constexpr wchar_t CfgKeyMusicVolume[] = L"MusicVolume";

    // Login
    inline constexpr wchar_t CfgKeyRememberMe[]        = L"RememberMe";
    inline constexpr wchar_t CfgKeySavePassword[]      = L"SavePassword";
    inline constexpr wchar_t CfgKeyLanguage[]          = L"Language";
    inline constexpr wchar_t CfgKeyEncryptedUsername[] = L"EncryptedUsername";
    inline constexpr wchar_t CfgKeyEncryptedPassword[] = L"EncryptedPassword";

    // Connection
    inline constexpr wchar_t CfgKeyServerIP[]   = L"ServerIP";
    inline constexpr wchar_t CfgKeyServerPort[] = L"ServerPort";

    // UI
    inline constexpr wchar_t CfgKeyUILocale[] = L"Locale";
    inline constexpr wchar_t CfgKeyFont[]     = L"Font";
    // Active RmlUi theme name ("legacy"/"modern"), Data/Interface/RmlUi/themes/<name>/. Not yet
    // a full runtime hot-swap -- read once at startup. See UI::RmlBridge::RmlTheme.
    inline constexpr wchar_t CfgKeyRmlTheme[] = L"RmlTheme";
    // Global RmlUi UI scale, as a percentage (100 = normal). Drives
    // Rml::Context::SetDensityIndependentPixelRatio(). Only RCSS authored in `dp` units responds
    // to this; existing `px`-authored windows are unaffected until retrofitted.
    inline constexpr wchar_t CfgKeyUIScalePercent[] = L"UIScalePercent";
    // Per-window user-dragged position (GameConfig::GetWindowPosition/SetWindowPosition) --
    // "WindowPos.<id>.X"/".Y"/".HasOverride", <id> a short caller-chosen identifier (e.g.
    // "my_inventory"), not a display name. See GameConfig.h's own comment for why these bypass
    // the Load()/Save() member-field pattern every other CfgKeys* entry uses.
    inline constexpr wchar_t CfgKeyWindowPositionPrefix[] = L"WindowPos.";

    // Camera
    inline constexpr wchar_t CfgKeyZoom[] = L"Zoom";

    // Render
    // Core Profile GL context flip. 0 = compatibility (rollback), 1 = core.
    inline constexpr wchar_t CfgKeyCoreProfile[] = L"CoreProfile";
    inline constexpr wchar_t CfgKeySortParticleDraws[] = L"SortParticleDraws";
    inline constexpr wchar_t CfgKeyVSync[] = L"VSync";
    inline constexpr wchar_t CfgKeyRenderBackend[] = L"Backend";
    // DXP-23's per-system effect-cost toggles (MainScene.h's SetDisableEffects/SetDisableParticles/
    // SetDisableSkillEffectModels/SetDisableBoids/SetDisableWingShadow), originally console-only
    // ($effects ...) diagnostics -- promoted to persisted, in-game-options-exposed settings since
    // they're functionally just "reduce rendering cost for a low-end PC."
    inline constexpr wchar_t CfgKeyDisableEffects[] = L"DisableEffects";
    inline constexpr wchar_t CfgKeyDisableParticles[] = L"DisableParticles";
    inline constexpr wchar_t CfgKeyDisableSkillEffectModels[] = L"DisableSkillEffectModels";
    inline constexpr wchar_t CfgKeyDisableBoids[] = L"DisableBoids";
    inline constexpr wchar_t CfgKeyDisableWingShadow[] = L"DisableWingShadow";
    // User-chosen FPS cap when VSync is off; -1 = uncapped. Same promotion story as the toggles
    // above, this time for the console-only `$fps <N>` diagnostic.
    inline constexpr wchar_t CfgKeyFpsCap[] = L"FpsCap";
}

namespace CfgDefaults
{
    inline constexpr int  CfgDefaultWindowWidth  = 1024;
    inline constexpr int  CfgDefaultWindowHeight = 768;
    inline constexpr bool CfgDefaultWindowed     = true;

    inline constexpr int  CfgDefaultSoundVolume = 5;
    inline constexpr int  CfgDefaultMusicVolume = 5;

    inline constexpr bool CfgDefaultRememberMe = false;
    inline constexpr bool CfgDefaultSavePassword = false;
    inline constexpr wchar_t CfgDefaultLanguage[] = L"Eng";
    inline constexpr wchar_t CfgDefaultEncryptedUsername[] = L"";
    inline constexpr wchar_t CfgDefaultEncryptedPassword[] = L"";

    inline constexpr wchar_t CfgDefaultServerIP[] = L"localhost";
    inline constexpr int CfgDefaultServerPort = 44405;

    inline constexpr int CfgDefaultZoom = 1735;  // OrbitalCamera DEFAULT_RADIUS — matches Default-cam camera-to-Hero distance

    // I18N locale code; "en" is the default the resx generator falls back to.
    inline constexpr wchar_t CfgDefaultUILocale[] = L"en";

    // UI font family name. Empty = each platform's built-in default (Tahoma on
    // Windows, fontconfig "sans-serif" on Linux), so the look is unchanged until
    // the user picks a font. Any value is passed through as the GDI face name.
    inline constexpr wchar_t CfgDefaultFont[] = L"";

    // "legacy" reproduces the pre-RmlUi look with programmatic RCSS and no sprite dependency.
    // Defaults to legacy so nothing changes for existing players until they opt in.
    inline constexpr wchar_t CfgDefaultRmlTheme[] = L"legacy";

    // 100% = unscaled. The options window's UI tab offers a fixed ladder of values
    // (UI::Options::UIScalePercentChoices()); a hand-edited config.ini may hold anything in
    // [CfgMinUIScalePercent, CfgMaxUIScalePercent].
    inline constexpr int CfgDefaultUIScalePercent = 100;

    // Bounds GameConfig::SetUIScalePercent() clamps to. The floor is a usability limit, not a
    // technical one: below 50% the options window's own rows are too small to click the setting
    // back up. The ceiling is the point past which a small window cannot show a full panel.
    inline constexpr int CfgMinUIScalePercent = 50;
    inline constexpr int CfgMaxUIScalePercent = 300;

    // Legacy config compatibility only. SDL GPU ignores this key and default.
    inline constexpr bool CfgDefaultCoreProfile = true;

    // Opt-in until real effects have been visually checked on target hardware.
    inline constexpr bool CfgDefaultSortParticleDraws = false;
    inline constexpr bool CfgDefaultVSync = true;

    inline constexpr bool CfgDefaultDisableEffects = false;
    inline constexpr bool CfgDefaultDisableParticles = false;
    inline constexpr bool CfgDefaultDisableSkillEffectModels = false;
    inline constexpr bool CfgDefaultDisableBoids = false;
    inline constexpr bool CfgDefaultDisableWingShadow = false;
    inline constexpr int CfgDefaultFpsCap = -1;  // Uncapped

    // "default" = this app's own platform-aware pick (prefers Vulkan on Windows to avoid
    // D3D12's vsync-cap bug; SDL's own auto-pick elsewhere). Other accepted values:
    // "vulkan", "direct3d12" (alias "d3d12"), "metal" -- forces that SDL_gpu driver on any
    // platform, falling back to auto-pick with a warning log if it's unavailable.
    inline constexpr wchar_t CfgDefaultRenderBackend[] = L"default";
}
