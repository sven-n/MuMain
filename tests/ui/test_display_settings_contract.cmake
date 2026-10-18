foreach(required_var IN ITEMS MU_CONFIG_CONSTANTS MU_CONFIG_HEADER MU_CONFIG_SOURCE MU_LOOK_SETTING_SOURCE
        MU_WINMAIN_SOURCE MU_CONSOLE_SOURCE)
    if(NOT DEFINED ${required_var})
        message(FATAL_ERROR "${required_var} is required")
    endif()
endforeach()

# The checks read code only: a line that is commented out counts as missing.
function(strip_cpp_comments source_variable output_variable)
    set(source "${${source_variable}}")
    string(REGEX REPLACE "/\\*([^*]|\\*+[^*/])*\\*+/" "" source "${source}")
    string(REGEX REPLACE "//[^\r\n]*" "" source "${source}")
    set(${output_variable} "${source}" PARENT_SCOPE)
endfunction()

function(read_code path output_variable)
    file(READ "${path}" source)
    strip_cpp_comments(source code)
    set(${output_variable} "${code}" PARENT_SCOPE)
endfunction()

function(require_text source_variable text failure_message)
    string(FIND "${${source_variable}}" "${text}" found_at)
    if(found_at EQUAL -1)
        message(FATAL_ERROR "${failure_message}: ${text}")
    endif()
endfunction()

function(forbid_text source_variable text failure_message)
    string(FIND "${${source_variable}}" "${text}" found_at)
    if(NOT found_at EQUAL -1)
        message(FATAL_ERROR "${failure_message}: ${text}")
    endif()
endfunction()

# The function from `signature` to its closing brace in the first column.
function(function_body source_variable signature output_variable)
    string(FIND "${${source_variable}}" "${signature}" begin)
    if(begin EQUAL -1)
        message(FATAL_ERROR "Missing function: ${signature}")
    endif()
    string(SUBSTRING "${${source_variable}}" ${begin} -1 rest)
    string(FIND "${rest}" "\n}" length)
    string(SUBSTRING "${rest}" 0 ${length} body)
    set(${output_variable} "${body}" PARENT_SCOPE)
endfunction()

# The position of the #endif that closes the #if at `begin`, past every #if/#ifdef/#ifndef inside
# it; -1 when it is missing, or when the #if has an #else or #elif of its own, since what follows
# that is not guarded by it.
function(matching_endif source_variable begin output_variable)
    set(depth 1)
    math(EXPR position "${begin} + 1")
    while(depth GREATER 0)
        string(SUBSTRING "${${source_variable}}" ${position} -1 rest)
        string(FIND "${rest}" "#if" next_if)
        string(FIND "${rest}" "#endif" next_endif)
        string(FIND "${rest}" "#el" next_else)
        if(next_endif EQUAL -1)
            set(${output_variable} -1 PARENT_SCOPE)
            return()
        endif()
        if(depth EQUAL 1 AND NOT next_else EQUAL -1 AND next_else LESS next_endif
           AND (next_if EQUAL -1 OR next_else LESS next_if))
            set(${output_variable} -1 PARENT_SCOPE)
            return()
        endif()
        if(NOT next_if EQUAL -1 AND next_if LESS next_endif)
            math(EXPR depth "${depth} + 1")
            math(EXPR position "${position} + ${next_if} + 1")
        else()
            math(EXPR depth "${depth} - 1")
            math(EXPR position "${position} + ${next_endif} + 1")
        endif()
    endwhile()
    math(EXPR position "${position} - 1")
    set(${output_variable} ${position} PARENT_SCOPE)
endfunction()

read_code("${MU_CONFIG_CONSTANTS}" config_constants)
read_code("${MU_CONFIG_HEADER}" config_header)
read_code("${MU_CONFIG_SOURCE}" config_source)
read_code("${MU_LOOK_SETTING_SOURCE}" look_setting_source)
read_code("${MU_WINMAIN_SOURCE}" winmain_source)
read_code("${MU_CONSOLE_SOURCE}" console_source)
# For checks that span lines: one space for every run of whitespace, none inside parentheses.
function(squeeze_whitespace source_variable output_variable)
    string(REGEX REPLACE "[ \t\r\n]+" " " code "${${source_variable}}")
    string(REPLACE "( " "(" code "${code}")
    string(REPLACE " )" ")" code "${code}")
    set(${output_variable} "${code}" PARENT_SCOPE)
