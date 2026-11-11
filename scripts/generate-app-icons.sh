#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
master_svg="$repo_root/src/source/App/Resources/MuMainIcon.svg"
work_dir="${MU_ICON_WORK_DIR:-$(mktemp -d)}"
mkdir -p "$work_dir"
if [[ -z "${MU_ICON_WORK_DIR:-}" ]]; then
    trap 'rm -rf "$work_dir"' EXIT
fi

renderer="$work_dir/render_svg_icon"
small_svg="$work_dir/MuMainIconSmall.svg"
iconset="$work_dir/MuMain.iconset"
mkdir -p "$iconset"

clang -fobjc-arc -framework AppKit "$repo_root/scripts/render_svg_icon.m" -o "$renderer"
sed \
    -e 's/#ffe69a/#fff2b0/g' \
    -e 's/#dca34a/#efb85b/g' \
    "$master_svg" > "$small_svg"

render() {
    "$renderer" "$1" "$2" "$3"
}

render "$master_svg" "$repo_root/src/source/App/Resources/MuMainIcon1024.png" 1024
render "$small_svg" "$work_dir/MuMainIconSmall1024.png" 1024
sips -z 256 256 "$repo_root/src/source/App/Resources/MuMainIcon1024.png" \
    --out "$repo_root/src/bin/MuMainIcon.png" >/dev/null

resize() {
    sips -z "$3" "$3" "$1" --out "$2" >/dev/null
}

resize "$work_dir/MuMainIconSmall1024.png" "$iconset/icon_16x16.png" 16
resize "$work_dir/MuMainIconSmall1024.png" "$iconset/icon_16x16@2x.png" 32
resize "$work_dir/MuMainIconSmall1024.png" "$iconset/icon_32x32.png" 32
resize "$work_dir/MuMainIconSmall1024.png" "$iconset/icon_32x32@2x.png" 64
resize "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$iconset/icon_128x128.png" 128
resize "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$iconset/icon_128x128@2x.png" 256
resize "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$iconset/icon_256x256.png" 256
resize "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$iconset/icon_256x256@2x.png" 512
resize "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$iconset/icon_512x512.png" 512
cp "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$iconset/icon_512x512@2x.png"

python3 "$repo_root/scripts/build_icns.py" \
    "$repo_root/src/source/App/Platform/macOS/MuMain.icns" \
    "icp4=$iconset/icon_16x16.png" \
    "icp5=$iconset/icon_32x32.png" \
    "icp6=$iconset/icon_32x32@2x.png" \
    "ic07=$iconset/icon_128x128.png" \
    "ic08=$iconset/icon_256x256.png" \
    "ic09=$iconset/icon_512x512.png" \
    "ic10=$iconset/icon_512x512@2x.png"

resize "$work_dir/MuMainIconSmall1024.png" "$work_dir/icon-16.png" 16
resize "$work_dir/MuMainIconSmall1024.png" "$work_dir/icon-32.png" 32
resize "$work_dir/MuMainIconSmall1024.png" "$work_dir/icon-48.png" 48
resize "$repo_root/src/source/App/Resources/MuMainIcon1024.png" "$work_dir/icon-256.png" 256
python3 "$repo_root/scripts/build_ico.py" \
    "$repo_root/src/source/App/Platform/Windows/icon1.ico" \
    "16=$work_dir/icon-16.png" \
    "32=$work_dir/icon-32.png" \
    "48=$work_dir/icon-48.png" \
    "256=$work_dir/icon-256.png"
