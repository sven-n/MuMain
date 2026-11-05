#!/usr/bin/env python3
"""Deletes the older entries of each cache kind on this run's branch, so that one of each is left.

ci.yml names a cache after what it holds: the compiler caches after the commit, the library and vcpkg caches after the
files that pick their content. A run that saves under a new name leaves the older entries behind. GitHub keeps them
until they go unused for 7 days, or until the repository's caches pass 10 GB; then it deletes the least recently used
entries first, which can be the newest entry of a cache that only the nightly run reads. Run it after the jobs that
save caches, with `actions: write` and the token in GH_TOKEN:

    python3 scripts/ci_cache_prune.py
"""

import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

from ci_job_annotations import REQUEST_ATTEMPTS, REQUEST_ERRORS, api_headers, get_pages, retry_delay

# The kinds of entry ci.yml saves, by the start of their key. Each keeps its newest entry; the workflow contract test
# checks that every cache ci.yml's build jobs restore by a key prefix is one of them.
KINDS = (
    "ccache-linux-x64-",
    "ccache-macos-arm64-",
    "linux-x64-release-dep-sources-",
    "macos-arm64-release-dep-sources-",
    "windows-x64-release-dep-sources-",
    "vcpkg-windows-x64-v2-",
)


def newest(entries):
    """The entry saved last, or None when there is none."""
    return max(entries, key=lambda entry: (entry["created_at"], entry["id"]), default=None)


def describe(entry):
    return f"{entry['key']} ({entry['size_in_bytes'] / 1e6:.0f} MB, created {entry['created_at']})"


def prune(kinds, list_kind, delete):
    """Deletes all but the newest entry of each kind and returns the bytes that frees."""
    freed = 0
    for kind in kinds:
        entries = [entry for entry in list_kind(kind) if entry["key"].startswith(kind)]
        kept = newest(entries)
        for entry in entries:
            if entry is kept:
                continue
            delete(entry)
            print(f"Deleted {describe(entry)}.")
            freed += entry["size_in_bytes"]
        if kept is not None:
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

    freed = prune(KINDS, list_kind, lambda entry: delete_entry(f"{caches}/{entry['id']}", token))
    print(f"Freed {freed / 1e9:.2f} GB on {ref}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