endfunction()
squeeze_whitespace(config_header config_header_code)
squeeze_whitespace(look_setting_source look_setting_code)

foreach(required_text IN ITEMS CfgKeyVSync CfgDefaultVSync CfgKeyLook CfgDefaultLook CfgKeyAnisotropy
        CfgDefaultAnisotropy CfgKeyAntialiasing CfgDefaultAntialiasing)
    require_text(config_constants "${required_text}" "GameConfigConstants.h must define")
endforeach()

# The look preset turns the Enhanced features on; the player can switch these off one by one
# ([Render] EnhancedLighting, DynamicLights, LightMood, PostProcessing; on by default). Each switch
# is one member of GameConfig, which the options window reaches through ChooseEnhancedFeature and
# the look preset through SavedChoices.
function(require_enhanced_switch key member feature choice)
    foreach(required_text IN ITEMS "CfgKey${key}[] = L\"${key}\"" "CfgDefault${key} = true;")
        require_text(config_constants "${required_text}"
            "GameConfigConstants.h must define the Enhanced feature switch")
    endforeach()
    foreach(required_text IN ITEMS
            "${member} = ReadBool(CfgSectionRender, CfgKey${key}, CfgDefault${key});"
            "WriteBool(CfgSectionRender, CfgKey${key}, ${member});")
        require_text(config_source "${required_text}"
            "GameConfig.cpp must persist the Enhanced feature switch through")
    endforeach()
    foreach(required_text IN ITEMS
            "bool Get${key}() const { return ${member}; }"
            "void Set${key}(bool enabled) { ${member} = enabled; }")
        require_text(config_header_code "${required_text}"
            "GameConfig.h must keep the Enhanced feature switch in ${member}")
    endforeach()
    foreach(required_text IN ITEMS
            "case EnhancedFeature::${feature}: return config.Get${key}();"
            "case EnhancedFeature::${feature}: config.Set${key}(enabled); break;"
            "choices.${choice} = offOnly(config.Get${key}());")
        require_text(look_setting_code "${required_text}"
            "LookPresetSetting.cpp must map the Enhanced feature switch through")
    endforeach()
endfunction()
require_enhanced_switch(EnhancedLighting m_enhancedLighting Lighting enhancedLighting)
require_enhanced_switch(DynamicLights m_dynamicLights DynamicLights dynamicLights)
require_enhanced_switch(LightMood m_lightMood LightMood lightMood)
require_enhanced_switch(PostProcessing m_postProcessing PostProcessing postProcessing)

# Texture filtering and the bloom strength follow the look preset: no switch of their own.
foreach(feature IN ITEMS TextureFiltering BloomStrength)
    foreach(forbidden_text IN ITEMS "CfgKey${feature}" "L\"${feature}\"")
        forbid_text(config_constants "${forbidden_text}" "GameConfigConstants.h must not define a per-feature key")
    endforeach()
endforeach()

foreach(required_text IN ITEMS
        "ReadBool(CfgSectionRender, CfgKeyVSync, CfgDefaultVSync)"
        "WriteBool(CfgSectionRender, CfgKeyVSync, m_vsyncEnabled)")
    require_text(config_source "${required_text}" "GameConfig.cpp must persist VSync through")
endforeach()

# A missing or invalid Look counts as CfgDefaultLook, which is Classic: Enhanced is opt-in.
require_text(config_constants "CfgDefaultLook[] = L\"Classic\""
    "GameConfigConstants.h must default the look preset to Classic")

foreach(required_text IN ITEMS
        "m_look = ReadLookPreset(m_configPath);"
        "WriteString(CfgSectionRender, CfgKeyLook, GameConfigValidation::FormatLook(m_look))"
        "ReadInt(CfgSectionRender, CfgKeyAnisotropy, CfgDefaultAnisotropy)"
        "WriteInt(CfgSectionRender, CfgKeyAnisotropy, m_anisotropy)"
        "ReadInt(CfgSectionRender, CfgKeyAntialiasing, CfgDefaultAntialiasing)"
        "WriteInt(CfgSectionRender, CfgKeyAntialiasing, m_antialiasing)")
    require_text(config_source "${required_text}" "GameConfig.cpp must persist the look preset through")
