#!/usr/bin/env python3

import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
POWERSHELL = shutil.which("pwsh") or shutil.which("powershell")
sys.path.insert(0, str(ROOT / "scripts"))
import ci_build_plan
from ci_build_plan import (build_plan, check_results, last_successful_main_run, needs_native_build,
                           scheduled_release_needed)


class BuildPlanTests(unittest.TestCase):
    def plan(self, event_name="pull_request", paths=("src/source/Main.cpp",), draft=False,
             repository="sven-n/MuMain", ref="refs/heads/main"):
        with patch("ci_build_plan.scheduled_release_needed", return_value=True):
            return build_plan(event_name, ref, repository, {"pull_request": {"draft": draft}}, paths)

    def test_code_pr_builds_every_platform(self):
        self.assertEqual(self.plan(), dict(linux=True, macos=True, windows=True, release=False))

    def test_main_push_builds_every_platform(self):
        self.assertEqual(self.plan("push"), dict(linux=True, macos=True, windows=True, release=False))

    def test_drafts_defer_builds_until_ready(self):
        self.assertFalse(any(self.plan(draft=True).values()))
        self.assertTrue(self.plan(draft=False)["macos"])

    def test_docs_skip_compilation(self):
        paths = ("README.md", "docs/build/README.md", "docs/image.png")
        for event_name in ("pull_request", "push"):
            with self.subTest(event_name=event_name):
                self.assertFalse(any(self.plan(event_name, paths).values()))

    def test_asset_prs_preserve_native_validation_without_rebuilding_main(self):
        paths = ("src/bin/Data/item.bmd", "src/bin/fonts/font.ttf")
        for platform in ("linux", "macos", "windows"):
            self.assertTrue(self.plan(paths=paths)[platform])
        self.assertFalse(any(self.plan("push", paths).values()))

    def test_mixed_deleted_renamed_and_build_inputs_require_compilation(self):
        for path in ("src/CMakeLists.txt", "tests/test_file.py", "src/shaders/file.hlsl",
                     "ClientLibrary/library.cs", ".github/workflows/ci.yml", "Tools/tool.py"):
            with self.subTest(path=path):
                self.assertTrue(needs_native_build(("README.md", path)))
        self.assertTrue(needs_native_build([]))

    def test_nightly_and_manual_releases_build_every_platform(self):
        for event_name in ("schedule", "workflow_dispatch"):
            with self.subTest(event_name=event_name):
                self.assertTrue(all(self.plan(event_name, ("README.md",)).values()))

    def test_forks_do_not_publish_releases(self):
        self.assertFalse(any(self.plan("schedule", repository="user/MuMain").values()))

    def test_manual_release_rejects_feature_branches(self):
        with self.assertRaisesRegex(ValueError, "main only"):
            self.plan("workflow_dispatch", ref="refs/heads/feature")

    def results(self, plan, event_name="pull_request"):
        needs = {"quality": {"result": "success", "outputs": {
            key: str(value).lower() for key, value in plan.items()
        }}}
        for platform in ("linux", "macos", "windows"):
            needs[f"build-{platform}"] = {"result": "success" if plan[platform] else "skipped"}
        needs["caches"] = {"result": "skipped" if event_name == "pull_request" else "success"}
        return needs

    def test_required_check_accepts_only_planned_skips(self):
        check_results(self.results(self.plan()), "pull_request")
        check_results(self.results(self.plan(paths=("README.md",))), "pull_request")
        check_results(self.results(self.plan("schedule"), "schedule"), "schedule")
        for result in ("failure", "cancelled", "skipped"):
            needs = self.results(self.plan())
            needs["build-linux"]["result"] = result
            with self.subTest(result=result), self.assertRaises(ValueError):
                check_results(needs, "pull_request")
        needs = self.results(self.plan(paths=("README.md",)))
        needs["build-macos"]["result"] = "success"
        with self.assertRaises(ValueError):
            check_results(needs, "pull_request")

    def test_required_check_needs_the_cache_prune_outside_prs(self):
        for event_name in ("push", "schedule"):
            for result in ("failure", "cancelled", "skipped"):
                needs = self.results(self.plan(event_name), event_name)
                needs["caches"]["result"] = result
                with self.subTest(event_name=event_name, result=result), self.assertRaises(ValueError):
                    check_results(needs, event_name)
        needs = self.results(self.plan())
        needs["caches"]["result"] = "success"
        with self.assertRaises(ValueError):
            check_results(needs, "pull_request")

    def test_required_check_rejects_failed_quality_and_missing_plan(self):
        for result in ("failure", "cancelled", "skipped"):
            needs = self.results(self.plan())
            needs["quality"]["result"] = result
            with self.subTest(result=result), self.assertRaises(ValueError):
                check_results(needs, "pull_request")
        needs = self.results(self.plan())
        needs["quality"]["outputs"]["linux"] = ""
        with self.assertRaises(ValueError):
            check_results(needs, "pull_request")

    def prepare_renamed_repository(self, repo):
        def git(*args):
            return self.git(repo, *args)
        git("init")
        git("config", "user.email", "ci@example.invalid")
        git("config", "user.name", "CI test")
        (repo / "code.cpp").write_text("int main() {}", encoding="utf-8")
        (repo / "deleted.cpp").write_text("int value;", encoding="utf-8")
        git("add", ".")
        git("commit", "-m", "base")
        base = git("rev-parse", "HEAD")
        git("mv", "code.cpp", "README.md")
        git("rm", "deleted.cpp")
        git("commit", "-m", "rename and delete")
        return base

    def git(self, repo, *args):
        return subprocess.check_output(
            ["git", "-C", str(repo), "-c", "commit.gpgsign=false", *args],
            stderr=subprocess.STDOUT,
        ).decode().strip()

    def run_plan(self, repo, event_name, event):
        event_path = repo / "event.json"
        event_path.write_text(json.dumps(event), encoding="utf-8")
        output = repo / "output.txt"
        output.write_text("", encoding="utf-8")
        env = dict(os.environ, GITHUB_EVENT_PATH=str(event_path), GITHUB_EVENT_NAME=event_name,
                   GITHUB_REF="refs/heads/main", GITHUB_REPOSITORY="sven-n/MuMain",
                   GITHUB_OUTPUT=str(output))
        subprocess.run([sys.executable, str(ROOT / "scripts/ci_build_plan.py")],
                       cwd=repo, env=env, check=True, capture_output=True)
        return dict(line.split("=", 1) for line in output.read_text(encoding="utf-8").splitlines())

    def run_main_push(self, repo, before, built):
        """write_plan for a main push, with `built` as the commit main's last successful run built."""
        event_path = repo / "event.json"
        event_path.write_text(json.dumps({"before": before}), encoding="utf-8")
        output = repo / "output.txt"
        output.write_text("", encoding="utf-8")
        env = dict(GITHUB_EVENT_PATH=str(event_path), GITHUB_EVENT_NAME="push", GITHUB_REF="refs/heads/main",
                   GITHUB_REPOSITORY="sven-n/MuMain", GITHUB_OUTPUT=str(output))
        current = os.getcwd()
        os.chdir(repo)
        try:
            with patch.dict(os.environ, env), patch("ci_build_plan.last_successful_main_run", return_value=built), \
                    redirect_stdout(io.StringIO()):
                ci_build_plan.write_plan()
        finally:
            os.chdir(current)
        return dict(line.split("=", 1) for line in output.read_text(encoding="utf-8").splitlines())

    def commit(self, repo, path, text):
        (repo / path).parent.mkdir(parents=True, exist_ok=True)
        (repo / path).write_text(text, encoding="utf-8")
        self.git(repo, "add", path)
        self.git(repo, "commit", "-q", "-m", path)
        return self.git(repo, "rev-parse", "HEAD")

    def test_git_diff_uses_both_sides_of_a_rename_and_keeps_deletions(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            base = self.prepare_renamed_repository(repo)
            self.assertEqual(self.run_main_push(repo, base, base)["linux"], "true")

    def test_main_push_builds_what_a_replaced_push_brought(self):
        # Code merge B waited while A's run built, and docs-only merge C replaced B's pending run.
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            self.git(repo, "init", "-q")
            self.git(repo, "config", "user.email", "ci@example.invalid")
            self.git(repo, "config", "user.name", "CI test")
            a = self.commit(repo, "src/a.cpp", "int a;")
            b = self.commit(repo, "src/b.cpp", "int b;")
            self.commit(repo, "docs/c.md", "c")
            plan = self.run_main_push(repo, before=b, built=a)
            self.assertEqual((plan["linux"], plan["windows"], plan["diff_base"]), ("true", "true", a))
            # With B built, C's documentation alone builds nothing.
            self.assertEqual(self.run_main_push(repo, before=b, built=b)["linux"], "false")
            # Without a successful run to compare with, or with one git does not know, everything builds.
            for built in (None, "0" * 40):
                with self.subTest(built=built):
                    plan = self.run_main_push(repo, before=b, built=built)
                    self.assertEqual((plan["linux"], plan["diff_base"]), ("true", b))

    def test_last_successful_main_run_asks_github_for_this_workflow(self):
        env = dict(GITHUB_WORKFLOW_REF="sven-n/MuMain/.github/workflows/ci.yml@refs/heads/main", GH_TOKEN="token",
                   GITHUB_REPOSITORY="sven-n/MuMain", GITHUB_API_URL="https://api.example")
        urls = []

        def fetch(request, runs=({"head_sha": "abc"},)):
            urls.append(request.full_url)
            return {"workflow_runs": list(runs)}

        with patch.dict(os.environ, env):
            self.assertEqual(last_successful_main_run(fetch), "abc")
            self.assertEqual(urls, ["https://api.example/repos/sven-n/MuMain/actions/workflows/ci.yml/runs"
                                    "?branch=main&event=push&status=success&per_page=1"])
            self.assertIsNone(last_successful_main_run(lambda request: {"workflow_runs": []}))

            def failing(request):
                raise OSError("unreachable")
            with redirect_stderr(io.StringIO()):
                self.assertIsNone(last_successful_main_run(failing))
        with patch.dict(os.environ, {**env, "GH_TOKEN": ""}):
            self.assertIsNone(last_successful_main_run(fetch))

    def test_pull_request_diffs_against_current_main_not_stale_base_sha(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            stale = self.prepare_renamed_repository(repo)
            main = self.git(repo, "rev-parse", "HEAD")
            self.git(repo, "checkout", "-q", "-b", "pr", stale)
            (repo / "docs").mkdir()
            (repo / "docs/note.md").write_text("note", encoding="utf-8")
            self.git(repo, "add", "docs/note.md")
            self.git(repo, "commit", "-m", "docs: add note")
            self.git(repo, "checkout", "-q", "--detach", main)
            self.git(repo, "merge", "--no-ff", "-m", "merge", "pr")
            plan = self.run_plan(repo, "pull_request", {"pull_request": {"draft": False, "base": {"sha": stale}}})
            self.assertEqual(plan["linux"], "false")

    def test_nightly_skips_idle_and_non_release_commits_but_manual_forces_build(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            self.prepare_renamed_repository(repo)
            self.git(repo, "tag", "v1.2.3")
            self.assertEqual(self.run_plan(repo, "schedule", {})["release"], "false")
            for kind in ("docs", "chore", "ci", "test", "build", "style"):
                self.git(repo, "commit", "--allow-empty", "-m", f"{kind}(ci): update files")
                plan = self.run_plan(repo, "schedule", {})
                self.assertTrue(all(plan[platform] == "false" for platform in ("linux", "macos", "windows", "release")))
            self.assertEqual(self.run_plan(repo, "workflow_dispatch", {})["release"], "true")

    def test_nightly_checks_entire_release_range_and_scoped_changes(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            self.prepare_renamed_repository(repo)
            self.git(repo, "tag", "v1.2.3")
            for message in ("fix(network): correct packet", "docs: explain correction"):
                self.git(repo, "commit", "--allow-empty", "-m", message)
            self.assertEqual(self.run_plan(repo, "schedule", {})["release"], "true")
            self.git(repo, "tag", "v1.2.4")
            self.assertEqual(self.run_plan(repo, "schedule", {})["release"], "false")

    def test_nightly_builds_breaking_changes_and_initial_release(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            self.prepare_renamed_repository(repo)
            self.assertEqual(self.run_plan(repo, "schedule", {})["release"], "true")
            for index, message in enumerate(("feat(ui): add layout", "perf(render): reduce draws", "refactor(network): split parser",
                                             "feat(ui)!: replace layout", "feat: new format\n\nBREAKING CHANGE: replace format",
                                             "feat: new format\n\nBREAKING-CHANGE: replace format", 'Revert "fix: previous change"',
                                             "custom: new format\n\n* BREAKING CHANGE: replace format",
                                             "FEAT: older convention", "FIX: older convention")):
                self.git(repo, "tag", f"v1.2.{index}")
                self.git(repo, "commit", "--allow-empty", "-m", message)
                self.assertEqual(self.run_plan(repo, "schedule", {})["release"], "true")

    def test_unpublished_release_rejects_main_advancing_during_build(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            base = self.prepare_renamed_repository(repo)
            latest = self.git(repo, "rev-parse", "HEAD")
            self.git(repo, "checkout", "--detach", base)
            self.git(repo, "branch", "-f", "main", base)
            self.git(repo, "remote", "add", "origin", str(repo))
            command = [sys.executable, str(ROOT / "scripts/ci_build_plan.py"), "check-release"]
            subprocess.run(command, cwd=repo, check=True, capture_output=True)
            self.git(repo, "branch", "-f", "main", latest)
            result = subprocess.run(command, cwd=repo, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("main advanced", result.stderr)

    def test_unknown_release_rules_build_conservatively(self):
        config = {"plugins": [["@semantic-release/commit-analyzer", {
            "preset": "conventionalcommits",
            "releaseRules": [{"scope": "new-policy", "release": "minor"}],
        }]]}
        with patch("ci_build_plan.subprocess.check_output", return_value="v1.2.3\n"), \
                patch("ci_build_plan.Path.read_text", return_value=json.dumps(config)):
            self.assertTrue(scheduled_release_needed())

    @unittest.skipUnless(POWERSHELL, "PowerShell is required for the Windows cache-key test")
    def test_vcpkg_cache_tracks_image_compiler_and_vcpkg(self):
        command = """
            function Get-Command { [pscustomobject]@{Source=$env:TEST_COMPILER} }
            function git { $global:LASTEXITCODE=0; $env:TEST_VCPKG_REVISION }
            & $env:TEST_CACHE_SCRIPT
        """
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            compiler = Path(temporary) / "cl.exe"
            output = Path(temporary) / "output.txt"
            identities = set()
            compilers = []
            inputs = (("image1", b"compiler1", "vcpkg1"),
                      ("image2", b"compiler1", "vcpkg1"),
                      ("image1", b"compiler2", "vcpkg1"),
                      ("image1", b"compiler1", "vcpkg2"))
            for image, binary, revision in inputs:
                compiler.write_bytes(binary)
                output.write_text("", encoding="utf-8")
                env = dict(os.environ, ImageVersion=image, TEST_COMPILER=str(compiler),
                           TEST_VCPKG_REVISION=revision, GITHUB_OUTPUT=str(output),
                           TEST_CACHE_SCRIPT=str(ROOT / "scripts/ci_vcpkg_cache_key.ps1"))
                subprocess.run([POWERSHELL, "-NoProfile", "-NonInteractive", "-Command", command],
                               env=env, check=True, capture_output=True)
                lines = dict(line.split("=", 1) for line in output.read_text(encoding="utf-8-sig").split())
                identities.add(lines["identity"])
                compilers.append(lines["compiler"])
            self.assertEqual(len(identities), len(inputs))
            # The compiler cache's name follows the compiler alone.
            self.assertEqual(compilers[0], compilers[1])
            self.assertEqual(compilers[0], compilers[3])
            self.assertNotEqual(compilers[0], compilers[2])
            self.assertRegex(compilers[0], r"^[0-9a-f]{16}$")


if __name__ == "__main__":
    unittest.main()
