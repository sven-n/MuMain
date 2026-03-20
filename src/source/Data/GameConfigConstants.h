#pragma once

namespace CfgSections
{
inline constexpr wchar_t CfgSectionWindow[] = L"Window";
inline constexpr wchar_t CfgSectionGraphics[] = L"Graphics";
inline constexpr wchar_t CfgSectionAudio[] = L"Audio";
inline constexpr wchar_t CfgSectionLogin[] = L"LOGIN";
// Section name must match config.ini exactly (case-sensitive, space included).
inline constexpr wchar_t CfgSectionConnectionSettings[] = L"CONNECTION SETTINGS";
} // namespace CfgSections

namespace CfgKeys
{
// Window
inline constexpr wchar_t CfgKeyWidth[] = L"Width";
inline constexpr wchar_t CfgKeyHeight[] = L"Height";
inline constexpr wchar_t CfgKeyWindowed[] = L"Windowed";

// Graphics
inline constexpr wchar_t CfgKeyColorDepth[] = L"ColorDepth";
inline constexpr wchar_t CfgKeyRenderTextType[] = L"RenderTextType";

// Audio
inline constexpr wchar_t CfgKeySoundEnabled[] = L"SoundEnabled";
inline constexpr wchar_t CfgKeyMusicEnabled[] = L"MusicEnabled";
inline constexpr wchar_t CfgKeyVolumeLevel[] = L"VolumeLevel";
// Story 5.4.1: Separate BGM and SFX volume persistence
inline constexpr wchar_t CfgKeyBGMVolumeLevel[] = L"BGMVolumeLevel";
inline constexpr wchar_t CfgKeySFXVolumeLevel[] = L"SFXVolumeLevel";

// Login
inline constexpr wchar_t CfgKeyRememberMe[] = L"RememberMe";
inline constexpr wchar_t CfgKeyLanguage[] = L"Language";
inline constexpr wchar_t CfgKeyEncryptedUsername[] = L"EncryptedUsername";
inline constexpr wchar_t CfgKeyEncryptedPassword[] = L"EncryptedPassword";

// Connection
inline constexpr wchar_t CfgKeyServerIP[] = L"ServerIP";
inline constexpr wchar_t CfgKeyServerPort[] = L"ServerPort";
} // namespace CfgKeys

namespace CfgDefaults
{
inline constexpr int CfgDefaultWindowWidth = 1024;
inline constexpr int CfgDefaultWindowHeight = 768;
inline constexpr bool CfgDefaultWindowed = true;

inline constexpr int CfgDefaultColorDepth = 0;

inline constexpr bool CfgDefaultSoundEnabled = true;
inline constexpr bool CfgDefaultMusicEnabled = false;
inline constexpr int CfgDefaultVolumeLevel = 5;
// Story 5.4.1: Separate BGM and SFX volume defaults (0-10 int scale)
inline constexpr int CfgDefaultBGMVolumeLevel = 5;
inline constexpr int CfgDefaultSFXVolumeLevel = 5;

inline constexpr int CfgDefaultRenderTextType = 0;

inline constexpr bool CfgDefaultRememberMe = false;
inline constexpr wchar_t CfgDefaultLanguage[] = L"Eng";
inline constexpr wchar_t CfgDefaultEncryptedUsername[] = L"";
inline constexpr wchar_t CfgDefaultEncryptedPassword[] = L"";

inline constexpr wchar_t CfgDefaultServerIP[] = L"localhost";
inline constexpr int CfgDefaultServerPort = 44405;
} // namespace CfgDefaults