# Pull Request Rules

These rules apply to every pull request, by humans and by AI agents alike. The
code itself follows [`CODING_RULES.md`](CODING_RULES.md); this file covers what
the PR has to show: screenshots or videos of visible changes, what was tested,
and that the coding rules were followed.

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

## 2. Write Down What Was Tested

The PR description lists what was done to check the change, so reviewers know
what is covered and what still needs a look. "Tested" alone is not enough.

- **Build:** which platform and configuration it was built on (for example
  Windows x64 Release, Linux, macOS).
- **Tested in game:** each case played through, with the steps, the server
  (for example OpenMU and its version), the map, the character or class, and
  the result. Bug fixes also say that the bug was reproduced on the base branch
  first.
- **In-game tests:** which scenarios of the [in-game tests](in-game-tests.md)
  were run and their result; attach the report when it helps.
- **Automated tests:** which tests were added or changed, and that the test
  suite passes.
- **Not tested:** what wasn't checked and why (a platform you don't have, a
  case that needs two accounts, ...).

A list of checkboxes works well:

```markdown
## Testing

- [x] Built on Windows x64 Release
- [x] In game (OpenMU): sold an item at the Lorencia shop, Zen updated, item gone
- [x] Bug reproduced on main first: item stayed in the inventory
- [x] In-game tests: `trade-item` passed (report attached)
- [x] `ctest` passes, new test `ShopSellTests`
- [ ] Not tested on Linux/macOS
```

Docs-only or CI-only PRs say so in one line.

## 3. Confirm the Coding Rules

The PR description confirms that [`CODING_RULES.md`](CODING_RULES.md) and this
file were read and followed for the change. Where the change departs from a
rule on purpose (for example to keep a diff small in old code), name the rule
and say why, so the reviewer doesn't have to find it.
