#pragma once

#include <RmlUi/Core.h>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace UI::Tests
{
// Diagnostic output deliberately preserves defects until each rollout fix is accepted.
class RmlTextLayoutAudit
{
public:
    struct Scenario
    {
        std::string theme, window, locale, character;
        int width = 1024, height = 768, percent = 75, tab = -1;
        float contentScale = 1.f;
    };

    explicit RmlTextLayoutAudit(const std::filesystem::path& directory);
    void Inspect(Rml::ElementDocument& document, const Scenario& scenario);
    void Finish(size_t expectedScenarios);
    size_t FailureCount(const std::string& window) const;
    // Failing lines of one element, by the name text-lines.csv records.
    size_t FailureCount(const std::string& window, const std::string& element) const;

private:
    void InspectText(Rml::ElementText& text, Rml::Element& panel,
                     const Rml::ElementList& controls, const Rml::ElementList& texts, const Scenario& scenario);
    std::ofstream m_Lines;
    std::ofstream m_Scenarios;
    std::map<std::string, size_t> m_Failures;
    std::map<std::pair<std::string, std::string>, size_t> m_ElementFailures;
    size_t m_ScenarioCount = 0;
};
} // namespace UI::Tests
