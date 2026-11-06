#!/usr/bin/env bash
# Writes the release tag into each client archive as release-version.txt, next to the client, so MuError.log names
# the release a report came from. semantic-release's prepare step runs it (.releaserc.json), after it chose the tag
# and before @semantic-release/github uploads the archives.
#   stamp_release_version.sh TAG ARCHIVE.tar.gz...
set -eu

tag="${1:?usage: stamp_release_version.sh TAG ARCHIVE.tar.gz...}"
shift
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

for archive in "$@"; do
  # The macOS archive holds Main.app, whose client runs in Contents/MacOS; the others hold the client's folder.
  if tar -tzf "$archive" | grep -q '^Main\.app/'; then
    folder="Main.app/Contents/MacOS"
  else
    folder="."
  fi
  rm -rf "$work/stamp"
  mkdir -p "$work/stamp/$folder"
  printf '%s\n' "$tag" > "$work/stamp/$folder/release-version.txt"

  # Appended to the unpacked tar, so every other entry stays as the build wrote it.
  gzip -dc "$archive" > "$work/archive.tar"
  tar -rf "$work/archive.tar" -C "$work/stamp" "$folder/release-version.txt"
  gzip -n -c "$work/archive.tar" > "$archive"
  echo "Stamped $archive with $tag ($folder/release-version.txt)"
done
