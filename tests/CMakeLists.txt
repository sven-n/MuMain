include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(MuTests
    core/test_timer.cpp
)
target_link_libraries(MuTests PRIVATE Catch2::Catch2WithMain MUCore)

include(CTest)
include(Catch)
catch_discover_tests(MuTests)

# Build system validation tests (CMake script mode, no Catch2)
add_subdirectory(build)
