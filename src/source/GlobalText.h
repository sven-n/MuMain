#pragma once

#include "stringset.h"
#include <cwchar>
#include <cwctype>
#include <string>

namespace detail
{
    template <class T>
    std::basic_string<T> NormalizePrintfSpecifiers(const T* text)
    {
        return std::basic_string<T>(text);
    }

    template <>
    inline std::basic_string<wchar_t> NormalizePrintfSpecifiers(const wchar_t* text)
    {
        std::wstring result;
        result.reserve(wcslen(text));

        bool inFormat = false;
        bool hasLengthModifier = false;
        for (size_t i = 0; text[i] != L'\0'; ++i)
        {
            const wchar_t ch = text[i];
            if (!inFormat)
            {
                if (ch == L'%')
                {
                    inFormat = true;
                    hasLengthModifier = false;
                }
                result.push_back(ch);
                continue;
            }

            if (ch == L'%')
            {
                inFormat = false;
                result.push_back(L'%');
                continue;
            }

            if (wcschr(L"hlLjztI", ch))
            {
                hasLengthModifier = true;
                result.push_back(ch);
                continue;
            }

            if (wcschr(L"cCdiouxXeEfgGaAnpsS", ch))
            {
                if ((ch == L's' || ch == L'S') && !hasLengthModifier)
                {
                    result.push_back(L'l');
                    result.push_back(L's');
                }
                else
                {
                    result.push_back(ch == L'S' ? L's' : ch);
                }
                inFormat = false;
                continue;
            }

            result.push_back(ch);
        }

        return result;
    }
}

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <type_traits>
#include <vector>

template <class T>
class TGlobalText
{
#pragma pack(push, 1)
    struct GLOBALTEXT_HEADER
    {
        std::uint16_t	wSignature;
        std::uint32_t	dwNumberOfText;
    };
    struct GLOBALTEXT_STRING_HEADER
    {
        std::uint32_t	dwKey;
        std::uint32_t	dwSizeOfString;
    };
#pragma pack(pop)

    class CKeyCode
    {
        std::wstring	m_strKeyCode;
    public:
        explicit CKeyCode(int key)
        {
            m_strKeyCode = std::to_wstring(key);
            if (m_strKeyCode.size() < 9)
            {
                const int nInsertChar = static_cast<int>(9 - m_strKeyCode.size());
                m_strKeyCode.insert(m_strKeyCode.begin(), nInsertChar, L'0');
            }
            m_strKeyCode.resize(9);
        }

        std::uint16_t GetTypeCode() const
        {
            std::wstring strTypeCode(m_strKeyCode, 0, 2);
            return static_cast<std::uint16_t>(std::stoi(strTypeCode));
        }
        std::uint16_t GetCountryCode() const
        {
            std::wstring strCountryCode(m_strKeyCode, 2, 3);
            return static_cast<std::uint16_t>(std::stoi(strCountryCode));
        }
        std::uint16_t GetIndexCode() const
        {
            std::wstring strIndexCode(m_strKeyCode, 5, 4);
            return static_cast<std::uint16_t>(std::stoi(strIndexCode));
        }
    };

    leaf::TStringSet<T>		m_StringSet;

public:

    TGlobalText() {}
    ~TGlobalText() { RemoveAll(); }

    enum
    {
        MAX_NUMBER_OF_TEXTS = 9999,

        LD_COMMON_TEXTS = 0x00000000,

        LD_USA_CANADA_TEXTS = 0x00000001,
        LD_JAPAN_A_TEXTS = 0x00000002,
        LD_TAIWAN_TEXTS = 0x00000004,
        LD_PHILIPPINES_TEXTS = 0x00000008,
        LD_CHINA_A_TEXTS = 0x00000010,
        LD_BRAZIL_A_TEXTS = 0x00000020,
        LD_SOUTH_KOREA_TEXTS = 0x00000040,
        LD_THAILAND_TEXTS = 0x00000080,
        LD_VIETNAM_TEXTS = 0x00000100,

        LD_FOREIGN_TEXTS = 0X80000000,

        LD_ALL_TEXTS = 0xFFFFFFFF
    };

