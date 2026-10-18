cmake_minimum_required(VERSION 3.25)

if(NOT DEFINED MU_SOURCE_DIR)
    message(FATAL_ERROR "MU_SOURCE_DIR is required")
endif()

file(READ "${MU_SOURCE_DIR}/CMakeLists.txt" cmake_source)
file(READ "${MU_SOURCE_DIR}/MuEditor/Core/MuEditorCore.h" editor_header)
file(READ "${MU_SOURCE_DIR}/MuEditor/Core/MuEditorCore.cpp" editor_source)
file(READ "${MU_SOURCE_DIR}/source/App/Platform/Windows/Winmain.cpp" app_source)
file(READ "${MU_SOURCE_DIR}/source/Render/Renderer/MuRendererSDLGpu.cpp" renderer_source)
file(READ "${MU_SOURCE_DIR}/source/Scenes/SceneManager.cpp" scene_manager_source)
file(READ "${MU_SOURCE_DIR}/source/Scenes/LoadingScene.cpp" loading_scene_source)

function(strip_cpp_comments source_variable output_variable)
    set(source "${${source_variable}}")
    string(REGEX REPLACE "/\\*([^*]|\\*+[^*/])*\\*+/" "" source "${source}")
    string(REGEX REPLACE "//[^\r\n]*" "" source "${source}")
    set(${output_variable} "${source}" PARENT_SCOPE)
endfunction()

function(require_match source_variable pattern description)
    string(REGEX MATCH "${pattern}" match "${${source_variable}}")
    if(NOT match)
        message(FATAL_ERROR "Missing SDL_GPU editor contract: ${description}")
    endif()
endfunction()

