// The discovery escapes the comma for doctest's filter, which then reads "\\" as one '\' and splits at the comma.
TEST_CASE("x \\, y")
{
    CHECK(true);
}
