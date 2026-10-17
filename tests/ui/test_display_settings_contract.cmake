foreach(required_var IN ITEMS MU_CONFIG_CONSTANTS MU_CONFIG_SOURCE MU_WINMAIN_SOURCE MU_CONSOLE_SOURCE)
    if(NOT DEFINED ${required_var})
        message(FATAL_ERROR "${required_var} is required")
    endif()
endforeach()

file(READ "${MU_CONFIG_CONSTANTS}" config_constants)
file(READ "${MU_CONFIG_SOURCE}" config_source)
file(READ "${MU_WINMAIN_SOURCE}" winmain_source)
file(READ "${MU_CONSOLE_SOURCE}" console_source)

foreach(required_text IN ITEMS CfgKeyVSync CfgDefaultVSync CfgKeyLook CfgDefaultLook CfgKeyAnisotropy
        CfgDefaultAnisotropy CfgKeyAntialiasing CfgDefaultAntialiasing)
    string(FIND "${config_constants}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "GameConfigConstants.h must define ${required_text}")
    endif()
endforeach()

# The look preset is the only setting of the Enhanced features: no per-feature switch.
foreach(forbidden_text IN ITEMS TextureFiltering EnhancedLighting DynamicLights PostProcessing BloomStrength
        LightMood)
    string(FIND "${config_constants}" "CfgKey${forbidden_text}" found_at)
    if(NOT found_at EQUAL -1)
        message(FATAL_ERROR "GameConfigConstants.h must not define a per-feature key CfgKey${forbidden_text}")
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

# A missing or invalid Look counts as CfgDefaultLook, which is Classic: Enhanced is opt-in.
string(FIND "${config_constants}" "CfgDefaultLook[] = L\"Classic\"" found_at)
if(found_at EQUAL -1)
    message(FATAL_ERROR "GameConfigConstants.h must default the look preset to Classic")
endif()

foreach(required_text IN ITEMS
        "m_look = ReadLookPreset(m_configPath);"
        "WriteString(CfgSectionRender, CfgKeyLook, GameConfigValidation::FormatLook(m_look))"
        "ReadInt(CfgSectionRender, CfgKeyAnisotropy, CfgDefaultAnisotropy)"
        "WriteInt(CfgSectionRender, CfgKeyAnisotropy, m_anisotropy)"
        "ReadInt(CfgSectionRender, CfgKeyAntialiasing, CfgDefaultAntialiasing)"
        "WriteInt(CfgSectionRender, CfgKeyAntialiasing, m_antialiasing)")
    string(FIND "${config_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "GameConfig.cpp must persist the look preset through: ${required_text}")
    endif()
endforeach()

# Start-up applies the preset through the one function the options window and $look use.
foreach(required_text IN ITEMS
        "Render::Look::ApplyLookPreset(GameConfig::GetInstance().GetLook());")
    string(FIND "${winmain_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Winmain.cpp must apply the look preset at startup: ${required_text}")
    endif()
endforeach()
foreach(forbidden_text IN ITEMS "LightMood::ApplyPreference();" "ApplyEnhancedLightingPreference();"
        "ApplyPostProcessingPreference();" "ApplyDynamicLightsPreference();" "GetTextureFiltering()")
    string(FIND "${winmain_source}" "${forbidden_text}" found_at)
    if(NOT found_at EQUAL -1)
        message(FATAL_ERROR "Winmain.cpp must not apply a per-feature setting: ${forbidden_text}")
    endif()
endforeach()

# $look classic|enhanced in every build; the per-feature commands only in developer builds and
# without persisting anything.
string(FIND "${console_source}" "#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)" developer_begin)
string(FIND "${console_source}" "Render::Look::HandleFeatureOverrideCommand(command)" override_position)
string(SUBSTRING "${console_source}" ${developer_begin} -1 developer_block)
string(FIND "${developer_block}" "#endif" developer_length)
math(EXPR developer_end "${developer_begin} + ${developer_length}")
string(FIND "${console_source}" "Render::Look::HandleLookPresetCommand(strCommand)" preset_position)
if(developer_begin EQUAL -1 OR override_position EQUAL -1 OR NOT developer_begin LESS override_position
   OR NOT override_position LESS developer_end)
    message(FATAL_ERROR "muConsoleDebug.cpp must offer the per-feature commands only in developer builds")
endif()
if(preset_position EQUAL -1 OR NOT developer_end LESS preset_position)
    message(FATAL_ERROR "muConsoleDebug.cpp must offer $look classic|enhanced outside the developer block")
endif()
foreach(forbidden_text IN ITEMS "SetEnhancedLightingPreference" "SetDynamicLightsPreference"
        "SetPostProcessingPreference" "MuSetTextureFilteringPreference" "LightMood::HandleCommand(")
    string(FIND "${console_source}" "${forbidden_text}" found_at)
    if(NOT found_at EQUAL -1)
        message(FATAL_ERROR "muConsoleDebug.cpp must not persist a per-feature switch: ${forbidden_text}")
    endif()
endforeach()

# Anti-aliasing belongs to the Enhanced look: start-up and the preset switch it with the look,
# and Classic renders one sample per pixel.
foreach(required_text IN ITEMS
        "MuSetAntialiasingActive(Render::Look::PresetFeatures(GameConfig::GetInstance().GetLook()).antialiasing)"
        "mu::GetRenderer().SetAntialiasingSamples(active ? GameConfig::GetInstance().GetAntialiasing() : 0)")
    string(FIND "${winmain_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "Winmain.cpp must apply anti-aliasing with the look through: ${required_text}")
    endif()
endforeach()

foreach(required_text IN ITEMS
        "MuSetAntialiasingPreference(*samples)")
    string(FIND "${console_source}" "${required_text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "muConsoleDebug.cpp must persist $msaa requests: ${required_text}")
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
string(FIND "${winmain_source}" "                RenderProfiledFrame();" begin_position)
if(apply_position EQUAL -1 OR begin_position EQUAL -1 OR NOT apply_position LESS begin_position)
    message(FATAL_ERROR "Pending VSync must be applied before BeginFrame")
endif()

string(FIND "${winmain_source}" "                Bitmaps.UpdateTextureFiltering();" filtering_position)
if(filtering_position EQUAL -1 OR NOT filtering_position LESS begin_position)
    message(FATAL_ERROR "Texture filtering switches and mip chain builds must run before BeginFrame")
endif()