function(forbid_symbol source_variable symbol)
    string(FIND "${${source_variable}}" "${symbol}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "Forbidden SDL_GPU editor symbol remains: ${symbol}")
    endif()
endfunction()

function(require_call_count source_variable call count description)
    string(REGEX MATCHALL "${call}[ \t\r\n]*\\(" calls "${${source_variable}}")
    list(LENGTH calls actual_count)
    if(NOT actual_count EQUAL count)
        message(FATAL_ERROR
            "Invalid SDL_GPU editor contract: ${description}; expected ${count}, found ${actual_count}")
    endif()
endfunction()

function(strip_editor_sections source_variable output_variable)
    set(source "${${source_variable}}")
    string(REPLACE ";" "__MU_CMAKE_SEMICOLON__" source "${source}")
    string(REPLACE "\n" ";" lines "${source}")
    set(editor_depth 0)
    set(keep_editor_else false)
    set(output "")
    foreach(line IN LISTS lines)
        if(editor_depth GREATER 0)
            if(line MATCHES "^[ \t]*#[ \t]*(if|ifdef|ifndef)([ \t]|$)")
                if(keep_editor_else)
                    string(APPEND output "${line}\n")
                endif()
                math(EXPR editor_depth "${editor_depth} + 1")
            elseif(line MATCHES "^[ \t]*#[ \t]*else([ \t]|$)")
                if(editor_depth EQUAL 1)
                    set(keep_editor_else true)
                elseif(keep_editor_else)
                    string(APPEND output "${line}\n")
                endif()
            elseif(line MATCHES "^[ \t]*#[ \t]*endif([ \t]|$)")
                if(editor_depth GREATER 1 AND keep_editor_else)
                    string(APPEND output "${line}\n")
                endif()
                math(EXPR editor_depth "${editor_depth} - 1")
                if(editor_depth EQUAL 0)
                    set(keep_editor_else false)
                endif()
            elseif(keep_editor_else)
                string(APPEND output "${line}\n")
            endif()
        elseif(line MATCHES "^[ \t]*#[ \t]*(ifdef[ \t]+_EDITOR|if[ \t]+defined[ \t]*\\([ \t]*_EDITOR[ \t]*\\))")
            set(editor_depth 1)
        else()
            string(APPEND output "${line}\n")
        endif()
    endforeach()
    string(REPLACE "__MU_CMAKE_SEMICOLON__" ";" output "${output}")
    set(${output_variable} "${output}" PARENT_SCOPE)
endfunction()

set(editor_else_probe [[
#ifdef _EDITOR
RenderEditorOverlayPass
#else
g_MuEditorCore
#endif
]])
strip_editor_sections(editor_else_probe editor_else_probe_off)
require_match(editor_else_probe_off "g_MuEditorCore" "editor-OFF extraction retains an outer _EDITOR #else branch")
forbid_symbol(editor_else_probe_off "RenderEditorOverlayPass")

function(require_editor_before_cursor source_variable start_marker end_marker description)
    string(FIND "${${source_variable}}" "${start_marker}" region_start)
    if(region_start EQUAL -1)
        message(FATAL_ERROR "Could not isolate ${description}: missing start marker")
    endif()
    string(SUBSTRING "${${source_variable}}" ${region_start} -1 region_tail)
    string(FIND "${region_tail}" "${end_marker}" region_end)
    if(region_end EQUAL -1)
        message(FATAL_ERROR "Could not isolate ${description}: missing end marker")
    endif()
    string(SUBSTRING "${region_tail}" 0 ${region_end} region)
    require_call_count(region "g_MuEditorCore\\.Render" 1 "${description} queues exactly one editor marker")
    require_call_count(region "RenderCursor" 1 "${description} records exactly one game cursor")
    string(FIND "${region}" "g_MuEditorCore.Render()" editor_position)
    string(FIND "${region}" "RenderCursor()" cursor_position)
    if(editor_position EQUAL -1 OR cursor_position EQUAL -1 OR NOT editor_position LESS cursor_position)
        message(FATAL_ERROR "${description} must queue the editor marker before game cursor commands")
    endif()
endfunction()

string(REGEX REPLACE "#[^\r\n]*" "" cmake_code "${cmake_source}")
strip_cpp_comments(editor_header editor_header_code)
strip_cpp_comments(editor_source editor_code)
strip_cpp_comments(app_source app_code)
strip_cpp_comments(renderer_source renderer_code)
strip_cpp_comments(scene_manager_source scene_manager_code)
strip_cpp_comments(loading_scene_source loading_scene_code)

require_match(cmake_code
    "ThirdParty/imgui/backends/imgui_impl_sdlgpu3\\.cpp"
    "src/CMakeLists.txt compiles imgui_impl_sdlgpu3.cpp")
forbid_symbol(cmake_code "imgui_impl_opengl2.cpp")

require_match(editor_header_code
    "void[ \t\r\n]+Initialize[ \t\r\n]*\\([ \t\r\n]*SDL_Window[ \t]*\\*[ \t]*window[ \t\r\n]*\\)[ \t]*;"
    "MuEditorCore exposes window-only Initialize(SDL_Window*)")
require_match(editor_header_code
    "void[ \t\r\n]+PrepareDrawData[ \t\r\n]*\\([ \t\r\n]*SDL_GPUCommandBuffer[ \t]*\\*[ \t]*commandBuffer[ \t\r\n]*\\)[ \t]*;"
    "MuEditorCore exposes the SDL_GPU draw-data prepare hook")
require_match(editor_header_code
    "void[ \t\r\n]+RenderDrawData[ \t\r\n]*\\([ \t\r\n]*SDL_GPUCommandBuffer[ \t]*\\*[ \t]*commandBuffer[ \t]*,[ \t\r\n]*SDL_GPURenderPass[ \t]*\\*[ \t]*renderPass[ \t\r\n]*\\)[ \t]*;"
    "MuEditorCore exposes the SDL_GPU draw-data render hook")
require_match(editor_header_code "bool[ \t]+m_bDrawDataReady[ \t]*;" "MuEditorCore tracks current-frame draw-data readiness")
foreach(symbol IN ITEMS "glContext" "SDL_GLContext")
    forbid_symbol(editor_header_code "${symbol}")
endforeach()

foreach(symbol IN ITEMS
        "imgui_impl_opengl2.h"
        "ImGui_ImplOpenGL2"
        "ImGui_ImplSDL3_InitForOpenGL"
        "glContext")
    forbid_symbol(editor_code "${symbol}")
endforeach()

require_match(editor_code
    "void[ \t\r\n]+CMuEditorCore::Initialize[ \t\r\n]*\\([ \t\r\n]*SDL_Window[ \t]*\\*[ \t]*window[ \t\r\n]*\\)"
    "MuEditorCore defines window-only initialization")
require_match(editor_code "mu::GetRenderer\\(\\)\\.GetDevice\\(\\)" "editor obtains the live renderer device")
require_match(editor_code "SDL_GetGPUSwapchainTextureFormat[ \t\r\n]*\\(" "editor obtains the swapchain color format")
require_match(editor_code
    "if[ \t\r\n]*\\([ \t\r\n]*![ \t\r\n]*ImGui_ImplSDL3_InitForSDLGPU[ \t\r\n]*\\("
    "editor checks ImGui SDL3 platform initialization")
require_match(editor_code
    "if[ \t\r\n]*\\([ \t\r\n]*![ \t\r\n]*ImGui_ImplSDLGPU3_Init[ \t\r\n]*\\("
    "editor checks ImGui SDL_GPU renderer initialization")
require_match(editor_code "initInfo\\.Device[ \t]*=[ \t]*device[ \t]*;" "renderer init uses the live device")
require_match(editor_code
    "initInfo\\.ColorTargetFormat[ \t]*=[ \t]*colorTargetFormat[ \t]*;"
    "renderer init uses the swapchain color format")
require_match(editor_code
    "initInfo\\.MSAASamples[ \t]*=[ \t]*SDL_GPU_SAMPLECOUNT_1[ \t]*;"
    "renderer init uses SDL_GPU_SAMPLECOUNT_1")
require_match(editor_code "ImGui_ImplSDLGPU3_NewFrame[ \t\r\n]*\\(\\)[ \t]*;" "editor starts the SDL_GPU frame")
require_match(editor_code "ImGui::Render[ \t\r\n]*\\(\\)[ \t]*;" "editor finalizes ImGui draw data")

string(FIND "${editor_code}" "void CMuEditorCore::Update()" editor_update_start)
string(FIND "${editor_code}" "void CMuEditorCore::Render()" editor_render_start)
string(FIND "${editor_code}" "void CMuEditorCore::PrepareDrawData" editor_prepare_start)
string(FIND "${editor_code}" "void CMuEditorCore::RenderDrawData" editor_draw_start)
if(editor_update_start EQUAL -1 OR editor_render_start EQUAL -1 OR editor_prepare_start EQUAL -1
        OR editor_draw_start EQUAL -1 OR NOT editor_update_start LESS editor_render_start
        OR NOT editor_render_start LESS editor_prepare_start OR NOT editor_prepare_start LESS editor_draw_start)
    message(FATAL_ERROR "Could not isolate Task 4.2 MuEditorCore hooks")
endif()

math(EXPR editor_update_length "${editor_render_start} - ${editor_update_start}")
string(SUBSTRING "${editor_code}" ${editor_update_start} ${editor_update_length} editor_update)
require_match(editor_update
    "if[ \t\r\n]*\\([ \t]*![ \t]*m_bFrameStarted[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*m_bDrawDataReady[ \t]*=[ \t]*false[ \t]*;[ \t\r\n]*ImGui_ImplSDLGPU3_NewFrame"
    "starting a new editor frame invalidates unconsumed stale draw data")

math(EXPR editor_render_length "${editor_prepare_start} - ${editor_render_start}")
string(SUBSTRING "${editor_code}" ${editor_render_start} ${editor_render_length} editor_render)
string(FIND "${editor_render}" "ImGui::Render()" imgui_render_position)
string(FIND "${editor_render}" "m_bDrawDataReady = true" draw_ready_position)
string(FIND "${editor_render}" "mu::QueueEditorRenderCommand()" queue_marker_position)
string(FIND "${editor_render}" "m_bFrameStarted = false" frame_finished_position)
if(imgui_render_position EQUAL -1 OR draw_ready_position EQUAL -1 OR queue_marker_position EQUAL -1
        OR frame_finished_position EQUAL -1 OR NOT imgui_render_position LESS draw_ready_position
        OR NOT draw_ready_position LESS queue_marker_position OR NOT queue_marker_position LESS frame_finished_position)
    message(FATAL_ERROR "Editor Render() must finalize, mark ready, queue the marker, then finish the frame")
endif()
require_call_count(editor_render "mu::QueueEditorRenderCommand" 1
    "editor Render() must queue exactly one editor marker")

math(EXPR editor_prepare_length "${editor_draw_start} - ${editor_prepare_start}")
string(SUBSTRING "${editor_code}" ${editor_prepare_start} ${editor_prepare_length} editor_prepare)
require_match(editor_prepare
    "if[ \t\r\n]*\\([ \t]*![ \t]*m_bDrawDataReady[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*return[ \t]*;[ \t\r\n]*\\}"
    "prepare hook ignores stale draw data")
require_match(editor_prepare
    "ImGui_ImplSDLGPU3_PrepareDrawData[ \t\r\n]*\\([ \t\r\n]*ImGui::GetDrawData[ \t\r\n]*\\(\\)[ \t]*,[ \t\r\n]*commandBuffer[ \t\r\n]*\\)[ \t]*;"
    "prepare hook uploads current ImGui draw data with the engine command buffer")
forbid_symbol(editor_prepare "ImGui_ImplSDLGPU3_RenderDrawData")

string(SUBSTRING "${editor_code}" ${editor_draw_start} -1 editor_draw)
require_match(editor_draw
    "if[ \t\r\n]*\\([ \t]*![ \t]*m_bDrawDataReady[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*return[ \t]*;[ \t\r\n]*\\}"
    "render hook ignores stale draw data")
string(FIND "${editor_draw}" "ImGui_ImplSDLGPU3_RenderDrawData" backend_draw_position)
string(FIND "${editor_draw}" "m_bDrawDataReady = false" draw_consumed_position)
if(backend_draw_position EQUAL -1 OR draw_consumed_position EQUAL -1
        OR NOT backend_draw_position LESS draw_consumed_position)
    message(FATAL_ERROR "Editor render hook must consume then clear current-frame draw data")
endif()
require_match(editor_draw
    "ImGui_ImplSDLGPU3_RenderDrawData[ \t\r\n]*\\([ \t\r\n]*ImGui::GetDrawData[ \t\r\n]*\\(\\)[ \t]*,[ \t\r\n]*commandBuffer[ \t]*,[ \t\r\n]*renderPass[ \t\r\n]*\\)[ \t]*;"
    "render hook draws current ImGui data in the engine render pass")

string(FIND "${editor_code}" "void CMuEditorCore::Shutdown()" editor_shutdown_start)
string(FIND "${editor_code}" "void CMuEditorCore::Update()" editor_shutdown_end)
if(editor_shutdown_start EQUAL -1 OR editor_shutdown_end EQUAL -1 OR editor_shutdown_end LESS editor_shutdown_start)
    message(FATAL_ERROR "Could not isolate CMuEditorCore::Shutdown()")
endif()
math(EXPR editor_shutdown_length "${editor_shutdown_end} - ${editor_shutdown_start}")
string(SUBSTRING "${editor_code}" ${editor_shutdown_start} ${editor_shutdown_length} editor_shutdown)
string(FIND "${editor_shutdown}" "mu::WaitForSDLGpuIdle()" idle_position)
string(FIND "${editor_shutdown}" "ImGui_ImplSDLGPU3_Shutdown()" renderer_backend_position)
string(FIND "${editor_shutdown}" "ImGui_ImplSDL3_Shutdown()" platform_backend_position)
if(idle_position EQUAL -1 OR renderer_backend_position EQUAL -1 OR platform_backend_position EQUAL -1)
    message(FATAL_ERROR "Editor shutdown must wait for GPU idle and shut down both ImGui backends")
endif()
if(NOT idle_position LESS renderer_backend_position
        OR NOT renderer_backend_position LESS platform_backend_position)
    message(FATAL_ERROR "Editor shutdown must wait for GPU idle before shutting down renderer then platform backends")
endif()

foreach(symbol IN ITEMS
        "SDL_BeginGPURenderPass"
        "SDL_SubmitGPUCommandBuffer")
    forbid_symbol(editor_code "${symbol}")
    forbid_symbol(app_code "${symbol}")
endforeach()

# The editor is not a command of the game's list: it is drawn after RmlUi, in a pass of its own.
forbid_symbol(renderer_code "RenderCmdType::EditorOverlay")

string(FIND "${renderer_code}" "void QueueEditorRenderCommand()" queue_function_start)
string(FIND "${renderer_code}" "static void RenderEditorOverlayPass" overlay_pass_start)
string(FIND "${renderer_code}" "class MuRendererSDLGpu" renderer_class_start)
if(queue_function_start EQUAL -1 OR overlay_pass_start EQUAL -1 OR renderer_class_start EQUAL -1
        OR NOT queue_function_start LESS overlay_pass_start OR NOT overlay_pass_start LESS renderer_class_start)
    message(FATAL_ERROR "Could not isolate the editor queue function and its overlay pass")
endif()
math(EXPR queue_function_length "${overlay_pass_start} - ${queue_function_start}")
string(SUBSTRING "${renderer_code}" ${queue_function_start} ${queue_function_length} queue_function)
require_match(queue_function
    "if[ \t\r\n]*\\([ \t]*![ \t]*s_frameActive[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*return[ \t]*;[ \t\r\n]*\\}[ \t\r\n]*s_editorDrawPending[ \t]*=[ \t]*true[ \t]*;"
    "queue function marks the current frame's editor draw as pending")
forbid_symbol(queue_function "s_renderCmds")

string(FIND "${renderer_code}" "static std::vector<Uint16> s_stripIdxScratch" overlay_pass_end)
if(overlay_pass_end EQUAL -1 OR NOT overlay_pass_start LESS overlay_pass_end)
    message(FATAL_ERROR "Could not isolate the editor overlay pass")
endif()
math(EXPR overlay_pass_length "${overlay_pass_end} - ${overlay_pass_start}")
string(SUBSTRING "${renderer_code}" ${overlay_pass_start} ${overlay_pass_length} overlay_pass)
require_match(overlay_pass
    "static[ \t]+void[ \t]+RenderEditorOverlayPass[ \t\r\n]*\\([ \t\r\n]*SDL_GPUCommandBuffer[ \t]*\\*[ \t]*commandBuffer[ \t]*,[ \t\r\n]*SDL_GPUTexture[ \t]*\\*[ \t]*colorTexture[ \t\r\n]*\\)"
    "editor overlay pass is file-local")
require_call_count(overlay_pass "g_MuEditorCore\\.PrepareDrawData" 1 "editor overlay pass uploads the draw data once")
require_call_count(overlay_pass "SDL_BeginGPURenderPass" 1 "editor overlay pass opens exactly one render pass")
require_call_count(overlay_pass "g_MuEditorCore\\.RenderDrawData" 1 "editor overlay pass draws the draw data once")
require_match(overlay_pass "colorTarget\\.load_op[ \t]*=[ \t]*SDL_GPU_LOADOP_LOAD"
    "editor overlay pass loads the frame, RmlUi included, instead of clearing it")
require_match(overlay_pass
    "SDL_BeginGPURenderPass[ \t\r\n]*\\([ \t]*commandBuffer[ \t]*,[ \t]*&colorTarget[ \t]*,[ \t]*1[ \t]*,[ \t]*nullptr[ \t]*\\)"
    "editor overlay pass has no depth target, as the ImGui pipeline")
string(FIND "${overlay_pass}" "g_MuEditorCore.PrepareDrawData" overlay_prepare_position)
string(FIND "${overlay_pass}" "SDL_BeginGPURenderPass" overlay_begin_position)
string(FIND "${overlay_pass}" "g_MuEditorCore.RenderDrawData" overlay_draw_position)
string(FIND "${overlay_pass}" "SDL_EndGPURenderPass" overlay_end_position)
if(NOT overlay_prepare_position LESS overlay_begin_position OR NOT overlay_begin_position LESS overlay_draw_position
        OR NOT overlay_draw_position LESS overlay_end_position)
    message(FATAL_ERROR "Editor overlay pass must upload, open its pass, draw, then close it")
endif()

string(FIND "${renderer_code}" "void EndFrame() override" end_frame_start)
string(FIND "${renderer_code}" "[[nodiscard]] bool RequestFramePixels() override" end_frame_end)
if(end_frame_start EQUAL -1 OR end_frame_end EQUAL -1 OR NOT end_frame_start LESS end_frame_end)
    message(FATAL_ERROR "Could not isolate MuRendererSDLGpu::EndFrame()")
endif()
math(EXPR end_frame_length "${end_frame_end} - ${end_frame_start}")
string(SUBSTRING "${renderer_code}" ${end_frame_start} ${end_frame_length} end_frame)

# EndFrame replays the recorded frame through file-local helpers: one pass for a Classic frame, or
# scene, bloom and UI passes for a post-processed one. The helpers sit right before EndFrame.
string(FIND "${renderer_code}" "struct ReplayView" frame_replay_start)
if(frame_replay_start EQUAL -1 OR NOT frame_replay_start LESS end_frame_start)
    message(FATAL_ERROR "Could not isolate the frame replay helpers")
endif()
math(EXPR frame_replay_length "${end_frame_end} - ${frame_replay_start}")
string(SUBSTRING "${renderer_code}" ${frame_replay_start} ${frame_replay_length} frame_replay)
string(FIND "${frame_replay}" "static void ReplayCommands" replay_commands_start)
string(FIND "${frame_replay}" "static SDL_GPUDepthStencilTargetInfo MakeDepthTarget" replay_commands_end)
if(replay_commands_start EQUAL -1 OR replay_commands_end EQUAL -1
        OR NOT replay_commands_start LESS replay_commands_end)
    message(FATAL_ERROR "Could not isolate ReplayCommands()")
endif()
math(EXPR replay_commands_length "${replay_commands_end} - ${replay_commands_start}")
string(SUBSTRING "${frame_replay}" ${replay_commands_start} ${replay_commands_length} replay_commands)

# EndFrame begins one render pass itself: the post-RmlUi pass that draws content which must sit
# above RmlUi (cursor, legacy text overlays) and therefore LOADs on top of it. The frame's own
# passes go through the replay helpers.
require_call_count(end_frame "SDL_BeginGPURenderPass" 1
    "EndFrame must begin only the post-RmlUi pass itself; the frame passes go through the replay helpers")
require_match(end_frame "postUiColorTarget\\.load_op = SDL_GPU_LOADOP_LOAD"
    "EndFrame's post-RmlUi pass must load the main pass and RmlUi content rather than clear it")
require_call_count(end_frame "SDL_SubmitGPUCommandBuffer" 1 "EndFrame must keep one direct frame submission")
require_call_count(end_frame "SDL_SubmitGPUCommandBufferAndAcquireFence" 0
    "EndFrame must not add a second frame submission through a fence")
require_call_count(end_frame "RenderEditorOverlayPass" 1 "EndFrame must draw the editor exactly once")
forbid_symbol(end_frame "g_MuEditorCore")
forbid_symbol(frame_replay "g_MuEditorCore")
string(FIND "${end_frame}" "s_preSubmitCallback()" pre_submit_position)
string(FIND "${end_frame}" "RenderEditorOverlayPass(s_cmdBuf, frameColorTexture)" overlay_call_position)
string(FIND "${end_frame}" "s_postRmlUiCallback()" post_rmlui_position)
if(pre_submit_position EQUAL -1 OR overlay_call_position EQUAL -1 OR post_rmlui_position EQUAL -1
        OR NOT pre_submit_position LESS overlay_call_position OR NOT overlay_call_position LESS post_rmlui_position)
    message(FATAL_ERROR "EndFrame must draw the editor after RmlUi and before the post-RmlUi pass (game cursor)")
endif()
string(FIND "${end_frame}" "ReplayPostProcessedFrame(frameColorTexture" post_processed_replay_position)
string(FIND "${end_frame}" "ReplayFrame(frameColorTexture" classic_replay_position)
if(post_processed_replay_position EQUAL -1 OR classic_replay_position EQUAL -1
        OR NOT post_processed_replay_position LESS pre_submit_position
        OR NOT classic_replay_position LESS pre_submit_position)
    message(FATAL_ERROR "EndFrame must replay the frame before RmlUi draws over it")
endif()
string(FIND "${frame_replay}" "static ReplayView FullFrameView()" viewport_init_start)
string(FIND "${frame_replay}" "static bool IsMultisampled()" viewport_init_end)
if(viewport_init_start EQUAL -1 OR viewport_init_end EQUAL -1 OR NOT viewport_init_start LESS viewport_init_end)
    message(FATAL_ERROR "Could not isolate replay viewport initialization")
endif()
math(EXPR viewport_init_length "${viewport_init_end} - ${viewport_init_start}")
string(SUBSTRING "${frame_replay}" ${viewport_init_start} ${viewport_init_length} viewport_init)
foreach(symbol IN ITEMS "{0.0f, 0.0f" "s_swapW" "s_swapH" "0.0f, 1.0f}")
    string(FIND "${viewport_init}" "${symbol}" viewport_symbol_position)
    if(viewport_symbol_position EQUAL -1)
        message(FATAL_ERROR "Missing SDL_GPU editor contract: replay initializes the active viewport to the full swapchain")
    endif()
endforeach()
require_match(replay_commands
    "Render::SdlGpuReplayState[ \t]+replayState[ \t]*;[ \t\r\n]*if[ \t]*\\([ \t]*replayState\.SelectViewport[ \t\r\n]*\\([ \t]*view\.viewport[ \t]*\\)[ \t]*\\)[ \t\r\n]*SDL_SetGPUViewport[ \t\r\n]*\\([ \t]*s_renderPass[ \t]*,[ \t]*&view\.viewport[ \t]*\\)[ \t]*;[ \t\r\n]*if[ \t]*\\([ \t]*replayState\.SelectScissor[ \t\r\n]*\\([ \t]*view\.scissor[ \t]*\\)[ \t]*\\)[ \t\r\n]*SDL_SetGPUScissor[ \t\r\n]*\\([ \t]*s_renderPass[ \t]*,[ \t]*&view\.scissor[ \t]*\\)[ \t]*;"
    "each replay pass owns a render-pass-local replay cache and starts from the carried viewport and scissor")
string(REGEX MATCHALL "Render::SdlGpuReplayState[ \t]+replayState" replay_caches "${replay_commands}")
list(LENGTH replay_caches replay_cache_count)
if(NOT replay_cache_count EQUAL 1)
    message(FATAL_ERROR "Invalid SDL_GPU editor contract: ReplayCommands must own exactly one replay cache")
endif()
require_match(replay_commands
    "case[ \t]+RenderCmdType::SetViewport[ \t]*:[ \t\r\n]*\\{[ \t\r\n]*view\.viewport[ \t]*=[ \t]*cmd\.viewport[ \t]*;[ \t\r\n]*if[ \t]*\\([ \t]*replayState\.SelectViewport[ \t\r\n]*\\([ \t]*view\.viewport[ \t]*\\)[ \t]*\\)[ \t\r\n]*SDL_SetGPUViewport[ \t\r\n]*\\([ \t]*s_renderPass[ \t]*,[ \t]*&view\.viewport[ \t]*\\)[ \t]*;"
    "replay caches and applies the active viewport")
require_match(replay_commands
    "case[ \t]+RenderCmdType::SetScissor[ \t]*:[ \t\r\n]*\\{[ \t\r\n]*view\.scissor[ \t]*=[ \t]*cmd\.scissor[ \t]*;[ \t\r\n]*if[ \t]*\\([ \t]*replayState\.SelectScissor[ \t\r\n]*\\([ \t]*view\.scissor[ \t]*\\)[ \t]*\\)[ \t\r\n]*SDL_SetGPUScissor[ \t\r\n]*\\([ \t]*s_renderPass[ \t]*,[ \t]*&view\.scissor[ \t]*\\)[ \t]*;"
    "replay caches and applies the active scissor")
foreach(helper IN ITEMS
        "BindReplayPipeline"
        "PushReplayVertexUniforms"
        "PushReplayFragmentUniforms"
        "BindReplayFragmentSampler"
        "BindReplayIndexBuffer"
        "ReplayDrawCommand")
    require_match(renderer_code "${helper}" "renderer must use ${helper} for cached replay")
endforeach()

forbid_symbol(frame_replay "if (!s_texturesInvalidated)")
require_match(renderer_code
    "IsUnsafeInvalidatedDrawCommand[ \t\r\n]*\\([ \t]*RenderCmdType[ \t]+type[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*return[ \t]+IsDrawCommand[ \t\r\n]*\\([ \t]*type[ \t]*\\)[ \t]*;"
    "texture invalidation skips game draws but preserves state commands")
string(FIND "${replay_commands}" "const RenderCmd& cmd = s_renderCmds[index];" replay_loop_position)
string(FIND "${replay_commands}" "if (s_texturesInvalidated && IsUnsafeInvalidatedDrawCommand(cmd.type))"
    invalidated_skip_position)
string(FIND "${replay_commands}" "++s_dbgRenderCmdsReplayedThisFrame" replay_count_position)
string(FIND "${replay_commands}" "switch (cmd.type)" replay_switch_position)
if(replay_loop_position EQUAL -1 OR invalidated_skip_position EQUAL -1 OR replay_count_position EQUAL -1
        OR replay_switch_position EQUAL -1
        OR NOT replay_loop_position LESS invalidated_skip_position
        OR NOT invalidated_skip_position LESS replay_count_position
        OR NOT replay_count_position LESS replay_switch_position)
    message(FATAL_ERROR
        "Replay must skip unsafe invalidated game draws before counting processed commands")
endif()

strip_editor_sections(renderer_code renderer_editor_off_code)
foreach(symbol IN ITEMS
        "QueueEditorRenderCommand"
        "RenderEditorOverlayPass"
        "s_editorDrawPending"
        "g_MuEditorCore"
        "ImGui_ImplSDLGPU3_PrepareDrawData"
        "ImGui_ImplSDLGPU3_RenderDrawData")
    forbid_symbol(renderer_editor_off_code "${symbol}")
endforeach()

forbid_symbol(app_code "g_sdlGLContext")
require_match(app_code
    "g_MuEditorCore\\.Initialize[ \t\r\n]*\\([ \t\r\n]*g_sdlWindow[ \t\r\n]*\\)[ \t]*;"
    "app initializes the editor with the SDL window only")

string(FIND "${app_code}" "static void ShutdownRuntime" runtime_shutdown_start)
string(FIND "${app_code}" "static void WriteStartupDiagnostics" runtime_shutdown_end)
if(runtime_shutdown_start EQUAL -1 OR runtime_shutdown_end EQUAL -1 OR runtime_shutdown_end LESS runtime_shutdown_start)
    message(FATAL_ERROR "Could not isolate ShutdownRuntime()")
endif()
math(EXPR runtime_shutdown_length "${runtime_shutdown_end} - ${runtime_shutdown_start}")
string(SUBSTRING "${app_code}" ${runtime_shutdown_start} ${runtime_shutdown_length} runtime_shutdown)
string(FIND "${runtime_shutdown}" "g_MuEditorCore.Shutdown()" editor_shutdown_position)
string(FIND "${runtime_shutdown}" "ShutdownRendererWindow()" renderer_shutdown_position)
if(editor_shutdown_position EQUAL -1 OR renderer_shutdown_position EQUAL -1)
    message(FATAL_ERROR "ShutdownRuntime must shut down both the editor and renderer window")
endif()
if(NOT editor_shutdown_position LESS renderer_shutdown_position)
    message(FATAL_ERROR "ShutdownRuntime must shut down the editor before renderer/device teardown")
endif()

# The editor's own windows (ImGui viewports outside the game window): updated after every frame ImGui
# ended and drawn once the game's frame went to the GPU, from the main loop, or before the next editor
# frame starts for a frame that ended elsewhere.
string(FIND "${editor_code}" "void CMuEditorCore::RenderPlatformWindows()" platform_windows_start)
if(platform_windows_start EQUAL -1 OR NOT editor_render_start LESS platform_windows_start
        OR NOT platform_windows_start LESS editor_prepare_start)
    message(FATAL_ERROR "Could not isolate CMuEditorCore::RenderPlatformWindows()")
endif()
math(EXPR platform_windows_length "${editor_prepare_start} - ${platform_windows_start}")
string(SUBSTRING "${editor_code}" ${platform_windows_start} ${platform_windows_length} platform_windows)
require_match(platform_windows
    "if[ \t\r\n]*\\([ \t]*![ \t]*m_bPlatformWindowsPending[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*return[ \t]*;[ \t\r\n]*\\}[ \t\r\n]*m_bPlatformWindowsPending[ \t]*=[ \t]*false[ \t]*;"
    "the editor's own windows run once per ended editor frame")
require_call_count(platform_windows "ImGui::UpdatePlatformWindows" 1
    "the editor updates its own windows once per ended frame, also while they are off")
require_match(platform_windows
    "platformWindowsOn[ \t]*=[ \t]*\\([ \t]*ImGui::GetIO\\(\\)\\.ConfigFlags[ \t]*&[ \t]*ImGuiConfigFlags_ViewportsEnable[ \t]*\\)[ \t]*!=[ \t]*0[ \t]*;"
    "the editor knows whether its own windows are on")
require_match(platform_windows
    "gameWindowHidden[ \t\r\n]*=[^;]*SDL_WINDOW_MINIMIZED[^;]*m_skippedGameFrames[^;]*;"
    "the editor knows whether the game window is out of sight (minimized, or skipping frame after frame)")
require_match(platform_windows
    "if[ \t\r\n]*\\([ \t]*platformWindowsOn[ \t]*&&[ \t]*\\([ \t]*m_bGameFrameActive[ \t]*\\|\\|[ \t]*gameWindowHidden[ \t]*\\)[ \t]*\\)[ \t\r\n]*\\{[ \t\r\n]*ImGui::RenderPlatformWindowsDefault\\(\\)[ \t]*;"
    "the editor draws its own windows while they are on, unless the game skipped a frame of a window that is shown")
string(FIND "${platform_windows}" "ImGui::UpdatePlatformWindows()" platform_update_position)
string(FIND "${platform_windows}" "ImGui::RenderPlatformWindowsDefault()" platform_render_position)
if(NOT platform_update_position LESS platform_render_position)
    message(FATAL_ERROR "The editor must update its own windows before drawing them")
endif()

string(FIND "${editor_render}" "m_bGameFrameActive = mu::GetRenderer().IsFrameActive()" game_frame_active_position)
if(game_frame_active_position EQUAL -1 OR NOT game_frame_active_position LESS imgui_render_position)
    message(FATAL_ERROR "Editor Render() must note whether the game draws the frame before ImGui::Render() ends it")
endif()
string(FIND "${editor_render}" "m_bPlatformWindowsPending = true" platform_pending_position)
if(platform_pending_position EQUAL -1 OR NOT imgui_render_position LESS platform_pending_position
        OR NOT platform_pending_position LESS frame_finished_position)
    message(FATAL_ERROR "Editor Render() must mark its own windows pending once ImGui::Render() ended the frame")
endif()

string(FIND "${editor_update}" "RenderPlatformWindows();" update_platform_windows_position)
string(FIND "${editor_update}" "ImGui::NewFrame();" update_new_frame_position)
if(update_platform_windows_position EQUAL -1 OR update_new_frame_position EQUAL -1
        OR NOT update_platform_windows_position LESS update_new_frame_position)
    message(FATAL_ERROR "Editor Update() must finish the last frame's own windows before ImGui::NewFrame()")
endif()

require_call_count(app_code "g_MuEditorCore\\.RenderPlatformWindows" 1
    "the main loop draws the editor's own windows once per frame")
string(FIND "${app_code}" "RenderProfiledFrame();" app_frame_position)
string(FIND "${app_code}" "g_MuEditorCore.RenderPlatformWindows();" app_platform_windows_position)
if(app_frame_position EQUAL -1 OR NOT app_frame_position LESS app_platform_windows_position)
    message(FATAL_ERROR "The main loop must draw the editor's own windows after the game's frame went to the GPU")
endif()

require_editor_before_cursor(scene_manager_code "if (Success)" "CheckServerConnection()" "SceneManager editor/cursor region")
require_editor_before_cursor(loading_scene_code "::EndOpengl();" "UI::Reconnect::RenderDialog()"
    "LoadingScene editor/cursor region")

message(STATUS "Validated SDL_GPU ImGui backend source contract")
