cmake_minimum_required(VERSION 3.25)

# Unpacks the one folder of an archive into DESTINATION, replacing what is in it. SDL3_ttf's patch step runs it first
# for SDL_ttf's own release archive, which the download step keeps (ARCHIVE), and then for each library SDL_ttf vendors
# (URL), to put it where its git submodule would be. A library's archive is kept in ARCHIVE_DIR and downloaded only
# when it is missing or differs from SHA256; its archives of other commits are removed there. Every run unpacks again,
# so the patches after it start from the archives' sources also when the patch step runs again, e.g. after a patch
# changed.

if(DEFINED ARCHIVE)
    set(required_variables ARCHIVE DESTINATION)
else()
    set(required_variables URL SHA256 ARCHIVE_DIR DESTINATION)
endif()
foreach(required_variable IN LISTS required_variables)
    if(NOT DEFINED ${required_variable} OR "${${required_variable}}" STREQUAL "")
        message(FATAL_ERROR "${required_variable} must be set")
    endif()
endforeach()

if(DEFINED ARCHIVE)
    if(NOT EXISTS "${ARCHIVE}")
        message(FATAL_ERROR
            "${ARCHIVE} is missing. The download step keeps it there; remove the sub-build folder that holds it, so "
            "the next configure downloads it again.")
    endif()
    set(archive "${ARCHIVE}")
else()
    get_filename_component(library "${DESTINATION}" NAME)
    get_filename_component(archive_name "${URL}" NAME)
    set(archive "${ARCHIVE_DIR}/${library}-${archive_name}")
endif()

# The configure's TLS and netrc settings (CMAKE_TLS_VERIFY, CMAKE_TLS_CAINFO, CMAKE_TLS_VERSION, CMAKE_NETRC,
# CMAKE_NETRC_FILE) come as -D values, as ExternalProject hands them to its own downloads; file(DOWNLOAD) reads them.
# Without a setting, TLS is verified.
if(NOT DEFINED CMAKE_TLS_VERIFY AND "$ENV{CMAKE_TLS_VERIFY}" STREQUAL "")
    set(CMAKE_TLS_VERIFY ON)
endif()

if(NOT DEFINED ARCHIVE AND EXISTS "${archive}")
    file(SHA256 "${archive}" archive_sha256)
    if(NOT archive_sha256 STREQUAL SHA256)
        file(REMOVE "${archive}")
    endif()
endif()
if(NOT EXISTS "${archive}")
    # Like ExternalProject's own downloads, a failed connection (no name, no connection, a broken answer, a timeout, a
    # failed TLS handshake) is tried again after a pause, and a stalled download times out. So are a transfer that
    # breaks off (curl 18, 56, 92) and an HTTP answer of 408, 429 or 5xx (curl 22), which GitHub's archive service
    # gives now and then; other HTTP errors, such as a 404 for a wrong commit, stop at once. EXPECTED_HASH would make
    # the first failure final, so the hash is checked afterwards.
    set(retry_results 6 7 8 15 18 28 35 56 92)
    foreach(attempt RANGE 1 5)
        file(DOWNLOAD "${URL}" "${archive}.part"
            INACTIVITY_TIMEOUT 60
            STATUS download_status
            LOG download_log
        )
        list(GET download_status 0 download_result)
        set(try_again FALSE)
        if(download_result IN_LIST retry_results)
            set(try_again TRUE)
        elseif(download_result EQUAL 22)
            # The last answer in the log is the one after the redirects.
            string(REGEX MATCHALL "HTTP/[0-9.]+ [0-9][0-9][0-9]" http_answers "${download_log}")
            list(POP_BACK http_answers http_answer)
            if(http_answer)
                string(APPEND download_status " (${http_answer})")
            endif()
            if(NOT http_answer OR http_answer MATCHES " (408|429|5[0-9][0-9])$")
                set(try_again TRUE)
            endif()
        endif()
        if(NOT try_again OR attempt EQUAL 5)
            break()
        endif()
        math(EXPR pause_seconds "${attempt} * 5")
        message(STATUS "Downloading ${URL} failed (${download_status}); trying again in ${pause_seconds} s")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep ${pause_seconds})
    endforeach()
    if(NOT download_result EQUAL 0)
        file(REMOVE "${archive}.part")
        message(FATAL_ERROR "Could not download ${URL}: ${download_status}")
    endif()
    file(SHA256 "${archive}.part" downloaded_sha256)
    if(NOT downloaded_sha256 STREQUAL SHA256)
        file(REMOVE "${archive}.part")
        message(FATAL_ERROR "${URL} has the SHA-256 ${downloaded_sha256}, not ${SHA256}")
    endif()
    file(RENAME "${archive}.part" "${archive}")
endif()

# Archives of the library's other commits, kept from before a version change, go: the download folder is part of
# every build folder and of CI's library caches.
if(NOT DEFINED ARCHIVE)
    file(GLOB library_archives "${ARCHIVE_DIR}/${library}-*.tar.gz" "${ARCHIVE_DIR}/${library}-*.tar.gz.part")
    foreach(library_archive IN LISTS library_archives)
        get_filename_component(library_archive_name "${library_archive}" NAME)
        if(NOT library_archive_name STREQUAL "${library}-${archive_name}")
            file(REMOVE "${library_archive}")
        endif()
    endforeach()
endif()

set(work_dir "${DESTINATION}.unpack")
file(REMOVE_RECURSE "${work_dir}")
# TOUCH: the files get the time of unpacking, so a build after a version change recompiles them.
file(ARCHIVE_EXTRACT INPUT "${archive}" DESTINATION "${work_dir}" TOUCH)
file(GLOB top_entries LIST_DIRECTORIES true "${work_dir}/*")
list(LENGTH top_entries top_entry_count)
if(NOT top_entry_count EQUAL 1 OR NOT IS_DIRECTORY "${top_entries}")
    message(FATAL_ERROR "${archive} must hold exactly one folder; it holds: ${top_entries}")
endif()

# DESTINATION itself stays and only what is in it is replaced: for SDL_ttf's own archive it is the folder the patch
# step runs in, which Windows doesn't let a process remove.
file(MAKE_DIRECTORY "${DESTINATION}")
file(GLOB old_entries LIST_DIRECTORIES true "${DESTINATION}/*")
if(old_entries)
    file(REMOVE_RECURSE ${old_entries})
endif()
file(GLOB new_entries LIST_DIRECTORIES true "${top_entries}/*")
foreach(new_entry IN LISTS new_entries)
    get_filename_component(entry_name "${new_entry}" NAME)
    file(RENAME "${new_entry}" "${DESTINATION}/${entry_name}")
endforeach()
file(REMOVE_RECURSE "${work_dir}")
