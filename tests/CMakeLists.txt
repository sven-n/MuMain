include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(MuTests
    core/test_timer.cpp
    platform/test_platform_types.cpp
    platform/test_platform_compat.cpp
    platform/test_platform_keys.cpp
    platform/test_platform_library.cpp
    platform/test_platform_window.cpp
)
target_link_libraries(MuTests PRIVATE Catch2::Catch2WithMain MUCore MUPlatform)

include(CTest)
include(Catch)
catch_discover_tests(MuTests)

# Build system validation tests (CMake script mode, no Catch2)
add_subdirectory(build)

# Platform abstraction header tests (Story 1.2.1)
add_subdirectory(platform)
