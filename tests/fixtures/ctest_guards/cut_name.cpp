// A canary of the CTest guards in tests/CMakeLists.txt, registered with WILL_FAIL: CTest's discovery cuts this name at
// its ';' into two entries that run no case, and the binary then has two entries for its one case. If either guard
// stopped working (a doctest update that changes its summary line, say), an entry would pass and the canary fail.
#include <doctest.h>

TEST_CASE("a canary name; cut in two")
{
    CHECK(true);
}
