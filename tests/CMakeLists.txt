# Test tree. New test modules go in their own subdirectory and call
# add_subdirectory() below. Keep the entry point here minimal — module
# CMakeLists.txt files own their own sources, link, and add_test() lines.

add_library(mu_test_main STATIC main.cpp)
target_include_directories(mu_test_main PUBLIC third_party/doctest)
target_compile_features(mu_test_main PUBLIC cxx_std_20)

# Helper for module CMakeLists. Usage:
#   mu_add_test(NAME <test_name> SOURCES a.cpp b.cpp LINK_LIBS lib1 lib2)
function(mu_add_test)
    cmake_parse_arguments(MAT "" "NAME" "SOURCES;LINK_LIBS" ${ARGN})
    add_executable(${MAT_NAME} ${MAT_SOURCES})
    target_link_libraries(${MAT_NAME} PRIVATE mu_test_main ${MAT_LINK_LIBS})
    target_include_directories(${MAT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/source
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest
    )
    if(WIN32 AND NOT CMAKE_HOST_WIN32)
        # Cross-compiled Windows binary on a non-Windows host — run via wine.
        if(NOT WINE_EXECUTABLE)
            find_program(WINE_EXECUTABLE wine)
            if(NOT WINE_EXECUTABLE)
                message(FATAL_ERROR
                    "BUILD_TESTING=ON on a non-Windows host requires wine to run "
                    "the cross-compiled test binaries. Install it (Ubuntu/Debian: "
                    "sudo apt-get install wine wine32) or configure with "
                    "-DBUILD_TESTING=OFF.")
            endif()
        endif()
        add_test(NAME ${MAT_NAME} COMMAND ${WINE_EXECUTABLE} $<TARGET_FILE:${MAT_NAME}>)
    else()
        add_test(NAME ${MAT_NAME} COMMAND ${MAT_NAME})
    endif()
endfunction()

add_subdirectory(text)
