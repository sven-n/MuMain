// Names CTest keeps whole. A comment that mentions TEST_CASE("a; b") is no case, and neither is this block:
/*
TEST_CASE("an example; in a comment")
*/
// nor a string that holds one, nor a macro's definition:
const char* example = "TEST_CASE(\"in a string; no case\")";
#define CASE_WITH_SUFFIX(name) TEST_CASE(name "; suffix")

TEST_CASE("one, two [tag][other]")
{
    CHECK(true);
}

TEST_CASE("a \"quoted\" word")
{
    CHECK(true);
}

TEST_CASE_FIXTURE(Scope, "a fixture's case, " // a comment between the literals
                         "in two literals [tag]")
{
    CHECK(true);
}

TEST_CASE_FIXTURE(Holder<std::pair<int, float>, 2>, "a fixture with template arguments")
{
    CHECK(true);
}

TEST_CASE(R"(a raw name, whole)")
{
    CHECK(true);
}

TEST_CASE("a name the compiler joins across a line \
splice")
{
    CHECK(true);
}

TEST_CASE("a decorated case [slow]" * doctest::timeout(5))
{
    CHECK(true);
}

TEST_CASE_TEMPLATE("a template case [tag]", T, int, float)
{
    CHECK(true);
}

SCENARIO("a scenario, whole")
{
    CHECK(true);
}

// doctest lists it as "  Scenario: [doctest] ...", which the discovery keeps.
SCENARIO("[doctest] a scenario keeps its prefix")
{
    CHECK(true);
}

TEST_SUITE("a suite") { DOCTEST_TEST_CASE("a case after code on its line, with the prefix") {} }