    bool Load(const std::wstring& strFilePath, std::uint32_t dwLoadDisposition)
    {
        std::ifstream file(strFilePath.c_str(), std::ios::binary);
        if (!file)
        {
            return false;
        }

        GLOBALTEXT_HEADER GTHeader{};
        file.read(reinterpret_cast<char*>(&GTHeader), sizeof(GLOBALTEXT_HEADER));

        if (!file || GTHeader.wSignature != 0x5447)
        {
            return false;
        }

        for (std::uint32_t i = 0; i < GTHeader.dwNumberOfText; i++)
        {
            GLOBALTEXT_STRING_HEADER GTStringHeader{};
            file.read(reinterpret_cast<char*>(&GTStringHeader), sizeof(GLOBALTEXT_STRING_HEADER));
            if (!file)
            {
                return false;
            }

            std::vector<std::uint8_t> stringBuffer(GTStringHeader.dwSizeOfString + 1u, 0);
            file.read(reinterpret_cast<char*>(stringBuffer.data()), GTStringHeader.dwSizeOfString);
            if (!file)
            {
                return false;
            }

            if (CheckLoadDisposition(static_cast<int>(GTStringHeader.dwKey), dwLoadDisposition) ||
                GTStringHeader.dwKey < MAX_NUMBER_OF_TEXTS)
            {
                BuxConvert(stringBuffer.data(), GTStringHeader.dwSizeOfString);
                if constexpr (std::is_same_v<T, wchar_t>)
                {
                    std::wstring decoded;
                    if (!Utf8ToWide(reinterpret_cast<const char*>(stringBuffer.data()), decoded))
                    {
                        return false;
                    }
                    auto normalized = detail::NormalizePrintfSpecifiers(decoded.c_str());
                    m_StringSet.Add(GTStringHeader.dwKey, normalized);
                }
                else
                {
                    auto* decrypted = reinterpret_cast<char*>(stringBuffer.data());
                    m_StringSet.Add(GTStringHeader.dwKey, decrypted);
                }
            }
        }
        return true;
    }
    bool Save(const std::wstring& strFilePath)
    {
        std::ofstream file(strFilePath.c_str(), std::ios::binary | std::ios::trunc);
        if (!file)
        {
            return false;
        }

        GLOBALTEXT_HEADER GTHeader{};
        GTHeader.wSignature = 0x5447;
        GTHeader.dwNumberOfText = m_StringSet.GetCount();
        file.write(reinterpret_cast<const char*>(&GTHeader), sizeof(GLOBALTEXT_HEADER));

        for (std::uint32_t i = 0; i < GTHeader.dwNumberOfText; i++)
        {
            const int key = m_StringSet.GetKey(static_cast<int>(i));
            if (key == -1)
            {
                return false;
            }

            const auto& strObj = m_StringSet.FindObj(key);
            GLOBALTEXT_STRING_HEADER GTStringHeader{};
            GTStringHeader.dwKey = static_cast<std::uint32_t>(key);
            GTStringHeader.dwSizeOfString = static_cast<std::uint32_t>(strObj.size());

            file.write(reinterpret_cast<const char*>(&GTStringHeader), sizeof(GLOBALTEXT_STRING_HEADER));

            std::vector<std::uint8_t> buffer(sizeof(T) * GTStringHeader.dwSizeOfString);
            std::memcpy(buffer.data(), m_StringSet[key], buffer.size());

            BuxConvert(buffer.data(), buffer.size());
            file.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        }
        return file.good();
    }

    bool Add(int key, const T* szString) { return m_StringSet.Add(key, szString); }
    bool Remove(int key) { return m_StringSet.Remove(key); }
    void RemoveAll() { m_StringSet.RemoveAll(); }

    const T* Get(int key) { return m_StringSet[key]; }
    size_t GetStringSize(int key) { return m_StringSet.FindObj(key).size(); }

    std::uint16_t GetNumCountryCode(const std::wstring& strAlpha3Code)
    {	//. strAlpha3Code: Official Alpha-3 Code
        if (0 == strAlpha3Code.compare(L"USA") || 0 == strAlpha3Code.compare(L"CAN")) return 130;
        if (0 == strAlpha3Code.compare(L"JPN")) return 450;
        if (0 == strAlpha3Code.compare(L"TPE")) return 471;
        if (0 == strAlpha3Code.compare(L"PHI")) return 480;
        if (0 == strAlpha3Code.compare(L"CHN")) return 690;
        if (0 == strAlpha3Code.compare(L"BRA")) return 789;
        if (0 == strAlpha3Code.compare(L"KOR")) return 880;
        if (0 == strAlpha3Code.compare(L"THA")) return 885;
        if (0 == strAlpha3Code.compare(L"VIE")) return 893;
        if (0 == strAlpha3Code.compare(L"FRN")) return 995;

        return 0;
    }

