#!/usr/bin/env bash
# Capture reproducible Wayland and Vulkan diagnostics for one client invocation.
set -euo pipefail

readonly DEFAULT_TIMEOUT_SECONDS=60
readonly TERMINATION_GRACE_SECONDS=5
readonly HEADLESS_STARTUP_SECONDS=10
readonly DEFAULT_RELEASE_CLIENT="out/build/linux-x64/src/Release/Main"
readonly DEFAULT_DEBUG_CLIENT="out/build/linux-x64/src/Debug/Main"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
timeout_seconds="$DEFAULT_TIMEOUT_SECONDS"
headless=false
create_archive=false
client_path=""
out_dir=""
client_args=()
weston_pid=""
headless_runtime_dir=""
wayland_socket=""
lavapipe_icd=""
validation_enabled=false
validation_reason="not requested"
process_classification="not run"
process_raw_status="not run"
process_detail=""

environment_file=""
versions_file=""
tools_file=""
wayland_globals_file=""
vulkan_summary_file=""
raw_stderr_file=""
stderr_file=""
validation_file=""
muerror_source=""
muerror_copy=""
muerror_previous_copy=""
summary_file=""
exit_status_file=""

usage() {
    cat <<'EOF'
Usage:
  scripts/wayland-diag.sh [options] [-- client-arguments]

Run Main once and collect a sanitized, issue-ready Wayland diagnostics bundle.
The client must already have runtime data beside it; this script never builds or
downloads game assets.

Options:
  --timeout N     Stop the client after N seconds (default: 60).
  --headless      Run under a private Weston headless compositor with lavapipe.
                  Fails before the client starts if either is unavailable.
  --out DIR       Write artifacts to DIR. Default: diag/wayland-<UTC>/.
  --client PATH   Client executable. Default search order:
                  out/build/linux-x64/src/Release/Main
                  out/build/linux-x64/src/Debug/Main
  --tar           Create a local DIR.tar.gz archive after capture.
  -h, --help      Show this help.

Use -- before arguments passed to Main. Existing SDL_*, VK_*, WAYLAND_*, and
MESA_* variables are preserved. Sensitive values are redacted from the records.
EOF
}

fail() {
    printf 'wayland-diag: %s\n' "$*" >&2
    exit 2
}

is_positive_integer() {
    [[ "$1" =~ ^[1-9][0-9]*$ ]]
}

require_value() {
    [[ $# -eq 2 && -n "$2" ]] || fail "$1 requires a value"
}

parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --timeout)
                require_value "$1" "${2:-}"
                is_positive_integer "$2" || fail "--timeout must be a positive integer"
                timeout_seconds="$2"
                shift 2
                ;;
            --headless)
                headless=true
                shift
                ;;
            --out)
                require_value "$1" "${2:-}"
                out_dir="$2"
                shift 2
                ;;
            --client)
                require_value "$1" "${2:-}"
                client_path="$2"
                shift 2
                ;;
            --tar)
                create_archive=true
                shift
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            --)
                shift
                client_args=("$@")
                return 0
                ;;
            *)
                fail "unknown option: $1 (use -- before client arguments)"
                ;;
        esac
    done
}

select_client() {
    local candidate

    if [[ -n "$client_path" ]]; then
        candidate="$client_path"
    else
        for candidate in "$repo_root/$DEFAULT_RELEASE_CLIENT" "$repo_root/$DEFAULT_DEBUG_CLIENT"; do
            if [[ -x "$candidate" ]]; then
                client_path="$candidate"
                return 0
            fi
        done
        fail "client not found; use --client PATH or build Main in $DEFAULT_RELEASE_CLIENT"
    fi

    [[ -x "$candidate" ]] || fail "client is not executable: $candidate"
    client_path="$(cd "$(dirname "$candidate")" && pwd)/$(basename "$candidate")"
}

