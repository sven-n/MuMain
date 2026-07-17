file(READ "${SOURCE_FILE}" renderer_source)

if(NOT renderer_source MATCHES "s_vtxScratch.resize\\(requiredSize\\)")
    message(FATAL_ERROR "Vertex recording must grow CPU staging instead of dropping late draws")
endif()

if(NOT renderer_source MATCHES "EnsureVertexBufferCapacity\\(s_vtxOffset\\)")
    message(FATAL_ERROR "GPU vertex buffers must grow to fit recorded frame data")
endif()

if(renderer_source MATCHES "dropping remaining frame draws")
    message(FATAL_ERROR "Stress frames must not drop UI or other late draw commands")
endif()
