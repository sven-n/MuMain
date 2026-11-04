// Names CTest keeps whole. A comment that mentions TEST_CASE (like this one) is no case, and neither is this block:
/*
TEST_CASE("an example; in a comment")
*/
TEST_CASE("one, two [tag][other]")
{
    CHECK(true);
}

TEST_CASE("a \"quoted\" word and a \\ inside")
{
    CHECK(true);
}

TEST_CASE_FIXTURE(Scope, "a fixture's case, " // a comment between the literals
                         "in two literals [tag]")
{
    CHECK(true);
}
