foreach(required_variable IN ITEMS MU_RENDERER_SOURCE)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "Missing required variable: ${required_variable}")
    endif()
endforeach()

file(READ "${MU_RENDERER_SOURCE}" renderer_source)

foreach(required_text IN ITEMS
        "MU_D3D12_DISABLE_CULLING"
        "direct3d12"
        "s_disableD3D12Culling"
        "s_pipelines3DNoCull"
        "s_pipelinesSkinnedNoCull")
    string(FIND "${renderer_source}" "${required_text}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing D3D12 culling diagnostic contract: ${required_text}")
    endif()
endforeach()
