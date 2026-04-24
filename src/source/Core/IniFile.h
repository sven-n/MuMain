#pragma once
// Flow Code: VS1-NET-CONFIG-SERVER
// Story: 3.4.2 - Server Connection Configuration
//
// Portable INI reader/writer using std::wifstream / std::wofstream + std::filesystem.
// Replaces Win32 GetPrivateProfileIntW / GetPrivateProfileStringW / WritePrivateProfileStringW
// in GameConfig.cpp. See development-standards.md §1 Phase 5/5.1.
//
// Format supported: [section] / key=value / ; comment lines

#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <locale>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "MuLogger.h"

namespace detail
{
// Build a locale with a UTF-8 <-> wchar_t facet. codecvt_utf8 is deprecated in
// C++17 but still the only standard way to get a portable UTF-8 facet without
// pulling in ICU or writing one by hand. Suppression is localized to this helper.
inline std::locale MakeUtf8Locale()
{
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    return std::locale(std::locale::classic(), new std::codecvt_utf8<wchar_t>);
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}
}  // namespace detail

class IniFile
{
    // Tag type for write-only construction (skips Load()).
    // Used by GameConfig::Save() which writes all keys from in-memory state
    // and does not need to read the existing file first. See HIGH-1 fix.
    struct SkipLoadTag
    {
    };

public:
    explicit IniFile(const std::filesystem::path& path) : m_path(path)
    {
        Load();
    }

    // Write-only constructor: sets path but does NOT read from disk.
    // Use this when you intend to write all keys explicitly and do not need
    // to preserve or inspect any existing file content.
    explicit IniFile(const std::filesystem::path& path, SkipLoadTag) : m_path(path) {}

    static SkipLoadTag WriteOnly()
    {
        return SkipLoadTag{};
    }

    std::wstring ReadString(const std::wstring& section, const std::wstring& key,
                            const std::wstring& defaultValue) const
    {
        auto secIt = m_sections.find(section);
        if (secIt == m_sections.end())
        {
            return defaultValue;
        }
        auto keyIt = secIt->second.find(key);
        if (keyIt == secIt->second.end())
        {
            return defaultValue;
        }
        return keyIt->second;
    }

    int ReadInt(const std::wstring& section, const std::wstring& key, int defaultValue) const
    {
        std::wstring val = ReadString(section, key, L"");
        if (val.empty())
        {
            return defaultValue;
        }
        try
        {
            return std::stoi(val);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    bool ReadBool(const std::wstring& section, const std::wstring& key, bool defaultValue) const
    {
        int val = ReadInt(section, key, defaultValue ? 1 : 0);
        return val != 0;
    }

    void WriteString(const std::wstring& section, const std::wstring& key, const std::wstring& value)
    {
        m_sections[section][key] = value;
        EnsureSection(m_sectionOrder, section);
        EnsureKey(m_keyOrder[section], key);
    }

    void WriteInt(const std::wstring& section, const std::wstring& key, int value)
    {
        WriteString(section, key, std::to_wstring(value));
    }

    void WriteBool(const std::wstring& section, const std::wstring& key, bool value)
    {
        WriteString(section, key, value ? L"1" : L"0");
    }

    void Save() const
    {
        std::wofstream out(m_path, std::ios::out | std::ios::trunc);
        if (!out.is_open())
        {
            mu::log::Get("core")->error("IniFile::Save failed to open '{}' for writing", m_path.string());
            return;
        }
        // Fixed UTF-8 facet: config.ini stays byte-identical across Windows (ACP
        // varies), Linux, and macOS. std::locale("") reads $LANG / system ACP and
        // silently produces different encodings per platform.
        out.imbue(detail::MakeUtf8Locale());

        for (const auto& sec : m_sectionOrder)
        {
            out << L"[" << sec << L"]\n";

            auto secIt = m_sections.find(sec);
            if (secIt == m_sections.end())
            {
                continue;
            }

            auto orderIt = m_keyOrder.find(sec);
            if (orderIt != m_keyOrder.end())
            {
                for (const auto& key : orderIt->second)
                {
                    auto keyIt = secIt->second.find(key);
                    if (keyIt != secIt->second.end())
                    {
                        out << key << L"=" << keyIt->second << L"\n";
                    }
                }
            }

            out << L"\n";
        }
    }

private:
    std::filesystem::path m_path;
    // Section name -> (key -> value)
    std::map<std::wstring, std::map<std::wstring, std::wstring>> m_sections;
    // Preserve ordering for deterministic round-trip serialization
    std::vector<std::wstring> m_sectionOrder;
    std::map<std::wstring, std::vector<std::wstring>> m_keyOrder;

    void Load()
    {
        std::wifstream in(m_path);
        if (!in.is_open())
        {
            // Intentional: if config.ini doesn't exist yet, all reads will use
            // caller-supplied defaults. This is the expected first-launch path.
            return;
        }
        // Match Save(): fixed UTF-8 facet so files written on any host round-trip
        // cleanly on any other host. See detail::MakeUtf8Locale().
        in.imbue(detail::MakeUtf8Locale());

        std::wstring currentSection;
        std::wstring line;
        while (std::getline(in, line))
        {
            // Strip carriage return for Windows-style line endings
            if (!line.empty() && line.back() == L'\r')
            {
                line.pop_back();
            }

            // Trim leading whitespace
            auto first = line.find_first_not_of(L" \t");
            if (first == std::wstring::npos)
            {
                continue;
            }
            line = line.substr(first);

            if (line.empty() || line[0] == L';' || line[0] == L'#')
            {
                continue; // comment or blank
            }

            if (line[0] == L'[')
            {
                auto close = line.find(L']');
                if (close != std::wstring::npos)
                {
                    currentSection = line.substr(1, close - 1);
                    EnsureSection(m_sectionOrder, currentSection);
                }
                continue;
            }

            auto eq = line.find(L'=');
            if (eq != std::wstring::npos && !currentSection.empty())
            {
                std::wstring key = line.substr(0, eq);
                std::wstring value = line.substr(eq + 1);

                // Trim trailing whitespace from key
                auto keyEnd = key.find_last_not_of(L" \t");
                if (keyEnd != std::wstring::npos)
                {
                    key.resize(keyEnd + 1);
                }

                // Trim leading whitespace from value
                auto valStart = value.find_first_not_of(L" \t");
                if (valStart != std::wstring::npos)
                {
                    value = value.substr(valStart);
                }
                else
                {
                    value = L"";
                }

                m_sections[currentSection][key] = value;
                EnsureKey(m_keyOrder[currentSection], key);
            }
        }
    }

    // Append section to order vector if not already present (in-place, O(n) but
    // config files have at most a handful of sections — negligible cost).
    static void EnsureSection(std::vector<std::wstring>& order, const std::wstring& section)
    {
        if (std::find(order.begin(), order.end(), section) == order.end())
        {
            order.push_back(section);
        }
    }

    // Append key to order vector if not already present (in-place).
    static void EnsureKey(std::vector<std::wstring>& order, const std::wstring& key)
    {
        if (std::find(order.begin(), order.end(), key) == order.end())
        {
            order.push_back(key);
        }
    }
};
