cmake_minimum_required(VERSION 3.25)

# mu_ensure_submodule leaves a submodule's history out. The library here has three commits and the superproject pins
# the middle one, as the real submodules pin a commit behind their branch's newest: the commit before the pinned one
# must not arrive (the newest one does, without its history).

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
set(superproject "${MU_WORK_DIR}/superproject")
file(REMOVE_RECURSE "${MU_WORK_DIR}")
file(MAKE_DIRECTORY "${library}" "${superproject}")

# Without the user's git configuration (GIT_CONFIG_GLOBAL needs git 2.32). Git refuses file:// URLs for submodules unless protocol.file.allow says so.
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

run_git("${library}" init -q)
foreach(number IN ITEMS 1 2 3)
    file(WRITE "${library}/indicator.txt" "${number}\n")
    run_git("${library}" add indicator.txt)
    run_git("${library}" commit -q -m "Commit ${number}")
    run_git("${library}" rev-parse HEAD)
    set(commit_${number} "${git_output}")
endforeach()

# A file:// URL, not a path: git ignores --depth for a local path.
if(library MATCHES "^/")
    set(library_url "file://${library}")
else()
    set(library_url "file:///${library}")
endif()
run_git("${superproject}" init -q)
file(WRITE "${superproject}/.gitmodules" "[submodule \"library\"]\n\tpath = library\n\turl = ${library_url}\n")
run_git("${superproject}" update-index --add --cacheinfo "160000,${commit_2},library")
run_git("${superproject}" add .gitmodules)
run_git("${superproject}" commit -q -m "Pin commit 2")

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/EnsureSubmodule.cmake")
set(REPO_ROOT "${superproject}")
set(CMAKE_CURRENT_SOURCE_DIR "${superproject}")
mu_ensure_submodule("library" INDICATOR "indicator.txt")

run_git("${superproject}/library" rev-parse HEAD)
if(NOT git_output STREQUAL commit_2)
    message(FATAL_ERROR "The submodule has ${git_output} checked out, not the pinned ${commit_2}")
endif()
execute_process(
    COMMAND "${GIT_EXECUTABLE}" cat-file -e "${commit_1}^{commit}"
    WORKING_DIRECTORY "${superproject}/library"
    RESULT_VARIABLE has_commit_1
    OUTPUT_QUIET ERROR_QUIET
)
if(has_commit_1 EQUAL 0)
    message(FATAL_ERROR
        "The submodule holds the commit before the pinned one: mu_ensure_submodule must leave the submodule's history "
        "out (git submodule update --init --depth 1).")
endif()

file(REMOVE_RECURSE "${MU_WORK_DIR}")
