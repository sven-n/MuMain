// Names CTest keeps whole. A comment that mentions TEST_CASE (like this one) is no case.
TEST_CASE("one, two [tag][other]")
{
    CHECK(true);
}

TEST_CASE("a \"quoted\" word and a \\ inside")
{
    CHECK(true);
}

TEST_CASE_FIXTURE(Scope, "a fixture's case, "
                         "in two literals [tag]")
{
    CHECK(true);
}
