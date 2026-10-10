// doctest unit tests for the byte order helpers the packet handlers use for
// big-endian fields the server sends.

#include "doctest.h"

#include "Network/Server/WSclient.h"

TEST_CASE("ntoh16 swaps the two bytes of a big-endian value [network][byte_order]")
{
    // TradePartnerLevel 400 (0x0190) arrives as the bytes 01 90, which a
    // little-endian WORD reads as 0x9001.
    CHECK(ntoh16(0x9001) == 400);
    CHECK(ntoh16(ntoh16(400)) == 400);
    CHECK(ntoh16(0x00FF) == 0xFF00);
    CHECK(ntoh16(0) == 0);
}

TEST_CASE("ntoh64 reverses the eight bytes of a big-endian value [network][byte_order]")
{
    CHECK(ntoh64(0x0102030405060708ULL) == 0x0807060504030201ULL);
    CHECK(ntoh64(ntoh64(123456789ULL)) == 123456789ULL);
}
