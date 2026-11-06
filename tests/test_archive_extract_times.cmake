cmake_minimum_required(VERSION 3.25)

# Every archive the build unpacks must give its files the time of unpacking, not the archive's older dates: with
# those, a version change brings sources older than the last build's objects, and ninja keeps the objects. So each
# FetchContent_Declare with a URL sets DOWNLOAD_EXTRACT_TIMESTAMP FALSE. CMP0135 would make that the default, but a
# declare copied from elsewhere with TRUE would bring the old dates back unnoticed. SDL3_ttf's patch step unpacks
# archives with cmake/UnpackArchive.cmake, which passes TOUCH for the same reason.

if(NOT DEFINED MU_ROOT_CMAKE)
    message(FATAL_ERROR "MU_ROOT_CMAKE is required")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/FetchContentDeclare.cmake")
mu_read_fetchcontent_declare_names("${MU_ROOT_CMAKE}" names)
set(archives "")
foreach(name IN LISTS names)
    mu_read_fetchcontent_declare("${MU_ROOT_CMAKE}" "${name}" cmake_code declare declare_at)
    if(NOT declare MATCHES "[ \t\r\n]URL[ \t\r\n]")
        continue()
    endif()
    list(APPEND archives "${name}")
    if(NOT declare MATCHES "[ \t\r\n]DOWNLOAD_EXTRACT_TIMESTAMP[ \t\r\n]+(FALSE|OFF|NO|0)[ \t\r\n)]")
        message(FATAL_ERROR
            "FetchContent_Declare(${name}) downloads an archive, so it must set DOWNLOAD_EXTRACT_TIMESTAMP FALSE: "
            "with the archive's dates, a build after a version change keeps the objects compiled from the previous "
            "sources")
    endif()
endforeach()
if(NOT archives)
    message(FATAL_ERROR "Found no FetchContent_Declare with a URL in ${MU_ROOT_CMAKE} (declares: ${names})")
endif()

file(READ "${CMAKE_CURRENT_LIST_DIR}/../cmake/UnpackArchive.cmake" unpack_script)
if(NOT unpack_script MATCHES "file\\(ARCHIVE_EXTRACT[^)]*[ \t\r\n]TOUCH[ \t\r\n)]")
    message(FATAL_ERROR
        "cmake/UnpackArchive.cmake must unpack with file(ARCHIVE_EXTRACT ... TOUCH), so the files get the time of "
        "unpacking, not the archives' dates")
endif()
