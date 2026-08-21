#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
winmain="$root/src/source/App/Platform/Windows/Winmain.cpp"
ui_manager="$root/src/source/UI/Legacy/UIMng.cpp"
server_client="$root/src/source/Network/Server/WSclient.cpp"

grep -q 'MU_INPUT_DIAGNOSTICS' "$winmain"
grep -q '\[InputDiag\].*focus' "$winmain"
grep -q '\[InputDiag\].*mouse' "$winmain"
grep -q 'MU_INPUT_DIAGNOSTICS' "$ui_manager"
grep -q '\[InputDiag\].*hover' "$ui_manager"
grep -q '\[InputDiag\].*active' "$ui_manager"
grep -q '\[InputDiag\].*server-list' "$server_client"
