#!/bin/sh
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
renderer="$root/src/source/Render/Renderer/MuRendererSDLGpu.cpp"
connection="$root/ClientLibrary/ConnectionWrapper.cs"

grep -q 'emitTimingDiagnostics = s_frameTimingEnabled' "$renderer"
! grep -q 's_dbgFrameCount % 300' "$renderer"
grep -q 'logger->debug("\[RENDER diag\]' "$renderer"
grep -q 'Get("render")->warn' "$renderer"
grep -q 'if (!this._networkDiagnosticsEnabled)' "$connection"
grep -q 'Console.Error.WriteLine($"\[OutboundQueue\]' "$connection"
