# rand_fps_check scales a chance by the frame time, so it belongs on work done every frame. CreateEffect,
# CreateJoint and CreateParticle run once per spawn; a gate in them makes their random variants rarer at
# high FPS, where a plain rand() pick keeps the 25 FPS odds.
if(NOT DEFINED MU_EFFECT_SOURCES)
    message(FATAL_ERROR "MU_EFFECT_SOURCES is required")
endif()

foreach(entry IN LISTS MU_EFFECT_SOURCES)
    string(REPLACE "=" ";" parts "${entry}")
    list(GET parts 0 function)
    list(GET parts 1 path)
    file(READ "${path}" source)

    string(REGEX MATCH "\n[A-Za-z][^\n;]* ${function}\\([^;]*\n{" header "${source}")
    if(NOT header)
        message(FATAL_ERROR "${function} is missing from ${path}")
    endif()
    string(FIND "${source}" "${header}" begin)
    string(SUBSTRING "${source}" ${begin} -1 rest)
    string(FIND "${rest}" "\n{" open)
    string(SUBSTRING "${rest}" ${open} -1 rest)
    string(FIND "${rest}" "\n}" end)
    string(SUBSTRING "${rest}" 0 ${end} body)

    string(FIND "${body}" "rand_fps_check(" gate)
    if(NOT gate EQUAL -1)
        message(FATAL_ERROR "${function} uses rand_fps_check; it runs once per spawn, so use a plain rand() pick")
    endif()
endforeach()
