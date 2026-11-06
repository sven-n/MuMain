cmake_minimum_required(VERSION 3.25)

# Downloads URL, checks it against SHA256 and unpacks the one folder the archive holds as DESTINATION, replacing what
# is there. SDL3_ttf's patch step runs it to put the libraries SDL_ttf vendors where its git submodules would be.
# DESTINATION.sha256 names the archive DESTINATION holds; while it matches, nothing is downloaded, because the patch
# step also runs again when its commands change.

foreach(required_variable URL SHA256 DESTINATION)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} must be set")
    endif()
endforeach()

set(unpacked_stamp "${DESTINATION}.sha256")
if(IS_DIRECTORY "${DESTINATION}" AND EXISTS "${unpacked_stamp}")
    file(READ "${unpacked_stamp}" unpacked_sha256)
    if(unpacked_sha256 STREQUAL SHA256)
        return()
    endif()
endif()

set(work_dir "${DESTINATION}.unpack")
file(REMOVE_RECURSE "${work_dir}")
file(DOWNLOAD "${URL}" "${work_dir}/archive.tar.gz"
    EXPECTED_HASH "SHA256=${SHA256}"
    TLS_VERIFY ON
    STATUS download_status
)
list(GET download_status 0 download_result)
if(NOT download_result EQUAL 0)
    message(FATAL_ERROR "Could not download ${URL}: ${download_status}")
endif()

# TOUCH: the files get the time of unpacking, so a build after a version change recompiles them.
file(ARCHIVE_EXTRACT INPUT "${work_dir}/archive.tar.gz" DESTINATION "${work_dir}/contents" TOUCH)
file(GLOB top_entries LIST_DIRECTORIES true "${work_dir}/contents/*")
list(LENGTH top_entries top_entry_count)
if(NOT top_entry_count EQUAL 1 OR NOT IS_DIRECTORY "${top_entries}")
    message(FATAL_ERROR "${URL} must hold exactly one folder; it holds: ${top_entries}")
endif()

file(REMOVE "${unpacked_stamp}")
file(REMOVE_RECURSE "${DESTINATION}")
file(RENAME "${top_entries}" "${DESTINATION}")
file(REMOVE_RECURSE "${work_dir}")
file(WRITE "${unpacked_stamp}" "${SHA256}")
