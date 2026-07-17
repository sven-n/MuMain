#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
header="$root/src/source/Network/IncomingPacketQueue.h"
source="$root/src/source/Network/IncomingPacketQueue.cpp"
client="$root/src/source/Network/Server/WSclient.cpp"

grep -q 'struct Stats' "$header"
grep -q 'oldestPacketAgeMs' "$header"
grep -q 'highWaterMark' "$header"
grep -q 'coalescedActionCount' "$header"
grep -q 'MarkSupersededActions' "$source"
grep -q 'SuppressOptionalPresentation' "$client"
! grep -q 'm_actionPackets' "$header"
! grep -q 'GetStats()' "$client"
