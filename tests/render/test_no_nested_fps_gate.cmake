# rand_fps_check and the *FpsChecked spawns each scale a chance by the frame time once. A second one in
# the same condition, or a *FpsChecked spawn under a rand_fps_check, scales it twice: at 250 FPS it fires
# a tenth as often per second as at 25 FPS. The inner chance stays a plain rand() pick.
if(NOT DEFINED MU_SOURCE_DIR)
    message(FATAL_ERROR "MU_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE sources "${MU_SOURCE_DIR}/*.cpp")
set(found "")
foreach(path IN LISTS sources)
    file(STRINGS "${path}" lines REGEX "rand_fps_check\\(")
    foreach(line IN LISTS lines)
        if(line MATCHES "^[ \t]*//" OR line MATCHES "\\|\\|" OR line MATCHES "^(bool|int|void) ")
            continue()
        endif()
        if(line MATCHES "rand_fps_check\\([^)]*\\)[^;]*(rand_fps_check|FpsChecked)\\(")
            string(STRIP "${line}" line)
            list(APPEND found "${path}: ${line}")
        endif()
    endforeach()
endforeach()

if(found)
    list(JOIN found "\n" found)
    message(FATAL_ERROR "Nested frame-rate gates:\n${found}")
endif()
