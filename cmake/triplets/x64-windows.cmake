# vcpkg's own x64-windows triplet, plus the static linkage that cryptopp, glslang and spirv-cross only support. vcpkg
# builds those three statically either way; said here, it does so without a note that it overrides the triplet.
# CI passes this folder as VCPKG_OVERLAY_TRIPLETS.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
if(PORT MATCHES "^(cryptopp|glslang|spirv-cross)$")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()
