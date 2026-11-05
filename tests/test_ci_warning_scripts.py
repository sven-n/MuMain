#!/usr/bin/env python3
"""Tests for scripts/ci_warnings.py and scripts/ci_job_annotations.py, the CI rule that no warning comes back."""

import email.message
import http.client
import importlib.util
import io
import os
import ssl
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

    def test_scan_escapes_the_annotation_title(self):
        # Two logs make a title with ", " in it (and a Windows path a ':'); either ends the title unless escaped.
        logs = [self.summary.parent / "a.log", self.summary.parent / "b.log"]
        for log in logs:
            log.write_text("2026-11-04T01:26:43.0922614Z a.cpp:1:2: warning: x\n", encoding="utf-8")
        result = self.run_script("scan", *map(str, logs))
        self.assertEqual(result.returncode, 1)
        line = result.stdout.decode().splitlines()[0]
        title, _, message = line.removeprefix("::error title=").partition("::")
        self.assertNotIn(",", title)
        self.assertNotIn(":", title)
        self.assertIn("%2C", title)
        self.assertEqual(message, "a.cpp:1:2: warning: x")
        self.assertEqual(ci_warnings.escape_property("D:\\a: b, 100%"), "D%3A\\a%3A b%2C 100%25")


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
        service_messages = [
            "Failed to restore: Cache service responded with 429",
            "Failed to restore: Cache service responded with 503",
            "Failed to save: Failed to CreateCacheEntry: Received non-retryable error: Failed request: (502) Bad Gateway",
            "Failed to save: Unable to reserve cache with key x, another job may be creating this cache.",
            "Failed to save: cache entry with the same key, version, and scope already exists",
            "Failed to restore: Request timeout: /twirp/github.actions.results.api.v1.CacheService/GetCacheEntryDownloadURL",
            "Failed to restore: getaddrinfo EAI_AGAIN results-receiver.actions.githubusercontent.com",
            "Failed to save: socket hang up",
        ]
        failing_messages = [
            "Failed to save: Cache size of ~11000 MB (11534336000 B) is over the 10GB limit, not saving cache.",
            "Failed to restore: Cache service responded with 400",
            "Node.js 20 is deprecated.",
            "The step Failed to restore: Cache service responded with 503",
        ]
        found = [("Linux", {"annotation_level": "warning", "message": message})
                 for message in service_messages + failing_messages]
        failing, service = ci_job_annotations.split_service_warnings(found)
        self.assertEqual([a["message"] for _, a in service], service_messages)
        self.assertEqual([a["message"] for _, a in failing], failing_messages)

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
    NOW = 1_000_000

    def fetch(self, *outcomes):
        """fetch_json against answers given in order: an exception is raised, anything else is the body."""
        answers = list(outcomes)
        self.sleeps = []

        def open_url(request, timeout):
            answer = answers.pop(0)
            if isinstance(answer, Exception):
                raise answer
            return io.BytesIO(answer.encode())

        return ci_job_annotations.fetch_json("request", open_url, self.sleeps.append, lambda: self.NOW)

    def test_a_server_error_is_retried(self):
        self.assertEqual(self.fetch(http_error(502), '{"jobs": []}'), {"jobs": []})
        self.assertEqual(self.sleeps, [5])

    def test_a_network_failure_is_retried(self):
        self.assertEqual(self.fetch(urllib.error.URLError("reset"), TimeoutError(), ssl.SSLError("bad record"), "[]"),
                         [])
        self.assertEqual(self.sleeps, [5, 10, 15])

    def test_a_body_cut_off_is_retried(self):
        self.assertEqual(self.fetch(http.client.IncompleteRead(b"{"), "[]"), [])
        self.assertEqual(self.fetch('{"jobs": [', "[]"), [])
        self.assertEqual(self.sleeps, [5])

    def test_a_rate_limit_waits_as_github_asks(self):
        self.assertEqual(
            self.fetch(http_error(403, retry_after="30"),
                       http_error(403, x_ratelimit_remaining="0", x_ratelimit_reset=str(self.NOW + 20)),
                       http_error(429), "[]"),
            [])
        self.assertEqual(self.sleeps, [30, 21, 15])

    def test_a_long_wait_fails_at_once(self):
        for error in (http_error(429, retry_after="3600"),
                      http_error(403, x_ratelimit_remaining="0", x_ratelimit_reset=str(self.NOW + 3000)),
                      http_error(403, x_ratelimit_remaining="0")):
            with self.subTest(headers=dict(error.headers)):
                with self.assertRaises(urllib.error.HTTPError):
                    self.fetch(error, "[]")
                self.assertEqual(self.sleeps, [])

    def test_a_refused_request_fails_at_once(self):
        with self.assertRaises(urllib.error.HTTPError):
            self.fetch(http_error(403), "[]")
        with self.assertRaises(urllib.error.HTTPError):
            self.fetch(http_error(404), "[]")
        self.assertEqual(self.sleeps, [])

    def test_gives_up_after_the_last_attempt(self):
        with self.assertRaises(urllib.error.HTTPError):
            self.fetch(*[http_error(503)] * ci_job_annotations.REQUEST_ATTEMPTS)
        self.assertEqual(self.sleeps, [5, 10, 15])


if __name__ == "__main__":
    unittest.main()