prepare_output_directory() {
    if [[ -z "$out_dir" ]]; then
        out_dir="$repo_root/diag/wayland-$(date -u +%Y%m%dT%H%M%SZ)"
    fi
    mkdir -p "$out_dir"
    out_dir="$(cd "$out_dir" && pwd)"

    environment_file="$out_dir/environment.txt"
    versions_file="$out_dir/versions.txt"
    tools_file="$out_dir/tools.txt"
    wayland_globals_file="$out_dir/wayland-globals.txt"
    vulkan_summary_file="$out_dir/vulkan-summary.txt"
    raw_stderr_file="$out_dir/client-stderr.raw.log"
    wayland_debug_file="$out_dir/wayland-debug.log"
    stderr_file="$out_dir/stderr.log"
    validation_file="$out_dir/vulkan-validation.log"
    muerror_source="$(dirname "$client_path")/MuError.log"
    muerror_copy="$out_dir/MuError.log"
    muerror_previous_copy="$out_dir/MuError.log.previous"
    summary_file="$out_dir/summary.txt"
    exit_status_file="$out_dir/exit-status.txt"

    : >"$raw_stderr_file"
    : >"$wayland_debug_file"
    : >"$stderr_file"
    : >"$validation_file"
    : >"$muerror_copy"
}

sanitize_value() {
    local name="$1"
    local value="$2"

    case "$name" in
        *[Tt][Oo][Kk][Ee][Nn]*|*[Kk][Ee][Yy]*|*[Ss][Ee][Cc][Rr][Ee][Tt]*|*[Pp][Aa][Ss][Ss][Ww][Oo][Rr][Dd]*|*[Cc][Oo][Oo][Kk][Ii][Ee]*|*[Pp][Rr][Oo][Xx][Yy]*)
            printf '[REDACTED]'
            return 0
            ;;
    esac
    if [[ -n "${HOME:-}" ]]; then
        value="${value//"$HOME"/\$HOME}"
    fi
    printf '%s' "$value"
}

