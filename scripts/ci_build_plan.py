#!/usr/bin/env python3
"""Select native CI jobs and verify their results for the required check."""

import json
import os
import re
import subprocess
import sys
import urllib.parse
import urllib.request
from pathlib import Path

from ci_job_annotations import REQUEST_ERRORS, api_headers, fetch_json


UPSTREAM_REPOSITORY = "sven-n/MuMain"
MAIN_REF = "refs/heads/main"
RELEASE_EVENTS = {"schedule", "workflow_dispatch"}
DATA_PREFIXES = ("src/bin/Data/", "src/bin/fonts/")
NON_NATIVE_PREFIXES = ("docs/", *DATA_PREFIXES)
NON_NATIVE_FILES = {"LICENSE"}


def commit_needs_release(message, release_types):
    message = message.strip()
    header = re.match(r"(\w+)(?:\([^\r\n]*\))?(!)?: ", message)
    return bool(
        (header and (header[1] in release_types or header[2]))
        or re.search(r"(?m)^[ \t|*]*BREAKING[ -]CHANGE(?::|[ \t])", message)
        or message.startswith('Revert "')
    )


def scheduled_release_needed():
    tags = subprocess.check_output(
        ["git", "tag", "--merged", "HEAD", "--list", "v*", "--sort=-version:refname"],
        text=True, encoding="utf-8",
    ).splitlines()
    latest = next((tag for tag in tags if re.fullmatch(r"v\d+\.\d+\.\d+", tag)), None)
    if latest is None:
        return True
    config = json.loads((Path(__file__).resolve().parents[1] / ".releaserc.json").read_text(encoding="utf-8"))
    analyzer = next(plugin[1] for plugin in config["plugins"]
                    if isinstance(plugin, list) and plugin[0] == "@semantic-release/commit-analyzer")
    if analyzer.get("preset") != "conventionalcommits" or "parserOpts" in analyzer:
        return True
    release_types = {"feat", "fix", "perf", "revert", "FEAT", "FIX"}
    for rule in analyzer["releaseRules"]:
        # shortcut: type-only release rules; more complex rules force builds until this preflight supports them.
        if set(rule) != {"type", "release"} or not re.fullmatch(r"\w+", rule["type"]):
            return True
        if rule["release"] is False:
            release_types.discard(rule["type"])
        else:
            release_types.add(rule["type"])
    messages = subprocess.check_output(
        ["git", "log", "--format=%B%x00", f"{latest}..HEAD"], text=True, encoding="utf-8",
    ).split("\0")
    return any(commit_needs_release(message, release_types) for message in messages)


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
    if release and event_name == "schedule":
        release = scheduled_release_needed()
    pull_request = event_name == "pull_request"
    draft = pull_request and event["pull_request"]["draft"]
    # Asset PRs still need native tests that load data and render shipped documents.
    native = not draft and (
        needs_native_build(paths)
        or (pull_request and any(path.startswith(DATA_PREFIXES) for path in paths))
    )
    main_push = event_name == "push" and ref == MAIN_REF
    # Every system builds every PR, so a break shows before the merge, and every main push, which saves the caches
    # the next PRs start from.
    build = release or (native and (pull_request or main_push))
    return {
        "linux": build,
        "macos": build,
        "windows": build,
        "release": release,
    }


def check_results(needs, event_name):
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
    # Every run but a PR's prunes the caches after its builds; a prune that fails lets them fill up again.
    expected = "skipped" if event_name == "pull_request" else "success"
    if needs["caches"]["result"] != expected:
        raise ValueError(f"caches: expected {expected}, got {needs['caches']['result']}")


def last_successful_main_run(fetch=fetch_json):
    """The commit that main's newest successful push run of this workflow built, or None when there is none or
    GitHub cannot tell. Needs `actions: read` and the token in GH_TOKEN."""
    workflow = re.search(r"/\.github/workflows/([^@/]+)@", os.environ.get("GITHUB_WORKFLOW_REF", ""))
    token = os.environ.get("GH_TOKEN")
    if not workflow or not token:
        return None
    api = os.environ.get("GITHUB_API_URL", "https://api.github.com")
    query = urllib.parse.urlencode({"branch": "main", "event": "push", "status": "success", "per_page": 1})
    url = f"{api}/repos/{os.environ['GITHUB_REPOSITORY']}/actions/workflows/{workflow[1]}/runs?{query}"
    try:
        runs = fetch(urllib.request.Request(url, headers=api_headers(token)))["workflow_runs"]
        return runs[0]["head_sha"] if runs else None
    except (*REQUEST_ERRORS, KeyError, TypeError) as error:
        print(f"Main's last successful run is unknown ({error!r}).", file=sys.stderr)
        return None


def commit_known(sha):
    return subprocess.run(["git", "cat-file", "-e", f"{sha}^{{commit}}"], capture_output=True).returncode == 0


def check_release_revision():
    head = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
    remote = subprocess.check_output(
        ["git", "ls-remote", "origin", MAIN_REF], text=True,
    ).split()
    if not remote or remote[0] != head:
        raise ValueError("No release was published and main advanced during the build; rerun CI manually from main")
    print("No release published; main still matches the tested revision")


def write_plan():
    event = json.loads(Path(os.environ["GITHUB_EVENT_PATH"]).read_text(encoding="utf-8"))
    event_name = os.environ["GITHUB_EVENT_NAME"]
    build_everything = False
    if event_name == "pull_request":
        # The checkout is GitHub's merge commit on current main; the event's base.sha can be weeks stale.
        base = "HEAD^1"
    elif event_name == "push":
        base = event["before"]
        if os.environ["GITHUB_REF"] == MAIN_REF:
            # A main push waits while the run before it builds, and a newer push replaces it if it is still waiting:
            # no run builds its commits, and the newer push's `before` skips them. So a main push compares against the
            # commit main's last successful run built, and builds everything when there is none.
            built = last_successful_main_run()
            if built and commit_known(built):
                base = built
            else:
                build_everything = True
    else:
        base = "HEAD~1"
    # No paths count as a change that needs every build (needs_native_build).
    paths = [] if build_everything else subprocess.check_output(
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
        check_results(json.loads(os.environ["CI_JOB_RESULTS"]), os.environ["GITHUB_EVENT_NAME"])
        print("All applicable CI checks passed")
    elif sys.argv[1:] == ["check-release"]:
        check_release_revision()
    elif not sys.argv[1:]:
        write_plan()
    else:
        sys.exit("Usage: ci_build_plan.py [check|check-release]")
