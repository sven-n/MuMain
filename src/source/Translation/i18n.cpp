#include "stdafx.h"
#include "i18n.h"
#include <algorithm>
#include <cctype>

namespace i18n {

Translator& Translator::GetInstance() {
    static Translator instance;
    return instance;
}

bool Translator::ParseJsonFile(const std::wstring& filePath, std::map<std::string, std::string>& outMap) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    outMap.clear();

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Simple JSON parser for flat key-value pairs
    // Format: { "key": "value", "key2": "value2" }

    size_t pos = 0;
    while (pos < content.length()) {
        // Find next key
        size_t keyStart = content.find('"', pos);
        if (keyStart == std::string::npos) break;
        keyStart++;

        size_t keyEnd = content.find('"', keyStart);
        if (keyEnd == std::string::npos) break;

        std::string key = content.substr(keyStart, keyEnd - keyStart);

        // Find colon
        size_t colonPos = content.find(':', keyEnd);
        if (colonPos == std::string::npos) break;

        // Find value
        size_t valueStart = content.find('"', colonPos);
        if (valueStart == std::string::npos) break;
        valueStart++;

        size_t valueEnd = content.find('"', valueStart);
        if (valueEnd == std::string::npos) break;

        std::string value = content.substr(valueStart, valueEnd - valueStart);

        // Handle escape sequences
        size_t escapePos = 0;
        while ((escapePos = value.find("\\n", escapePos)) != std::string::npos) {
            value.replace(escapePos, 2, "\n");
            escapePos++;
        }
        escapePos = 0;
        while ((escapePos = value.find("\\t", escapePos)) != std::string::npos) {
            value.replace(escapePos, 2, "\t");
            escapePos++;
        }
        escapePos = 0;
        while ((escapePos = value.find("\\\"", escapePos)) != std::string::npos) {
            value.replace(escapePos, 2, "\"");
            escapePos++;
        }

        outMap[key] = value;
        pos = valueEnd + 1;
    }

    return !outMap.empty();
}

bool Translator::LoadTranslations(Domain domain, const std::wstring& filePath) {
    std::map<std::string, std::string>* targetMap = nullptr;

    switch (domain) {
#ifdef _EDITOR
        case Domain::Editor:
            targetMap = &m_editorTranslations;
            break;
        case Domain::Metadata:
            targetMap = &m_metadataTranslations;
            break;
#endif
        case Domain::Game:
            targetMap = &m_gameTranslations;
            break;
        default:
            return false;
    }

    return ParseJsonFile(filePath, *targetMap);
}

void Translator::SetLocale(const std::string& locale) {
    m_currentLocale = locale;
}

const char* Translator::Translate(Domain domain, const char* key, const char* fallback) const {
    if (!key) return fallback ? fallback : "";

    const std::map<std::string, std::string>* sourceMap = nullptr;

    switch (domain) {
#ifdef _EDITOR
        case Domain::Editor:
            sourceMap = &m_editorTranslations;
            break;
        case Domain::Metadata:
            sourceMap = &m_metadataTranslations;
            break;
#endif
        case Domain::Game:
            sourceMap = &m_gameTranslations;
            break;
        default:
            return fallback ? fallback : key;
    }

    auto it = sourceMap->find(key);
    if (it != sourceMap->end()) {
        return it->second.c_str();
    }

    return fallback ? fallback : key;
}

bool Translator::HasTranslation(Domain domain, const char* key) const {
    if (!key) return false;

    const std::map<std::string, std::string>* sourceMap = nullptr;

    switch (domain) {
#ifdef _EDITOR
        case Domain::Editor:
            sourceMap = &m_editorTranslations;
            break;
        case Domain::Metadata:
            sourceMap = &m_metadataTranslations;
            break;
#endif
        case Domain::Game:
            sourceMap = &m_gameTranslations;
            break;
        default:
            return false;
    }

    return sourceMap->find(key) != sourceMap->end();
}

std::string Translator::ReplacePlaceholders(const std::string& format, const std::vector<std::string>& args) const {
    std::string result = format;

    for (size_t i = 0; i < args.size(); ++i) {
        std::string placeholder = "{" + std::to_string(i) + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), args[i]);
            pos += args[i].length();
        }
    }

    return result;
}

std::string Translator::Format(Domain domain, const char* key, const std::vector<std::string>& args) const {
    const char* format = Translate(domain, key, key);
    return ReplacePlaceholders(format, args);
}

bool Translator::SwitchLanguage(const std::string& locale) {
    // Try to load all translation files for the new locale
    std::wstring localeW(locale.begin(), locale.end());

    bool gameLoaded = false;
#ifdef _EDITOR
    bool editorLoaded = false;
    bool metadataLoaded = false;
#endif

    // Try multiple paths for game translations
    std::wstring gamePath1 = L"Translations\\" + localeW + L"\\game.json";
    std::wstring gamePath2 = L"bin\\Translations\\" + localeW + L"\\game.json";

    gameLoaded = LoadTranslations(Domain::Game, gamePath1);
    if (!gameLoaded) {
        gameLoaded = LoadTranslations(Domain::Game, gamePath2);
    }

#ifdef _EDITOR
    // Try multiple paths for editor translations
    std::wstring editorPath1 = L"Translations\\" + localeW + L"\\editor.json";
    std::wstring editorPath2 = L"bin\\Translations\\" + localeW + L"\\editor.json";

    editorLoaded = LoadTranslations(Domain::Editor, editorPath1);
    if (!editorLoaded) {
        editorLoaded = LoadTranslations(Domain::Editor, editorPath2);
    }

    // Try multiple paths for metadata translations
    std::wstring metadataPath1 = L"Translations\\" + localeW + L"\\metadata.json";
    std::wstring metadataPath2 = L"bin\\Translations\\" + localeW + L"\\metadata.json";

    metadataLoaded = LoadTranslations(Domain::Metadata, metadataPath1);
    if (!metadataLoaded) {
        metadataLoaded = LoadTranslations(Domain::Metadata, metadataPath2);
    }
#endif

    // Only change locale if at least game translations loaded successfully
    if (gameLoaded) {
        m_currentLocale = locale;
        return true;
    }

    return false;
}

void Translator::Clear() {
#ifdef _EDITOR
    m_editorTranslations.clear();
    m_metadataTranslations.clear();
#endif
    m_gameTranslations.clear();
}

} // namespace i18n
