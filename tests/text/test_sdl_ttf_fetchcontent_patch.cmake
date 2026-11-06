cmake_minimum_required(VERSION 3.25)

foreach(required_variable MU_ROOT_CMAKE SDL3_TTF_SOURCE MU_PROPERTY_HEADER MU_PATCH)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "${required_variable} must be set")
    endif()
endforeach()

set(expected_revision "a1ce3670aec736ecbf0936c43f2f0cc53aa61e5b")
set(upload_property "MuMain.SDL_ttf.gpu_text.uploaded_glyphs")
set(gpu_text_source "${SDL3_TTF_SOURCE}/src/SDL_gpu_textengine.c")

foreach(required_file "${MU_ROOT_CMAKE}" "${gpu_text_source}" "${MU_PROPERTY_HEADER}" "${MU_PATCH}")
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "required file is missing: ${required_file}")
    endif()
endforeach()

file(READ "${MU_ROOT_CMAKE}" root_cmake)
file(READ "${gpu_text_source}" gpu_text)
file(READ "${MU_PROPERTY_HEADER}" property_header)
file(READ "${MU_PATCH}" patch)

# SDL_ttf's release archive names the commit it was made from. A git clone given as FETCHCONTENT_SOURCE_DIR_SDL3_TTF,
# e.g. the sources of a build folder from before the archive, knows its own.
if(EXISTS "${SDL3_TTF_SOURCE}/.git-hash")
    file(STRINGS "${SDL3_TTF_SOURCE}/.git-hash" source_revision LIMIT_COUNT 1)
elseif(EXISTS "${SDL3_TTF_SOURCE}/.git")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${SDL3_TTF_SOURCE}" rev-parse HEAD
        RESULT_VARIABLE git_result
        OUTPUT_VARIABLE source_revision
        ERROR_VARIABLE git_error
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT git_result EQUAL 0)
        message(FATAL_ERROR "git rev-parse HEAD failed in ${SDL3_TTF_SOURCE}: ${git_error}")
    endif()
else()
    message(FATAL_ERROR
        "${SDL3_TTF_SOURCE} names its revision neither in .git-hash (SDL_ttf's release archive) nor as a git clone")
endif()
if(NOT source_revision STREQUAL expected_revision)
    message(FATAL_ERROR "SDL3_ttf must use exact revision ${expected_revision}; its sources are ${source_revision}")
endif()

string(FIND "${root_cmake}" "PATCH_COMMAND" patch_command_position)
if(patch_command_position EQUAL -1)
    message(FATAL_ERROR "SDL3_ttf FetchContent must apply the project patch")
endif()

string(REGEX MATCHALL "SDL_SubmitGPUCommandBuffer[ \\t\\r\\n]*\\(" submit_calls "${gpu_text}")
list(LENGTH submit_calls submit_call_count)
if(NOT submit_call_count EQUAL 2)
    message(FATAL_ERROR "SDL_gpu_textengine.c must contain exactly two GPU submissions; found ${submit_call_count}")
endif()

string(REGEX MATCHALL "SDL_BeginGPUCopyPass[ \\t\\r\\n]*\\(" copy_pass_calls "${gpu_text}")
list(LENGTH copy_pass_calls copy_pass_call_count)
if(NOT copy_pass_call_count EQUAL 1)
    message(FATAL_ERROR "glyph uploads must use exactly one GPU copy pass; found ${copy_pass_call_count}")
endif()

foreach(source_name gpu_text property_header patch)
    string(FIND "${${source_name}}" "${upload_property}" property_position)
    if(property_position EQUAL -1)
        message(FATAL_ERROR "${source_name} must contain upload property ${upload_property}")
    endif()
endforeach()

# Fallback glyphs are placed at the primary font's ascent, so they share its baseline.
file(READ "${SDL3_TTF_SOURCE}/src/SDL_ttf.c" ttf_source)
string(FIND "${ttf_source}" "pos->y = y + F26Dot6(font->ascent) - pos->y_offset;" baseline_position)
if(baseline_position EQUAL -1)
    message(FATAL_ERROR "SDL_ttf.c must place fallback glyphs at the primary font's ascent")
endif()
