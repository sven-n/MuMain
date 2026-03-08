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
{
    // Cross-platform path: mu_get_app_dir() is defined in PlatformCompat.h
    // for both Windows (GetModuleFileNameW) and non-Windows (/proc/self/exe etc.)
    m_configPath = mu_get_app_dir() / L"config.ini";
    Load();
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

    m_renderTextType = ini.ReadInt(CfgSectionGraphics, CfgKeyRenderTextType, CfgDefaultRenderTextType);

    m_rememberMe = ini.ReadBool(CfgSectionLogin, CfgKeyRememberMe, CfgDefaultRememberMe);
    m_languageSelection = ini.ReadString(CfgSectionLogin, CfgKeyLanguage, CfgDefaultLanguage);
    m_encryptedUsername = ini.ReadString(CfgSectionLogin, CfgKeyEncryptedUsername, CfgDefaultEncryptedUsername);
    m_encryptedPassword = ini.ReadString(CfgSectionLogin, CfgKeyEncryptedPassword, CfgDefaultEncryptedPassword);

    std::wstring rawServerIP = ini.ReadString(CfgSectionConnectionSettings, CfgKeyServerIP, CfgDefaultServerIP);
    int rawServerPort = ini.ReadInt(CfgSectionConnectionSettings, CfgKeyServerPort, CfgDefaultServerPort);

    // AC-4/AC-5: Validate and sanitize connection settings
    m_serverIP = GameConfig::ValidateServerIP(rawServerIP, CfgDefaultServerIP);
    m_serverPort = GameConfig::ValidateServerPort(rawServerPort, CfgDefaultServerPort);
}

void GameConfig::Save()
{
    using namespace CfgSections;
    using namespace CfgKeys;

    IniFile ini(m_configPath);

    ini.WriteInt(CfgSectionWindow, CfgKeyWidth, m_windowWidth);
    ini.WriteInt(CfgSectionWindow, CfgKeyHeight, m_windowHeight);
    ini.WriteBool(CfgSectionWindow, CfgKeyWindowed, m_windowMode);

    ini.WriteInt(CfgSectionGraphics, CfgKeyColorDepth, m_colorDepth);
    ini.WriteInt(CfgSectionGraphics, CfgKeyRenderTextType, m_renderTextType);

    ini.WriteBool(CfgSectionAudio, CfgKeySoundEnabled, m_soundEnabled);
    ini.WriteBool(CfgSectionAudio, CfgKeyMusicEnabled, m_musicEnabled);
    ini.WriteInt(CfgSectionAudio, CfgKeyVolumeLevel, m_volumeLevel);

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

    // Convert Hex String back to Binary Blob
    std::vector<BYTE> encryptedData = HexToBinary(hexInput);
    if (encryptedData.empty())
        return L"";

    DATA_BLOB dataIn, dataOut;
    dataIn.pbData = encryptedData.data();
    dataIn.cbData = static_cast<DWORD>(encryptedData.size());

    // Decrypt using Windows DPAPI
    if (CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut))
    {
        std::wstring result(reinterpret_cast<wchar_t*>(dataOut.pbData), dataOut.cbData / sizeof(wchar_t));
        LocalFree(dataOut.pbData); // Safety: Windows allocated this, we free it
        // The decrypted string might contain the null terminator, let's remove it if it exists.
        if (!result.empty() && result.back() == L'\0')
        {
            result.pop_back();
        }
        return result;
    }

    return L"";
}

std::wstring GameConfig::EncryptSetting(const wchar_t* input)
{
    if (!input || wcslen(input) == 0)
        return L"";

    DATA_BLOB dataIn, dataOut;
    dataIn.cbData = static_cast<DWORD>((wcslen(input) + 1) * sizeof(wchar_t));
    dataIn.pbData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(input));

    if (CryptProtectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut))
    {
        std::wstring hexResult = BinaryToHex(dataOut.pbData, dataOut.cbData);
        LocalFree(dataOut.pbData);
        return hexResult;
    }
    return L"";
}

void GameConfig::EncryptAndSaveCredentials(const wchar_t* user, const wchar_t* pass)
{
    std::wstring encUser = EncryptSetting(user);
    std::wstring encPass = EncryptSetting(pass);

    if (!encUser.empty() && !encPass.empty())
    {
        SetEncryptedUsername(encUser);
        SetEncryptedPassword(encPass);
        Save(); // Actually write to the .ini file
    }
}

// Helper INI read/write methods now delegate to IniFile
// (these private helpers are kept for backward compat with GameConfig.h declaration)
int GameConfig::ReadInt(const wchar_t* section, const wchar_t* key, int defaultValue)
{
    IniFile ini(m_configPath);
    return ini.ReadInt(section, key, defaultValue);
}

void GameConfig::WriteInt(const wchar_t* section, const wchar_t* key, int value)
{
    IniFile ini(m_configPath);
    ini.WriteInt(section, key, value);
    ini.Save();
}

bool GameConfig::ReadBool(const wchar_t* section, const wchar_t* key, bool defaultValue)
{
    IniFile ini(m_configPath);
    return ini.ReadBool(section, key, defaultValue);
}

void GameConfig::WriteBool(const wchar_t* section, const wchar_t* key, bool value)
{
    IniFile ini(m_configPath);
    ini.WriteBool(section, key, value);
    ini.Save();
}

std::wstring GameConfig::ReadString(const wchar_t* section, const wchar_t* key, const std::wstring& defaultValue)
{
    IniFile ini(m_configPath);
    return ini.ReadString(section, key, defaultValue);
}

void GameConfig::WriteString(const wchar_t* section, const wchar_t* key, const std::wstring& value)
{
    IniFile ini(m_configPath);
    ini.WriteString(section, key, value);
    ini.Save();
}
