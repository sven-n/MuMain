// An extra ']' joins the names after it as surely as an open '[': CMake's bracket count goes below zero.
TEST_CASE("a tag closed twice [tag]]")
{
    CHECK(true);
}
