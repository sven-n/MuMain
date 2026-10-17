cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MU_RENDERER_SOURCE)
    message(FATAL_ERROR "MU_RENDERER_SOURCE must be set")
endif()

file(READ "${MU_RENDERER_SOURCE}" renderer_source)

function(require_source source pattern failure_message)
    string(REGEX MATCH "${pattern}" match "${source}")
    if(NOT match)
        message(FATAL_ERROR "${failure_message}")
    endif()
endfunction()

function(reject_source source pattern failure_message)
    string(REGEX MATCH "${pattern}" match "${source}")
    if(match)
        message(FATAL_ERROR "${failure_message}")
    endif()
endfunction()

function(require_pipeline_descriptor helper destination set_name depth_test depth_write layout cull)
    string(CONCAT pattern
        "${helper}[ \t\r\n]*\\([ \t\r\n]*${destination}\\[i\\][ \t\r\n]*,[ \t\r\n]*blendState[ \t\r\n]*,[ \t\r\n]*"
        "\\{\"${set_name}\"[ \t\r\n]*,[ \t\r\n]*table\\[i\\]\\.name[ \t\r\n]*,[ \t\r\n]*i[ \t\r\n]*,[ \t\r\n]*"
        "${depth_test}[ \t\r\n]*,[ \t\r\n]*${depth_write}[ \t\r\n]*,[ \t\r\n]*VertexLayout::${layout}[ \t\r\n]*,[ \t\r\n]*"
        "${cull}[ \t\r\n]*\\}[ \t\r\n]*\\);"
    )
    require_source("${create_source}" "${pattern}"
        "${destination} must remain ${helper} with its exact pipeline descriptor")
endfunction()

function(require_shader_load_assignment handle asset stage stage_enum)
    string(CONCAT pattern
        "${handle}[ \t\r\n]*=[ \t\r\n]*createShader[ \t\r\n]*\\([ \t\r\n]*\"${asset}\"[ \t\r\n]*,[ \t\r\n]*"
        "\"${stage}\"[ \t\r\n]*,[ \t\r\n]*SDL_GPU_SHADERSTAGE_${stage_enum}[ \t\r\n]*,"
    )
    require_source("${executable_load_source}" "${pattern}" "${handle} must load ${asset}.${stage}")
endfunction()

string(FIND "${renderer_source}" "[[nodiscard]] static bool Init(void* pNativeWindow, std::string_view fontFamily" init_start)
string(FIND "${renderer_source}" "[[nodiscard]] static bool LoadShaders(const char* driverName)" load_start)
string(FIND "${renderer_source}" "static void ReleaseShaders()" release_start)
string(FIND "${renderer_source}" "[[nodiscard]] static SDL_GPUDepthStencilState MakeDepthStencilState" depth_state_start)
string(FIND "${renderer_source}" "[[nodiscard]] static PipelineBuildResult BuildBlendPipeline" build_start)
string(FIND "${renderer_source}" "static void LogPipelineFailure" log_start)
string(FIND "${renderer_source}" "[[nodiscard]] static bool CreatePipelines()" create_start)
string(FIND "${renderer_source}" "static void DestroyPipelines()" destroy_start)
if(init_start EQUAL -1
        OR load_start EQUAL -1
        OR release_start EQUAL -1
        OR depth_state_start EQUAL -1
        OR build_start LESS depth_state_start
        OR log_start EQUAL -1
        OR create_start EQUAL -1
        OR destroy_start EQUAL -1)
    message(FATAL_ERROR "renderer pipeline lifecycle functions are missing")
endif()

math(EXPR load_length "${release_start} - ${load_start}")
math(EXPR depth_state_length "${build_start} - ${depth_state_start}")
math(EXPR build_length "${log_start} - ${build_start}")
math(EXPR log_length "${create_start} - ${log_start}")
math(EXPR init_length "${create_start} - ${init_start}")
math(EXPR create_length "${destroy_start} - ${create_start}")
string(SUBSTRING "${renderer_source}" ${load_start} ${load_length} load_source)
string(SUBSTRING "${renderer_source}" ${depth_state_start} ${depth_state_length} depth_state_source)
string(SUBSTRING "${renderer_source}" ${build_start} ${build_length} build_source)
string(SUBSTRING "${renderer_source}" ${log_start} ${log_length} log_source)
string(SUBSTRING "${renderer_source}" ${init_start} ${init_length} init_source)
string(SUBSTRING "${renderer_source}" ${create_start} ${create_length} create_source)
string(REGEX REPLACE "//[^\r\n]*" "" executable_load_source "${load_source}")
string(REGEX REPLACE "/\\*[^*]*\\*/" "" executable_load_source "${executable_load_source}")

