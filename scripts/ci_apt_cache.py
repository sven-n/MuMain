#!/usr/bin/env python3
"""Gathers the files of an apt package cache in ci.yml (Gather apt packages):

    python3 scripts/ci_apt_cache.py gather LISTING ARCHIVES DESTINATION

LISTING is the output of `apt-get install --print-uris`, which names the files apt fetches for the packages. They are
copied from apt's archive folder (ARCHIVES) into a new DESTINATION, the cache's folder, and complete=true goes to
GITHUB_OUTPUT only when every one of them was there. A missing file gets a notice: the save then stores nothing, and
the job would otherwise pass without a word while every later run fetches the files again.
"""

import os
import shutil
import sys
from pathlib import Path

# A notice names this many missing files, then counts the rest.
NAMED_MISSING = 10


def listed_files(text):
    """The file names in `apt-get install --print-uris` output. Each file is a line 'URI' name size hash; other lines,
    such as a message a newer apt prints, name no file."""
    names = []
    for line in text.splitlines():
        fields = line.split()
        if len(fields) >= 4 and len(fields[0]) > 2 and fields[0].startswith("'") and fields[0].endswith("'"):
            names.append(fields[1])
    return names


def gather(names, archives, destination):
    """Copies the named files from archives into destination, which holds nothing else afterwards, and returns the
    names that archives lacks."""
    shutil.rmtree(destination, ignore_errors=True)
    destination.mkdir(parents=True)
    missing = []
    for name in names:
        source = archives / name
        if source.is_file():
            shutil.copy2(source, destination / name)
        else:
            missing.append(name)
    return missing


def missing_notice(missing, listed, archives):
    named = " ".join(missing[:NAMED_MISSING])
    more = f" and {len(missing) - NAMED_MISSING} more" if len(missing) > NAMED_MISSING else ""
    return (f"::notice title=apt packages not saved::{len(missing)} of {listed} listed files are not in {archives}, "
            f"so Save apt packages stores nothing: {named}{more}")


def write_output(name, value):
    """A step output, to GITHUB_OUTPUT when the runner names it, else to stdout."""
    line = f"{name}={value}\n"
    path = os.environ.get("GITHUB_OUTPUT")
    if path:
        with open(path, "a", encoding="utf-8") as output:
            output.write(line)
    else:
        sys.stdout.write(line)


def main(argv):
    if len(argv) != 4 or argv[0] != "gather":
        raise SystemExit(__doc__)
    listing, archives, destination = (Path(arg) for arg in argv[1:])
    names = listed_files(listing.read_text(encoding="utf-8"))
    missing = gather(names, archives, destination)
    if missing:
        print(missing_notice(missing, len(names), archives))
    elif not names:
        print("apt lists no file to fetch, so there is nothing to save.")
    write_output("complete", "true" if names and not missing else "false")


if __name__ == "__main__":
    main(sys.argv[1:])
