#!/bin/bash
#
# convert_all_assets.sh
# =====================
#
# Wrapper script to convert all MU Online legacy assets to modern formats.
# This script orchestrates both texture and model conversion pipelines.
#
# Usage:
#   ./convert_all_assets.sh [OPTIONS]
#
# Options:
#   --textures-only                   Only convert textures (skip models)
#   --models-only                     Only convert models (skip textures)
#   --worlds LIST                     Convert only selected worlds (ex: 74 or 74,75)
#   --no-embed-textures               Keep external PNG references in GLBs
#   --keep-object-png-textures         Keep object PNG files even when embedding into GLB
#   --disable-embedded-texture-cleanup Skip deletion of embedded PNGs in output folders
#   --dry-run                         Show what would be done without executing
#   --force                           Force reconversion of all files
#   --verbose                         Enable verbose logging
#   --help                            Show this help message
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUST_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
REPO_ROOT="$(cd "$RUST_ROOT/.." && pwd)"

# Default paths
#LEGACY_ROOT="${LEGACY_ROOT:-cpp/MuClient5.2/bin/Data}"
#LEGACY_ROOT="${LEGACY_ROOT:-cpp/MuClient5.2/bin/DataNorm}"
LEGACY_ROOT="${LEGACY_ROOT:-/home/allanbatista/Workspaces/Mu/MU_Red_1_20_61_Full/DataNorm2}"
#LEGACY_ROOT="${LEGACY_ROOT:-/home/allanbatista/Workspaces/MuData/Season2/DataNorm}"
#LEGACY_ROOT="${LEGACY_ROOT:-/home/allanbatista/Workspaces/MuData/Season20/DataNorm}"
OUTPUT_ROOT="${OUTPUT_ROOT:-$RUST_ROOT/assets}"
BMD_CONVERTER="${BMD_CONVERTER:-$SCRIPT_DIR/bmd_converter.py}"
PYTHON_BIN="${PYTHON_BIN:-}"

if [[ -z "$PYTHON_BIN" ]]; then
    if [[ -x "$SCRIPT_DIR/.venv/bin/python" ]]; then
        PYTHON_BIN="$SCRIPT_DIR/.venv/bin/python"
    else
        PYTHON_BIN="$(command -v python3 || command -v python)"
    fi
fi

if [[ "$LEGACY_ROOT" != /* ]]; then
    LEGACY_ROOT="$REPO_ROOT/$LEGACY_ROOT"
fi
if [[ "$OUTPUT_ROOT" != /* ]]; then
    OUTPUT_ROOT="$REPO_ROOT/$OUTPUT_ROOT"
fi
if [[ "$BMD_CONVERTER" != /* ]]; then
    BMD_CONVERTER="$SCRIPT_DIR/$BMD_CONVERTER"
fi
if [[ "$PYTHON_BIN" != /* ]]; then
    PYTHON_BIN="$REPO_ROOT/$PYTHON_BIN"
fi
if [ ! -x "$PYTHON_BIN" ]; then
    log_error "Python interpreter not found or not executable: $PYTHON_BIN"
    exit 1
fi

# Normalize data output path to avoid .../data/data when OUTPUT_ROOT already ends with data.
DATA_OUTPUT_ROOT="$OUTPUT_ROOT"
if [[ "${DATA_OUTPUT_ROOT##*/}" != "data" ]]; then
    DATA_OUTPUT_ROOT="$DATA_OUTPUT_ROOT/data"
fi

# Flags
CONVERT_TEXTURES=true
CONVERT_MODELS=true
DRY_RUN=""
FORCE=""
VERBOSE=""
WORLD_FILTER_ARGS=()
MODEL_TEXTURE_ARGS=()

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

function print_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Convert MU Online legacy assets to modern formats."
    echo ""
    echo "Options:"
    echo "  --textures-only     Only convert textures (skip models)"
    echo "  --models-only       Only convert models (skip textures)"
    echo "  --worlds LIST       Convert only selected worlds (ex: 74 or 74,75)"
    echo "  --no-embed-textures               Keep external PNG references in GLBs"
    echo "  --keep-object-png-textures         Keep object PNG files even when embedding into GLB"
    echo "  --disable-embedded-texture-cleanup Skip deletion of embedded PNGs in output folders"
    echo "  --dry-run                         Show what would be done without executing"
    echo "  --force                           Force reconversion of all files"
    echo "  --verbose                         Enable verbose logging"
    echo "  --help                            Show this help message"
    echo ""
    echo "Environment Variables:"
    echo "  LEGACY_ROOT         Path to legacy assets (default: $LEGACY_ROOT)"
    echo "  OUTPUT_ROOT         Path to output directory (default: $OUTPUT_ROOT)"
    echo "  BMD_CONVERTER       Path to bmd_converter.py (default: $BMD_CONVERTER)"
    echo ""
}