write_environment_inventory() {
    local name value
    local -a selected_names=()

    {
        printf 'XDG_CURRENT_DESKTOP=%s\n' "$(sanitize_value XDG_CURRENT_DESKTOP "${XDG_CURRENT_DESKTOP:-}")"
        printf 'XDG_SESSION_TYPE=%s\n' "$(sanitize_value XDG_SESSION_TYPE "${XDG_SESSION_TYPE:-}")"
        printf 'WAYLAND_DISPLAY=%s\n' "$(sanitize_value WAYLAND_DISPLAY "${WAYLAND_DISPLAY:-}")"
        printf 'DISPLAY=%s\n' "$(sanitize_value DISPLAY "${DISPLAY:-}")"
        printf '\nSelected environment variables before instrumentation:\n'
    } >"$environment_file"

    while IFS='=' read -r name value; do
        case "$name" in
            SDL_*|VK_*|WAYLAND_*|MESA_*|HTTP_PROXY|HTTPS_PROXY|ALL_PROXY|NO_PROXY|http_proxy|https_proxy|all_proxy|no_proxy)
                selected_names+=("$name")
                ;;
        esac
    done < <(env)

    if [[ ${#selected_names[@]} -eq 0 ]]; then
        printf '  (none)\n' >>"$environment_file"
        return 0
    fi

    while IFS= read -r name; do
        value="${!name-}"
        printf '%s=%s\n' "$name" "$(sanitize_value "$name" "$value")" >>"$environment_file"
    done < <(printf '%s\n' "${selected_names[@]}" | sort -u)
}

write_tool_inventory() {
    local tool

    : >"$tools_file"
    for tool in git wayland-info vulkaninfo weston tar; do
        if command -v "$tool" >/dev/null 2>&1; then
            printf '%s: available\n' "$tool" >>"$tools_file"
        else
            printf '%s: missing\n' "$tool" >>"$tools_file"
        fi
    done
}

write_version_inventory() {
    local distro_name="unknown"
    local repo_commit="unknown"
    local repo_describe="unknown"
    local repo_state="unknown"
    local sdl_version="unknown"

    if [[ -r /etc/os-release ]]; then
        # shellcheck disable=SC1091
        distro_name="$(. /etc/os-release; printf '%s' "${PRETTY_NAME:-unknown}")"
    fi
    if command -v git >/dev/null 2>&1; then
        repo_commit="$(git -C "$repo_root" rev-parse HEAD 2>/dev/null || printf unknown)"
        repo_describe="$(git -C "$repo_root" describe --always --dirty 2>/dev/null || printf unknown)"
        if [[ "$repo_commit" != unknown ]]; then
            if [[ -n "$(git -C "$repo_root" status --porcelain 2>/dev/null)" ]]; then
                repo_state="dirty"
            else
                repo_state="clean"
            fi
        fi
        sdl_version="$(git -C "$repo_root/src/ThirdParty/SDL" describe --tags --always --dirty 2>/dev/null || printf unknown)"
    fi

    {
        printf 'kernel=%s\n' "$(uname -srmo)"
        printf 'distro=%s\n' "$distro_name"
        printf 'repo_commit=%s\n' "$repo_commit"
        printf 'repo_describe=%s\n' "$repo_describe"
        printf 'repo_state=%s\n' "$repo_state"
        printf 'sdl_version=%s\n' "$sdl_version"
    } >"$versions_file"
}

find_lavapipe_in_directory() {
    local directory="$1"
    local file contents

    [[ -d "$directory" ]] || return 1
    for file in "$directory"/*lvp*.json "$directory"/*lavapipe*.json; do
        [[ -r "$file" ]] || continue
        contents="$(cat "$file")"
        case "$contents" in
            *lvp*|*lavapipe*)
                printf '%s\n' "$file"
                return 0
                ;;
        esac
    done
    return 1
}

find_lavapipe_icd() {
    local candidate directory
    local -a candidates=()
    local -a data_directories=()

    if [[ -n "${VK_ICD_FILENAMES:-}" ]]; then
        IFS=':' read -r -a candidates <<<"$VK_ICD_FILENAMES"
        for candidate in "${candidates[@]}"; do
            [[ -r "$candidate" ]] || continue
            case "$(cat "$candidate")" in
                *lvp*|*lavapipe*) printf '%s\n' "$candidate"; return 0 ;;
            esac
        done
    fi

    if [[ -n "${XDG_DATA_DIRS:-}" ]]; then
        IFS=':' read -r -a data_directories <<<"$XDG_DATA_DIRS"
        for directory in "${data_directories[@]}"; do
            candidate="$(find_lavapipe_in_directory "$directory/vulkan/icd.d" 2>/dev/null || true)"
            [[ -n "$candidate" ]] && { printf '%s\n' "$candidate"; return 0; }
        done
    fi

    for directory in /usr/share/vulkan/icd.d /etc/vulkan/icd.d; do
        candidate="$(find_lavapipe_in_directory "$directory" 2>/dev/null || true)"
        [[ -n "$candidate" ]] && { printf '%s\n' "$candidate"; return 0; }
    done
    return 1
}

validation_layer_available() {
    local directory file contents
    local -a directories=(/usr/share/vulkan/explicit_layer.d /etc/vulkan/explicit_layer.d)

    if [[ -n "${VK_LAYER_PATH:-}" ]]; then
        IFS=':' read -r -a directories <<<"$VK_LAYER_PATH"
        directories+=(/usr/share/vulkan/explicit_layer.d /etc/vulkan/explicit_layer.d)
    fi
    for directory in "${directories[@]}"; do
        [[ -d "$directory" ]] || continue
        for file in "$directory"/*.json; do
            [[ -r "$file" ]] || continue
            contents="$(cat "$file")"
            case "$contents" in
                *VK_LAYER_KHRONOS_validation*) return 0 ;;
            esac
        done
    done
    return 1
}

configure_validation() {
    if [[ -n "${VK_INSTANCE_LAYERS+x}" || -n "${VK_LOADER_LAYERS_ENABLE+x}" ]]; then
        validation_reason="preserved user setting"
    elif validation_layer_available; then
        validation_enabled=true
        validation_reason="enabled VK_LAYER_KHRONOS_validation"
    else
        validation_reason="validation layer unavailable"
    fi
}

run_with_effective_environment() {
    if [[ "$headless" == true ]]; then
        env \
            XDG_RUNTIME_DIR="$headless_runtime_dir" \
            WAYLAND_DISPLAY="$wayland_socket" \
            VK_ICD_FILENAMES="$lavapipe_icd" \
            "$@"
    else
        "$@"
    fi
}

capture_optional_inventory() {
    if command -v wayland-info >/dev/null 2>&1; then
        run_with_effective_environment wayland-info >"$wayland_globals_file" 2>&1 || true
    else
        printf 'wayland-info: missing\n' >"$wayland_globals_file"
    fi
    if command -v vulkaninfo >/dev/null 2>&1; then
        run_with_effective_environment vulkaninfo --summary >"$vulkan_summary_file" 2>&1 || true
    else
        printf 'vulkaninfo: missing\n' >"$vulkan_summary_file"
    fi
}

write_capture_method() {
    cat >"$out_dir/capture-method.txt" <<'EOF'
WAYLAND_DEBUG=client writes Wayland protocol records to the client's standard
error stream. The harness saves that stream to client-stderr.raw.log until the
client exits, then routes libwayland records to wayland-debug.log and all other
client standard error to stderr.log. It recognizes both the current
[HH:MM:SS.mmm] {queue} prefix and the older [ 1234567.890] prefix. Vulkan
validation messages are copied to vulkan-validation.log.
EOF
}

is_wayland_protocol_line() {
    [[ "$1" =~ ^\[([0-9]{2}:[0-9]{2}:[0-9]+\.[0-9]+|[[:space:]]*[0-9]+\.[0-9]+)\]([[:space:]]+\{[^}]*\})?[[:space:]]+ ]]
}

is_validation_line() {
    case "$1" in
        *VUID-*|*Validation*|*validation*) return 0 ;;
        *) return 1 ;;
    esac
}

split_client_stderr() {
    local line

    while IFS= read -r line || [[ -n "$line" ]]; do
        if is_wayland_protocol_line "$line"; then
            printf '%s\n' "$line" >>"$wayland_debug_file"
        else
            printf '%s\n' "$line" >>"$stderr_file"
        fi
        if is_validation_line "$line"; then
            printf '%s\n' "$line" >>"$validation_file"
        fi
    done <"$raw_stderr_file"
}

wait_for_process_exit() {
    local pid="$1"
    local deadline="$2"

    while kill -0 "$pid" 2>/dev/null; do
        (( SECONDS < deadline )) || return 1
        sleep 1
    done
    return 0
}

preserve_existing_muerror() {
    [[ -f "$muerror_source" ]] || return 0

    cp "$muerror_source" "$muerror_previous_copy"
    rm "$muerror_source"
}

run_client() {
    local client_dir client_pid raw_status signal_name
    local -a client_environment=(WAYLAND_DEBUG=client)

    client_dir="$(dirname "$client_path")"
    if [[ "$headless" == true ]]; then
        client_environment+=(
            "XDG_RUNTIME_DIR=$headless_runtime_dir"
            "WAYLAND_DISPLAY=$wayland_socket"
            "VK_ICD_FILENAMES=$lavapipe_icd"
        )
    fi
    if [[ -z "${SDL_LOGGING+x}" ]]; then
        client_environment+=("SDL_LOGGING=video=verbose,gpu=verbose")
    fi
    if [[ "$validation_enabled" == true ]]; then
        client_environment+=(
            "VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation"
            "VK_LOADER_LAYERS_ENABLE=VK_LAYER_KHRONOS_validation"
        )
    fi

    preserve_existing_muerror
    (
        cd "$client_dir"
        exec env "${client_environment[@]}" "$client_path" "${client_args[@]}"
    ) 2>"$raw_stderr_file" &
    client_pid=$!

    if wait_for_process_exit "$client_pid" $((SECONDS + timeout_seconds)); then
        set +e
        wait "$client_pid"
        raw_status=$?
        set -e
        if [[ "$raw_status" -eq 126 || "$raw_status" -eq 127 ]]; then
            process_classification="failed-to-start"
        elif [[ "$raw_status" -ge 128 ]]; then
            signal_name="$(kill -l "$((raw_status - 128))" 2>/dev/null || printf '%s' "$((raw_status - 128))")"
            process_classification="signaled SIG${signal_name#SIG}"
        else
            process_classification="exited $raw_status"
        fi
    else
        kill -TERM "$client_pid" 2>/dev/null || true
        if ! wait_for_process_exit "$client_pid" $((SECONDS + TERMINATION_GRACE_SECONDS)); then
            kill -KILL "$client_pid" 2>/dev/null || true
        fi
        set +e
        wait "$client_pid"
        raw_status=$?
        set -e
        process_classification="timed-out"
    fi

    process_raw_status="$raw_status"
    if [[ -f "$muerror_source" ]]; then
        cp "$muerror_source" "$muerror_copy"
    else
        printf 'MuError.log was not created by this run.\n' >"$muerror_copy"
    fi
    split_client_stderr
}

stop_weston() {
    if [[ -n "$weston_pid" ]] && kill -0 "$weston_pid" 2>/dev/null; then
        kill -TERM "$weston_pid" 2>/dev/null || true
        wait "$weston_pid" 2>/dev/null || true
    fi
    weston_pid=""
}

start_weston_backend() {
    local backend="$1"
    local deadline socket_path

    socket_path="$headless_runtime_dir/$wayland_socket"
    env XDG_RUNTIME_DIR="$headless_runtime_dir" weston --backend="$backend" --socket="$wayland_socket" --log="$out_dir/weston.log" >>"$out_dir/weston.log" 2>&1 &
    weston_pid=$!
    deadline=$((SECONDS + HEADLESS_STARTUP_SECONDS))
    while [[ ! -S "$socket_path" ]]; do
        kill -0 "$weston_pid" 2>/dev/null || return 1
        (( SECONDS < deadline )) || return 1
        sleep 1
    done
    return 0
}

prepare_headless() {
    local -a missing=()

    command -v weston >/dev/null 2>&1 || missing+=(weston)
    lavapipe_icd="$(find_lavapipe_icd || true)"
    [[ -n "$lavapipe_icd" ]] || missing+=(lavapipe-icd)
    if [[ ${#missing[@]} -gt 0 ]]; then
        process_classification="failed-to-start"
        process_raw_status=2
        process_detail="headless prerequisites missing: ${missing[*]}"
        return 1
    fi

    headless_runtime_dir="$(mktemp -d)"
    chmod 700 "$headless_runtime_dir"
    wayland_socket="wayland-diag-$$"
    if ! start_weston_backend headless-backend.so; then
        stop_weston
        start_weston_backend headless || {
            process_classification="failed-to-start"
            process_raw_status=2
            process_detail="Weston did not create its headless Wayland socket"
            return 1
        }
    fi
    {
        printf '\nEffective headless environment:\n'
        printf 'WAYLAND_DISPLAY=%s\n' "$wayland_socket"
        printf 'VK_ICD_FILENAMES=%s\n' "$(sanitize_value VK_ICD_FILENAMES "$lavapipe_icd")"
    } >>"$environment_file"
    return 0
}

first_line_matching() {
    local pattern="$1"
    local file line
    shift

    for file in "$@"; do
        line="$(grep -Eim 1 "$pattern" "$file" 2>/dev/null || true)"
        if [[ -n "$line" ]]; then
            printf '%s\n' "$line"
            return 0
        fi
    done
    return 0
}

normalize_summary_line() {
    local line="$1"

    if [[ -n "${HOME:-}" ]]; then
        line="${line//"$HOME"/\$HOME}"
    fi
    line="${line//"$repo_root"/<repo>}"
    line="${line//"$out_dir"/<output>}"
    printf '%s\n' "$line" | sed -E \
        -e 's#(^|[[:space:](=])/[[:graph:]]+#\1<path>#g' \
        -e 's/[0-9]{4}-[0-9]{2}-[0-9]{2}[ T][0-9:.+-]+/<timestamp>/g' \
        -e 's/\[[0-9]{2}:[0-9]{2}:[0-9]+\.[0-9]+\]/[<time>]/g'
}

global_state() {
    local interface="$1"

    if command -v wayland-info >/dev/null 2>&1; then
        grep -Fq "$interface" "$wayland_globals_file" 2>/dev/null && printf 'present' || printf 'absent'
    elif [[ -s "$wayland_debug_file" ]]; then
        grep -Fq "$interface" "$wayland_debug_file" 2>/dev/null && printf 'present' || printf 'absent'
    else
        printf 'unknown (no Wayland globals captured)'
    fi
}

write_registry_bind_counts() {
    local line interface sorted_counts
    declare -A counts=()

    while IFS= read -r line; do
        [[ "$line" == *"wl_registry"*".bind("* ]] || continue
        if [[ "$line" =~ \"([^\"]+)\" ]]; then
            interface="${BASH_REMATCH[1]}"
            counts["$interface"]=$(( ${counts["$interface"]:-0} + 1 ))
        fi
    done <"$wayland_debug_file"

    if [[ ${#counts[@]} -eq 0 ]]; then
        printf 'none observed'
        return 0
    fi

    sorted_counts="$(
        for interface in "${!counts[@]}"; do
            printf '%s=%s\n' "$interface" "${counts["$interface"]}"
        done | sort
    )"
    printf '%s' "${sorted_counts//$'\n'/, }"
}

count_protocol_calls() {
    local pattern="$1"
    grep -Ec "$pattern" "$wayland_debug_file" 2>/dev/null || true
}

write_missing_tools() {
    local line missing_tool_list=""

    while IFS= read -r line; do
        [[ "$line" == *": missing" ]] || continue
        [[ -z "$missing_tool_list" ]] || missing_tool_list+=", "
        missing_tool_list+="${line%: missing}"
    done <"$tools_file"
    [[ -n "$missing_tool_list" ]] && printf '%s' "$missing_tool_list" || printf 'none'
}

write_summary() {
    local effective_video="unknown"
    local fallback_line device_line device_detail validation_line sdl_warning sdl_error missing_tools
    local sdl_logging_description wayland_display

    if [[ "$headless" == true ]]; then
        wayland_display="$wayland_socket"
    else
        wayland_display="$(sanitize_value WAYLAND_DISPLAY "${WAYLAND_DISPLAY:-}")"
    fi
    if [[ -s "$wayland_debug_file" ]]; then
        effective_video="wayland (protocol traffic observed)"
    fi
    fallback_line="$(first_line_matching 'fifo-v1|Failed to connect to the X11 display server' "$stderr_file" "$muerror_copy")"
    if [[ "$fallback_line" == *"Failed to connect to the X11 display server"* ]]; then
        effective_video="x11 (connection failed)"
    fi
    device_line="$(first_line_matching 'SDL_gpu -- device driver' "$stderr_file" "$muerror_copy")"
    device_detail="$(first_line_matching 'Vulkan Device:|llvmpipe|lavapipe' "$stderr_file" "$muerror_copy" "$vulkan_summary_file")"
    validation_line="$(first_line_matching 'VUID-|Validation|validation' "$validation_file")"
    sdl_warning="$(first_line_matching 'SDL.*([Ww]arn|[Ww]arning)|fifo-v1' "$stderr_file" "$muerror_copy")"
    sdl_error="$(first_line_matching 'SDL.*([Ee]rror|[Ff]ailed)|video init failed' "$stderr_file" "$muerror_copy")"
    missing_tools="$(write_missing_tools)"
    if [[ -n "${SDL_LOGGING+x}" ]]; then
        sdl_logging_description="preserved user setting"
    else
        sdl_logging_description="default video=verbose,gpu=verbose"
    fi

    {
        printf 'Environment\n'
        printf '  desktop: %s\n' "$(sanitize_value XDG_CURRENT_DESKTOP "${XDG_CURRENT_DESKTOP:-}")"
        printf '  session type: %s\n' "$(sanitize_value XDG_SESSION_TYPE "${XDG_SESSION_TYPE:-}")"
        printf '  Wayland display: %s\n' "$wayland_display"
        printf '  display: %s\n' "$(sanitize_value DISPLAY "${DISPLAY:-}")"
        printf '  headless: %s\n' "$headless"
        printf '  globals: fifo-v1=%s; presentation-time=%s; fractional-scale=%s; viewporter=%s; tearing-control=%s; explicit-sync=%s; xdg-decoration=%s\n' \
            "$(global_state wp_fifo_manager_v1)" \
            "$(global_state wp_presentation)" \
            "$(global_state wp_fractional_scale_manager_v1)" \
            "$(global_state wp_viewporter)" \
            "$(global_state wp_tearing_control_manager_v1)" \
            "$(global_state zwp_linux_explicit_synchronization_v1)" \
            "$(global_state zxdg_decoration_manager_v1)"
        printf '\nSteps\n'
        printf '  client: %s\n' "$(basename "$client_path")"
        printf '  client working directory: <client-directory>\n'
        printf '  client arguments: %s supplied\n' "${#client_args[@]}"
        printf '  timeout: %ss\n' "$timeout_seconds"
        printf '  SDL_LOGGING: %s\n' "$sdl_logging_description"
        printf '  Vulkan validation: %s\n' "$validation_reason"
        printf '  WAYLAND_DEBUG: client (line-routed; see capture-method.txt)\n'
        printf '\nResult\n'
        printf '  video backend evidence: %s\n' "$effective_video"
        if [[ -n "$fallback_line" ]]; then
            printf '  video fallback: %s\n' "$(normalize_summary_line "$fallback_line")"
        else
            printf '  video fallback: none observed\n'
        fi
        if [[ -n "$device_line" ]]; then
            printf '  Vulkan device: %s\n' "$(normalize_summary_line "$device_line")"
        else
            printf '  Vulkan device: not reported\n'
        fi
        if [[ -n "$device_detail" ]]; then
            printf '  Vulkan device detail: %s\n' "$(normalize_summary_line "$device_detail")"
        fi
        if [[ -n "$validation_line" ]]; then
            printf '  Vulkan validation: %s\n' "$(normalize_summary_line "$validation_line")"
        else
            printf '  Vulkan validation: no validation messages observed\n'
        fi
        printf '  WAYLAND_DEBUG: registry binds [%s]; configure=%s; ack_configure=%s; commits=%s; frame callbacks=%s\n' \
            "$(write_registry_bind_counts)" \
            "$(count_protocol_calls '\.configure\(')" \
            "$(count_protocol_calls '\.ack_configure\(')" \
            "$(count_protocol_calls '\.commit\(')" \
            "$(count_protocol_calls '\.frame\(')"
        if [[ -n "$sdl_warning" ]]; then
            printf '  first SDL warning: %s\n' "$(normalize_summary_line "$sdl_warning")"
        fi
        if [[ -n "$sdl_error" ]]; then
            printf '  first SDL error: %s\n' "$(normalize_summary_line "$sdl_error")"
        fi
        printf '  process: %s (raw exit code: %s)\n' "$process_classification" "$process_raw_status"
        if [[ -n "$process_detail" ]]; then
            printf '  detail: %s\n' "$(sanitize_value detail "$process_detail")"
        fi
        printf '  missing tools: %s\n' "$missing_tools"
        if [[ "$missing_tools" != none ]]; then
            printf '  evidence incomplete\n'
        fi
    } >"$summary_file"

    printf 'classification: %s\nraw-exit-code: %s\n' "$process_classification" "$process_raw_status" >"$exit_status_file"
    if [[ -n "$process_detail" ]]; then
        printf 'detail: %s\n' "$(sanitize_value detail "$process_detail")" >>"$exit_status_file"
    fi
}

create_archive_if_requested() {
    local archive_path

    [[ "$create_archive" == true ]] || return 0
    command -v tar >/dev/null 2>&1 || fail "--tar requested but tar is missing"
    archive_path="${out_dir}.tar.gz"
    tar -czf "$archive_path" -C "$(dirname "$out_dir")" "$(basename "$out_dir")"
    printf 'archive: %s\n' "$(basename "$archive_path")" >>"$summary_file"
}

cleanup() {
    stop_weston
    if [[ -n "$headless_runtime_dir" && -d "$headless_runtime_dir" ]]; then
        rm -rf "$headless_runtime_dir"
    fi
}

main() {
    parse_arguments "$@"
    select_client
    prepare_output_directory
    trap cleanup EXIT INT TERM

    write_environment_inventory
    write_tool_inventory
    write_version_inventory
    configure_validation
    write_capture_method

    if [[ "$headless" == true ]] && ! prepare_headless; then
        capture_optional_inventory
        write_summary
        create_archive_if_requested
        printf 'wayland-diag: %s\n' "$process_detail" >&2
        return 2
    fi

    capture_optional_inventory
    run_client
    write_summary
    create_archive_if_requested
    printf 'wayland-diag: artifacts written to %s\n' "$out_dir"

    if [[ "$process_classification" == failed-to-start ]]; then
        return 2
    fi
    return 0
}

main "$@"
