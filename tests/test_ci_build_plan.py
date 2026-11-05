#!/usr/bin/env python3

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
POWERSHELL = shutil.which("pwsh") or shutil.which("powershell")
sys.path.insert(0, str(ROOT / "scripts"))
from ci_build_plan import build_plan, check_results, needs_native_build, scheduled_release_needed


class BuildPlanTests(unittest.TestCase):
    def plan(self, event_name="pull_request", paths=("src/source/Main.cpp",), draft=False,
             repository="sven-n/MuMain", ref="refs/heads/main"):
        with patch("ci_build_plan.scheduled_release_needed", return_value=True):
            return build_plan(event_name, ref, repository, {"pull_request": {"draft": draft}}, paths)

    def test_code_pr_builds_both_platforms(self):
        self.assertEqual(self.plan(), dict(linux=True, macos=True, windows=False, release=False))

    def test_main_push_only_builds_linux(self):
        self.assertEqual(self.plan("push"), dict(linux=True, macos=False, windows=False, release=False))

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
        self.assertTrue(self.plan(paths=paths)["linux"])
        self.assertTrue(self.plan(paths=paths)["macos"])
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

    def results(self, plan):
        needs = {"quality": {"result": "success", "outputs": {
            key: str(value).lower() for key, value in plan.items()
        }}}
        for platform in ("linux", "macos", "windows"):
            needs[f"build-{platform}"] = {"result": "success" if plan[platform] else "skipped"}
        return needs

    def test_required_check_accepts_only_planned_skips(self):
        check_results(self.results(self.plan()))
        check_results(self.results(self.plan(paths=("README.md",))))
        check_results(self.results(self.plan("schedule")))
        for result in ("failure", "cancelled", "skipped"):
            needs = self.results(self.plan())
            needs["build-linux"]["result"] = result
            with self.subTest(result=result), self.assertRaises(ValueError):
                check_results(needs)
        needs = self.results(self.plan(paths=("README.md",)))
        needs["build-macos"]["result"] = "success"
        with self.assertRaises(ValueError):
            check_results(needs)

    def test_required_check_rejects_failed_quality_and_missing_plan(self):
        for result in ("failure", "cancelled", "skipped"):
            needs = self.results(self.plan())
            needs["quality"]["result"] = result
            with self.subTest(result=result), self.assertRaises(ValueError):
                check_results(needs)
        needs = self.results(self.plan())
        needs["quality"]["outputs"]["linux"] = ""
        with self.assertRaises(ValueError):
            check_results(needs)

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

    def test_git_diff_uses_both_sides_of_a_rename_and_keeps_deletions(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            base = self.prepare_renamed_repository(repo)
            self.assertEqual(self.run_plan(repo, "push", {"before": base})["linux"], "true")

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
                identities.add(output.read_text(encoding="utf-8-sig").strip())
            self.assertEqual(len(identities), len(inputs))


if __name__ == "__main__":
    unittest.main()
