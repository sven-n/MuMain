// The ';' sits where two literals join, in a fixture's case.
TEST_CASE_FIXTURE(Scope, "one half;"
                         " the other half [tag]")
{
    CHECK(true);
}
