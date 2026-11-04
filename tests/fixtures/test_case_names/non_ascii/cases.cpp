// A name with letters the Windows ANSI code page lacks: the check still reports it.
TEST_CASE("Żółć łąka; cut")
{
    CHECK(true);
}
