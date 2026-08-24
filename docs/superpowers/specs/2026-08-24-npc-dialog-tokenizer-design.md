# NPC Dialogue Tokenizer Fix

## Problem

`DivideStringByPixel()` passes `&szNewlineChar` to `wcstok_s()`. The tokenizer
expects a NUL-terminated delimiter string. Release builds place uninitialized
stack storage immediately after that character, so its contents can become
additional delimiters. NPC dialogue then splits into fragments such as `av`,
`waiti`, and `g`.

## Design

Create a local two-character delimiter array containing `szNewlineChar` and
`L'\0'`. Pass that array to both tokenizer calls. Preserve the existing API,
newline behavior, wrapping algorithm, renderer, and all callers.

## Scope

- Change only `DivideStringByPixel()` delimiter construction.
- Add one deterministic regression contract rejecting the unsafe single-character
  pointer pattern.
- Do not change the proposed renderer middleware or the 96 measurement callers.
- Do not touch ground-item rendering.

## Verification

- Observe the regression test fail before the production change.
- Run the focused regression test after the change.
- Run the existing UI tests and Release build.
- Reopen NPC 257 at 1024x768 and visually confirm normal paragraph wrapping.
