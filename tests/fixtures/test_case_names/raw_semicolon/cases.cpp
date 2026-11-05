// A raw string is read as written: its ';' cuts the name like any other.
TEST_CASE(R"x(a raw; name)x")
{
    CHECK(true);
}
