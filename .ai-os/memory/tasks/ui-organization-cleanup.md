# UI organization cleanup

Status: complete. Implemented and validated on 2026-10-04.
Date: 2026-10-04. Base: `9a56ff680`, branch `dev/rmlui-ui-system`.
Source: [organization review](ui-organization-review.md).

## Scope and completed work

- [x] Correct the new-UI guide: RmlUi/base.rcss is the default for visible controls;
  native widgets apply to documented companions. Remove obsolete RadioButton
  collision/text-entry fallback guidance.
- [x] Move `ChatInputBox.{h,cpp}` and `SlideWindow.{h,cpp}` from
  `src/source/UI/Widgets/Window` to `src/source/UI/HUD`.
- [x] Rename Social file pairs: ChatRoom -> ChatRoomView, LetterRead -> LetterReadView,
  LetterWrite -> LetterWriteView, FriendWindowView -> FriendWindowViews.
- [x] Update includes and the FriendWindow comment; keep class names, namespaces,
  document paths, registration and behavior intact.
- [x] Carry remaining review findings into named follow-up tasks.
- [x] Compile and link Windows RelWithDebInfo; no introduced build errors.
- [x] Record final checks and close this task.

## Validation

Source collection uses CMake CONFIGURE_DEPENDS. No explicit test/build references to
the old moved paths were found. Verify moved files against their original contents
with only the expected include changes, search old paths, and run git diff --check.
Build log: `out/build/windows-x64/ui-organization-cleanup-build.log`.

Results:

- Windows RelWithDebInfo: exit 0; recompiled affected sources, linked MuClient.lib
  and Main.exe successfully after CMake detected the file moves.
- RML/RCSS syntax, geometry ownership, theme drift and renderer guards passed.
- All 12 moved files were compared with HEAD: only expected include paths changed
  (line endings normalized for comparison).
- No old moved paths remain in tracked source, tests, Tools or docs searches.
- `git diff --check` passed. No gameplay, scale, transform or asset changes.
- BUILD_TESTING remains OFF; no CTest or interactive checks were run for these file moves.
- The existing VS wrapper emitted a vswhere lookup diagnostic, but compilation and
  linking completed successfully. No build-environment files were changed.

## Remaining work outside this batch

- [Social manager cleanup](ui-social-manager-cleanup.md): optional type extraction
  and the outstanding AddWindow placement/failure-flow cleanup.
- [Portrait drag-release investigation](ui-portrait-drag-release-investigation.md):
  separate correctness investigation, still unconfirmed at runtime.

No commits or pushes requested. Review documents retain their historical findings
with explicit closure/disposition notes rather than silently rewriting the audit.
