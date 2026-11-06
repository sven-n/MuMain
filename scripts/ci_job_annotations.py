#!/usr/bin/env python3
"""Fails when a finished job of this workflow run carries a warning annotation.

GitHub annotates jobs itself, e.g. "Node.js 20 is deprecated" when an action targets a Node.js version its runners
no longer ship, and steps add annotations with ::warning::. Notices are listed but pass: GitHub posts some about its
own state ("Due to capacity constraints, jobs targeting macOS arm64 runners may experience longer queue times"),
which no change here can remove. For the same reason, the warnings that actions/cache logs when GitHub's cache
service fails to restore or save an entry (a rate limit, a server error, a timeout; the build goes on without it)
are listed and pass; its other warnings, e.g. a cache over the size limit, fail like any other. Run it from the last job of a workflow, with `actions: read` and `checks: read` and the token in GH_TOKEN:

    python3 scripts/ci_job_annotations.py

The job that runs it is still in progress and is not checked.
"""

import http.client
import json
import os
import re
import sys
import time
import urllib.error
import urllib.request

FAILING_LEVEL = "warning"
LISTED_LEVEL = "notice"
PAGE_SIZE = 100
# A request that GitHub fails or rate-limits is sent again after 5, 10 and 15 seconds before the check gives up.
REQUEST_ATTEMPTS = 4
RETRY_DELAY_SECONDS = 5
# A wait GitHub asks for (Retry-After, or until a rate limit resets) is kept up to this long; a longer one fails the
# check at once instead of holding the job.
MAX_WAIT_SECONDS = 60
# What a request can fail with on its way: OSError covers urllib's errors, timeouts, resets and SSL failures,
# HTTPException a body cut off while it is read, JSONDecodeError a body that ended early.
REQUEST_ERRORS = (OSError, http.client.HTTPException, json.JSONDecodeError)
# Warnings about the state of GitHub's services rather than this repository: actions/cache's "Failed to restore: ..."
# and "Failed to save: ..." when the cache service is busy, down, unreachable or raced by another job. The same prefix
# also opens failures the workflow causes ("Failed to save: Cache size of ~11000 MB is over the 10GB limit"), which
# still fail the check.
SERVICE_WARNINGS = (
    re.compile(
        r"^Failed to (?:restore|save): .*(?:"
        r"Cache service responded with (?:429|5\d\d)\b"  # rate limit or server error
        r"|\((?:429|5\d\d)\)"  # the same from the cache service's newer client: "Failed request: (503) ..."
        r"|Unable to reserve cache|already exists"  # another job saves the same key at the same time
        r"|timed? ?out|ETIMEDOUT|ECONNRESET|ECONNREFUSED|EAI_AGAIN|socket hang up"  # the network
        r")",
        re.IGNORECASE,
    ),
)


def retry_delay(error, attempt, now):
    """Seconds to wait before a failed request goes out again, or None when sending it again cannot help."""
    backoff = RETRY_DELAY_SECONDS * attempt
    if not isinstance(error, urllib.error.HTTPError):
        return backoff
    if error.code >= 500:
        return backoff
    if error.code not in (403, 429):
        return None
    headers = error.headers or {}
    retry_after = (headers.get("retry-after") or "").strip()
    reset = (headers.get("x-ratelimit-reset") or "").strip()
    if retry_after:
        # Seconds, or an HTTP date, for which the backoff stands in.
        wait = int(retry_after) if retry_after.isdigit() else backoff
    elif headers.get("x-ratelimit-remaining") == "0":
        # The hourly limit is used up; waiting helps only when it resets soon.
        wait = max(1, int(reset) - int(now) + 1) if reset.isdigit() else MAX_WAIT_SECONDS + 1
    elif error.code == 429:
        wait = backoff
    else:
        return None  # a 403 that is no rate limit, e.g. a missing permission
    return wait if wait <= MAX_WAIT_SECONDS else None


