#!/usr/bin/env bash
#
# Configures, builds, and runs the MU client for native Linux, mirroring
# docs/build/linux/console.md and .github/workflows/linux-build.yml.
#
# Usage:
#   ./scripts/build-and-run.sh [options] [-- extra args passed to Main]
#
# Options:
#   -c, --config <Debug|Release>   Build configuration (default: Release)
#   -e, --editor                   Enable the ImGui in-game editor (ENABLE_EDITOR=ON,
#                                   launches with --editor)
#       --clean                    Remove the build directory first
#       --no-build                 Skip configure/build, just (re)launch the existing binary
#       --detached                 Launch the client detached instead of blocking
#   -h, --help                     Show this help and exit
#
# Examples:
#   ./scripts/build-and-run.sh
#   ./scripts/build-and-run.sh --editor --config Debug
#   ./scripts/build-and-run.sh --no-build --detached
#   ./scripts/build-and-run.sh -- connect /u192.168.0.20 /p55902

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
repo_root="$(dirname -- "$script_dir")"

config="Release"
editor="OFF"
clean=0
no_build=0
detached=0
client_args=()

usage() {
    cat <<'EOF'
Usage:
  ./scripts/build-and-run.sh [options] [-- extra args passed to Main]

Options:
  -c, --config <Debug|Release>   Build configuration (default: Release)
  -e, --editor                   Enable the ImGui in-game editor (ENABLE_EDITOR=ON,
                                  launches with --editor)
      --clean                    Remove the build directory first
      --no-build                 Skip configure/build, just (re)launch the existing binary
      --detached                 Launch the client detached instead of blocking
  -h, --help                     Show this help and exit

Examples:
  ./scripts/build-and-run.sh
  ./scripts/build-and-run.sh --editor --config Debug
  ./scripts/build-and-run.sh --no-build --detached
  ./scripts/build-and-run.sh -- connect /u192.168.0.20 /p55902
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--config)
            config="$2"
            shift 2
            ;;
        -e|--editor)
            editor="ON"
            shift
            ;;
        --clean)
            clean=1
            shift
            ;;
        --no-build)
            no_build=1
            shift
            ;;
        --detached)
            detached=1
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            client_args=("$@")
            break
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

case "$config" in
    Debug|Release) ;;
    *)
        echo "Invalid --config '$config' (expected Debug or Release)" >&2
        exit 1
        ;;
esac

build_dir="$repo_root/build-linux"
exe_dir="$build_dir/src"
exe_path="$exe_dir/Main"

if [[ $no_build -eq 0 ]]; then
    if [[ $clean -eq 1 && -d "$build_dir" ]]; then
        echo "Removing $build_dir ..."
        rm -rf "$build_dir"
    fi

    echo "Configuring (config=$config, editor=$editor) ..."
    cmake -S "$repo_root" -B "$build_dir" -G Ninja \
        -DCMAKE_BUILD_TYPE="$config" \
        -DENABLE_EDITOR="$editor"

    echo "Building ..."
    cmake --build "$build_dir" -j"$(nproc)"
fi

if [[ ! -x "$exe_path" ]]; then
    echo "Main not found (or not executable) at '$exe_path'. Run without --no-build first." >&2
    exit 1
fi

launch_args=()
if [[ "$editor" == "ON" ]]; then
    launch_args+=("--editor")
fi
launch_args+=("${client_args[@]}")

if [[ $detached -eq 1 ]]; then
    ( cd "$exe_dir" && nohup "$exe_path" "${launch_args[@]}" >/dev/null 2>&1 & )
    echo "Launched detached: $exe_path"
else
    cd "$exe_dir"
    exec "$exe_path" "${launch_args[@]}"
fi
