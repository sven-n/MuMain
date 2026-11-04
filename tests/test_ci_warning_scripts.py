#!/usr/bin/env python3
"""Tests for scripts/ci_warnings.py and scripts/ci_job_annotations.py, the CI rule that no warning comes back."""

import importlib.util
import os
import subprocess
import sys
import unittest
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parents[1] / "scripts"


def load(name):
    spec = importlib.util.spec_from_file_location(name, SCRIPTS / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


ci_warnings = load("ci_warnings")
ci_job_annotations = load("ci_job_annotations")


class WarningLineTests(unittest.TestCase):
    def test_reports_what_the_builds_printed(self):
        for line in (
            "../../../src/source/UI/Chat/Chat.cpp:260:11: warning: address of array 'c->Union' will always "
            "evaluate to 'true' [-Wpointer-bool-conversion]",
            "src/source/Engine/Object/ZzzObject.cpp(5858): warning C4312: 'type cast': conversion from "
            "'unsigned int' to 'OBJECT *' of greater size",
            "D:\\a\\MuMain\\MuMain\\tests\\third_party\\doctest\\doctest.h(539): note: see declaration of 'std::tuple'",
            "cl : Command line warning D9025 : overriding '/W2' with '/w'",
            "-- Note: cryptopp only supports static library linkage. Building static library.",
            "CMake Warning (deprecated) at out/build/linux-ci/_deps/glm-src/CMakeLists.txt:1 (cmake_minimum_required):",
            "CMake Deprecation Warning at CMakeLists.txt:3 (cmake_policy):",
            "ld: warning: ignoring duplicate libraries: 'src/Release/libMuClient.a'",
            "src/shaders/skinned_shadow.vert.hlsl:6:7: warning: 'location' attribute ignored [-Wignored-attributes]",
            "WARNING: 0:12: 'x' : unused variable",
            "/src/ThirdParty/SDL/wayland-protocols/wayland.xml:2314: element event: validity error : No declaration "
            "for attribute deprecated-since of element event",
            "(node:2426) [DEP0040] DeprecationWarning: The `punycode` module is deprecated.",
            "4 warnings generated.",
            "MSBUILD : warning MSB4011: \"a.props\" cannot be imported again.",
            "C:\\x\\ClientLibrary.csproj : warning NU1603: Package A depends on B (>= 1.0)",
            "2026-11-04T01:26:43.0922614Z cc1plus: note: source object is likely at address",
            "WARNHarfBuzz has a Meson port and tries to migrate all the other build systems to it",
            "[WARN] cache miss",
        ):
            with self.subTest(line=line):
                self.assertIsNotNone(ci_warnings.find_problem(line))

    def test_passes_ordinary_build_output(self):
        for line in (
            "[336/1767] Building CXX object src/CMakeFiles/MuClient.dir/Release/source/Data/ItemDataWarnings.cpp.o",
            "--   Build deprecated features:          OFF",
            "-- Performing Test C_FLAG_Wall - Success",
            "-- Looking for strstr - found",
            "Start  995: what can't be used is left out with a warning",
            "Warnings: 0",
            "Warnings are errors in the game's own code",
            "Installing 3/11 cryptopp:x64-windows@2026-03-02...",
            "-- Build files have been written to: D:/a/MuMain/MuMain/out/build/windows-ci-x64-release",
        ):
            with self.subTest(line=line):
                self.assertIsNone(ci_warnings.find_problem(line))

    def test_ignores_terminal_colours(self):
        self.assertIsNotNone(ci_warnings.find_problem("\x1b[1ma.cpp:1:2: \x1b[35mwarning:\x1b[0m x"))


class RunTests(unittest.TestCase):
    def run_python(self, mode, code):
        return ci_warnings.main([mode, sys.executable, "-c", code])

    def test_clean_output_passes(self):
        self.assertEqual(self.run_python("run", "print('[1/1] Linking CXX executable Main')"), 0)

    def test_a_warning_fails_the_step(self):
        self.assertEqual(self.run_python("run", "print('a.cpp:1:2: warning: x [-Wfoo]')"), 1)

    def test_the_commands_own_failure_wins(self):
        self.assertEqual(self.run_python("run", "import sys; print('a.cpp:1:2: warning: x'); sys.exit(3)"), 3)

    def test_report_lists_without_failing(self):
        self.assertEqual(self.run_python("report", "print('a.cpp:1:2: warning: x [-Wfoo]')"), 0)

    def test_separator_is_optional(self):
        self.assertEqual(ci_warnings.main(["run", "--", sys.executable, "-c", "print('ok')"]), 0)

    def test_text_outside_the_code_page_passes_through(self):
        # As in the Windows build: stdout is a pipe in a code page that cannot hold Korean.
        child = [sys.executable, "-c", "import sys; sys.stdout.buffer.write('\\ud55c\\uad6d\\n'.encode())"]
        result = subprocess.run([sys.executable, str(SCRIPTS / "ci_warnings.py"), "run", *child],
                                capture_output=True, env={**os.environ, "PYTHONIOENCODING": "cp1252"})
        self.assertEqual(result.returncode, 0, result.stderr.decode(errors="replace"))


class AnnotationTests(unittest.TestCase):
    JOBS = [
        {"id": 1, "name": "Linux", "status": "completed"},
        {"id": 2, "name": "macOS", "status": "completed"},
        {"id": 3, "name": "CI Required", "status": "in_progress"},
    ]
    ANNOTATIONS = {
        1: [{"annotation_level": "warning", "message": "Node.js 20 is deprecated.", "path": ".github",
             "start_line": 1, "title": ""}],
        2: [{"annotation_level": "notice", "message": "Due to capacity constraints, jobs may queue longer.",
             "path": ".github", "start_line": 1, "title": ""}],
        3: [{"annotation_level": "warning", "message": "not finished yet", "path": ".github", "start_line": 1}],
    }

    def test_warnings_of_finished_jobs_fail(self):
        found = ci_job_annotations.find_annotations(self.JOBS, self.ANNOTATIONS.__getitem__, "warning")
        self.assertEqual([(name, a["message"]) for name, a in found], [("Linux", "Node.js 20 is deprecated.")])

    def test_notices_are_listed_separately(self):
        found = ci_job_annotations.find_annotations(self.JOBS, self.ANNOTATIONS.__getitem__, "notice")
        self.assertEqual([name for name, _ in found], ["macOS"])

    def test_cache_service_warnings_are_listed_separately(self):
        found = [
            ("Linux", {"annotation_level": "warning", "message": "Failed to restore: Cache service responded with 429"}),
            ("macOS", {"annotation_level": "warning", "message": "Failed to save: Unable to upload the cache"}),
            ("Windows", {"annotation_level": "warning", "message": "Node.js 20 is deprecated."}),
            ("Linux", {"annotation_level": "warning", "message": "The step Failed to restore: x"}),
        ]
        failing, service = ci_job_annotations.split_service_warnings(found)
        self.assertEqual([name for name, _ in service], ["Linux", "macOS"])
        self.assertEqual([a["message"] for _, a in failing],
                         ["Node.js 20 is deprecated.", "The step Failed to restore: x"])

    def test_description_names_job_and_message(self):
        name, annotation = ci_job_annotations.find_annotations(
            self.JOBS, self.ANNOTATIONS.__getitem__, "warning")[0]
        self.assertEqual(ci_job_annotations.describe(name, annotation), "Linux: warning: Node.js 20 is deprecated.")


if __name__ == "__main__":
    unittest.main()
