#!/usr/bin/env python3
"""Exercises the scripts Quality Gates and the pre-push hook share, in throwaway git repositories."""

import os
import re
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
CMAKE = sys.argv.pop(1) if len(sys.argv) > 1 else shutil.which("cmake")
CLANG_FORMAT = shutil.which("clang-format")


def find_bash():
    if os.name != "nt":
        return shutil.which("bash")
    # Git for Windows' bash; the System32 bash.exe would start WSL instead.
    git = shutil.which("git")
    if git:
        for parent in Path(git).resolve().parents[:3]:
            if (parent / "bin" / "bash.exe").is_file():
                return str(parent / "bin" / "bash.exe")
    return None


BASH = find_bash()
BAD = "int bad( int c ){return c;}\n"
STATIC_PROJECT = ("cmake_minimum_required(VERSION 3.25)\nproject(Fake NONE)\nenable_testing()\n"
                  "add_test(NAME guard COMMAND \"${{CMAKE_COMMAND}}\" -E {guard})\n")


def clang_format_version():
    output = subprocess.run([CLANG_FORMAT, "--version"], capture_output=True, text=True).stdout
    return re.search(r"version ([0-9.]+)", output).group(1)


@unittest.skipUnless(BASH, "bash is required")
class StaticCheckScriptTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.repo = Path(temporary.name)
        for path in ("scripts/changed_cpp_files.sh", "scripts/check_format.sh", ".githooks/pre-push", ".clang-format"):
            (self.repo / path).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy(ROOT / path, self.repo / path)
        self.git("init", "-q")
        self.git("config", "user.email", "ci@example.invalid")
        self.git("config", "user.name", "CI test")
        self.write("src/source/legacy.cpp", "int legacy( int a ){return a;}\n")
        self.write("src/source/ThirdParty/vendor.cpp", BAD)
        self.write("src/source/Dotnet/generated.cpp", BAD)
        self.pin_clang_format("0.0.0")
        self.commit("base")
        self.base = self.git("rev-parse", "HEAD")

    def git(self, *args):
        return subprocess.check_output(["git", "-C", str(self.repo), "-c", "commit.gpgsign=false", *args],
                                       text=True).strip()

    def write(self, path, text):
        (self.repo / path).parent.mkdir(parents=True, exist_ok=True)
        (self.repo / path).write_text(text, encoding="utf-8", newline="\n")

    def commit(self, message):
        self.git("add", "-A")
        self.git("commit", "-q", "-m", message)

    def pin_clang_format(self, version):
        self.write(".github/workflows/ci.yml", f"run: pip install clang-format=={version}\n")

    def bash(self, *args, stdin="", env=None):
        # Bytes in, so Windows text mode cannot turn the file list's LF into CRLF.
        result = subprocess.run([BASH, *args], cwd=self.repo, input=stdin.encode(), capture_output=True, env=env)
        return subprocess.CompletedProcess(result.args, result.returncode,
                                           result.stdout.decode(errors="replace").replace("\r\n", "\n"),
                                           result.stderr.decode(errors="replace").replace("\r\n", "\n"))

    def changed(self, base):
        return self.bash("scripts/changed_cpp_files.sh", base)

    def test_changed_files_skip_vendored_generated_and_deleted_sources(self):
        self.write("src/source/new.cpp", "int added;\n")
        self.write("src/source/ThirdParty/vendor.cpp", "int vendor;\n")
        self.write("src/source/Dotnet/generated.cpp", "int generated;\n")
        self.write("docs/note.md", "note\n")
        (self.repo / "src/source/legacy.cpp").unlink()
        self.commit("changes")
        result = self.changed(self.base)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout.split(), ["src/source/new.cpp"])

    def test_changed_files_succeed_when_nothing_matches(self):
        result = self.changed(self.base)
        self.assertEqual((result.returncode, result.stdout), (0, ""), result.stderr)

    def test_changed_files_fail_on_an_unknown_base(self):
        result = self.changed("0" * 40)
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(result.stdout, "")

    @unittest.skipUnless(CLANG_FORMAT, "clang-format is required")
    def test_format_checks_only_changed_lines(self):
        self.write("src/source/legacy.cpp", "int legacy( int a ){return a;}\n\nint fine(int b)\n{\n    return b;\n}\n")
        self.commit("clean addition")
        files = self.changed(self.base).stdout
        result = self.bash("scripts/check_format.sh", self.base, stdin=files)
        self.assertEqual(result.returncode, 0, result.stderr)
        with (self.repo / "src/source/legacy.cpp").open("a", encoding="utf-8", newline="\n") as source:
            source.write("\n" + BAD)
        self.commit("bad addition")
        result = self.bash("scripts/check_format.sh", self.base, stdin=files)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("legacy.cpp", result.stderr)

    def hook(self, guard="true"):
        self.write("tests/static/CMakeLists.txt", STATIC_PROJECT.format(guard=guard))
        env = dict(os.environ, MU_PUSH_BASE=self.base)
        env["PATH"] = str(Path(CMAKE).parent) + os.pathsep + env["PATH"]
        return self.bash(".githooks/pre-push", env=env)

    @unittest.skipUnless(CMAKE, "cmake is required")
    def test_hook_does_not_leak_its_repository_environment_to_checks(self):
        with tempfile.TemporaryDirectory() as temporary:
            child = Path(temporary) / "child"
            local_names = self.git("rev-parse", "--local-env-vars").splitlines()
            clean = {key: value for key, value in os.environ.items() if key not in local_names}
            subprocess.run(["git", "clone", "-q", "--no-hardlinks", str(self.repo), str(child)],
                           env=clean, check=True, capture_output=True)
            self.write("tests/static/CMakeLists.txt",
                       'cmake_minimum_required(VERSION 3.25)\nproject(Fake NONE)\nenable_testing()\n'
                       'add_test(NAME guard COMMAND "${CMAKE_COMMAND}" -P '
                       '"${CMAKE_CURRENT_SOURCE_DIR}/probe.cmake")\n')
            self.write("tests/static/probe.cmake",
                       'cmake_minimum_required(VERSION 3.25)\n'
                       f'set(child [===[{child.as_posix()}]===])\n'
                       'foreach(kind branch tag)\n'
                       '  execute_process(COMMAND git -C "${child}" ${kind} hook-env-sentinel HEAD\n'
                       '                  RESULT_VARIABLE result)\n'
                       '  if(NOT result EQUAL 0)\n'
                       '    message(FATAL_ERROR "Child Git mutation failed: ${result}")\n'
                       '  endif()\nendforeach()\n')
            def snapshot():
                status = self.git("status", "--porcelain=v1", "--untracked-files=no")
                return (self.git("rev-parse", "HEAD"), self.git("show-ref"),
                        (self.repo / ".git/index").read_bytes(), status)
            before = snapshot()
            env = dict(clean, GIT_DIR=str(self.repo / ".git"), GIT_WORK_TREE=str(self.repo),
                       GIT_INDEX_FILE=str(self.repo / ".git/index"), MU_PUSH_BASE=self.base)
            env["PATH"] = str(Path(CMAKE).parent) + os.pathsep + env["PATH"]
            # The real hook's CTest consumer deliberately inherits this environment unchanged.
            result = self.bash(".githooks/pre-push", env=env)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(snapshot(), before)
            for ref in ("refs/heads/hook-env-sentinel", "refs/tags/hook-env-sentinel"):
                actual = subprocess.check_output(["git", "-C", str(child), "rev-parse", ref],
                                                 env=clean, text=True).strip()
                self.assertEqual(actual, self.base)

    @unittest.skipUnless(CMAKE, "cmake is required")
    def test_hook_blocks_a_failing_static_check(self):
        result = self.hook(guard="false")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("guard", result.stdout)

    @unittest.skipUnless(CMAKE, "cmake is required")
    def test_hook_skips_formatting_without_the_pinned_clang_format(self):
        result = self.hook()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("formatting skipped, needs clang-format 0.0.0", result.stderr)

    @unittest.skipUnless(CMAKE and CLANG_FORMAT, "cmake and clang-format are required")
    def test_hook_blocks_badly_formatted_changes_with_the_pinned_clang_format(self):
        self.pin_clang_format(clang_format_version())
        self.write("src/source/new.cpp", BAD)
        self.commit("bad addition")
        result = self.hook()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("new.cpp", result.stderr)


if __name__ == "__main__":
    unittest.main()
