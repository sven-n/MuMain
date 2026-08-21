#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
winmain="$root/src/source/App/Platform/Windows/Winmain.cpp"
controls="$root/src/source/UI/Legacy/UIControls.cpp"
controls_h="$root/src/source/UI/Legacy/UIControls.h"
ttf="$root/src/source/Render/Text/CUIRenderTextSDLTtf.cpp"

# Button events must update virtual coordinates even without a preceding motion event.
grep -q 'HandleMouseMotion(e.button.x, e.button.y);' "$winmain"

# Adjacent text fields must not claim overlapping padded hit regions or share a stale click latch.
grep -q 'CheckMouseIn(m_iPos_x, m_iPos_y, m_iWidth, m_iHeight)' "$controls"
! grep -q 's_mousePressHandled' "$controls"

# Caret placement must use the active renderer's glyph metrics, not fallback GDI cell widths.
grep -q 'virtual SIZE MeasureText' "$controls_h"
grep -q 'g_pRenderText->MeasureText' "$controls"
grep -q 'SIZE CUIRenderTextSDLTtf::MeasureText' "$ttf"
