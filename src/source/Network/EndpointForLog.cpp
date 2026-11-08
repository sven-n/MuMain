#include "stdafx.h"

#include "EndpointForLog.h"

#include "Core/Platform/WinSock.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace Network
{
namespace
{
enum class AddressKind
{
    Local,
    Private,
    PublicIPv4,
    PublicIPv6,
    HostName,
};

constexpr std::size_t IPv6Size = 16;
// ::ffff:a.b.c.d carries an IPv4 address in its last 4 bytes, after 10 zero bytes and two 0xff.
constexpr std::size_t MappedIPv4Offset = 12;

std::string_view Trimmed(std::string_view text)
{
    const auto isSpace = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };
    while (!text.empty() && isSpace(text.front()))
        text.remove_prefix(1);
    while (!text.empty() && isSpace(text.back()))
        text.remove_suffix(1);
    return text;
}

// localhost and every name below it resolve to the loopback address (RFC 6761).
bool IsLocalName(std::string_view host)
{
    std::string name(host);
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (!name.empty() && name.back() == '.')
        name.pop_back();

    constexpr std::string_view Localhost = "localhost";
    constexpr std::string_view LocalhostSuffix = ".localhost";
    return name == Localhost || (name.size() > LocalhostSuffix.size() && name.ends_with(LocalhostSuffix));
}

AddressKind ClassifyIPv4(const unsigned char* b)
{
    // 0.0.0.0/8 is "this host", which only reaches the local machine.
    if (b[0] == 127 || b[0] == 0)
        return AddressKind::Local;
    const bool isPrivate = b[0] == 10 || (b[0] == 172 && (b[1] & 0xF0) == 16) || (b[0] == 192 && b[1] == 168) ||
                           (b[0] == 169 && b[1] == 254) || (b[0] == 100 && (b[1] & 0xC0) == 64);
    return isPrivate ? AddressKind::Private : AddressKind::PublicIPv4;
}

AddressKind ClassifyIPv6(const unsigned char* b)
{
    const bool firstTenZero = std::all_of(b, b + 10, [](unsigned char c) { return c == 0; });
    if (firstTenZero && b[10] == 0xFF && b[11] == 0xFF)
        return ClassifyIPv4(b + MappedIPv4Offset);

    // :: and ::1.
    if (firstTenZero && std::all_of(b + 10, b + IPv6Size - 1, [](unsigned char c) { return c == 0; }) &&
        b[IPv6Size - 1] <= 1)
        return AddressKind::Local;

    const bool uniqueLocal = (b[0] & 0xFE) == 0xFC;
    const bool linkLocal = b[0] == 0xFE && (b[1] & 0xC0) == 0x80;
    return (uniqueLocal || linkLocal) ? AddressKind::Private : AddressKind::PublicIPv6;
}

AddressKind Classify(const std::string& address, bool& isIPv6)
{
    isIPv6 = false;
    std::array<unsigned char, IPv6Size> bytes{};
    if (inet_pton(AF_INET, address.c_str(), bytes.data()) == 1)
        return ClassifyIPv4(bytes.data());

    // A zone (fe80::1%eth0) names the local interface; the address is the part before it.
    const std::string withoutZone = address.substr(0, address.find('%'));
    if (inet_pton(AF_INET6, withoutZone.c_str(), bytes.data()) == 1)
    {
        isIPv6 = true;
        return ClassifyIPv6(bytes.data());
    }
    return IsLocalName(address) ? AddressKind::Local : AddressKind::HostName;
}
} // namespace

std::string DescribeEndpointForLog(std::string_view host, int port)
{
    host = Trimmed(host);
    if (host.size() >= 2 && host.front() == '[' && host.back() == ']')
        host = host.substr(1, host.size() - 2);
    const std::string portText = ":" + std::to_string(port);
    if (host.empty())
        return "no address" + portText;

    const std::string address(host);
    bool isIPv6 = false;
    switch (Classify(address, isIPv6))
    {
    case AddressKind::PublicIPv4:
        return "a public IPv4 address" + portText;
    case AddressKind::PublicIPv6:
        return "a public IPv6 address" + portText;
    case AddressKind::HostName:
        return "a host name" + portText;
    case AddressKind::Private:
        return (isIPv6 ? "[" + address + "]" : address) + portText + " (private network)";
    case AddressKind::Local:
        break;
    }
    return (isIPv6 ? "[" + address + "]" : address) + portText;
}

std::string DescribeEndpointForLog(std::u16string_view host, int port)
{
    std::string narrow;
    narrow.reserve(host.size());
    for (const char16_t c : host)
        narrow += c < 0x80 ? static_cast<char>(c) : '?';
    return DescribeEndpointForLog(std::string_view(narrow), port);
}
} // namespace Network
