// The name goes on in a macro after its literal, so the literal is not the whole name.
#define SUFFIX "; tail"

TEST_CASE("head" SUFFIX)
{
    CHECK(true);
}
