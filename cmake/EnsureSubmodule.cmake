# mu_ensure_submodule(<relative_path>
#   INDICATOR <file_relative_to_path>
#   [URL <git_url>])
#
# Ensures a git submodule at <relative_path> (relative to the calling
# CMakeLists.txt) is initialized.  INDICATOR is a file expected to exist
# inside the submodule once it's populated; its absence triggers init.
# Init leaves the submodule's history out: git fetches the pinned commit and
# the default branch's newest commit, each without the commits before it.
# URL allows the helper to `git submodule add` the entry if it's not yet
# registered in the index — only useful for first-time bootstraps.
#
# Requires REPO_ROOT to be set in the calling scope (the repository root
# used as the working directory for git operations).

# Leaves in <out_var> the name .gitmodules gives the submodule at <path>. git names a submodule after its path
# unless .gitmodules says otherwise; the name picks the submodule's git directory and its settings.
function(_mu_submodule_name path out_var)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} config -f .gitmodules --get-regexp "^submodule\\..*\\.path$"
    WORKING_DIRECTORY ${REPO_ROOT}
    OUTPUT_VARIABLE entries
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  string(REPLACE "\n" ";" entries "${entries}")
  set(name "${path}")
  foreach(entry IN LISTS entries)
    if (entry MATCHES "^submodule\\.(.+)\\.path (.+)$")
      if (CMAKE_MATCH_2 STREQUAL path)
        set(name "${CMAKE_MATCH_1}")
      endif()
    endif()
  endforeach()
  set(${out_var} "${name}" PARENT_SCOPE)
endfunction()

# A shallow clone tracks only the default branch. `git submodule update --remote` needs the branch .gitmodules
# names for the submodule (RmlUi's integration/sdl-gpu-parity), so the clone's remote gets that branch too.
function(_mu_track_submodule_branch name submodule_dir)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} config -f .gitmodules --get "submodule.${name}.branch"
    WORKING_DIRECTORY ${REPO_ROOT}
    OUTPUT_VARIABLE branch
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  # "." stands for the superproject's current branch, not a branch of the submodule's own.
  if (branch STREQUAL "" OR branch STREQUAL ".")
    return()
  endif()
  execute_process(
    COMMAND ${GIT_EXECUTABLE} remote set-branches --add origin "${branch}"
    WORKING_DIRECTORY "${submodule_dir}"
    RESULT_VARIABLE rc
    ERROR_VARIABLE err
  )
  if (NOT rc EQUAL 0)
    message(FATAL_ERROR "Submodule ${name}: adding its branch ${branch} to the clone's remote failed:\n${err}")
  endif()
endfunction()

# Runs `git submodule update --init [<args>...] -- <path>`; leaves its exit code in update_rc and its output in
# update_output.
function(_mu_submodule_update path)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init ${ARGN} -- "${path}"
    WORKING_DIRECTORY ${REPO_ROOT}
    RESULT_VARIABLE rc
    ERROR_VARIABLE err
    OUTPUT_VARIABLE out
  )
  set(update_rc "${rc}" PARENT_SCOPE)
  set(update_output "${err}${out}" PARENT_SCOPE)
endfunction()

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
      "Git not found. Run 'git submodule update --init --depth 1' manually from ${REPO_ROOT}.")
  endif()
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --absolute-git-dir
    WORKING_DIRECTORY ${REPO_ROOT}
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE git_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (NOT rc EQUAL 0)
    message(FATAL_ERROR "Submodule ${rel_path} is missing, and ${REPO_ROOT} is not a git checkout.")
  endif()

  # Two configures of a fresh checkout at once (two IDE profiles, say) would both init the submodule, and the
  # second would delete the first's clone below. The lock lets one finish first.
  file(LOCK "${git_dir}/mu-submodule-init.lock" GUARD FUNCTION)
  if (EXISTS "${indicator_file}")
    return()
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
    _mu_submodule_name("${submodule_repo_rel}" name)
    set(module_git_dir "${git_dir}/modules/${name}")
    set(new_clone FALSE)
    if (NOT EXISTS "${module_git_dir}")
      set(new_clone TRUE)
    endif()
    # --depth 1 leaves the submodule's history out. `shallow = true` in .gitmodules would not: git then clones the
    # default branch's newest commit alone and fetches the pinned one, which is older, with all its history.
    # A git directory that already holds the history (left by `git submodule deinit`, say) keeps it.
    set(depth --depth 1)
    if (NOT new_clone AND NOT EXISTS "${module_git_dir}/shallow")
      set(depth "")
    endif()
    _mu_submodule_update("${submodule_repo_rel}" ${depth})
    # The shallow clone fetches the pinned commit by its hash. GitHub sends any commit; a server that sends only
    # branch tips refuses one that is none, as SDL's and ImGui's pins are, and the new clone then fetches the whole
    # history instead.
    if (NOT update_rc EQUAL 0 AND new_clone)
      message(STATUS "Submodule ${rel_path}: the shallow fetch failed, fetching the whole history...")
      file(REMOVE_RECURSE "${module_git_dir}" "${submodule_dir}")
      set(depth "")
      _mu_submodule_update("${submodule_repo_rel}")
    endif()
    if (NOT update_rc EQUAL 0)
      message(FATAL_ERROR "Submodule ${rel_path} init failed:\n${update_output}")
    endif()
    if (new_clone AND depth)
      _mu_track_submodule_branch("${name}" "${submodule_dir}")
    endif()
  endif()

  if (NOT EXISTS "${indicator_file}")
    message(FATAL_ERROR
      "Submodule ${rel_path} init reported success but ${MES_INDICATOR} is missing.")
  endif()

  message(STATUS "Submodule ${rel_path} initialized successfully")
endfunction()
