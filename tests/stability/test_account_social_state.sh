#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
ui="$root/src/source/UI/Legacy/UIWindows.cpp"
network="$root/src/source/Network/Server/WSclient.cpp"
selection="$root/src/source/Input/Selection.cpp"

if grep -q 'new wchar_t\[MAX_TEXT_LENGTH + 1\]' "$ui"; then
    echo "friend invite still transports manually allocated text" >&2
    exit 1
fi

character_list=$(sed -n '/void ReceiveCharacterListExtended/,/CurrentProtocolState = RECEIVE_CHARACTERS_LIST/p' "$network")
printf '%s\n' "$character_list" | grep -q 'ClearCharacters();'
printf '%s\n' "$character_list" | grep -q 'SelectedCharacter = -1;'
printf '%s\n' "$character_list" | grep -q 'SelectedHero = -1;'

character_selection=$(sed -n '/void SelectObjects()/,/if (g_pOption->IsAutoAttack()/p' "$selection")
printf '%s\n' "$character_selection" | grep -q 'if (SceneFlag == CHARACTER_SCENE)'
printf '%s\n' "$character_selection" | grep -q 'SelectedCharacter = SelectCharacter(KIND_PLAYER);'
printf '%s\n' "$character_selection" | grep -q 'return;'
