#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace UI::Combat
{
class GuardGuildLists
{
public:
    struct Declaration
    {
        std::wstring name;
        int markCount = 0;
        bool gaveUp = false;
        int order = 0;
    };
    struct SiegeGuild
    {
        std::wstring name;
        int joinSide = 0;
        int involvement = 0;
        int score = 0;
    };

    void AddDeclaration(Declaration entry);
    void SortDeclarations();
    void ClearDeclarations();
    void AddSiegeGuild(SiegeGuild entry);
    void ClearSiegeGuilds();
    bool SelectDeclaration(std::wstring_view name);
    bool SelectSiegeGuild(std::wstring_view name);
    const SiegeGuild* ScoreGuild() const;
    static bool IsOwnDeclaration(const Declaration& entry, std::wstring_view guild, std::wstring_view alliance);

    std::span<const Declaration> Declarations() const { return m_Declarations; }
    std::span<const SiegeGuild> SiegeGuilds() const { return m_SiegeGuilds; }
    const std::wstring& SelectedDeclaration() const { return m_SelectedDeclaration; }
    const std::wstring& SelectedSiegeGuild() const { return m_SelectedSiegeGuild; }
    unsigned Revision() const { return m_Revision; }

private:
    static constexpr size_t kMaxGuilds = 150;
    std::vector<Declaration> m_Declarations;
    std::vector<SiegeGuild> m_SiegeGuilds;
    std::wstring m_SelectedDeclaration;
    std::wstring m_SelectedSiegeGuild;
    bool m_ShowScore = false;
    unsigned m_Revision = 0;
};
}
