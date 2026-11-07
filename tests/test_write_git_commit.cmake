cmake_minimum_required(VERSION 3.25)

# cmake/WriteGitCommit.cmake on a small repository: the commit, whether tracked files changed, and the data release
# id. The id must be the one data-assets.yml and ci.yml compute in bash,
#   printf '%s\n%s\n' "$data_tree" "$fonts_tree" | sha256sum | cut -c1-16
# which for this repository's two trees (eee046f2..., ea3430a8...) is e0404a47611c6276. The files have no line
# ending: file(WRITE) writes CRLF on Windows, which would change the trees.

if(NOT DEFINED MU_WORK_DIR OR NOT DEFINED MU_SCRIPT)
    message(FATAL_ERROR "MU_WORK_DIR and MU_SCRIPT are required")
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

set(repo "${MU_WORK_DIR}/repo")
set(header "${MU_WORK_DIR}/MuGitCommit.h")
file(REMOVE_RECURSE "${MU_WORK_DIR}")
file(MAKE_DIRECTORY "${repo}/src/bin/Data" "${repo}/src/bin/fonts")
file(WRITE "${repo}/src/bin/Data/a.txt" "data")
file(WRITE "${repo}/src/bin/fonts/b.ttf" "font")

# Without the user's git configuration (autocrlf would change the trees), and without the repository the build
# folder sits in: git stops looking for one at the work folder.
file(WRITE "${MU_WORK_DIR}/gitconfig" "")
set(ENV{GIT_CONFIG_NOSYSTEM} 1)
set(ENV{GIT_CONFIG_GLOBAL} "${MU_WORK_DIR}/gitconfig")
set(ENV{GIT_CEILING_DIRECTORIES} "${MU_WORK_DIR}")
set(ENV{GIT_CONFIG_COUNT} 2)
set(ENV{GIT_CONFIG_KEY_0} "user.name")
set(ENV{GIT_CONFIG_VALUE_0} "WriteGitCommit test")
set(ENV{GIT_CONFIG_KEY_1} "user.email")
set(ENV{GIT_CONFIG_VALUE_1} "write-git-commit@example.invalid")

function(run_git)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" ${ARGN}
        WORKING_DIRECTORY "${repo}"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "git ${ARGN} exited ${result}:\n${error}${output}")
    endif()
    set(git_output "${output}" PARENT_SCOPE)
endfunction()

# Runs the script on the repository and leaves the header in header_text.
function(write_header)
    execute_process(
        COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${repo}" "-DOUTPUT=${header}" -P "${MU_SCRIPT}"
        RESULT_VARIABLE result
        ERROR_VARIABLE error
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "WriteGitCommit.cmake exited ${result}:\n${error}")
    endif()
    file(READ "${header}" text)
    set(header_text "${text}" PARENT_SCOPE)
endfunction()

function(expect_define name value when)
    string(FIND "${header_text}" "#define ${name} ${value}\n" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "${when}: expected #define ${name} ${value}, the header is:\n${header_text}")
    endif()
endfunction()

write_header()
expect_define(MU_GIT_COMMIT "\"unknown\"" "Outside a repository")
expect_define(MU_DATA_ASSET_ID "\"\"" "Outside a repository")

run_git(init -q)
run_git(add .)
run_git(commit -q -m "Data and fonts")
run_git(rev-parse HEAD)
set(head "${git_output}")

write_header()
expect_define(MU_GIT_COMMIT "\"${head}\"" "A clean checkout")
expect_define(MU_GIT_COMMIT_CHANGED 0 "A clean checkout")
expect_define(MU_DATA_ASSET_ID "\"data-e0404a47611c6276\"" "A clean checkout")

# A changed tracked file marks the build; the data id stays the commit's.
file(WRITE "${repo}/src/bin/Data/a.txt" "changed")
write_header()
expect_define(MU_GIT_COMMIT_CHANGED 1 "A changed Data file")
expect_define(MU_DATA_ASSET_ID "\"data-e0404a47611c6276\"" "A changed Data file")
