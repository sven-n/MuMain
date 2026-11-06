// The test binary lists this name on two lines, and the discovery reads each line as a name.
TEST_CASE("line one\nline two")
{
    CHECK(true);
}
