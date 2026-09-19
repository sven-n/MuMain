# Reviewed Changelog Releases Design

## Goal

Prevent release automation from committing directly to `main`. Require
`CHANGELOG.md` changes to arrive through the normal pull-request workflow while
preserving automatic version calculation, tags, GitHub releases, and assets.

## Current behavior

- `@semantic-release/changelog` updates `CHANGELOG.md` during a release.
- `@semantic-release/git` commits that update directly to `main`.
- Existing release tags point to those bot commits. Rewriting them would disturb
  published history and is out of scope.

## Design

Remove `@semantic-release/changelog` and `@semantic-release/git` from the
semantic-release configuration and GitHub Actions plugin installation list.
Keep commit analysis, generated GitHub release notes, GitHub tags, uploaded
assets, and the existing `CHANGELOG.md` history unchanged.

Future changelog edits are authored before release and reviewed in pull
requests. Semantic-release does not modify the file or push commits to
`main`.

## Verification

- Parse `.releaserc.json` as JSON.
- Parse `.github/workflows/ci.yml` as YAML.
- Confirm neither removed plugin remains in release configuration.
- Inspect the diff to ensure no release behavior beyond changelog commits was
  removed.
