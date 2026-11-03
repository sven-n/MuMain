#!/usr/bin/env python3
"""Select native CI jobs and verify their results for the required check."""

import json
import os
import subprocess
import sys
from pathlib import Path


UPSTREAM_REPOSITORY = "sven-n/MuMain"
MAIN_REF = "refs/heads/main"
RELEASE_EVENTS = {"schedule", "workflow_dispatch"}
DATA_PREFIXES = ("src/bin/Data/", "src/bin/fonts/")
NON_NATIVE_PREFIXES = ("docs/", *DATA_PREFIXES)
NON_NATIVE_FILES = {"LICENSE"}


def needs_native_build(paths):
    if not paths:
        return True
    return any(
        not path.startswith(NON_NATIVE_PREFIXES)
        and path not in NON_NATIVE_FILES
        and not ("/" not in path and path.endswith(".md"))
        for path in paths
    )


def build_plan(event_name, ref, repository, event, paths):
    if event_name in RELEASE_EVENTS and ref != MAIN_REF:
        raise ValueError("Run scheduled/manual release builds from main only")
    release = event_name in RELEASE_EVENTS and repository == UPSTREAM_REPOSITORY
    pull_request = event_name == "pull_request"
    draft = pull_request and event["pull_request"]["draft"]
    # Asset PRs still need native tests that load data and render shipped documents.
    native = not draft and (
        needs_native_build(paths)
        or (pull_request and any(path.startswith(DATA_PREFIXES) for path in paths))
    )
    main_push = event_name == "push" and ref == MAIN_REF
    return {
        "linux": release or (native and (pull_request or main_push)),
        "macos": release or (native and pull_request),
        "windows": release,
        "release": release,
    }


def check_results(needs):
    if needs["quality"]["result"] != "success":
        raise ValueError("Quality Gates did not succeed")
    outputs = needs["quality"]["outputs"]
    for platform in ("linux", "macos", "windows"):
        if outputs[platform] not in {"true", "false"}:
            raise ValueError(f"Missing or invalid build plan for {platform}")
        expected = "success" if outputs[platform] == "true" else "skipped"
        actual = needs[f"build-{platform}"]["result"]
        if actual != expected:
            raise ValueError(f"{platform}: expected {expected}, got {actual}")


def write_plan():
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text(encoding="utf-8"))
    event_name = os.environ["GITHUB_EVENT_NAME"]
    if event_name == "pull_request":
        base = event["pull_request"]["base"]["sha"]
    elif event_name == "push":
        base = event["before"]
    else:
        base = "HEAD~1"
    paths = subprocess.check_output(
        ["git", "diff", "--name-only", "--no-renames", "-z", f"{base}...HEAD"],
    ).decode("utf-8").split("\0")[:-1]
    plan = build_plan(
        event_name, os.environ["GITHUB_REF"], os.environ["GITHUB_REPOSITORY"], event, paths,
    )
    with Path(os.environ["GITHUB_OUTPUT"]).open("a", encoding="utf-8") as output:
        output.write(f"diff_base={base}\n")
        for name, enabled in plan.items():
            output.write(f"{name}={str(enabled).lower()}\n")
    print(json.dumps(plan, sort_keys=True))


if __name__ == "__main__":
    if sys.argv[1:] == ["check"]:
        check_results(json.loads(os.environ["CI_JOB_RESULTS"]))
        print("All applicable CI checks passed")
    elif not sys.argv[1:]:
        write_plan()
    else:
        sys.exit("Usage: ci_build_plan.py [check]")