require_source("${renderer_source}" "struct[ \t\r\n]+PipelineBuildResult" "pipeline builds must return structured results")
require_source("${renderer_source}" "PipelineBuildResult[ \t\r\n]+BuildBlendPipeline" "BuildBlendPipeline must return PipelineBuildResult")
require_source("${renderer_source}" "PipelineBuildDescription[ \t\r\n]+description;" "pipeline results must retain build state")
require_source("${renderer_source}" "std::string[ \t\r\n]+error;" "pipeline results must retain the SDL error")
require_source("${renderer_source}" "result\\.error[ \t\r\n]*=[^;]*SDL_GetError" "pipeline builds must capture the immediate SDL error")
require_source("${renderer_source}" "constexpr[ \t]+int[ \t]+k_PipelineCount[ \t]*=[ \t]*9;"
    "the nine blend variants must remain explicit")
require_shader_load_assignment(s_vertShader2D basic_textured vert VERTEX)
require_shader_load_assignment(s_fragShaderTex basic_textured frag FRAGMENT)
require_shader_load_assignment(s_vertShaderSkinned skinned_textured vert VERTEX)
require_shader_load_assignment(s_vertShaderSkinnedShadow skinned_shadow vert VERTEX)
require_source("${executable_load_source}"
    "s_vertShaderSkinnedShadow[ \t\r\n]*=[^;]*createShader[^;]*\"skinned_shadow\"[^;]*SDL_GPU_SHADERSTAGE_VERTEX[^;]*false[ \t\r\n]*\\);"
    "skinned_shadow.vert must be optional")
require_source("${executable_load_source}"
    "s_vertShaderSkinned[ \t\r\n]*=[^;]*createShader[^;]*\"skinned_textured\"[^;]*SDL_GPU_SHADERSTAGE_VERTEX[^;]*true[ \t\r\n]*\\);"
    "skinned_textured.vert must be fatal")
require_shader_load_assignment(s_fragShaderTexFog basic_textured_fog frag FRAGMENT)
require_shader_load_assignment(s_fragShaderLit skinned_lit frag FRAGMENT)
require_source("${executable_load_source}"
    "s_fragShaderTexFog[ \t\r\n]*=[^;]*createShader[^;]*\"basic_textured_fog\"[^;]*false[ \t\r\n]*\\);"
    "basic_textured_fog.frag must be optional")
require_source("${executable_load_source}"
    "s_fragShaderLit[ \t\r\n]*=[^;]*createShader[^;]*\"skinned_lit\"[^;]*false[ \t\r\n]*\\);"
    "skinned_lit.frag must be optional")
require_source("${executable_load_source}"
    "if[ \t\r\n]*\\(!s_vertShaderSkinned\\)[ \t\r\n]*\\{[^}]*ReleaseShaders[ \t\r\n]*\\([^}]*return false;"
    "skinned shader failure must release earlier shader handles")
require_source("${create_source}"
    "for[ \t]*\\([ \t]*int[ \t]+i[ \t]*=[ \t]*0;[ \t]*i[ \t]*<[ \t]*k_PipelineCount;[ \t]*\\+\\+i[ \t]*\\)"
    "pipeline creation must iterate over every blend variant")

string(REGEX MATCHALL "buildRequiredPipeline[ \t\r\n]*\\(" required_calls "${create_source}")
string(REGEX MATCHALL "buildOptionalPipeline[ \t\r\n]*\\(" optional_calls "${create_source}")
list(LENGTH required_calls required_call_count)
list(LENGTH optional_calls optional_call_count)
# Nine required Classic sets; optional: the 2D depth-on set, the two GPU body-shadow sets, and the four
# calls of one Enhanced lighting family (buildEnhancedSet), which builds the three Enhanced families.
if(NOT required_call_count EQUAL 9 OR NOT optional_call_count EQUAL 7)
    message(FATAL_ERROR "CreatePipelines must build exactly nine required sets and the optional 2D, shadow and Enhanced sets")
