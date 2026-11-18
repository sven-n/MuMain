# Pull Request Rules

These rules apply to every pull request, by humans and by AI agents alike. The
code itself follows [`CODING_RULES.md`](CODING_RULES.md); this file covers what
the PR has to show.

---

## 1. Before and After Screenshots or Videos for Visible Changes

A PR whose change can be seen in the game or in the editor has screenshots or
videos of the result before and after the change. Reviewers judge a visible
change by looking at it, and most of them can't build and run every branch.
Wherever this rule says screenshots, a video (or GIF) works too.

### When they are required

Every new feature, bug fix or other change that changes what is on screen or
how something behaves in game, for example:

- UI: windows, dialogs, HUD, tooltips, texts and translations, fonts, cursors.
- Rendering: models, textures, effects, lighting, animations, maps, camera.
- Gameplay that shows on screen: movement, skills, items, monsters, NPCs,
  inventory, trade, shop, chat.
- MuEditor windows and tools.

### When they are not required

Changes with no visible effect: refactors that keep the behaviour, build, CI,
tests, tooling, docs, and internal code paths nobody can see. If you are
unsure, add them.

### How to take them

- **Before** is a build of the PR's base branch (usually current `main`),
  **after** is a build of the PR branch.
- Same scene for both: same map and place, camera, character, options and
  window size, so the two can be compared directly.
- Put them side by side or one after the other, labelled *Before* and *After*,
  for example in a table:

  | Before | After |
  | --- | --- |
  | ![before](...) | ![after](...) |

- A new feature with nothing to compare against shows *after* only; add a
  *before* when it helps to see where the feature appears.
- A bug fix shows the bug in *before* and the fixed result in *after*.
- Crop to the part that changed but keep enough around it to recognise the
  place. Mark small changes (an arrow or a box).
- Videos are welcome for any change and are the better choice for things a
  still image can't show well (an animation, timing, movement). Keep them
  short. A change that is only heard (sound, music) can use a video with
  sound, or says in the PR how it was checked.
- Upload the images and videos into the PR description on GitHub (drag and
  drop or paste). Don't commit them to the repository.

### PRs written by AI agents

An agent that can't run the game and take screenshots or videos says so in the PR
description and leaves the *Before* / *After* section for the person who opens
the PR. The PR is ready for review only once the screenshots or videos are
there.
