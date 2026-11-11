cmake_minimum_required(VERSION 3.25)

# The Zen coin heap copies mesh 0 out of the shared VertexTransform scratch after RenderZen's lazy
# Transform(). It has to materialize that mesh first, or it draws whatever mesh 0 another model
# left in the scratch (an opaque gold plate over the hero).

if(NOT DEFINED MU_BMD_SOURCE)
    message(FATAL_ERROR "MU_BMD_SOURCE must be set")
endif()

file(READ "${MU_BMD_SOURCE}" bmd_source)

string(FIND "${bmd_source}" "int BMD::AddToCoinHeap(" heap_start)
string(FIND "${bmd_source}" "void BMD::EndRenderCoinHeap(" heap_end)
if(heap_start EQUAL -1 OR heap_end LESS heap_start)
    message(FATAL_ERROR "Could not isolate BMD::AddToCoinHeap")
endif()

math(EXPR heap_length "${heap_end} - ${heap_start}")
string(SUBSTRING "${bmd_source}" ${heap_start} ${heap_length} heap_source)

string(FIND "${heap_source}" "EnsureCpuVertices(meshIndex)" ensure_position)
string(FIND "${heap_source}" "VertexTransform[meshIndex]" read_position)
if(read_position EQUAL -1)
    message(FATAL_ERROR "BMD::AddToCoinHeap no longer reads VertexTransform; update this contract")
endif()
if(ensure_position EQUAL -1 OR ensure_position GREATER read_position)
    message(FATAL_ERROR
        "BMD::AddToCoinHeap must call EnsureCpuVertices(meshIndex) before reading VertexTransform")
endif()
