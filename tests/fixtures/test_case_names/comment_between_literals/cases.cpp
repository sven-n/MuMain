// The ';' sits in the literal after a comment; the compiler still joins both literals into the name.
TEST_CASE("first half " // a comment between the literals
          "second; half [tag]")
{
    CHECK(true);
}