endif()
string(REGEX MATCHALL "buildEnhancedSet[ \t\r\n]*\\([ \t\r\n]*s_pipelines[A-Za-z0-9]+" enhanced_calls "${create_source}")
set(expected_enhanced_calls)
foreach(enhanced_set IN ITEMS s_pipelines3DEnhancedFog s_pipelinesSkinnedEnhancedFog s_pipelinesSkinnedLit)
    string(REGEX MATCH "buildEnhancedSet[ \t\r\n]*\\([ \t\r\n]*${enhanced_set}[ \t\r\n]*," enhanced_call "${create_source}")
    if(NOT enhanced_call)
        message(FATAL_ERROR "CreatePipelines must build the Enhanced lighting set ${enhanced_set}")
    endif()
endforeach()
list(LENGTH enhanced_calls enhanced_call_count)
if(NOT enhanced_call_count EQUAL 3)
    message(FATAL_ERROR "CreatePipelines must build exactly three Enhanced lighting sets")
endif()
require_source("${create_source}" "s_enhancedLightingAvailable[ \t\r\n]*=[^;]*IsComplete"
    "Enhanced lighting must be available only when every Enhanced set is complete")

require_pipeline_descriptor(buildOptionalPipeline s_pipelines2D "2d-depth-on" true true TwoDimensional false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelines2DDepthOff "2d-depth-off" false false TwoDimensional false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelines3D "3d-culled" true true ThreeDimensional true)
require_pipeline_descriptor(buildRequiredPipeline s_pipelines3DNoCull "3d-no-cull" true true ThreeDimensional false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelines3DDepthOff "3d-depth-off" false false ThreeDimensional false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelines3DDepthReadOnly "3d-depth-read-only" true false ThreeDimensional false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelinesSkinned "skinned-culled" true true Skinned true)
require_pipeline_descriptor(buildRequiredPipeline s_pipelinesSkinnedNoCull "skinned-no-cull" true true Skinned false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelinesSkinnedDepthOff "skinned-depth-off" false false Skinned false)
require_pipeline_descriptor(buildRequiredPipeline s_pipelinesSkinnedDepthReadOnly "skinned-depth-read-only" true false Skinned false)
require_pipeline_descriptor(buildOptionalPipeline s_pipelinesSkinnedShadow "skinned-shadow" true false SkinnedShadow false)
require_pipeline_descriptor(buildOptionalPipeline s_pipelinesSkinnedShadowDepthOff "skinned-shadow-depth-off" false false SkinnedShadow false)
reject_source("${create_source}" "if[ \t\r\n]*\\([ \t\r\n]*s_vertShaderSkinned[ \t\r\n]*\\)"
    "required skinned pipelines must not be guarded")
require_source("${create_source}" "if[ \t\r\n]*\\([ \t\r\n]*s_vertShaderSkinnedShadow[ \t\r\n]*\\)"
    "shadow pipelines must be skipped without their optional shader")

