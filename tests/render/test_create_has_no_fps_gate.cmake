# rand_fps_check and FPS_ANIMATION_FACTOR scale by the frame time, so they belong on work done every frame.
# CreateEffect, CreateJoint and CreateParticle run once per spawn: a gate in them makes their random
# variants rarer at high FPS, and the factor shrinks spawn offsets, start angles and start rates.
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

    # The trail length still makes up for tails laid once per move.
    string(REPLACE "MaxTails / FPS_ANIMATION_FACTOR" "" scaled "${body}")
    string(FIND "${scaled}" "FPS_ANIMATION_FACTOR" factor)
    if(NOT factor EQUAL -1)
        message(FATAL_ERROR "${function} uses FPS_ANIMATION_FACTOR; it runs once per spawn, so its values take none")
    endif()
endforeach()