    const T* operator [] (int key)
    {
        return Get(key);
    }

protected:
    static void BuxConvert(std::uint8_t* data, std::size_t length)
    {
        static constexpr std::array<std::uint8_t, 3> byBuxCode{ 0xfc,0xcf,0xab };
        for (std::size_t i = 0; i < length; ++i)
        {
            data[i] ^= byBuxCode[i % byBuxCode.size()];
        }
    }
    bool CheckLoadDisposition(int key, std::uint32_t dwLoadDisposition)
    {
        CKeyCode KeyCode(key);
        if (((dwLoadDisposition & LD_USA_CANADA_TEXTS) == LD_USA_CANADA_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"USA") || KeyCode.GetCountryCode() == GetNumCountryCode(L"CAN")))
            return true;
        if (((dwLoadDisposition & LD_JAPAN_A_TEXTS) == LD_JAPAN_A_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"JPN")))
            return true;
        if (((dwLoadDisposition & LD_TAIWAN_TEXTS) == LD_TAIWAN_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"TPE")))
            return true;
        if (((dwLoadDisposition & LD_PHILIPPINES_TEXTS) == LD_PHILIPPINES_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"PHI")))
            return true;
        if (((dwLoadDisposition & LD_CHINA_A_TEXTS) == LD_CHINA_A_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"CHN")))
            return true;
        if (((dwLoadDisposition & LD_BRAZIL_A_TEXTS) == LD_BRAZIL_A_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"BRA")))
            return true;
        if (((dwLoadDisposition & LD_SOUTH_KOREA_TEXTS) == LD_SOUTH_KOREA_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"KOR")))
            return true;
        if (((dwLoadDisposition & LD_THAILAND_TEXTS) == LD_THAILAND_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"THA")))
            return true;
        if (((dwLoadDisposition & LD_VIETNAM_TEXTS) == LD_VIETNAM_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"VIE")))
            return true;
        if (((dwLoadDisposition & LD_FOREIGN_TEXTS) == LD_FOREIGN_TEXTS) &&
            (KeyCode.GetCountryCode() == GetNumCountryCode(L"FRN")))
            return true;
        return false;
    }

    static bool Utf8ToWide(const char* utf8, std::wstring& out)
    {
        out.clear();
        if (utf8 == nullptr)
        {
            return false;
        }

        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(utf8);
        std::size_t i = 0;
        while (bytes[i] != '\0')
        {
            std::uint32_t codepoint = 0;
            unsigned char c = bytes[i];
            std::size_t extra = 0;
            if (c <= 0x7F)
            {
                codepoint = c;
                extra = 0;
            }
            else if ((c & 0xE0) == 0xC0)
            {
                codepoint = c & 0x1F;
                extra = 1;
            }
            else if ((c & 0xF0) == 0xE0)
            {
                codepoint = c & 0x0F;
                extra = 2;
            }
            else if ((c & 0xF8) == 0xF0)
            {
                codepoint = c & 0x07;
                extra = 3;
            }
            else
            {
                return false;
            }

            for (std::size_t j = 0; j < extra; ++j)
            {
                ++i;
                unsigned char cc = bytes[i];
                if ((cc & 0xC0) != 0x80)
                {
                    return false;
                }
                codepoint = (codepoint << 6) | (cc & 0x3F);
            }

            if ((codepoint >= 0xD800 && codepoint <= 0xDFFF) || codepoint > 0x10FFFF)
            {
                return false;
            }

            if ((extra == 1 && codepoint < 0x80) ||
                (extra == 2 && codepoint < 0x800) ||
                (extra == 3 && codepoint < 0x10000))
            {
                return false;
            }
            if constexpr (sizeof(wchar_t) == 2)
            {
                if (codepoint <= 0xFFFF)
                {
                    out.push_back(static_cast<wchar_t>(codepoint));
                }
                else
                {
                    codepoint -= 0x10000;
                    out.push_back(static_cast<wchar_t>((codepoint >> 10) + 0xD800));
                    out.push_back(static_cast<wchar_t>((codepoint & 0x3FF) + 0xDC00));
                }
            }
            else
            {
                out.push_back(static_cast<wchar_t>(codepoint));
            }
            ++i;
        }
        return true;
    }
};

typedef TGlobalText<char>		CGlobalText;
typedef TGlobalText<wchar_t>	CGlobalTextW;

extern CGlobalTextW GlobalText;

