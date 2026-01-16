#pragma once

#ifdef _EDITOR

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>

// Lightweight JSON parser for simple key-value translations
namespace i18n {

// Translation domain (separate namespaces for editor, game, metadata)
enum class Domain {
    Editor,
    Game,
    Metadata
};

// Main translator class
class Translator {
public:
    static Translator& GetInstance();

    // Load translations from JSON file
    bool LoadTranslations(Domain domain, const std::wstring& filePath);

    // Set current locale
    void SetLocale(const std::string& locale);

    // Get current locale
    const std::string& GetLocale() const { return m_currentLocale; }

    // Translate a key with fallback
    const char* Translate(Domain domain, const char* key, const char* fallback = nullptr) const;

    // Check if translation exists
    bool HasTranslation(Domain domain, const char* key) const;

    // Format string with arguments
    std::string Format(Domain domain, const char* key, const std::vector<std::string>& args) const;

    // Clear all translations
    void Clear();

private:
    Translator() : m_currentLocale("en") {}
    ~Translator() = default;

    // Prevent copying
    Translator(const Translator&) = delete;
    Translator& operator=(const Translator&) = delete;

    // Parse simple JSON file
    bool ParseJsonFile(const std::wstring& filePath, std::map<std::string, std::string>& outMap);

    // Replace placeholders in format string
    std::string ReplacePlaceholders(const std::string& format, const std::vector<std::string>& args) const;

    std::string m_currentLocale;

    // Separate translation maps for each domain
    std::map<std::string, std::string> m_editorTranslations;
    std::map<std::string, std::string> m_gameTranslations;
    std::map<std::string, std::string> m_metadataTranslations;
};

// Convenience functions
inline const char* TranslateEditor(const char* key, const char* fallback = nullptr) {
    return Translator::GetInstance().Translate(Domain::Editor, key, fallback);
}

inline const char* TranslateGame(const char* key, const char* fallback = nullptr) {
    return Translator::GetInstance().Translate(Domain::Game, key, fallback);
}

inline const char* TranslateMetadata(const char* key, const char* fallback = nullptr) {
    return Translator::GetInstance().Translate(Domain::Metadata, key, fallback);
}

inline bool HasTranslation(Domain domain, const char* key) {
    return Translator::GetInstance().HasTranslation(domain, key);
}

inline std::string FormatEditor(const char* key, const std::vector<std::string>& args) {
    return Translator::GetInstance().Format(Domain::Editor, key, args);
}

} // namespace i18n

// Convenience macros
#define EDITOR_TEXT(key) i18n::TranslateEditor(key, key)
#define GAME_TEXT(key) i18n::TranslateGame(key, key)
#define META_TEXT(key, fallback) i18n::TranslateMetadata(key, fallback)

#endif // _EDITOR
