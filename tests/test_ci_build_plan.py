#!/usr/bin/env python3

import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
POWERSHELL = shutil.which("pwsh") or shutil.which("powershell")
sys.path.insert(0, str(ROOT / "scripts"))
from ci_build_plan import build_plan, check_results, needs_native_build


class BuildPlanTests(unittest.TestCase):
    def plan(self, event_name="pull_request", paths=("src/source/Main.cpp",), draft=False,
             repository="sven-n/MuMain", ref="refs/heads/main"):
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
            return subprocess.check_output(
                ["git", "-C", str(repo), "-c", "commit.gpgsign=false", *args],
                stderr=subprocess.STDOUT,
            ).decode().strip()
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

    def test_git_diff_uses_both_sides_of_a_rename_and_keeps_deletions(self):
        with tempfile.TemporaryDirectory(dir=ROOT) as temporary:
            repo = Path(temporary)
            base = self.prepare_renamed_repository(repo)
            event = repo / "event.json"
            event.write_text(json.dumps({"before": base}), encoding="utf-8")
            output = repo / "output.txt"
            env = dict(os.environ, GITHUB_EVENT_PATH=str(event), GITHUB_EVENT_NAME="push",
                       GITHUB_REF="refs/heads/main", GITHUB_REPOSITORY="sven-n/MuMain",
                       GITHUB_OUTPUT=str(output))
            subprocess.run([sys.executable, str(ROOT / "scripts/ci_build_plan.py")],
                           cwd=repo, env=env, check=True, capture_output=True)
            self.assertIn("linux=true", output.read_text(encoding="utf-8"))

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
