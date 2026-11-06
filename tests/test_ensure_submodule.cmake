cmake_minimum_required(VERSION 3.25)

# mu_ensure_submodule on a library laid out like the real submodules: the pinned commit is on a side branch, which
# .gitmodules names, and not on the default branch.
#
#   main: 1 - 2 - 3
#   side:   \- s1 - s2      (s1 pinned)
#
# A new submodule leaves the history out: commit 1, before the pinned one, must not arrive (main's newest commit
# does, without its history). `git submodule update --remote` then moves it to s2. A submodule whose git directory
# already holds the history keeps it, and a server that sends only branch tips gets the whole history fetched.

if(NOT DEFINED MU_WORK_DIR)
    message(FATAL_ERROR "MU_WORK_DIR is required")
endif()
find_package(Git REQUIRED)

# A git hook that runs this test exports GIT_DIR and the like, which would point the git calls below at the
# developer's repository. git lists their names.
execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --local-env-vars
    OUTPUT_VARIABLE repository_variables
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REPLACE "\n" ";" repository_variables "${repository_variables}")
foreach(variable IN LISTS repository_variables)
    unset(ENV{${variable}})
endforeach()

set(library "${MU_WORK_DIR}/library")
file(REMOVE_RECURSE "${MU_WORK_DIR}")
file(MAKE_DIRECTORY "${library}")

# Without the user's git configuration (GIT_CONFIG_GLOBAL needs git 2.32). Git refuses file:// URLs for submodules
# unless protocol.file.allow says so.
file(WRITE "${MU_WORK_DIR}/gitconfig" "")
set(ENV{GIT_CONFIG_NOSYSTEM} 1)
set(ENV{GIT_CONFIG_GLOBAL} "${MU_WORK_DIR}/gitconfig")
set(ENV{GIT_CONFIG_COUNT} 4)
set(ENV{GIT_CONFIG_KEY_0} "protocol.file.allow")
set(ENV{GIT_CONFIG_VALUE_0} "always")
set(ENV{GIT_CONFIG_KEY_1} "init.defaultBranch")
set(ENV{GIT_CONFIG_VALUE_1} "main")
set(ENV{GIT_CONFIG_KEY_2} "user.name")
set(ENV{GIT_CONFIG_VALUE_2} "EnsureSubmodule test")
set(ENV{GIT_CONFIG_KEY_3} "user.email")
set(ENV{GIT_CONFIG_VALUE_3} "ensure-submodule@example.invalid")

# Runs git in <dir> and leaves its output in git_output.
function(run_git dir)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" ${ARGN}
        WORKING_DIRECTORY "${dir}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "git ${ARGN} in ${dir} exited ${result}:\n${error}${output}")
    endif()
    set(git_output "${output}" PARENT_SCOPE)
endfunction()

# Commits <name> on the library's current branch and leaves its hash in commit_<name>.
function(commit name)
    file(WRITE "${library}/indicator.txt" "${name}\n")
    run_git("${library}" add indicator.txt)
    run_git("${library}" commit -q -m "Commit ${name}")
    run_git("${library}" rev-parse HEAD)
    set(commit_${name} "${git_output}" PARENT_SCOPE)
endfunction()

run_git("${library}" init -q)
commit(1)
run_git("${library}" checkout -q -b side)
commit(s1)
commit(s2)
run_git("${library}" checkout -q main)
commit(2)
commit(3)

# A file:// URL, not a path: git ignores --depth for a local path.
if(library MATCHES "^/")
    set(library_url "file://${library}")
else()
    set(library_url "file:///${library}")
endif()

