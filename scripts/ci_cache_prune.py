#!/usr/bin/env python3
"""Deletes the older entries of each cache kind on this run's branch, so that one of each is left (of the compiler
caches, one for each compiler).

ci.yml names a cache after what it holds: the compiler caches after the commit, the library and vcpkg caches after the
files that pick their content. A run that saves under a new name leaves the older entries behind. GitHub keeps them
until they go unused for 7 days, or until the repository's caches pass 10 GB; then it deletes the least recently used
entries first, and those can be the newest entries that the next PRs and main pushes would start from. Run it after
the jobs that save caches, with `actions: write` and the token in GH_TOKEN:

    python3 scripts/ci_cache_prune.py
"""

import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from datetime import datetime, timedelta, timezone

from ci_job_annotations import REQUEST_ATTEMPTS, REQUEST_ERRORS, api_headers, get_pages, retry_delay

# A job that found an entry may still be downloading it, and deleting the entry then would fail that restore with a
# warning, which fails CI Required. GitHub sets last_accessed_at when a restore finds the entry, and actions/cache gives
# up on a download after 10 minutes (SEGMENT_DOWNLOAD_TIMEOUT_MINS); an entry read less than this long ago is left for
# a later run to delete, unless its last read was this run's own: the build jobs pass when their restores ended
# (CACHES_READ_AT_<SYSTEM>), and the entry a build started from is otherwise always read within the last few minutes.
IN_USE = timedelta(minutes=15)
SYSTEMS = ("linux", "macos", "windows")

# The kinds of entry ci.yml saves, by the start of their key. Each keeps its newest entry; the workflow contract test
# checks that every cache ci.yml's build jobs restore by a key prefix is one of them.
KINDS = (
    "ccache-linux-x64-",
    "ccache-macos-arm64-",
    "ccache-windows-x64-",
    "linux-x64-release-dep-sources-",
    "macos-arm64-release-dep-sources-",
    "windows-x64-release-dep-sources-",
    "vcpkg-windows-x64-v2-",
)
# Kinds whose restore step only reads entries of its own compiler, the part of the key after the kind: while GitHub
# moves the runners to a new image, jobs land on both images, and one with another compiler cannot use the newest
# entry of the kind. These keep the newest entry of each compiler; GitHub drops a compiler's last entry 7 days after
# its last use.
PER_COMPILER = (
    "ccache-linux-x64-",
    "ccache-macos-arm64-",
    "ccache-windows-x64-",
)


def group(kind, key):
    """What a restore step searches for the key by: its kind, and for PER_COMPILER kinds also its compiler."""
    if kind not in PER_COMPILER:
        return kind
    compiler = key[len(kind):].partition("-")[0]
    return f"{kind}{compiler}-"


def timestamp(text):
    """A time from the GitHub API. Its fraction of a second varies in length, so the text alone does not sort."""
    return datetime.fromisoformat(text)


def system(kind):
    """The build job whose restores read a kind: the system its key names, or None."""
    return next((name for name in SYSTEMS if name in kind), None)


def newest(entries):
    """The entry saved last, or None when there is none."""
    return max(entries, key=lambda entry: (timestamp(entry["created_at"]), entry["id"]), default=None)


def describe(entry):
    return f"{entry['key']} ({entry['size_in_bytes'] / 1e6:.0f} MB, created {entry['created_at']})"


def prune(kinds, list_kind, delete, now, read_at=None):
    """Deletes all but the newest entry of each group (see group()), except entries read within IN_USE before `now` by
    a job other than this run's, and returns the bytes that frees. `read_at` maps a system to when this run's build job
    for it ended its restores; a system missing from it had no build job."""
    read_at = read_at or {}
    freed = 0
    for kind in kinds:
        ours = read_at.get(system(kind))
        groups = {}
        for entry in list_kind(kind):
            if entry["key"].startswith(kind):
                groups.setdefault(group(kind, entry["key"]), []).append(entry)
        for entries in groups.values():
            kept = newest(entries)
            for entry in entries:
                if entry is kept:
                    continue
                last_read = timestamp(entry["last_accessed_at"])
                if now - last_read < IN_USE and (ours is None or last_read > ours):
                    print(f"Left {describe(entry)} for a later run: a job read it at {entry['last_accessed_at']}.")
                    continue
                delete(entry)
                print(f"Deleted {describe(entry)}.")
                freed += entry["size_in_bytes"]
            print(f"Kept {describe(kept)}.")
    return freed


def delete_entry(url, token, open_url=urllib.request.urlopen, sleep=time.sleep, clock=time.time):
    """Deletes one cache entry, sent again while GitHub fails; an entry that is already gone counts as deleted."""
    request = urllib.request.Request(url, method="DELETE", headers=api_headers(token))
    for attempt in range(1, REQUEST_ATTEMPTS + 1):
        try:
            with open_url(request, timeout=30):
                return
        except REQUEST_ERRORS as error:
            # Another run on the branch can prune the same entry first.
            if isinstance(error, urllib.error.HTTPError) and error.code == 404:
                return
            delay = retry_delay(error, attempt, clock()) if attempt < REQUEST_ATTEMPTS else None
            if delay is None:
                raise
            print(f"Deleting {url} failed ({error!r}); attempt {attempt + 1} of {REQUEST_ATTEMPTS} follows in "
                  f"{delay} s.", flush=True)
            sleep(delay)


def main():
    api = os.environ.get("GITHUB_API_URL", "https://api.github.com")
    caches = f"{api}/repos/{os.environ['GITHUB_REPOSITORY']}/actions/caches"
    ref = os.environ["GITHUB_REF"]
    token = os.environ.get("GH_TOKEN") or os.environ["GITHUB_TOKEN"]

    def list_kind(kind):
        query = urllib.parse.urlencode({"key": kind, "ref": ref})
        return get_pages(f"{caches}?{query}#actions_caches", token)

    read_at = {name: timestamp(os.environ[f"CACHES_READ_AT_{name.upper()}"]) for name in SYSTEMS
               if os.environ.get(f"CACHES_READ_AT_{name.upper()}")}
    freed = prune(KINDS, list_kind, lambda entry: delete_entry(f"{caches}/{entry['id']}", token),
                  datetime.now(timezone.utc), read_at)
    print(f"Freed {freed / 1e9:.2f} GB on {ref}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
