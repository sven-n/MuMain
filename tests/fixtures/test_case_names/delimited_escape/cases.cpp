// "\x{3b}" is a delimited escape (C++23, and an extension in GCC and Clang before it) for a ';'.
TEST_CASE("a delimited escape\x{3b} cut [tag]")
{
    CHECK(true);
}
