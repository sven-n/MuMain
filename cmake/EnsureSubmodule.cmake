# mu_ensure_submodule(<relative_path>
#   INDICATOR <file_relative_to_path>
#   [URL <git_url>])
#
# Ensures a git submodule at <relative_path> (relative to the calling
# CMakeLists.txt) is initialized.  INDICATOR is a file expected to exist
# inside the submodule once it's populated; its absence triggers init.
# URL allows the helper to `git submodule add` the entry if it's not yet
# registered in the index — only useful for first-time bootstraps.
#
# Requires REPO_ROOT to be set in the calling scope (the repository root
# used as the working directory for git operations).

function(mu_ensure_submodule rel_path)
  set(oneValueArgs INDICATOR URL)
  cmake_parse_arguments(MES "" "${oneValueArgs}" "" ${ARGN})

  if (NOT MES_INDICATOR)
    message(FATAL_ERROR "mu_ensure_submodule(${rel_path}): INDICATOR is required")
  endif()
  if (NOT DEFINED REPO_ROOT)
    message(FATAL_ERROR "mu_ensure_submodule: REPO_ROOT must be set in the calling scope")
  endif()

  set(submodule_dir "${CMAKE_CURRENT_SOURCE_DIR}/${rel_path}")
  set(indicator_file "${submodule_dir}/${MES_INDICATOR}")

  if (EXISTS "${indicator_file}")
    return()
  endif()

  find_package(Git QUIET)
  if (NOT GIT_FOUND)
    message(FATAL_ERROR
      "Git not found. Run 'git submodule update --init' manually from ${REPO_ROOT}.")
  endif()

  message(STATUS "Initializing submodule ${rel_path}...")

  if (EXISTS "${submodule_dir}")
    message(STATUS "Removing incomplete ${rel_path} directory...")
    file(REMOVE_RECURSE "${submodule_dir}")
  endif()

  cmake_path(RELATIVE_PATH submodule_dir
    BASE_DIRECTORY "${REPO_ROOT}"
    OUTPUT_VARIABLE submodule_repo_rel)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} ls-files -s "${submodule_repo_rel}"
    WORKING_DIRECTORY ${REPO_ROOT}
    OUTPUT_VARIABLE in_index
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if (in_index STREQUAL "")
    if (NOT MES_URL)
      message(FATAL_ERROR
        "Submodule ${rel_path} not registered in git index and no URL provided.")
    endif()
    message(STATUS "Submodule ${rel_path} not in index, adding ${MES_URL}...")
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule add --force "${MES_URL}" "${submodule_repo_rel}"
      WORKING_DIRECTORY ${REPO_ROOT}
      RESULT_VARIABLE rc
      ERROR_VARIABLE err
      OUTPUT_VARIABLE out
    )
    if (NOT rc EQUAL 0)
      message(FATAL_ERROR "Failed to add submodule ${rel_path}:\n${err}${out}")
    endif()
  else()
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule sync -- "${submodule_repo_rel}"
      WORKING_DIRECTORY ${REPO_ROOT}
      OUTPUT_QUIET ERROR_QUIET
    )
    execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init -- "${submodule_repo_rel}"
      WORKING_DIRECTORY ${REPO_ROOT}
      RESULT_VARIABLE rc
      ERROR_VARIABLE err
      OUTPUT_VARIABLE out
    )
    if (NOT rc EQUAL 0)
      message(FATAL_ERROR "Submodule ${rel_path} init failed:\n${err}${out}")
    endif()
  endif()

  if (NOT EXISTS "${indicator_file}")
    message(FATAL_ERROR
      "Submodule ${rel_path} init reported success but ${MES_INDICATOR} is missing.")
  endif()

  message(STATUS "Submodule ${rel_path} initialized successfully")
endfunction()