require_source("${create_source}" "requiredFailureCount[ \t\r\n]*!=[ \t\r\n]*0" "required failures must be aggregated")
require_source("${create_source}" "firstRequiredError" "the first required SDL error must be retained")
require_source("${create_source}" "first error:[^;]*firstRequiredError" "the first required SDL error must be summarized")
require_source("${renderer_source}" "driver=\\{\\}" "pipeline diagnostics must include the driver")
require_source("${renderer_source}" "set=\\{\\}" "pipeline diagnostics must include the set")
require_source("${renderer_source}" "layout=\\{\\}" "pipeline diagnostics must include the layout")
require_source("${renderer_source}" "blend=\\{\\}" "pipeline diagnostics must include the blend mode")
require_source("${renderer_source}" "index=\\{\\}" "pipeline diagnostics must include the blend index")
require_source("${renderer_source}" "depth_test=\\{\\}" "pipeline diagnostics must include depth-test state")
require_source("${renderer_source}" "depth_write=\\{\\}" "pipeline diagnostics must include depth-write state")
require_source("${renderer_source}" "cull=\\{\\}" "pipeline diagnostics must include cull state")
require_source("${renderer_source}" "error=\\{\\}" "pipeline diagnostics must include the SDL error")
require_source("${build_source}"
    "pipelineInfo\\.vertex_shader[ \t\r\n]*=[ \t\r\n]*description\\.vertexLayout[ \t\r\n]*==[ \t\r\n]*VertexLayout::Skinned[ \t\r\n]*\\?[ \t\r\n]*s_vertShaderSkinned[ \t\r\n]*:[ \t\r\n]*description\\.vertexLayout[ \t\r\n]*==[ \t\r\n]*VertexLayout::SkinnedShadow[ \t\r\n]*\\?[ \t\r\n]*s_vertShaderSkinnedShadow[ \t\r\n]*:[ \t\r\n]*s_vertShader2D"
    "pipeline vertex shader handles must remain selected from the pipeline layout")
require_source("${build_source}"
    "pipelineInfo\\.fragment_shader[ \t\r\n]*=[ \t\r\n]*GetFragmentShader\\(description\\.fragmentShader\\)"
    "pipeline fragment shader handles must be selected from the pipeline description")
require_source("${renderer_source}"
    "FragmentShader[ \t\r\n]+fragmentShader[ \t\r\n]*=[ \t\r\n]*FragmentShader::Textured;"
    "pipeline descriptions must default to basic_textured.frag")
require_source("${renderer_source}"
    "case FragmentShader::Textured:[ \t\r\n]*break;[ \t\r\n]*\\}[ \t\r\n]*return s_fragShaderTex;"
    "the Textured fragment shader must remain basic_textured.frag")
# The classic client compares depth with GL_LEQUAL. With strict LESS, a later depth-writing pass
# over the same mesh (the Bill of Balrog's textured body over its chrome body) loses every
# equal-depth fragment.
require_source("${build_source}"
    "pipelineInfo\\.depth_stencil_state[ \t\r\n]*=[ \t\r\n]*MakeDepthStencilState[ \t\r\n]*\\([ \t\r\n]*description[ \t\r\n]*\\)"
    "every pipeline must take its depth state from MakeDepthStencilState")
require_source("${depth_state_source}" "\\.compare_op[ \t\r\n]*=[ \t\r\n]*SDL_GPU_COMPAREOP_LESS_OR_EQUAL;"
    "pipelines must compare depth with LESS_OR_EQUAL, as the classic client's GL_LEQUAL")
reject_source("${renderer_source}" "SDL_GPU_COMPAREOP_LESS([^_]|$)"
    "no pipeline may compare depth with strict LESS")
require_source("${log_source}"
    "vertexShaderName[ \t\r\n]*=[ \t\r\n]*description\\.vertexLayout[ \t\r\n]*==[ \t\r\n]*VertexLayout::Skinned[ \t\r\n]*\\?[ \t\r\n]*\"skinned_textured\\.vert\"[ \t\r\n]*:[ \t\r\n]*description\\.vertexLayout[ \t\r\n]*==[ \t\r\n]*VertexLayout::SkinnedShadow[ \t\r\n]*\\?[ \t\r\n]*\"skinned_shadow\\.vert\"[ \t\r\n]*:[ \t\r\n]*\"basic_textured\\.vert\""
    "pipeline diagnostics must map the built vertex layout to its exact shader identifier")
require_source("${log_source}"
    "fragmentShaderName[ \t\r\n]*=[ \t\r\n]*GetFragmentShaderName\\(description\\.fragmentShader\\)"
    "pipeline diagnostics must name the exact fragment shader identifier")

string(REGEX REPLACE "\"[ \t\r\n]*\"" "" compact_log_source "${log_source}")
string(REGEX REPLACE "[ \t\r\n]+" " " compact_log_source "${compact_log_source}")
set(log_fields
    "driver={} set={} layout={} vertex_shader={} fragment_shader={} blend={} index={} depth_test={} depth_write={} cull={} error={}")
