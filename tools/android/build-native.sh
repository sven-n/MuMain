#!/usr/bin/env bash
set -euo pipefail

readonly GL4ES_REV="81547d986798e876de8b434193920b606a72363f"
readonly CURL_REV="5d6dc816785358f505ba6922ceb507b4521c5421"
readonly LIBJPEG_TURBO_REV="2646fa33f8d67373c27b558b47e77b2d8d517e31"
readonly ANDROID_API="29"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
output_root="${MU_ANDROID_OUTPUT_DIR:-${repo_root}/out/android}"
abi="${MU_ANDROID_ABI:-arm64-v8a}"
ndk_root="${ANDROID_NDK_ROOT:-${ANDROID_NDK_HOME:-}}"

if [[ -z "${ndk_root}" || ! -f "${ndk_root}/build/cmake/android.toolchain.cmake" ]]; then
    echo "Set ANDROID_NDK_ROOT to Android NDK r28 or newer." >&2
    exit 1
fi

case "${abi}" in
    arm64-v8a|x86_64) ;;
    *)
        echo "Unsupported MU_ANDROID_ABI='${abi}'; use arm64-v8a or x86_64." >&2
        exit 1
        ;;
esac

readonly toolchain="${ndk_root}/build/cmake/android.toolchain.cmake"
readonly dependency_root="${output_root}/dependencies"
readonly build_root="${output_root}/build/${abi}"
readonly native_output="${output_root}/native/${abi}"

prepare_source()
{
    local name="$1"
    local url="$2"
    local revision="$3"
    local source_dir="${dependency_root}/${name}"

    if [[ ! -d "${source_dir}/.git" ]]; then
        mkdir -p "${dependency_root}"
        git clone --filter=blob:none "${url}" "${source_dir}"
    fi

    git -C "${source_dir}" fetch --depth=1 origin "${revision}" >&2
    git -C "${source_dir}" checkout --detach "${revision}" >&2
    printf '%s\n' "${source_dir}"
}

configure_dependency()
{
    local source_dir="$1"
    local binary_dir="$2"
    shift 2

    cmake -S "${source_dir}" -B "${binary_dir}" -G Ninja \
        -DCMAKE_TOOLCHAIN_FILE="${toolchain}" \
        -DANDROID_ABI="${abi}" \
        -DANDROID_PLATFORM="android-${ANDROID_API}" \
        -DCMAKE_BUILD_TYPE=Release \
        "$@"
}

gl4es_source="$(prepare_source gl4es https://github.com/ptitSeb/gl4es.git "${GL4ES_REV}")"
curl_source="$(prepare_source curl https://github.com/curl/curl.git "${CURL_REV}")"
jpeg_source="$(prepare_source libjpeg-turbo https://github.com/libjpeg-turbo/libjpeg-turbo.git "${LIBJPEG_TURBO_REV}")"

gl4es_build="${build_root}/gl4es"
configure_dependency "${gl4es_source}" "${gl4es_build}" \
    -DNOX11=ON \
    -DGBM=OFF \
    -DDEFAULT_ES=2
cmake --build "${gl4es_build}" --parallel

mkdir -p "${native_output}"
cp "${gl4es_source}/lib/libGL.so.1" "${native_output}/libGL.so"
patchelf --set-soname libGL.so "${native_output}/libGL.so"

curl_build="${build_root}/curl"
configure_dependency "${curl_source}" "${curl_build}" \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_STATIC_LIBS=OFF \
    -DBUILD_CURL_EXE=OFF \
    -DBUILD_TESTING=OFF \
    -DCURL_USE_OPENSSL=OFF \
    -DCURL_USE_LIBPSL=OFF \
    -DUSE_LIBIDN2=OFF \
    -DCURL_USE_LIBSSH=OFF \
    -DCURL_USE_LIBSSH2=OFF \
    -DCURL_USE_GSSAPI=OFF
cmake --build "${curl_build}" --target libcurl_shared --parallel

jpeg_build="${build_root}/libjpeg-turbo"
configure_dependency "${jpeg_source}" "${jpeg_build}" \
    -DENABLE_SHARED=ON \
    -DENABLE_STATIC=OFF \
    -DWITH_TURBOJPEG=ON \
    -DWITH_JAVA=OFF \
    -DWITH_TESTS=OFF
cmake --build "${jpeg_build}" --target turbojpeg --parallel

main_build="${build_root}/mumain"
cmake -S "${repo_root}" -B "${main_build}" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="${toolchain}" \
    -DANDROID_ABI="${abi}" \
    -DANDROID_PLATFORM="android-${ANDROID_API}" \
    -DANDROID_ALLOW_UNDEFINED_SYMBOLS=ON \
    -DCMAKE_DISABLE_FIND_PACKAGE_PkgConfig=TRUE \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_EDITOR=OFF \
    -DBUILD_TESTING=OFF \
    -DMU_RENDER_BACKEND=OpenGL \
    -DMU_ANDROID_GL4ES_ROOT="${gl4es_source}" \
    -DMU_ANDROID_GL4ES_LIBRARY="${native_output}/libGL.so" \
    -DCURL_INCLUDE_DIR="${curl_source}/include" \
    -DCURL_LIBRARY="${curl_build}/lib/libcurl.so" \
    -DTURBOJPEG_LIBRARY="${jpeg_build}/libturbojpeg.so"
cmake --build "${main_build}" --target Main --parallel

strip_tool="${ndk_root}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip"
cp "${main_build}/src/libmain.so" "${native_output}/libmain.so"
cp "${curl_build}/lib/libcurl.so" "${native_output}/libcurl.so"
cp "${jpeg_build}/libturbojpeg.so" "${native_output}/libturbojpeg.so"
cp "${main_build}/src/Release/MUnique.Client.Library.so" \
    "${native_output}/libMUniqueClient.so"

"${strip_tool}" "${native_output}/libmain.so"
"${strip_tool}" "${native_output}/libcurl.so"
"${strip_tool}" "${native_output}/libturbojpeg.so"
"${strip_tool}" "${native_output}/libMUniqueClient.so"

printf 'renderer=OpenGL\nabi=%s\napi=%s\n' "${abi}" "${ANDROID_API}" \
    > "${native_output}/build.properties"
sha256sum "${native_output}"/*.so > "${native_output}/sha256sums.txt"

echo "Android native libraries are ready in ${native_output}"
