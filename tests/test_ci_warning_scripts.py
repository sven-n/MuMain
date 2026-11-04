#!/usr/bin/env python3
"""Tests for scripts/ci_warnings.py and scripts/ci_job_annotations.py, the CI rule that no warning comes back."""

import email.message
import importlib.util
import io
import os
import subprocess
import sys
import tempfile
import unittest
import urllib.error
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
            "tools/wire_sizes.py:12: SyntaxWarning: invalid escape sequence '\\d'",
            "x.py:3: UserWarning: the option is deprecated",
            "warn : NU1603: Package A depends on B (>= 1.0)",
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
    """Runs the script as CI does, in its own process. Without its own GITHUB_STEP_SUMMARY, the test's warnings
    would land in the summary of the CI step that runs the tests, and its ::error lines in that step's log."""

    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.summary = Path(directory.name) / "summary.md"

    def run_script(self, *arguments, **environment):
        env = {**os.environ, "GITHUB_STEP_SUMMARY": str(self.summary), **environment}
        return subprocess.run([sys.executable, str(SCRIPTS / "ci_warnings.py"), *arguments],
                              capture_output=True, env=env)

    def run_python(self, mode, code):
        return self.run_script(mode, sys.executable, "-c", code)

    def test_clean_output_passes(self):
        result = self.run_python("run", "print('[1/1] Linking CXX executable Main')")
        self.assertEqual(result.returncode, 0)
        self.assertIn(b"Linking CXX executable Main", result.stdout)
        self.assertFalse(self.summary.exists())

    def test_a_warning_fails_the_step(self):
        result = self.run_python("run", "print('a.cpp:1:2: warning: x [-Wfoo]')")
        self.assertEqual(result.returncode, 1)
        self.assertIn(b"::error title=diagnostic in ", result.stdout)
        self.assertIn("a.cpp:1:2: warning: x [-Wfoo]", self.summary.read_text(encoding="utf-8"))

    def test_the_commands_own_failure_wins(self):
        result = self.run_python("run", "import sys; print('a.cpp:1:2: warning: x'); sys.exit(3)")
        self.assertEqual(result.returncode, 3)

    def test_report_lists_without_failing(self):
        result = self.run_python("report", "print('a.cpp:1:2: warning: x [-Wfoo]')")
        self.assertEqual(result.returncode, 0)
        self.assertNotIn(b"::error", result.stdout)
        self.assertIn("a.cpp:1:2: warning: x [-Wfoo]", self.summary.read_text(encoding="utf-8"))

    def test_separator_is_optional(self):
        self.assertEqual(self.run_script("run", "--", sys.executable, "-c", "print('ok')").returncode, 0)

    def test_text_outside_the_code_page_passes_through(self):
        # As in the Windows build: stdout is a pipe in a code page that cannot hold Korean.
        child = [sys.executable, "-c", "import sys; sys.stdout.buffer.write('\\ud55c\\uad6d\\n'.encode())"]
        result = self.run_script("run", *child, PYTHONIOENCODING="cp1252")
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


def http_error(code, **headers):
    message = email.message.Message()
    for name, value in headers.items():
        message[name.replace("_", "-")] = value
    return urllib.error.HTTPError("https://api.github.com/x", code, "failed", message, None)


class RequestRetryTests(unittest.TestCase):
    def fetch(self, *outcomes):
        """fetch_json against answers given in order: an exception is raised, anything else is the JSON body."""
        answers = list(outcomes)
        self.sleeps = []

        def open_url(request, timeout):
            answer = answers.pop(0)
            if isinstance(answer, Exception):
                raise answer
            return io.BytesIO(answer.encode())

        return ci_job_annotations.fetch_json("request", open_url, self.sleeps.append)

    def test_a_server_error_is_retried(self):
        self.assertEqual(self.fetch(http_error(502), '{"jobs": []}'), {"jobs": []})
        self.assertEqual(self.sleeps, [5])

    def test_a_network_failure_is_retried(self):
        self.assertEqual(self.fetch(urllib.error.URLError("reset"), TimeoutError(), "[]"), [])
        self.assertEqual(self.sleeps, [5, 10])

    def test_a_rate_limit_is_retried(self):
        self.assertEqual(self.fetch(http_error(403, x_ratelimit_remaining="0"), http_error(429), "[]"), [])
        self.assertEqual(self.sleeps, [5, 10])

    def test_a_refused_request_fails_at_once(self):
        with self.assertRaises(urllib.error.HTTPError):
            self.fetch(http_error(403), "[]")
        self.assertEqual(self.sleeps, [])

    def test_gives_up_after_the_last_attempt(self):
        with self.assertRaises(urllib.error.HTTPError):
            self.fetch(*[http_error(503)] * ci_job_annotations.REQUEST_ATTEMPTS)
        self.assertEqual(self.sleeps, [5, 10, 15])


if __name__ == "__main__":
    unittest.main()
