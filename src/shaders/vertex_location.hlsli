// A vertex input's location for the SPIR-V compile (glslang would otherwise number the inputs 0, 1, 2, ...).
// The Direct3D 12 compile binds inputs by semantic and warns that it ignores vk::location, so it gets none.
#ifdef MU_D3D12_RAW_STORAGE_BUFFER
#define MU_VERTEX_LOCATION(index)
#else
#define MU_VERTEX_LOCATION(index) [[vk::location(index)]]
#endif
