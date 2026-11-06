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
  # Unpacked once: the tag is appended to the tar, so every other entry stays as the build wrote it.
  gzip -dc "$archive" > "$work/archive.tar"
  tar -tf "$work/archive.tar" > "$work/entries.txt"

  # The macOS archive holds Main.app, whose client runs in Contents/MacOS; the others hold the client's folder.
  if grep -q '^Main\.app/' "$work/entries.txt"; then
    folder="Main.app/Contents/MacOS"
  else
    folder="."
  fi

  # The client reads the tag from its own folder, so that folder must hold the client: an archive laid out another
  # way fails the release rather than shipping clients that say "none (not a release download)".
  if [ "$folder" = "." ]; then
    clients=(-e ./Main.exe -e ./Main -e Main.exe -e Main)
  else
    clients=(-e "$folder/Main")
  fi
  if ! grep -Fxq "${clients[@]}" "$work/entries.txt"; then
    echo "$archive has no client in $folder; not stamping an archive laid out another way" >&2
    exit 1
  fi

  rm -rf "$work/stamp"
  mkdir -p "$work/stamp/$folder"
  printf '%s\n' "$tag" > "$work/stamp/$folder/release-version.txt"

  tar -rf "$work/archive.tar" -C "$work/stamp" "$folder/release-version.txt"
  gzip -n -c "$work/archive.tar" > "$archive"
  echo "Stamped $archive with $tag ($folder/release-version.txt)"
done
