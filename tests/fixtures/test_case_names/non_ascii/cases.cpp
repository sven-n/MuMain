// On Windows the test binary reads its --test-case argument in the ANSI code page, not as the UTF-8 the name is
// compiled to, so the entry runs no case. The check prints the name even where that code page lacks its letters.
TEST_CASE("Żółć łąka")
{
    CHECK(true);
}
