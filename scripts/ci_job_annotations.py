#!/usr/bin/env python3
"""Fails when a finished job of this workflow run carries a warning annotation.

GitHub annotates jobs itself, e.g. "Node.js 20 is deprecated" when an action targets a Node.js version its runners
no longer ship, and steps add annotations with ::warning::. Notices are listed but pass: GitHub posts some about its
own state ("Due to capacity constraints, jobs targeting macOS arm64 runners may experience longer queue times"),
which no change here can remove. For the same reason, the warnings that actions/cache logs when GitHub's cache
service fails to restore or save an entry (a rate limit, a timeout; the build goes on without it) are listed and
pass. Run it from the last job of a workflow, with `actions: read` and `checks: read` and the token in GH_TOKEN:

    python3 scripts/ci_job_annotations.py

The job that runs it is still in progress and is not checked.
"""

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
# Warnings about the state of GitHub's services rather than this repository: actions/cache's
# "Failed to restore: Cache service responded with 429" and "Failed to save: ..." when the service fails.
SERVICE_WARNINGS = (re.compile(r"^Failed to (?:restore|save): "),)


def is_worth_retrying(error):
    """Whether a failed request may pass when sent again: GitHub's own failures and its rate limits."""
    if isinstance(error, urllib.error.HTTPError):
        rate_limited = error.code == 429 or (
            error.code == 403
            and (error.headers.get("retry-after") or error.headers.get("x-ratelimit-remaining") == "0"))
        return error.code >= 500 or rate_limited
    return isinstance(error, (urllib.error.URLError, TimeoutError, ConnectionError))


def fetch_json(request, open_url=urllib.request.urlopen, sleep=time.sleep):
    """The JSON body of a GitHub API request, sent up to REQUEST_ATTEMPTS times while GitHub fails."""
    for attempt in range(1, REQUEST_ATTEMPTS + 1):
        try:
            with open_url(request, timeout=30) as response:
                return json.load(response)
        except (urllib.error.URLError, TimeoutError, ConnectionError) as error:
            if attempt == REQUEST_ATTEMPTS or not is_worth_retrying(error):
                raise
            print(f"GitHub API request failed ({error}); attempt {attempt + 1} of {REQUEST_ATTEMPTS} follows.",
                  flush=True)
            sleep(RETRY_DELAY_SECONDS * attempt)
    raise AssertionError("unreachable")


def get_pages(url, token):
    """Every item of a paginated GitHub REST list; `url` names the list's key after a '#'."""
    url, _, key = url.partition("#")
    items = []
    page = 1
    while True:
        separator = "&" if "?" in url else "?"
        request = urllib.request.Request(
            f"{url}{separator}per_page={PAGE_SIZE}&page={page}",
            headers={
                "Accept": "application/vnd.github+json",
                "Authorization": f"Bearer {token}",
                "X-GitHub-Api-Version": "2022-11-28",
            },
        )
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
