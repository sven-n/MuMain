# Portrait drag-release correctness investigation

Status: open investigation; source observation, not a reproduced runtime defect.
Source: [shallow review](ui-shallow-cleanup-review.md).
Rechecked at `9a56ff680` on 2026-10-04.

## Evidence and hypothesis

`src/source/UI/Social/PhotoViewerControl.cpp` listens for Mousemove/Mouseup on the
owning document. A left press starts m_Turning; mouse-up or Detach clears it.
The vendored RmlUi context dispatches pointer events to the current hover target.
A release over another document or the background may not reach this listener,
leaving turning active when the cursor returns. Verify dispatch/capture behavior
before treating this hypothesis as confirmed.

## Reproduction matrix

1. Turn a letter portrait, leave its slot but remain inside the letter, then release.
2. Repeat, releasing over a different document and over empty background.
3. Hide/close the letter during a drag; reopen or open another letter afterward.
4. Lose application focus during a drag, release, then return.
5. Repeat at representative UI scales in legacy and modern themes; check right-click
   reset, help, zoom and ordinary turning.

After releasing, returning without pressing must not rotate the portrait. Record
which cases were actually run and whether the hypothesis reproduces.

## If confirmed

Choose the smallest existing input-lifecycle mechanism for release/cancellation.
Check listener removal, document reload/destruction and focus loss. Keep drag delta
conversion through FloatingWorkspaceTransform and preserve portrait scale/placement.
Add a focused regression check where feasible and run RelWithDebInfo.

## Completion

Close with reproducible evidence plus validated fix, or explain with verified event
behavior why the reported failure cannot occur. Do not close based on compilation alone.
