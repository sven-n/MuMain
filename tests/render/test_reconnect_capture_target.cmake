file(READ "${SOURCE_FILE}" renderer_source)

if(renderer_source MATCHES "blit\\.source\\.texture = s_swapchainTexture")
    message(FATAL_ERROR "Swapchain texture cannot be a blit source because it lacks SAMPLER usage")
endif()

if(NOT renderer_source MATCHES "reconnectCaptureTexture[^
]*frameColorTexture")
    message(FATAL_ERROR "Reconnect capture texture must participate in frame render-target selection")
endif()