def fetch_json(request, open_url=urllib.request.urlopen, sleep=time.sleep, clock=time.time):
    """The JSON body of a GitHub API request, sent up to REQUEST_ATTEMPTS times while GitHub fails."""
    for attempt in range(1, REQUEST_ATTEMPTS + 1):
        try:
            with open_url(request, timeout=30) as response:
                return json.load(response)
        except REQUEST_ERRORS as error:
            delay = retry_delay(error, attempt, clock()) if attempt < REQUEST_ATTEMPTS else None
            if delay is None:
                raise
            print(f"GitHub API request failed ({error!r}); attempt {attempt + 1} of {REQUEST_ATTEMPTS} follows in "
                  f"{delay} s.", flush=True)
            sleep(delay)
    raise AssertionError("unreachable")


def api_headers(token):
    return {
        "Accept": "application/vnd.github+json",
        "Authorization": f"Bearer {token}",
        "X-GitHub-Api-Version": "2022-11-28",
    }


def get_pages(url, token):
    """Every item of a paginated GitHub REST list; `url` names the list's key after a '#'."""
    url, _, key = url.partition("#")
    items = []
    page = 1
    while True:
        separator = "&" if "?" in url else "?"
        request = urllib.request.Request(
            f"{url}{separator}per_page={PAGE_SIZE}&page={page}", headers=api_headers(token))
        body = fetch_json(request)
        batch = body[key] if key else body
        items.extend(batch)
        if len(batch) < PAGE_SIZE:
            return items
        page += 1


def find_annotations(jobs, annotations_of, level):
    """(job name, annotation) for each annotation of the level on the finished jobs."""
    found = []
    for job in jobs:
        if job["status"] != "completed":
            continue
        for annotation in annotations_of(job["id"]):
            if annotation["annotation_level"] == level:
                found.append((job["name"], annotation))
    return found


def split_service_warnings(found):
    """The (job name, annotation) pairs split into those that fail and those about GitHub's services."""
    failing = []
    service = []
    for job_name, annotation in found:
        message = annotation["message"]
        (service if any(pattern.search(message) for pattern in SERVICE_WARNINGS) else failing).append(
            (job_name, annotation))
    return failing, service


def describe(job_name, annotation):
    title = annotation.get("title") or annotation["annotation_level"]
    message = " ".join(annotation["message"].split())
    path = annotation.get("path") or ""
    # Annotations of the job itself, not of a file, name the path ".github".
    if not path or path == ".github":
        return f"{job_name}: {title}: {message}"
    where = f"{path}:{annotation['start_line']}" if annotation.get("start_line") else path
    return f"{job_name}: {title}: {message} ({where})"


def main():
    api = os.environ.get("GITHUB_API_URL", "https://api.github.com")
    repository = os.environ["GITHUB_REPOSITORY"]
    run = f"{api}/repos/{repository}/actions/runs/{os.environ['GITHUB_RUN_ID']}"
    attempt = os.environ.get("GITHUB_RUN_ATTEMPT", "1")
    token = os.environ.get("GH_TOKEN") or os.environ["GITHUB_TOKEN"]

    jobs = get_pages(f"{run}/attempts/{attempt}/jobs#jobs", token)
    cache = {}

    def annotations_of(job_id):
        if job_id not in cache:
            cache[job_id] = get_pages(f"{api}/repos/{repository}/check-runs/{job_id}/annotations", token)
        return cache[job_id]

    warnings, service_warnings = split_service_warnings(find_annotations(jobs, annotations_of, FAILING_LEVEL))
    for job_name, annotation in find_annotations(jobs, annotations_of, LISTED_LEVEL):
        print(f"Notice, not checked: {describe(job_name, annotation)}")
    for job_name, annotation in service_warnings:
        print(f"Service warning, not checked: {describe(job_name, annotation)}")
    for job_name, annotation in warnings:
        print(f"::error title=Job annotation::{describe(job_name, annotation)}")
    finished = sum(job["status"] == "completed" for job in jobs)
    print(f"{len(warnings)} warning annotation(s) on {finished} finished job(s).")
    return 1 if warnings else 0


if __name__ == "__main__":
    sys.exit(main())
