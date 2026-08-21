if(NOT EXISTS "${SOURCE_FILE}")
    message(FATAL_ERROR "Camera projection source missing: ${SOURCE_FILE}")
endif()

file(READ "${SOURCE_FILE}" camera_source)

if(NOT camera_source MATCHES "gluPerspective\\(fov, aspect, zNear, zFar\\);")
    message(FATAL_ERROR "SetupPerspective must update the renderer projection matrix")
endif()
