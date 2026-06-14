// Non-Windows implementation of the .ini profile shim (WinIni.h).
// Stores the file as UTF-8 on disk; sections/keys are matched case-insensitively.
#ifndef _WIN32

#include "Core/Platform/WinIni.h"
#include "Core/Platform/WinNls.h"  // WideCharToMultiByte / MultiByteToWideChar
#include "Core/Platform/PathResolve.h"

#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>

namespace
{
    // Convert with the source's explicit length (no trailing null), so the
    // converters never write a terminator one past the string's logical end.
    std::string Narrow(LPCWSTR s)
    {
        if (!s) return std::string();
        const int len = static_cast<int>(wcslen(s));
        const int n = WideCharToMultiByte(CP_UTF8, 0, s, len, nullptr, 0, nullptr, nullptr);
        if (n <= 0) return std::string();
        std::string out(static_cast<size_t>(n), '\0');
        WideCharToMultiByte(CP_UTF8, 0, s, len, out.data(), n, nullptr, nullptr);
        return out;
    }

    std::wstring Widen(const std::string& s)
    {
        if (s.empty()) return std::wstring();
        const int len = static_cast<int>(s.size());
        const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), len, nullptr, 0);
        if (n <= 0) return std::wstring();
        std::wstring out(static_cast<size_t>(n), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), len, out.data(), n);
        return out;
    }

    std::string Trim(const std::string& s)
    {
        size_t a = 0, b = s.size();
        while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
        while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
        return s.substr(a, b - a);
    }

    bool IEquals(const std::string& a, const std::string& b)
    {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
                return false;
        return true;
    }

    struct Entry   { std::string key, value; };
    struct Section { std::string name; std::vector<Entry> entries; };

    std::vector<Section> Load(const std::string& path)
    {
        std::vector<Section> sections;
        FILE* fp = std::fopen(MuResolvePath(path.c_str()).c_str(), "rb");
        if (!fp) return sections;

        std::string content;
        char buf[4096];
        size_t r;
        while ((r = std::fread(buf, 1, sizeof(buf), fp)) > 0)
            content.append(buf, r);
        std::fclose(fp);

        Section* current = nullptr;
        size_t pos = 0;
        while (pos <= content.size())
        {
            size_t nl = content.find('\n', pos);
            const std::string raw = content.substr(pos, (nl == std::string::npos ? content.size() : nl) - pos);
            const std::string line = Trim(raw);
            if (nl == std::string::npos && line.empty()) break;
            pos = (nl == std::string::npos) ? content.size() + 1 : nl + 1;

            if (line.empty() || line[0] == ';' || line[0] == '#')
                continue;

            if (line.front() == '[' && line.back() == ']')
            {
                sections.push_back(Section{ line.substr(1, line.size() - 2), {} });
                current = &sections.back();
                continue;
            }

            const size_t eq = line.find('=');
            if (eq == std::string::npos || !current)
                continue;
            current->entries.push_back(Entry{ Trim(line.substr(0, eq)), Trim(line.substr(eq + 1)) });
        }
        return sections;
    }

    bool Store(const std::string& path, const std::vector<Section>& sections)
    {
        FILE* fp = std::fopen(MuResolvePath(path.c_str()).c_str(), "wb");
        if (!fp) return false;
        for (const Section& s : sections)
        {
            std::fprintf(fp, "[%s]\n", s.name.c_str());
            for (const Entry& e : s.entries)
                std::fprintf(fp, "%s=%s\n", e.key.c_str(), e.value.c_str());
            std::fputc('\n', fp);
        }
        std::fclose(fp);
        return true;
    }

    // Returns the value for app/key, or nullptr if absent.
    const std::string* Find(const std::vector<Section>& sections, const std::string& app, const std::string& key)
    {
        for (const Section& s : sections)
        {
            if (!IEquals(s.name, app)) continue;
            for (const Entry& e : s.entries)
                if (IEquals(e.key, key)) return &e.value;
        }
        return nullptr;
    }
}

DWORD GetPrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault,
                               LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
    if (!lpReturnedString || nSize == 0) return 0;

    const std::vector<Section> sections = Load(Narrow(lpFileName));
    const std::string* found = (lpAppName && lpKeyName)
        ? Find(sections, Narrow(lpAppName), Narrow(lpKeyName)) : nullptr;

    const std::wstring value = found ? Widen(*found) : (lpDefault ? std::wstring(lpDefault) : std::wstring());

    DWORD copied = static_cast<DWORD>(value.size());
    if (copied > nSize - 1) copied = nSize - 1;  // truncate, leaving room for the null
    std::wmemcpy(lpReturnedString, value.c_str(), copied);
    lpReturnedString[copied] = L'\0';
    return copied;
}

UINT GetPrivateProfileIntW(LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName)
{
    const std::vector<Section> sections = Load(Narrow(lpFileName));
    const std::string* found = (lpAppName && lpKeyName)
        ? Find(sections, Narrow(lpAppName), Narrow(lpKeyName)) : nullptr;
    if (!found) return static_cast<UINT>(nDefault);
    return static_cast<UINT>(std::strtol(found->c_str(), nullptr, 10));
}

BOOL WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName)
{
    if (!lpAppName) return FALSE;
    const std::string path = Narrow(lpFileName);
    const std::string app = Narrow(lpAppName);
    std::vector<Section> sections = Load(path);

    // Null key: delete the whole section.
    if (!lpKeyName)
    {
        for (size_t i = 0; i < sections.size(); ++i)
            if (IEquals(sections[i].name, app)) { sections.erase(sections.begin() + i); break; }
        return Store(path, sections) ? TRUE : FALSE;
    }

    const std::string key = Narrow(lpKeyName);

    Section* sec = nullptr;
    for (Section& s : sections)
        if (IEquals(s.name, app)) { sec = &s; break; }

    // Null value: delete the key (if its section exists).
    if (!lpString)
    {
        if (sec)
            for (size_t i = 0; i < sec->entries.size(); ++i)
                if (IEquals(sec->entries[i].key, key)) { sec->entries.erase(sec->entries.begin() + i); break; }
        return Store(path, sections) ? TRUE : FALSE;
    }

    if (!sec)
    {
        sections.push_back(Section{ app, {} });
        sec = &sections.back();
    }

    const std::string value = Narrow(lpString);
    for (Entry& e : sec->entries)
        if (IEquals(e.key, key)) { e.value = value; return Store(path, sections) ? TRUE : FALSE; }

    sec->entries.push_back(Entry{ key, value });
    return Store(path, sections) ? TRUE : FALSE;
}

#endif // !_WIN32
