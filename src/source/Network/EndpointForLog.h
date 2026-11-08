#pragma once

#include <string>
#include <string_view>

// A server address as MuError.log writes it. Players post their logs in public, so only addresses that can't lead
// to a server are written out: the local machine (127.0.0.0/8, 0.0.0.0/8, localhost and *.localhost, ::1, ::) and
// private networks (10/8, 172.16/12, 192.168/16, 169.254/16, 100.64/10, fc00::/7, fe80::/10), which get
// "(private network)". An IPv4 address in IPv6 form (::ffff:192.168.1.20) counts as that IPv4 address. Public
// addresses and host names show as "a public IPv4 address", "a public IPv6 address" or "a host name". The port is
// always written.
namespace Network
{
// e.g. "127.127.127.127:44405", "172.17.0.2:55901 (private network)", "a public IPv4 address:44405".
std::string DescribeEndpointForLog(std::string_view host, int port);

// The same for a UTF-16 host. Non-ASCII hosts are host names, so they need no conversion.
std::string DescribeEndpointForLog(std::u16string_view host, int port);
} // namespace Network
