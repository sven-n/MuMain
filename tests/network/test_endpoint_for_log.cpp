#include "doctest.h"

#include "Network/EndpointForLog.h"

using Network::DescribeEndpointForLog;

TEST_CASE("Local addresses are written out [network][log]")
{
    CHECK(DescribeEndpointForLog("127.0.0.1", 44405) == "127.0.0.1:44405");
    CHECK(DescribeEndpointForLog("127.127.127.127", 44405) == "127.127.127.127:44405");
    CHECK(DescribeEndpointForLog("127.255.255.254", 55901) == "127.255.255.254:55901");
    CHECK(DescribeEndpointForLog("localhost", 44406) == "localhost:44406");
    CHECK(DescribeEndpointForLog(" LocalHost. ", 44406) == "LocalHost.:44406");
    CHECK(DescribeEndpointForLog("mu.localhost", 44406) == "mu.localhost:44406");
    CHECK(DescribeEndpointForLog("::1", 55901) == "[::1]:55901");
    CHECK(DescribeEndpointForLog("[::1]", 55901) == "[::1]:55901");
    CHECK(DescribeEndpointForLog("0:0:0:0:0:0:0:1", 55901) == "[0:0:0:0:0:0:0:1]:55901");
    CHECK(DescribeEndpointForLog("::ffff:127.0.0.1", 55901) == "[::ffff:127.0.0.1]:55901");
    CHECK(DescribeEndpointForLog("0.0.0.0", 55901) == "0.0.0.0:55901");
    CHECK(DescribeEndpointForLog("::", 55901) == "[::]:55901");
    CHECK(DescribeEndpointForLog("::ffff:0.0.0.0", 55901) == "[::ffff:0.0.0.0]:55901");
}

TEST_CASE("Private network addresses are written out and marked [network][log]")
{
    CHECK(DescribeEndpointForLog("10.1.2.3", 44405) == "10.1.2.3:44405 (private network)");
    CHECK(DescribeEndpointForLog("172.16.0.1", 44405) == "172.16.0.1:44405 (private network)");
    CHECK(DescribeEndpointForLog("172.31.255.255", 44405) == "172.31.255.255:44405 (private network)");
    CHECK(DescribeEndpointForLog("172.17.0.2", 55901) == "172.17.0.2:55901 (private network)");
    CHECK(DescribeEndpointForLog("192.168.1.20", 44405) == "192.168.1.20:44405 (private network)");
    CHECK(DescribeEndpointForLog("169.254.10.1", 44405) == "169.254.10.1:44405 (private network)");
    CHECK(DescribeEndpointForLog("100.64.0.1", 44405) == "100.64.0.1:44405 (private network)");
    CHECK(DescribeEndpointForLog("100.127.255.255", 44405) == "100.127.255.255:44405 (private network)");
    CHECK(DescribeEndpointForLog("fd12:3456::1", 55901) == "[fd12:3456::1]:55901 (private network)");
    CHECK(DescribeEndpointForLog("fe80::1", 55901) == "[fe80::1]:55901 (private network)");
    CHECK(DescribeEndpointForLog("::ffff:192.168.1.20", 55901) == "[::ffff:192.168.1.20]:55901 (private network)");
}

TEST_CASE("Public addresses and host names are not written out [network][log]")
{
    CHECK(DescribeEndpointForLog("203.0.113.7", 44405) == "a public IPv4 address:44405");
    CHECK(DescribeEndpointForLog("172.32.0.1", 44405) == "a public IPv4 address:44405");
    CHECK(DescribeEndpointForLog("100.128.0.1", 44405) == "a public IPv4 address:44405");
    CHECK(DescribeEndpointForLog("::ffff:203.0.113.7", 44405) == "a public IPv4 address:44405");
    CHECK(DescribeEndpointForLog("2001:db8::7", 44405) == "a public IPv6 address:44405");
    CHECK(DescribeEndpointForLog("mu.example.com", 44405) == "a host name:44405");
    CHECK(DescribeEndpointForLog("notlocalhost", 44405) == "a host name:44405");
    // Short IPv4 forms like 127.1 are read as names, so they stay hidden.
    CHECK(DescribeEndpointForLog("127.1", 44405) == "a host name:44405");
    CHECK(DescribeEndpointForLog("", 44405) == "no address:44405");
}

TEST_CASE("UTF-16 hosts follow the same rule [network][log]")
{
    CHECK(DescribeEndpointForLog(u"127.127.127.127", 44405) == "127.127.127.127:44405");
    CHECK(DescribeEndpointForLog(u"203.0.113.7", 44405) == "a public IPv4 address:44405");
    CHECK(DescribeEndpointForLog(u"mü.example.com", 44405) == "a host name:44405");
}