function log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

function log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

function log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

function log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --textures-only)
            CONVERT_MODELS=false
            shift
            ;;
        --models-only)
            CONVERT_TEXTURES=false
            shift
            ;;
        --world|--worlds)
            if [[ -z "${2:-}" ]]; then
                log_error "Missing value for $1 (expected world list like 74 or 74,75)"
                exit 1
            fi
            WORLD_FILTER_ARGS+=("--world" "$2")
            shift 2
            ;;
        --no-embed-textures)
            MODEL_TEXTURE_ARGS+=("--no-embed-textures")
            shift
            ;;
        --keep-object-png-textures)
            MODEL_TEXTURE_ARGS+=("--keep-object-png-textures")
            shift
            ;;
        --disable-embedded-texture-cleanup)
            MODEL_TEXTURE_ARGS+=("--disable-embedded-texture-cleanup")
            shift
            ;;
        --dry-run)
            DRY_RUN="--dry-run"
            shift
            ;;
        --force)
            FORCE="--force"
            shift
            ;;
        --verbose)
            VERBOSE="--verbose"
            shift
            ;;
        --help)
            print_help
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            print_help
            exit 1
            ;;
    esac
done

# Validate paths
if [ ! -d "$LEGACY_ROOT" ]; then
    log_error "Legacy root directory not found: $LEGACY_ROOT"
    exit 1
fi

log_info "Starting asset conversion pipeline"
log_info "Legacy root: $LEGACY_ROOT"
log_info "Output root: $OUTPUT_ROOT"
log_info "Data output root: $DATA_OUTPUT_ROOT"
log_info "Python runtime: $PYTHON_BIN"
if [ ${#WORLD_FILTER_ARGS[@]} -gt 0 ]; then
    log_info "World filter args: ${WORLD_FILTER_ARGS[*]}"
fi
if [ ${#MODEL_TEXTURE_ARGS[@]} -gt 0 ]; then
    log_info "Model texture args: ${MODEL_TEXTURE_ARGS[*]}"
fi

START_TIME=$(date +%s)

# Step 1: Convert textures
if [ "$CONVERT_TEXTURES" = true ]; then
    log_info "Converting textures and auxiliary assets..."

    "$PYTHON_BIN" "$SCRIPT_DIR/assets_convert.py" \
        --legacy-root "$LEGACY_ROOT" \
        --output-root "$DATA_OUTPUT_ROOT" \
        --skip-models \
        "${WORLD_FILTER_ARGS[@]}" \
        $DRY_RUN \
        $FORCE \
        $VERBOSE \
        --report "$OUTPUT_ROOT/reports/textures_report.json"

    if [ $? -eq 0 ]; then
        log_success "Texture conversion completed"
    else
        log_error "Texture conversion failed"
        exit 1
    fi
else
    log_info "Skipping texture conversion (--models-only specified)"
fi

# Step 2: Convert models (BMD → GLB)
if [ "$CONVERT_MODELS" = true ]; then
    log_info "Converting 3D models (BMD → GLB)..."

    "$PYTHON_BIN" "$BMD_CONVERTER" \
        --bmd-root "$LEGACY_ROOT" \
        --output-root "$DATA_OUTPUT_ROOT" \
        --format glb \
        "${WORLD_FILTER_ARGS[@]}" \
        "${MODEL_TEXTURE_ARGS[@]}" \
        $DRY_RUN \
        $FORCE \
        $VERBOSE \
        --report "$OUTPUT_ROOT/reports/models_report.json"

    if [ $? -eq 0 ]; then
        log_success "Model conversion completed"
    else
        log_error "Model conversion failed"
        exit 1
    fi
else
    log_info "Skipping model conversion (--textures-only specified)"
fi

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

log_success "Asset conversion pipeline completed in ${DURATION}s"
log_info "Output directory: $OUTPUT_ROOT"

if [ -f "$OUTPUT_ROOT/reports/textures_report.json" ] || [ -f "$OUTPUT_ROOT/reports/models_report.json" ]; then
    log_info "Conversion reports available in: $OUTPUT_ROOT/reports/"
fi
