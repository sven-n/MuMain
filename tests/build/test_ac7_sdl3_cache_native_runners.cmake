# AC-7: Validate SDL3 FetchContent caching on both native runners
#
# Story: 7.4.1 - Native Platform CI Runners
# Flow Code: VS0-QUAL-CI-NATIVE
# Story tag: [7-4-1]
#
# RED PHASE: Test FAILS until both native jobs include actions/cache for SDL3 _deps/.
# GREEN PHASE: Passes once both build-macos and build-linux cache SDL3 FetchContent artifacts.
#
# Validates:
#   AC-7 — SDL3 FetchContent cache step exists in build-macos job (path: out/build/macos-arm64/_deps)
#   AC-7 — SDL3 FetchContent cache step exists in build-linux job (path: out/build/linux-x64/_deps)
#   AC-7 — Cache keys use hashFiles for invalidation on CMake config changes

cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED CI_WORKFLOW_FILE)
    message(FATAL_ERROR "AC-7: CI_WORKFLOW_FILE variable not defined")
endif()

if(NOT EXISTS "${CI_WORKFLOW_FILE}")
    message(FATAL_ERROR "AC-7: ci.yml does not exist: ${CI_WORKFLOW_FILE}")
endif()

file(READ "${CI_WORKFLOW_FILE}" CI_CONTENT)

# --- Check 1: SDL3 cache path for macOS present ---
string(FIND "${CI_CONTENT}" "out/build/macos-arm64/_deps" _pos_macos_deps)
if(_pos_macos_deps EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL [7-4-1]: SDL3 FetchContent cache path 'out/build/macos-arm64/_deps' not found in ci.yml")
endif()

# --- Check 2: SDL3 cache path for Linux present ---
string(FIND "${CI_CONTENT}" "out/build/linux-x64/_deps" _pos_linux_deps)
if(_pos_linux_deps EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL [7-4-1]: SDL3 FetchContent cache path 'out/build/linux-x64/_deps' not found in ci.yml")
endif()

# --- Check 3: Cache keys use hashFiles (at least 2 — one per native job) ---
string(REGEX MATCHALL "hashFiles" _hash_matches "${CI_CONTENT}")
list(LENGTH _hash_matches _hash_count)
if(_hash_count LESS 2)
    message(FATAL_ERROR "AC-7 FAIL [7-4-1]: Expected hashFiles-based cache keys in both native jobs (found ${_hash_count} occurrences)")
endif()

# --- Check 4: actions/cache used (at least 3: quality + macos + linux) ---
string(REGEX MATCHALL "actions/cache" _cache_matches "${CI_CONTENT}")
list(LENGTH _cache_matches _cache_count)
if(_cache_count LESS 3)
    message(FATAL_ERROR "AC-7 FAIL [7-4-1]: Expected actions/cache in at least 3 jobs (quality tools + build-macos + build-linux), found ${_cache_count}")
endif()

# --- Check 5: sdl3 cache key prefix present for macOS ---
string(FIND "${CI_CONTENT}" "sdl3-macos-" _pos_sdl3_macos_key)
if(_pos_sdl3_macos_key EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL [7-4-1]: SDL3 cache key prefix 'sdl3-macos-' not found in ci.yml")
endif()

# --- Check 6: sdl3 cache key prefix present for Linux ---
string(FIND "${CI_CONTENT}" "sdl3-linux-" _pos_sdl3_linux_key)
if(_pos_sdl3_linux_key EQUAL -1)
    message(FATAL_ERROR "AC-7 FAIL [7-4-1]: SDL3 cache key prefix 'sdl3-linux-' not found in ci.yml")
endif()

message(STATUS "AC-7 PASS [7-4-1]: SDL3 FetchContent caching with hashFiles keys found for both native runners")