set(log_arguments
    "driverName, description\\.setName, layoutName, vertexShaderName, fragmentShaderName, description\\.blendModeName, description\\.blendIndex, description\\.depthTestEnabled, description\\.depthWriteEnabled, cullMode, result\\.error")
require_source("${compact_log_source}" "required pipeline failed: ${log_fields}"
    "required pipeline diagnostics must include exact vertex and fragment shader fields")
require_source("${compact_log_source}" "optional pipeline failed: ${log_fields}"
    "optional pipeline diagnostics must include exact vertex and fragment shader fields")
string(REGEX MATCHALL "${log_arguments}" shader_log_arguments "${compact_log_source}")
list(LENGTH shader_log_arguments shader_log_argument_count)
if(NOT shader_log_argument_count EQUAL 2)
    message(FATAL_ERROR "required and optional pipeline diagnostics must pass both shader identifier values")
endif()
require_source("${create_source}" "(^|[\r\n])[ \t]*return false;[ \t]*([\r\n]|$)"
    "CreatePipelines must contain an executable failure return")
string(REGEX REPLACE "(^|[\r\n])[ \t]*//[^\r\n]*" "\\1" executable_create_source "${create_source}")
string(FIND "${executable_create_source}" "if (requiredFailureCount != 0)" required_failure_branch)
string(FIND "${executable_create_source}" "return false;" required_failure_return)
if(required_failure_branch EQUAL -1 OR required_failure_return LESS required_failure_branch)
    message(FATAL_ERROR "CreatePipelines must reject required failures")
endif()

# Init builds through CreatePipelinesForSampleCount, which retries with one
# sample when multisampled pipelines fail and otherwise returns CreatePipelines'.
string(FIND "${init_source}" "if (!CreatePipelinesForSampleCount())" failure_branch_start)
string(FIND "${init_source}" "// Shaders are no longer needed" failure_branch_end)
if(failure_branch_start EQUAL -1 OR failure_branch_end LESS failure_branch_start)
    message(FATAL_ERROR "CreatePipelines failure branch is missing")
endif()
math(EXPR failure_branch_length "${failure_branch_end} - ${failure_branch_start}")
string(SUBSTRING "${init_source}" ${failure_branch_start} ${failure_branch_length} failure_branch_source)
require_source("${failure_branch_source}" "(^|[\r\n])[ \t]*DestroyPipelines\\(\\);[ \t]*([\r\n]|$)"
    "pipeline failure cleanup must be an executable statement")
string(REGEX REPLACE "(^|[\r\n])[ \t]*//[^\r\n]*" "\\1" executable_failure_branch "${failure_branch_source}")
string(FIND "${executable_failure_branch}" "DestroyPipelines();" cleanup_position)
string(FIND "${executable_failure_branch}" "ReleaseShaders();" shader_release_position)
string(FIND "${executable_failure_branch}" "SDL_ReleaseWindowFromGPUDevice" window_release_position)
string(FIND "${executable_failure_branch}" "SDL_DestroyGPUDevice" device_destroy_position)
if(cleanup_position EQUAL -1
        OR shader_release_position LESS cleanup_position
        OR window_release_position LESS shader_release_position
        OR device_destroy_position LESS window_release_position)
    message(FATAL_ERROR "pipeline failure cleanup must precede shader, window, and device teardown")
endif()

string(FIND "${renderer_source}" "RenderSkinnedTriangles(std::span" skinned_start)
string(FIND "${renderer_source}" "const std::optional<BonePaletteSlot> palette" skinned_pipeline_end)
if(skinned_start EQUAL -1 OR skinned_pipeline_end LESS skinned_start)
    message(FATAL_ERROR "RenderSkinnedTriangles pipeline selection is missing")
endif()
math(EXPR skinned_length "${skinned_pipeline_end} - ${skinned_start}")
string(SUBSTRING "${renderer_source}" ${skinned_start} ${skinned_length} skinned_source)
require_source("${skinned_source}" "if[ \t\r\n]*\\(!pipeline\\)[ \t\r\n]*\\{[ \t\r\n]*return false;"
    "RenderSkinnedTriangles must reject missing required pipelines")
