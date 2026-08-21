#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
header="$root/src/source/Network/IncomingPacketQueue.h"
source="$root/src/source/Network/IncomingPacketQueue.cpp"
client="$root/src/source/Network/Server/WSclient.cpp"
console_debug="$root/src/source/Core/Utilities/Log/muConsoleDebug.cpp"

grep -q 'struct Stats' "$header"
grep -q 'oldestPacketAgeMs' "$header"
grep -q 'highWaterMark' "$header"
grep -q 'coalescedActionCount' "$header"
grep -q 'MarkSupersededActions' "$source"
grep -q 'OpenMuBasicMonsterAttackAnimation = 120' "$source"
grep -q 'action->Action != OpenMuBasicMonsterAttackAnimation' "$source"
grep -q '\[PacketQueue\]' "$source"
grep -q 'Get("network")->debug' "$source"
! grep -q 'fprintf(stderr' "$source"
grep -q 'SuppressOptionalPresentation' "$client"
! grep -q 'm_actionPackets' "$header"
! grep -q 'GetStats()' "$client"
grep -q 'MU_NETWORK_DIAGNOSTICS' "$console_debug"
