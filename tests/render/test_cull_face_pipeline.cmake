file(READ "${SOURCE_FILE}" renderer_source)

if(NOT renderer_source MATCHES "s_pipelines3DNoCull")
    message(FATAL_ERROR "SDL GPU renderer needs a no-cull opaque 3D pipeline family")
endif()

if(NOT renderer_source MATCHES "m_cullFaceEnabled[^
]*s_pipelines3D")
    message(FATAL_ERROR "Opaque 3D pipeline selection must honor SetCullFace state")
endif()
