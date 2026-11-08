#define WRAP_CASE(name) TEST_CASE(name "; suffix")
// The file starts with a UTF-8 byte order mark: the #define above is still a preprocessor line, not a case.
TEST_CASE("a case in a file with a byte order mark")
{
    CHECK(true);
}
