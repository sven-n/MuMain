foreach(required_var IN ITEMS MU_CONFIG_CONSTANTS MU_CONFIG_SOURCE MU_WINMAIN_SOURCE MU_CONSOLE_SOURCE)
    if(NOT DEFINED ${required_var})
        message(FATAL_ERROR "${required_var} is required")
    endif()
endforeach()

file(READ "${MU_CONFIG_CONSTANTS}" config_constants)
file(READ "${MU_CONFIG_SOURCE}" config_source)
file(READ "${MU_WINMAIN_SOURCE}" winmain_source)
file(READ "${MU_CONSOLE_SOURCE}" console_source)

foreach(required_text IN ITEMS CfgKeyVSync CfgDefaultVSync)
    string(FIND "${config_constants}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "GameConfigConstants.h must define ${required_text}")
    endif()
endforeach()

foreach(required_text IN ITEMS
        "ReadBool(CfgSectionRender, CfgKeyVSync, CfgDefaultVSync)"
        "WriteBool(CfgSectionRender, CfgKeyVSync, m_vsyncEnabled)")
    string(FIND "${config_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "GameConfig.cpp must persist VSync through: ${required_text}")
    endif()
endforeach()

foreach(required_text IN ITEMS
        SDL_GetFullscreenDisplayModes
        "SDL_free(modes)"
        "MuReapplyVSyncPreference();")
    string(FIND "${winmain_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Winmain.cpp is missing display-settings behavior: ${required_text}")
    endif()
endforeach()

foreach(required_text IN ITEMS
        "MuSetVSyncPreference(true)"
        "MuSetVSyncPreference(false)")
    string(FIND "${console_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "muConsoleDebug.cpp must persist deferred VSync requests: ${required_text}")
    endif()
endforeach()

string(FIND "${winmain_source}" "                ApplyPendingVSyncPreference();" apply_position)
string(FIND "${winmain_source}" "                mu::GetRenderer().BeginFrame();" begin_position)
if(apply_position EQUAL -1 OR begin_position EQUAL -1 OR NOT apply_position LESS begin_position)
    message(FATAL_ERROR "Pending VSync must be applied before BeginFrame")
endif()
