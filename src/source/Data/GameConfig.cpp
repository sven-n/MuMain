// Flow Code: VS1-NET-CONFIG-SERVER
// Story: 3.4.2 - Server Connection Configuration
//
// Cross-platform GameConfig implementation.
// Replaced Win32 APIs (GetModuleFileNameW, GetPrivateProfileIntW,
// WritePrivateProfileStringW) with portable alternatives:
//   - Path: mu_get_app_dir() shim (PlatformCompat.h) + std::filesystem
//   - INI:  Core/IniFile.h portable reader/writer (std::wifstream/wofstream)
//   - Validation: GameConfigValidation.h (ValidateServerPort, ValidateServerIP)

#include "stdafx.h"
#include "MuLogger.h"
#include "GameConfig.h"

#include "GameConfigConstants.h"
#include "GameConfigValidation.h"
#include "IniFile.h"
#include "PlatformCompat.h"

GameConfig& GameConfig::GetInstance()
{
    static GameConfig instance;
    return instance;
}

GameConfig::GameConfig()
    : m_windowWidth(CfgDefaults::CfgDefaultWindowWidth), m_windowHeight(CfgDefaults::CfgDefaultWindowHeight),
      m_windowMode(CfgDefaults::CfgDefaultWindowed), m_colorDepth(CfgDefaults::CfgDefaultColorDepth),
      m_soundEnabled(CfgDefaults::CfgDefaultSoundEnabled), m_musicEnabled(CfgDefaults::CfgDefaultMusicEnabled),
      m_volumeLevel(CfgDefaults::CfgDefaultVolumeLevel), m_bgmVolumeLevel(CfgDefaults::CfgDefaultBGMVolumeLevel),
      m_sfxVolumeLevel(CfgDefaults::CfgDefaultSFXVolumeLevel), m_renderTextType(CfgDefaults::CfgDefaultRenderTextType),
      m_rememberMe(CfgDefaults::CfgDefaultRememberMe), m_languageSelection(CfgDefaults::CfgDefaultLanguage),
      m_encryptedUsername(CfgDefaults::CfgDefaultEncryptedUsername),
      m_encryptedPassword(CfgDefaults::CfgDefaultEncryptedPassword), m_serverIP(CfgDefaults::CfgDefaultServerIP),
      m_serverPort(CfgDefaults::CfgDefaultServerPort)
{
    // Cross-platform path: mu_get_app_dir() is defined in PlatformCompat.h
    // for both Windows (GetModuleFileNameW) and non-Windows (/proc/self/exe etc.)
    m_configPath = mu_get_app_dir() / L"config.ini";
    // NOTE: Load() is intentionally NOT called here.
    // MuMain.cpp calls GameConfig::GetInstance().Load() explicitly at startup
    // (line ~998). Calling Load() from the constructor would cause two sequential
    // reads from disk (constructor fires on first GetInstance() access, which IS
    // the MuMain.cpp call site). See AC-STD-NFR-1.
}

void GameConfig::Load()
{
    using namespace CfgSections;
    using namespace CfgKeys;
    using namespace CfgDefaults;

    IniFile ini(m_configPath);

    m_windowWidth = ini.ReadInt(CfgSectionWindow, CfgKeyWidth, CfgDefaultWindowWidth);
    m_windowHeight = ini.ReadInt(CfgSectionWindow, CfgKeyHeight, CfgDefaultWindowHeight);
    m_windowMode = ini.ReadBool(CfgSectionWindow, CfgKeyWindowed, CfgDefaultWindowed);

    m_colorDepth = ini.ReadInt(CfgSectionGraphics, CfgKeyColorDepth, CfgDefaultColorDepth);

    m_soundEnabled = ini.ReadBool(CfgSectionAudio, CfgKeySoundEnabled, CfgDefaultSoundEnabled);
    m_musicEnabled = ini.ReadBool(CfgSectionAudio, CfgKeyMusicEnabled, CfgDefaultMusicEnabled);
    m_volumeLevel = ini.ReadInt(CfgSectionAudio, CfgKeyVolumeLevel, CfgDefaultVolumeLevel);

    // Story 5.4.1: Read separate BGM/SFX volume levels with migration fallback.
    // If new keys absent (sentinel -1), fall back to old VolumeLevel for seamless migration.
    m_bgmVolumeLevel = ini.ReadInt(CfgSectionAudio, CfgKeyBGMVolumeLevel, -1);
    m_sfxVolumeLevel = ini.ReadInt(CfgSectionAudio, CfgKeySFXVolumeLevel, -1);
    if (m_bgmVolumeLevel < 0)
        m_bgmVolumeLevel = m_volumeLevel;
    if (m_sfxVolumeLevel < 0)
        m_sfxVolumeLevel = m_volumeLevel;

    m_renderTextType = ini.ReadInt(CfgSectionGraphics, CfgKeyRenderTextType, CfgDefaultRenderTextType);

    m_rememberMe = ini.ReadBool(CfgSectionLogin, CfgKeyRememberMe, CfgDefaultRememberMe);
    m_languageSelection = ini.ReadString(CfgSectionLogin, CfgKeyLanguage, CfgDefaultLanguage);
    m_encryptedUsername = ini.ReadString(CfgSectionLogin, CfgKeyEncryptedUsername, CfgDefaultEncryptedUsername);
    m_encryptedPassword = ini.ReadString(CfgSectionLogin, CfgKeyEncryptedPassword, CfgDefaultEncryptedPassword);

    std::wstring rawServerIP = ini.ReadString(CfgSectionConnectionSettings, CfgKeyServerIP, CfgDefaultServerIP);
    int rawServerPort = ini.ReadInt(CfgSectionConnectionSettings, CfgKeyServerPort, CfgDefaultServerPort);

    // AC-4/AC-5: Validate and sanitize connection settings
    m_serverIP = GameConfigValidation::ValidateServerIP(rawServerIP, CfgDefaultServerIP);
    m_serverPort = GameConfigValidation::ValidateServerPort(rawServerPort, CfgDefaultServerPort);
}

