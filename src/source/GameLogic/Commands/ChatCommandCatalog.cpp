#include "GameLogic/Commands/ChatCommandCatalog.h"

#include "Data/Translation/MultiLanguage.h"

#include <cerrno>
#include <climits>
#include <cstdint>
#include <cwchar>

namespace GameLogic::Commands
{
namespace
{
// Offsets of the AvailableChatCommand message (C2 header with sub code).
constexpr int32_t IndexOffset = 5;
constexpr int32_t CountOffset = 6;
constexpr int32_t StatusOffset = 7;
constexpr int32_t ParameterCountOffset = 8;
constexpr int32_t CommandOffset = 9;
constexpr int32_t CommandLength = 32;
constexpr int32_t NameOffset = 41;
constexpr int32_t NameLength = 48;
constexpr int32_t DescriptionOffset = 89;
constexpr int32_t DescriptionLength = 256;
constexpr int32_t ParametersOffset = 345;

// Offsets within one parameter entry. Newer servers append the hints, which
// makes the entries bigger.
constexpr int32_t ParameterSize = 102;
constexpr int32_t ParameterWithHintsSize = 152;
constexpr int32_t ParameterRequiredOffset = 0;
constexpr int32_t ParameterTypeOffset = 1;
constexpr int32_t ParameterNameOffset = 2;
constexpr int32_t ParameterNameLength = 32;
constexpr int32_t ParameterShortNameOffset = 34;
constexpr int32_t ParameterShortNameLength = 20;
constexpr int32_t ParameterValidValuesOffset = 54;
constexpr int32_t ParameterValidValuesLength = 48;
constexpr int32_t ParameterValueReferenceOffset = 102;
constexpr int32_t ParameterHasRangeOffset = 103;
constexpr int32_t ParameterMinimumOffset = 104;
constexpr int32_t ParameterMaximumOffset = 112;
constexpr int32_t ParameterGroupWithOffset = 120;
constexpr int32_t ParameterGroupWithLength = 32;

// The strings are UTF-8 and padded with zeros.
std::wstring ReadString(const BYTE* data, int32_t offset, int32_t length)
{
    std::vector<wchar_t> buffer(static_cast<size_t>(length) + 1, L'\0');
    CMultiLanguage::ConvertFromUtf8(buffer.data(), reinterpret_cast<const char*>(data + offset), length);
    buffer[length] = L'\0';
    return std::wstring(buffer.data());
}

// The numbers of the range are signed and in little endian.
int64_t ReadInt64LittleEndian(const BYTE* data)
{
    uint64_t value = 0;
    for (int32_t i = static_cast<int32_t>(sizeof(value)) - 1; i >= 0; --i)
    {
        value = (value << CHAR_BIT) | data[i];
    }

    return static_cast<int64_t>(value);
}

ChatCommandValueReference ToValueReference(BYTE value)
{
    // A newer server may know kinds which we don't, and a hint we can't use is
    // no reason to drop the others.
    const bool isKnown = value <= static_cast<BYTE>(ChatCommandValueReference::LanguageIsoCode);
    return isKnown ? static_cast<ChatCommandValueReference>(value) : ChatCommandValueReference::None;
}

// The entries of older servers are smaller, and the size of the message is
// what tells them apart.
int32_t GetParameterEntrySize(int32_t size, int32_t parameterCount)
{
    if (parameterCount == 0)
    {
        return ParameterSize;
    }

    return (size - ParametersOffset) / parameterCount;
}

void ReadHints(const BYTE* entry, ChatCommandParameter& parameter)
{
    parameter.ValueReference = ToValueReference(entry[ParameterValueReferenceOffset]);

    const auto minimum = ReadInt64LittleEndian(entry + ParameterMinimumOffset);
    const auto maximum = ReadInt64LittleEndian(entry + ParameterMaximumOffset);
    parameter.HasRange = entry[ParameterHasRangeOffset] != 0 && minimum <= maximum;
    parameter.Minimum = parameter.HasRange ? minimum : 0;
    parameter.Maximum = parameter.HasRange ? maximum : 0;
}

// Returns false for a type we don't know, because we couldn't offer an input for it.
bool TryReadParameter(const BYTE* entry, int32_t entrySize, ChatCommandParameter& parameter, std::wstring& groupWith)
{
    const auto parameterType = entry[ParameterTypeOffset];
    if (parameterType > static_cast<BYTE>(ChatCommandParameterType::Boolean))
    {
        return false;
    }

    parameter.IsRequired = entry[ParameterRequiredOffset] != 0;
    parameter.Type = static_cast<ChatCommandParameterType>(parameterType);
    parameter.Name = ReadString(entry, ParameterNameOffset, ParameterNameLength);
    parameter.ShortName = ReadString(entry, ParameterShortNameOffset, ParameterShortNameLength);
    parameter.ValidValues = ReadString(entry, ParameterValidValuesOffset, ParameterValidValuesLength);

    if (entrySize >= ParameterWithHintsSize)
    {
        ReadHints(entry, parameter);
        groupWith = ReadString(entry, ParameterGroupWithOffset, ParameterGroupWithLength);
    }

    return true;
}

// The server names the parameter a value is grouped with, e.g. the group of
// an item number. Its index is what the client needs to get at its value.
void ResolveGroups(std::vector<ChatCommandParameter>& parameters, const std::vector<std::wstring>& groupWithNames)
{
    for (size_t i = 0; i < parameters.size(); ++i)
    {
        if (groupWithNames[i].empty())
        {
            continue;
        }

        for (size_t other = 0; other < parameters.size(); ++other)
        {
            if (other != i && parameters[other].Name == groupWithNames[i])
            {
                parameters[i].GroupWithIndex = static_cast<int>(other);
                break;
            }
        }
    }
}

bool TryParseInteger(const std::wstring& text, int64_t& number)
{
    wchar_t* end = nullptr;
    errno = 0;
    const auto parsed = std::wcstoll(text.c_str(), &end, 10);
    if (errno == ERANGE || end == text.c_str() || *end != L'\0')
    {
        return false;
    }

    number = parsed;
    return true;
}
} // namespace

bool ChatCommandParameter::Accepts(const std::wstring& value) const
{
    if (value.empty() || !this->HasRange)
    {
        return true;
    }

    int64_t number = 0;
    if (!TryParseInteger(value, number))
    {
        return false;
    }

    return number >= this->Minimum && number <= this->Maximum;
}

bool ChatCommand::CanExecuteDirectly() const
{
    for (const auto& parameter : this->Parameters)
    {
        if (parameter.IsRequired)
        {
            return false;
        }
    }

    return true;
}

std::wstring ChatCommand::GetUsage() const
{
    std::wstring usage = this->Command;
    for (const auto& parameter : this->Parameters)
    {
        usage += parameter.IsRequired ? L" {" : L" [";
        usage += parameter.Name;
        usage += parameter.IsRequired ? L"}" : L"]";
    }

    return usage;
}

ChatCommandCatalog& ChatCommandCatalog::Instance()
{
    static ChatCommandCatalog instance;
    return instance;
}

void ChatCommandCatalog::Reset()
{
    this->m_commands.clear();
    this->m_expectedCount = 0;
    this->m_isComplete = false;
    this->m_wasRequested = false;
}

bool ChatCommandCatalog::AddFromPacket(const BYTE* data, int32_t size)
{
    if (data == nullptr || size < ParametersOffset)
    {
        return false;
    }

    const auto parameterCount = data[ParameterCountOffset];
    const auto entrySize = GetParameterEntrySize(size, parameterCount);
    if (entrySize < ParameterSize)
    {
        return false;
    }

    const auto index = data[IndexOffset];
    const auto count = data[CountOffset];
    if (count == 0 || index >= count)
    {
        return false;
    }

    // The first message of a list starts a new one, so that a second request
    // doesn't append to the commands we already know.
    if (index == 0)
    {
        this->m_commands.clear();
        this->m_expectedCount = count;
        this->m_isComplete = false;
    }
    else if (this->m_isComplete || count != this->m_expectedCount || index != this->m_commands.size())
    {
        return false;
    }

    ChatCommand command;
    command.MinimumCharacterStatus = data[StatusOffset];
    command.Command = ReadString(data, CommandOffset, CommandLength);
    command.Name = ReadString(data, NameOffset, NameLength);
    command.Description = ReadString(data, DescriptionOffset, DescriptionLength);

    command.Parameters.resize(parameterCount);
    std::vector<std::wstring> groupWithNames(parameterCount);
    for (int32_t i = 0; i < parameterCount; ++i)
    {
        const auto* entry = data + ParametersOffset + i * entrySize;
        if (!TryReadParameter(entry, entrySize, command.Parameters[i], groupWithNames[i]))
        {
            return false;
        }
    }

    ResolveGroups(command.Parameters, groupWithNames);

    if (command.Command.empty())
    {
        return false;
    }

    this->m_commands.push_back(std::move(command));
    this->m_isComplete = this->m_commands.size() >= count;
    return true;
}

std::wstring ChatCommandCatalog::BuildCommandLine(const ChatCommand& command, const std::vector<std::wstring>& values)
{
    std::wstring line = command.Command;

    // The server switches to the "shortName=value" notation as soon as the line
    // contains an equals sign, which is what makes optional parameters skippable.
    const bool useShortNames = !command.Parameters.empty() && !command.Parameters.front().ShortName.empty();

    for (size_t i = 0; i < command.Parameters.size() && i < values.size(); ++i)
    {
        if (values[i].empty())
        {
            continue;
        }

        line += L' ';
        if (useShortNames && !command.Parameters[i].ShortName.empty())
        {
            line += command.Parameters[i].ShortName;
            line += L'=';
        }

        line += values[i];
    }

    return line;
}

} // namespace GameLogic::Commands