# Makes a superproject in <dir> whose submodule `library` is pinned to <pinned> and follows the side branch.
function(make_superproject dir pinned)
    file(MAKE_DIRECTORY "${dir}")
    run_git("${dir}" init -q)
    file(WRITE "${dir}/.gitmodules"
        "[submodule \"library\"]\n\tpath = library\n\turl = ${library_url}\n\tbranch = side\n")
    run_git("${dir}" update-index --add --cacheinfo "160000,${pinned},library")
    run_git("${dir}" add .gitmodules)
    run_git("${dir}" commit -q -m "Pin ${pinned}")
endfunction()

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/EnsureSubmodule.cmake")
function(ensure_library superproject)
    set(REPO_ROOT "${superproject}")
    set(CMAKE_CURRENT_SOURCE_DIR "${superproject}")
    mu_ensure_submodule("library" INDICATOR "indicator.txt")
endfunction()

function(expect_checked_out superproject expected what)
    run_git("${superproject}/library" rev-parse HEAD)
    if(NOT git_output STREQUAL expected)
        message(FATAL_ERROR "The submodule has ${git_output} checked out, not ${what} ${expected}")
    endif()
endfunction()

# Leaves in has_commit whether <superproject>'s submodule holds <commit>.
function(holds superproject commit)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" cat-file -e "${commit}^{commit}"
        WORKING_DIRECTORY "${superproject}/library"
        RESULT_VARIABLE result
        OUTPUT_QUIET ERROR_QUIET
    )
    if(result EQUAL 0)
        set(has_commit TRUE PARENT_SCOPE)
    else()
        set(has_commit FALSE PARENT_SCOPE)
    endif()
endfunction()

# A new submodule.
set(fresh "${MU_WORK_DIR}/fresh")
make_superproject("${fresh}" "${commit_s1}")
ensure_library("${fresh}")
expect_checked_out("${fresh}" "${commit_s1}" "the pinned")
holds("${fresh}" "${commit_1}")
if(has_commit)
    message(FATAL_ERROR
        "The submodule holds the commit before the pinned one: mu_ensure_submodule must leave the submodule's history "
        "out (git submodule update --init --depth 1).")
endif()
execute_process(
    COMMAND "${GIT_EXECUTABLE}" submodule update --remote -- library
    WORKING_DIRECTORY "${fresh}"
    RESULT_VARIABLE result
    ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "git submodule update --remote failed: the shallow clone must fetch the branch .gitmodules names.\n${error}")
endif()
expect_checked_out("${fresh}" "${commit_s2}" "the side branch's newest")

# A submodule with its whole history, its folder emptied by `git submodule deinit` and the pin moved to a commit the
# clone does not have yet.
set(full "${MU_WORK_DIR}/full")
make_superproject("${full}" "${commit_s1}")
run_git("${full}" submodule update --init -- library)
run_git("${library}" checkout -q side)
commit(s3)
run_git("${library}" checkout -q main)
run_git("${full}" update-index --cacheinfo "160000,${commit_s3},library")
run_git("${full}" commit -q -m "Pin s3")
run_git("${full}" submodule deinit -f -- library)
ensure_library("${full}")
expect_checked_out("${full}" "${commit_s3}" "the pinned")
run_git("${full}/library" rev-parse --is-shallow-repository)
if(NOT git_output STREQUAL "false")
    message(FATAL_ERROR "The submodule lost its history: mu_ensure_submodule must keep a whole history whole.")
endif()

# A server that sends only branch tips, as git's protocol version 0 does without uploadpack.allowReachableSHA1InWant:
# s1 is no tip any more.
set(tips_only "${MU_WORK_DIR}/tips-only")
make_superproject("${tips_only}" "${commit_s1}")
set(ENV{GIT_CONFIG_COUNT} 5)
set(ENV{GIT_CONFIG_KEY_4} "protocol.version")
set(ENV{GIT_CONFIG_VALUE_4} "0")
ensure_library("${tips_only}")
set(ENV{GIT_CONFIG_COUNT} 4)
expect_checked_out("${tips_only}" "${commit_s1}" "the pinned")

file(REMOVE_RECURSE "${MU_WORK_DIR}")