void GameConfig::Save()
{
    using namespace CfgSections;
    using namespace CfgKeys;

    // Write-only construction: skip Load() since all keys are written explicitly
    // from in-memory state. Avoids an unnecessary disk read on every Save() call.
    // See HIGH-1 fix (code review 3-4-2).
    IniFile ini(m_configPath, IniFile::WriteOnly());

    ini.WriteInt(CfgSectionWindow, CfgKeyWidth, m_windowWidth);
    ini.WriteInt(CfgSectionWindow, CfgKeyHeight, m_windowHeight);
    ini.WriteBool(CfgSectionWindow, CfgKeyWindowed, m_windowMode);

    ini.WriteInt(CfgSectionGraphics, CfgKeyColorDepth, m_colorDepth);
    ini.WriteInt(CfgSectionGraphics, CfgKeyRenderTextType, m_renderTextType);

    ini.WriteBool(CfgSectionAudio, CfgKeySoundEnabled, m_soundEnabled);
    ini.WriteBool(CfgSectionAudio, CfgKeyMusicEnabled, m_musicEnabled);
    ini.WriteInt(CfgSectionAudio, CfgKeyVolumeLevel, m_volumeLevel);
    // Story 5.4.1: Persist separate BGM/SFX volume levels
    ini.WriteInt(CfgSectionAudio, CfgKeyBGMVolumeLevel, m_bgmVolumeLevel);
    ini.WriteInt(CfgSectionAudio, CfgKeySFXVolumeLevel, m_sfxVolumeLevel);

    ini.WriteBool(CfgSectionLogin, CfgKeyRememberMe, m_rememberMe);
    ini.WriteString(CfgSectionLogin, CfgKeyLanguage, m_languageSelection);
    ini.WriteString(CfgSectionLogin, CfgKeyEncryptedUsername, m_encryptedUsername);
    ini.WriteString(CfgSectionLogin, CfgKeyEncryptedPassword, m_encryptedPassword);

    ini.WriteString(CfgSectionConnectionSettings, CfgKeyServerIP, m_serverIP);
    ini.WriteInt(CfgSectionConnectionSettings, CfgKeyServerPort, m_serverPort);

    ini.Save();
}

void GameConfig::SetWindowSize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
}

void GameConfig::SetWindowMode(bool windowed)
{
    m_windowMode = windowed;
}

void GameConfig::SetColorDepth(int depth)
{
    m_colorDepth = depth;
}

void GameConfig::SetSoundEnabled(bool enabled)
{
    m_soundEnabled = enabled;
}

void GameConfig::SetMusicEnabled(bool enabled)
{
    m_musicEnabled = enabled;
}

void GameConfig::SetVolumeLevel(int level)
{
    m_volumeLevel = level;
}

void GameConfig::SetBGMVolumeLevel(int level)
{
    m_bgmVolumeLevel = std::clamp(level, 0, 10);
}

void GameConfig::SetSFXVolumeLevel(int level)
{
    m_sfxVolumeLevel = std::clamp(level, 0, 10);
}

void GameConfig::SetRenderTextType(int type)
{
    m_renderTextType = type;
}

void GameConfig::SetRememberMe(bool remember)
{
    m_rememberMe = remember;
}

void GameConfig::SetLanguageSelection(const std::wstring& lang)
{
    m_languageSelection = lang;
}

void GameConfig::SetEncryptedUsername(const std::wstring& encryptedUsername)
{
    m_encryptedUsername = encryptedUsername;
}

void GameConfig::SetEncryptedPassword(const std::wstring& encryptedPassword)
{
    m_encryptedPassword = encryptedPassword;
}

void GameConfig::SetServerIP(const std::wstring& ip)
{
    m_serverIP = ip;
}

void GameConfig::SetServerPort(int port)
{
    m_serverPort = port;
}

