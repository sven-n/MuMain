// No character has this code (the last is U+10FFFF), so the compiler rejects it; the check reports it, not crashes.
TEST_CASE("an escape past the last character \U00110000")
{
    CHECK(true);
}
