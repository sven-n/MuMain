#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListEdits.h"

#include "ServerListStepLabel.h"

#include "Data/GameData/ServerListData/ThemeColor.h"
#include "Data/GameData/ServerListData/ServerListIds.h"

#include <algorithm>
#include <array>
#include <utility>

namespace MuEditor::ServerList
{
namespace
{
// The ids of a new VIP type or color whose name has no letter to start one with.
constexpr std::string_view VipTypeIdFallback = "vip";
constexpr std::string_view ColorIdFallback = "color";

// Moves the entry one place up (-1) or down (+1); false at the end of the list.
template <typename TEntry> bool MoveEntry(std::vector<TEntry>& entries, size_t index, int direction)
{
    const long long target = static_cast<long long>(index) + direction;
    if (index >= entries.size() || target < 0 || target >= static_cast<long long>(entries.size()))
    {
        return false;
    }
    std::swap(entries[index], entries[static_cast<size_t>(target)]);
    return true;
}

// The colors of the load section: of the bar and of the two texts.
std::array<std::string*, 5> GetLoadColors(Data::ServerList::LoadSettings& load)
{
    return {&load.bar.base, &load.bar.full, &load.bar.empty, &load.full.color, &load.overloaded.color};
}

std::array<const std::string*, 5> GetLoadColors(const Data::ServerList::LoadSettings& load)
{
    return {&load.bar.base, &load.bar.full, &load.bar.empty, &load.full.color, &load.overloaded.color};
}
} // namespace

void ServerListEdits::Reset(ServerListData list, bool complete)
{
    m_complete = complete;
    m_list = std::move(list);
    m_committed = m_list;
    m_saved = m_list;
    m_undo.clear();
    m_redo.clear();
    m_pendingLabel.clear();
}

void ServerListEdits::Commit()
{
    if (m_list == m_committed)
    {
        m_pendingLabel.clear();
        return;
    }
    std::string label = GetPendingLabel();
    m_undo.push_back({std::move(m_committed), std::move(label)});
    if (m_undo.size() > MaxUndoSteps)
    {
        m_undo.erase(m_undo.begin());
    }
    m_redo.clear();
    m_committed = m_list;
    m_pendingLabel.clear();
}

void ServerListEdits::Undo()
{
    Commit();
    if (m_undo.empty())
    {
        return;
    }
    Step step = std::move(m_undo.back());
    m_undo.pop_back();
    m_redo.push_back({std::move(m_list), std::move(step.label)});
    m_list = std::move(step.list);
    m_committed = m_list;
}

void ServerListEdits::Redo()
{
    Commit();
    if (m_redo.empty())
    {
        return;
    }
    Step step = std::move(m_redo.back());
    m_redo.pop_back();
    m_undo.push_back({std::move(m_list), std::move(step.label)});
    m_list = std::move(step.list);
    m_committed = m_list;
}

std::string ServerListEdits::GetUndoLabel() const
{
    if (m_list != m_committed)
    {
        return GetPendingLabel();
    }
    return m_undo.empty() ? std::string() : m_undo.back().label;
}

std::string ServerListEdits::GetRedoLabel() const
{
    return m_redo.empty() ? std::string() : m_redo.back().label;
}

std::string ServerListEdits::GetPendingLabel() const
{
    return m_pendingLabel.empty() ? StepLabel::DescribeFieldChange(m_committed, m_list) : m_pendingLabel;
}

std::optional<size_t> ServerListEdits::AddGroup(const std::string& name, std::optional<int> id)
{
    if (!id || IsIdUsed(*id))
    {
        id = FindFreeId();
    }
    if (!id)
    {
        return std::nullopt;
    }
    ServerGroup group;
    group.id = *id;
    group.name = name;
    m_list.groups.push_back(std::move(group));
    m_pendingLabel = StepLabel::Add(name);
    return m_list.groups.size() - 1;
}

void ServerListEdits::RemoveGroup(size_t index)
{
    if (index < m_list.groups.size())
    {
        m_pendingLabel = StepLabel::Remove(m_list.groups[index].name);
        m_list.groups.erase(m_list.groups.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

bool ServerListEdits::MoveGroup(size_t index, int direction)
{
    if (index >= m_list.groups.size())
    {
        return false;
    }
    const std::string name = m_list.groups[index].name;
    if (!MoveEntry(m_list.groups, index, direction))
    {
        return false;
    }
    m_pendingLabel = StepLabel::Move(name);
    return true;
}

bool ServerListEdits::IsIdUsed(int id) const
{
    return std::any_of(m_list.groups.begin(), m_list.groups.end(),
                       [id](const ServerGroup& group) { return group.id == id; });
}

std::optional<int> ServerListEdits::FindFreeId() const
{
    for (int id = 0; id <= Data::ServerList::MaxServerGroupId; ++id)
    {
        if (!IsIdUsed(id))
        {
            return id;
        }
    }
    return std::nullopt;
}

size_t ServerListEdits::AddVipType(const std::string& name)
{
    const std::string base = Data::ServerList::MakeListId(name, VipTypeIdFallback);
    std::string id = base;
    for (int number = 2; IsVipTypeIdUsed(id); ++number)
    {
        id = base + std::to_string(number);
    }
    VipType vipType;
    vipType.id = std::move(id);
    vipType.name.Set(Data::LocalizedString::NeutralLocale, name);
    m_list.vipTypes.push_back(std::move(vipType));
    m_pendingLabel = StepLabel::AddVipType(name);
    return m_list.vipTypes.size() - 1;
}

void ServerListEdits::RemoveVipType(size_t index)
{
    if (index >= m_list.vipTypes.size())
    {
        return;
    }
    const VipType& vipType = m_list.vipTypes[index];
    m_pendingLabel = StepLabel::RemoveVipType(vipType.name.GetNeutral());
    for (ServerGroup& group : m_list.groups)
    {
        for (Data::ServerList::ChannelSettings& channel : group.channels)
        {
            if (channel.vip == vipType.id)
            {
                channel.vip.clear();
            }
        }
    }
    std::erase_if(m_list.buttonColors, [&vipType](const ButtonColorRule& rule)
                  { return rule.vipMatch == Data::ServerList::RuleVip::VipType && rule.vip == vipType.id; });
    m_list.vipTypes.erase(m_list.vipTypes.begin() + static_cast<std::ptrdiff_t>(index));
}

bool ServerListEdits::MoveVipType(size_t index, int direction)
{
    if (index >= m_list.vipTypes.size())
    {
        return false;
    }
    const std::string name = m_list.vipTypes[index].name.GetNeutral();
    if (!MoveEntry(m_list.vipTypes, index, direction))
    {
        return false;
    }
    m_pendingLabel = StepLabel::MoveVipType(name);
    return true;
}

bool ServerListEdits::ChangeVipTypeId(size_t index, const std::string& id)
{
    if (index >= m_list.vipTypes.size() || !Data::ServerList::IsListId(id))
    {
        return false;
    }
    VipType& vipType = m_list.vipTypes[index];
    if (id == vipType.id)
    {
        return true;
    }
    if (IsVipTypeIdUsed(id))
    {
        return false;
    }
    for (ServerGroup& group : m_list.groups)
    {
        for (Data::ServerList::ChannelSettings& channel : group.channels)
        {
            if (channel.vip == vipType.id)
            {
                channel.vip = id;
            }
        }
    }
    for (ButtonColorRule& rule : m_list.buttonColors)
    {
        if (rule.vipMatch == Data::ServerList::RuleVip::VipType && rule.vip == vipType.id)
        {
            rule.vip = id;
        }
    }
    vipType.id = id;
    return true;
}

bool ServerListEdits::IsVipTypeIdUsed(std::string_view id) const
{
    return m_list.FindVipType(id) != nullptr;
}

size_t ServerListEdits::AddColor(const std::string& name)
{
    const std::string base = Data::ServerList::MakeListId(name, ColorIdFallback);
    std::string id = base;
    for (int number = 2; !IsColorIdFree(id); ++number)
    {
        id = base + std::to_string(number);
    }
    CustomColor color;
    color.id = std::move(id);
    color.name = name;
    m_list.colors.push_back(std::move(color));
    m_pendingLabel = StepLabel::AddColor(name);
    return m_list.colors.size() - 1;
}

void ServerListEdits::RemoveColor(size_t index)
{
    if (index >= m_list.colors.size())
    {
        return;
    }
    const CustomColor& color = m_list.colors[index];
    m_pendingLabel = StepLabel::RemoveColor(color.name);
    const char* defaultColor = Data::ServerList::GetThemeColorName(Data::ServerList::ThemeColor::Default);
    for (ButtonColorRule& rule : m_list.buttonColors)
    {
        if (rule.color == color.id)
        {
            rule.color = defaultColor;
        }
    }
    for (std::string* loadColor : GetLoadColors(m_list.load))
    {
        if (*loadColor == color.id)
        {
            *loadColor = defaultColor;
        }
    }
    m_list.colors.erase(m_list.colors.begin() + static_cast<std::ptrdiff_t>(index));
}

bool ServerListEdits::MoveColor(size_t index, int direction)
{
    if (index >= m_list.colors.size())
    {
        return false;
    }
    const std::string name = m_list.colors[index].name;
    if (!MoveEntry(m_list.colors, index, direction))
    {
        return false;
    }
    m_pendingLabel = StepLabel::MoveColor(name);
    return true;
}

bool ServerListEdits::ChangeColorId(size_t index, const std::string& id)
{
    if (index >= m_list.colors.size())
    {
        return false;
    }
    CustomColor& color = m_list.colors[index];
    if (id == color.id)
    {
        return true;
    }
    if (!IsColorIdFree(id))
    {
        return false;
    }
    for (ButtonColorRule& rule : m_list.buttonColors)
    {
        if (rule.color == color.id)
        {
            rule.color = id;
        }
    }
    for (std::string* loadColor : GetLoadColors(m_list.load))
    {
        if (*loadColor == color.id)
        {
            *loadColor = id;
        }
    }
    color.id = id;
    return true;
}

bool ServerListEdits::IsColorIdFree(std::string_view id) const
{
    return Data::ServerList::IsListId(id) && !Data::ServerList::FindThemeColor(id) &&
           m_list.FindCustomColor(id) == nullptr;
}

size_t ServerListEdits::CountRulesWithColor(std::string_view id) const
{
    return static_cast<size_t>(std::count_if(m_list.buttonColors.begin(), m_list.buttonColors.end(),
                                             [id](const ButtonColorRule& rule) { return rule.color == id; }));
}

size_t ServerListEdits::CountLoadUsesOfColor(std::string_view id) const
{
    const auto colors = GetLoadColors(m_list.load);
    return static_cast<size_t>(
        std::count_if(colors.begin(), colors.end(), [id](const std::string* color) { return *color == id; }));
}

std::optional<size_t> ServerListEdits::AddButtonColorRule()
{
    std::optional<ButtonColorRule> rule = FindFreeRule();
    if (!rule)
    {
        return std::nullopt;
    }
    m_list.buttonColors.push_back(std::move(*rule));
    m_pendingLabel = StepLabel::AddButtonColorRule();
    return m_list.buttonColors.size() - 1;
}

std::optional<ButtonColorRule> ServerListEdits::FindFreeRule() const
{
    using Data::ServerList::RuleVip;
    // Any channel first, then none, any VIP type and each type; each with either PvP, on and off.
    std::vector<ButtonColorRule> candidates;
    const auto addFor = [&candidates](RuleVip vipMatch, const std::string& vip)
    {
        for (const std::optional<bool> pvp :
             {std::optional<bool>{}, std::optional<bool>{true}, std::optional<bool>{false}})
        {
            ButtonColorRule rule;
            rule.vipMatch = vipMatch;
            rule.vip = vip;
            rule.pvp = pvp;
            candidates.push_back(rule);
        }
    };
    addFor(RuleVip::Any, {});
    addFor(RuleVip::NoVipType, {});
    addFor(RuleVip::AnyVipType, {});
    for (const VipType& vipType : m_list.vipTypes)
    {
        addFor(RuleVip::VipType, vipType.id);
    }
    const auto free = std::find_if(candidates.begin(), candidates.end(),
                                   [this](const ButtonColorRule& rule) { return !HasRuleLike(rule, std::nullopt); });
    return free != candidates.end() ? std::optional<ButtonColorRule>(*free) : std::nullopt;
}

void ServerListEdits::RemoveButtonColorRule(size_t index)
{
    if (index < m_list.buttonColors.size())
    {
        m_pendingLabel = StepLabel::RemoveButtonColorRule();
        m_list.buttonColors.erase(m_list.buttonColors.begin() + static_cast<std::ptrdiff_t>(index));
    }
}

bool ServerListEdits::HasRuleLike(const ButtonColorRule& rule, std::optional<size_t> index) const
{
    for (size_t other = 0; other < m_list.buttonColors.size(); ++other)
    {
        if (other != index && m_list.buttonColors[other].FitsAsMuchAs(rule))
        {
            return true;
        }
    }
    return false;
}

size_t ServerListEdits::CountRulesForVipType(std::string_view id) const
{
    return static_cast<size_t>(
        std::count_if(m_list.buttonColors.begin(), m_list.buttonColors.end(), [id](const ButtonColorRule& rule)
                      { return rule.vipMatch == Data::ServerList::RuleVip::VipType && rule.vip == id; }));
}

std::vector<std::string> ServerListEdits::FindChannelsWithVipType(std::string_view id) const
{
    std::vector<std::string> channels;
    for (const ServerGroup& group : m_list.groups)
    {
        for (size_t index = 0; index < group.channels.size(); ++index)
        {
            if (group.channels[index].vip == id)
            {
                channels.push_back(group.name + "-" + std::to_string(index + 1));
            }
        }
    }
    return channels;
}
} // namespace MuEditor::ServerList

#endif // _EDITOR
