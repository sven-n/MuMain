#pragma once

namespace CfgSections
{
    inline constexpr wchar_t CfgSectionWindow[]     = L"Window";
    inline constexpr wchar_t CfgSectionGraphics[]   = L"Graphics";
    inline constexpr wchar_t CfgSectionAudio[]      = L"Audio";
    inline constexpr wchar_t CfgSectionLogin[]      = L"LOGIN";
    inline constexpr wchar_t CfgSectionConnectionSettings[] = L"CONNECTION SETTINGS";
}

namespace CfgKeys
{
    // Window
    inline constexpr wchar_t CfgKeyWidth[]      = L"Width";
    inline constexpr wchar_t CfgKeyHeight[]     = L"Height";
    inline constexpr wchar_t CfgKeyWindowed[]   = L"Windowed";

    // Graphics
    inline constexpr wchar_t CfgKeyColorDepth[]     = L"ColorDepth";
    inline constexpr wchar_t CfgKeyRenderTextType[] = L"RenderTextType";

    // Audio
    inline constexpr wchar_t CfgKeySoundEnabled[] = L"SoundEnabled";
    inline constexpr wchar_t CfgKeyMusicEnabled[] = L"MusicEnabled";
    inline constexpr wchar_t CfgKeyVolumeLevel[]  = L"VolumeLevel";

    // Login
    inline constexpr wchar_t CfgKeyRememberMe[]        = L"RememberMe";
    inline constexpr wchar_t CfgKeyLanguage[]          = L"Language";
    inline constexpr wchar_t CfgKeyEncryptedUsername[] = L"EncryptedUsername";
    inline constexpr wchar_t CfgKeyEncryptedPassword[] = L"EncryptedPassword";

    // Connection
    inline constexpr wchar_t CfgKeyServerIP[]   = L"ServerIP";
    inline constexpr wchar_t CfgKeyServerPort[] = L"ServerPort";
}

namespace CfgDefaults
{
    inline constexpr int  CfgDefaultWindowWidth  = 1024;
    inline constexpr int  CfgDefaultWindowHeight = 768;
    inline constexpr bool CfgDefaultWindowed     = true;

    inline constexpr int  CfgDefaultColorDepth = 0;

    inline constexpr bool CfgDefaultSoundEnabled = true;
    inline constexpr bool CfgDefaultMusicEnabled = false;
    inline constexpr int  CfgDefaultVolumeLevel  = 5;

    inline constexpr int CfgDefaultRenderTextType = 0;

    inline constexpr bool CfgDefaultRememberMe = false;
    inline constexpr wchar_t CfgDefaultLanguage[] = L"Eng";
    inline constexpr wchar_t CfgDefaultEncryptedUsername[] = L"";
    inline constexpr wchar_t CfgDefaultEncryptedPassword[] = L"";

    inline constexpr wchar_t CfgDefaultServerIP[] = L"127.127.127.127";
    inline constexpr int CfgDefaultServerPort = 44406;
}