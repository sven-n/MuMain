cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MU_SCENE_MANAGER_SOURCE)
    message(FATAL_ERROR "MU_SCENE_MANAGER_SOURCE must be set")
endif()

file(READ "${MU_SCENE_MANAGER_SOURCE}" scene_manager_source)

string(REGEX MATCH
    "else[ \t]+if[ \t]*\\([ \t]*world[ \t]*==[ \t]*WD_2DEVIAS[ \t]*\\)[ \t\r\n]*rgb8[ \t]*\\([ \t]*0[ \t]*,[ \t]*0[ \t]*,[ \t]*10[ \t]*\\)"
    devias_clear_color
    "${scene_manager_source}")

if(NOT devias_clear_color)
    message(FATAL_ERROR "Devias clear/fog color must remain legacy abyss navy rgb8(0, 0, 10)")
endif()