// Helper function to convert binary data to hex string
std::wstring GameConfig::BinaryToHex(const BYTE* data, DWORD size)
{
    std::wstring hex;
    hex.reserve(size * 2);

    const wchar_t hexChars[] = L"0123456789ABCDEF";
    for (DWORD i = 0; i < size; ++i)
    {
        hex += hexChars[(data[i] >> 4) & 0x0F];
        hex += hexChars[data[i] & 0x0F];
    }

    return hex;
}

// Helper function to convert hex string to binary data
std::vector<BYTE> GameConfig::HexToBinary(const std::wstring& hex)
{
    std::vector<BYTE> binary;

    if (hex.empty() || hex.length() % 2 != 0)
        return binary;

    binary.reserve(hex.length() / 2);

    auto hex_char_to_byte = [](wchar_t c) -> BYTE
    {
        if (c >= L'0' && c <= L'9')
            return (c - L'0');
        if (c >= L'a' && c <= L'f')
            return (c - L'a' + 10);
        return (c - L'A' + 10);
    };

    for (size_t i = 0; i < hex.length(); i += 2)
    {
        wchar_t high = hex[i];
        wchar_t low = hex[i + 1];

        if (!iswxdigit(high) || !iswxdigit(low))
        {
            // Invalid hex character detected, return empty vector
            return {};
        }

        binary.push_back((hex_char_to_byte(high) << 4) | hex_char_to_byte(low));
    }

    return binary;
}

void GameConfig::DecryptCredentials(wchar_t* outUser, wchar_t* outPass, size_t userBufSize, size_t passBufSize)
{
    // Decrypt Username
    std::wstring user = DecryptSetting(GetEncryptedUsername());
    if (!user.empty())
    {
        wcsncpy_s(outUser, userBufSize, user.c_str(), _TRUNCATE);
    }

    // Decrypt Password
    std::wstring pass = DecryptSetting(GetEncryptedPassword());
    if (!pass.empty())
    {
        wcsncpy_s(outPass, passBufSize, pass.c_str(), _TRUNCATE);
    }
}

std::wstring GameConfig::DecryptSetting(const std::wstring& hexInput)
{
    if (hexInput.empty())
        return L"";

    // Story 7.6.3: Cross-platform decryption via mu_decrypt_blob (AES-256-GCM or no-op fallback).
    // Replaces Windows DPAPI. [VS0-QUAL-WIN32CLEAN-DATALAYER]
    // NOTE: Serialization format is platform-specific (wchar_t byte representation varies).
    // On Windows: sizeof(wchar_t)==2 (UTF-16LE); on Linux/macOS: sizeof(wchar_t)==4 (UTF-32).
    // Config files encrypted on one platform cannot be decrypted on another, but since keys are
    // machine-bound (PBKDF2 with hostname), cross-platform portability is intentionally out of scope.
    std::vector<BYTE> encryptedData = HexToBinary(hexInput);
    if (encryptedData.empty())
        return L"";

    std::vector<uint8_t> decrypted;
    if (!mu_decrypt_blob(encryptedData.data(), encryptedData.size(), decrypted))
        return L"";

    if (decrypted.empty())
        return L"";

    std::wstring result(reinterpret_cast<const wchar_t*>(decrypted.data()), decrypted.size() / sizeof(wchar_t));
    // The decrypted string might contain the null terminator, let's remove it if it exists.
    if (!result.empty() && result.back() == L'\0')
    {
        result.pop_back();
    }
    return result;
}

std::wstring GameConfig::EncryptSetting(const wchar_t* input)
{
    if (!input || wcslen(input) == 0)
        return L"";

    // Story 7.6.3: Cross-platform encryption via mu_encrypt_blob (AES-256-GCM or no-op fallback).
    // Replaces Windows DPAPI. [VS0-QUAL-WIN32CLEAN-DATALAYER]
    // NOTE: Serialization format is platform-specific (wchar_t byte representation varies).
    // On Windows: sizeof(wchar_t)==2 (UTF-16LE); on Linux/macOS: sizeof(wchar_t)==4 (UTF-32).
    // Config files encrypted on one platform cannot be decrypted on another, but since keys are
    // machine-bound (PBKDF2 with hostname), cross-platform portability is intentionally out of scope.
    size_t byteLen = (wcslen(input) + 1) * sizeof(wchar_t);

    std::vector<uint8_t> encrypted;
    if (!mu_encrypt_blob(input, byteLen, encrypted))
        return L"";

    return BinaryToHex(encrypted.data(), static_cast<DWORD>(encrypted.size()));
}

void GameConfig::EncryptAndSaveCredentials(const wchar_t* user, const wchar_t* pass)
{
    std::wstring encUser = EncryptSetting(user);
    std::wstring encPass = EncryptSetting(pass);

    if (encUser.empty() || encPass.empty())
    {
        mu::log::Get("data")->warn("[GameConfig] Failed to encrypt credentials, not saved");
        return;
    }

    SetEncryptedUsername(encUser);
    SetEncryptedPassword(encPass);
    Save(); // Actually write to the .ini file
}
