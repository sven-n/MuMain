@echo off
REM ============================================================
REM  Double-click to build the 64-bit MuMain client WITH the
REM  in-game MuEditor (Map Editor, Item/Skill/Dev editors).
REM  - Uses the ENABLE_EDITOR preset (defines _EDITOR, pulls in
REM    src\MuEditor, builds ImGui).
REM  - First run auto-configures the x64-mueditor build tree.
REM  - If Release\Data is missing, forces a full asset re-copy.
REM  - Recompiles only what changed.
REM  Output: out\build\windows-x64-mueditor\src\Release\Main.exe
REM  Run it with:  Main.exe --editor   (or press F12 in-game)
REM ============================================================
if not exist "%~dp0out\build\windows-x64-mueditor\CMakeCache.txt" (
  echo First run - configuring x64-mueditor build tree...
  call "%~dp0build-env-x64.cmd" cmake --preset windows-x64-mueditor
)
if not exist "%~dp0out\build\windows-x64-mueditor\src\Release\Data" (
  echo Release\Data missing - forcing a full asset re-copy...
  del /q "%~dp0out\build\windows-x64-mueditor\src\.assets_copied_Release.stamp" 2>nul
)
call "%~dp0build-env-x64.cmd" cmake --build --preset windows-x64-mueditor-release
if errorlevel 1 (echo. & echo BUILD FAILED. & pause & exit /b 1)
echo.
echo ============================================================
echo  Done. Editor client:
echo    out\build\windows-x64-mueditor\src\Release\Main.exe
echo  Launch with --editor, or press F12 in-game to toggle it.
echo ============================================================
pause
