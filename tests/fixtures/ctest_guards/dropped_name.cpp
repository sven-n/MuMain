// A canary of the entry count in tests/CMakeLists.txt, registered with WILL_FAIL: CTest's discovery drops this empty
// name, so the binary has no entry for its one case and its count entry must fail.
#include <doctest.h>

TEST_CASE("")
{
    CHECK(true);
}