endforeach()

# Start-up applies the preset, with the player's Enhanced switches, through the one function the
# options window and $look use.
require_text(winmain_source "Render::Look::ApplyLookPreset(GameConfig::GetInstance().GetLook());"
    "Winmain.cpp must apply the look preset at startup")
foreach(forbidden_text IN ITEMS "SetEnhancedLightingEnabled(" "SetDynamicLightsEnabled(" "SetMoodSwitch("
        "SetPostProcessingEnabled(" "GetEnhancedLighting()" "GetDynamicLights()" "GetLightMood()"
        "GetPostProcessing()")
    forbid_text(winmain_source "${forbidden_text}" "Winmain.cpp must not apply a per-feature setting")
endforeach()

# $look classic|enhanced in every build; the per-feature commands only in developer builds and
# without persisting anything.
string(FIND "${console_source}" "#if defined(MU_ENABLE_CONTROL_SOCKET) || defined(_EDITOR)" developer_begin)
if(developer_begin EQUAL -1)
    message(FATAL_ERROR "muConsoleDebug.cpp must offer the per-feature commands only in developer builds")
endif()
string(FIND "${console_source}" "Render::Look::HandleFeatureOverrideCommand(command)" override_position)
matching_endif(console_source ${developer_begin} developer_end)
string(FIND "${console_source}" "Render::Look::HandleLookPresetCommand(strCommand)" preset_position)
if(developer_end EQUAL -1 OR override_position EQUAL -1 OR NOT developer_begin LESS override_position
   OR NOT override_position LESS developer_end)
    message(FATAL_ERROR "muConsoleDebug.cpp must offer the per-feature commands only in developer builds")
endif()
if(preset_position EQUAL -1 OR NOT developer_end LESS preset_position)
    message(FATAL_ERROR "muConsoleDebug.cpp must offer $look classic|enhanced outside the developer block")
endif()
# Only the options window saves a switch.
foreach(forbidden_text IN ITEMS "ChooseEnhancedFeature(" "SetEnhancedLighting(" "SetDynamicLights("
        "SetLightMood(" "SetPostProcessing(")
    forbid_text(console_source "${forbidden_text}" "muConsoleDebug.cpp must not persist a per-feature switch")
endforeach()
function_body(look_setting_source "bool HandleFeatureOverrideCommand(std::wstring_view command)" override_command)
require_text(override_command "Controller().Override(*change);"
    "HandleFeatureOverrideCommand must apply the command as a session override")
foreach(forbidden_text IN ITEMS "ChooseEnhancedFeature(" "Save()" "SetLook(" "SetEnhancedLighting("
        "SetDynamicLights(" "SetLightMood(" "SetPostProcessing(")
    forbid_text(override_command "${forbidden_text}"
        "HandleFeatureOverrideCommand must not persist a per-feature switch")
endforeach()

# Anti-aliasing belongs to the Enhanced look: start-up and the preset switch it with the look,
# and Classic renders one sample per pixel.
foreach(required_text IN ITEMS
        "MuSetAntialiasingActive(Render::Look::PresetFeatures(GameConfig::GetInstance().GetLook()).antialiasing)"
        "mu::GetRenderer().SetAntialiasingSamples(active ? GameConfig::GetInstance().GetAntialiasing() : 0)")
    require_text(winmain_source "${required_text}" "Winmain.cpp must apply anti-aliasing with the look through")
endforeach()

require_text(console_source "MuSetAntialiasingPreference(*samples)" "muConsoleDebug.cpp must persist $msaa requests")

foreach(required_text IN ITEMS
        SDL_GetFullscreenDisplayModes
        "SDL_free(modes)"
        "MuReapplyVSyncPreference();")
    require_text(winmain_source "${required_text}" "Winmain.cpp is missing display-settings behavior")
endforeach()

foreach(required_text IN ITEMS
        "MuSetVSyncPreference(true)"
        "MuSetVSyncPreference(false)")
    require_text(console_source "${required_text}" "muConsoleDebug.cpp must persist deferred VSync requests")
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
