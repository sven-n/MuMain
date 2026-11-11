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

# The folder the client runs from in the unpacked archive $1; fails for an archive laid out another way.
client_folder() {
  local entries="$work/entries.txt" folder
  tar -tf "$1" > "$entries" || return 1

  # The macOS archive holds Main.app, whose client runs in Contents/MacOS; the others hold the client's folder.
  if grep -q '^Main\.app/' "$entries"; then
    folder="Main.app/Contents/MacOS"
  else
    folder="."
  fi

  # The client reads the tag from its own folder, so that folder must hold the client: an archive laid out another
  # way fails the release rather than shipping clients that say "none (not a release download)".
  local clients
  if [ "$folder" = "." ]; then
    clients=(-e ./Main.exe -e ./Main -e Main.exe -e Main)
  else
    clients=(-e "$folder/Main")
  fi
  grep -Fxq "${clients[@]}" "$entries" || return 1
  printf '%s\n' "$folder"
}

# Every archive is checked before any is rewritten, so a failure leaves all of them as the build wrote them.
# Each is unpacked once: the tag is appended to the tar, so every other entry stays as the build wrote it.
folders=()
index=0
for archive in "$@"; do
  gzip -dc "$archive" > "$work/$index.tar"
  if ! folder=$(client_folder "$work/$index.tar"); then
    echo "$archive has no client where the tag would go; not stamping an archive laid out another way" >&2
    exit 1
  fi
  folders+=("$folder")
  index=$((index + 1))
done

index=0
for archive in "$@"; do
  folder="${folders[$index]}"
  rm -rf "$work/stamp"
  mkdir -p "$work/stamp/$folder"
  printf '%s\n' "$tag" > "$work/stamp/$folder/release-version.txt"

  tar -rf "$work/$index.tar" -C "$work/stamp" "$folder/release-version.txt"
  gzip -n -c "$work/$index.tar" > "$archive"
  echo "Stamped $archive with $tag ($folder/release-version.txt)"
  index=$((index + 1))
done
