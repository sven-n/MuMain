#!/usr/bin/env python3
"""Runs tests/check_test_case_names.py on each fixture in tests/fixtures/test_case_names: the check must report the
fixture's offence and exit 1, and pass the clean fixture with every name it holds."""

import os
import subprocess
import sys
import unittest
from pathlib import Path

TESTS = Path(__file__).resolve().parent
CHECK = TESTS / "check_test_case_names.py"
FIXTURES = TESTS / "fixtures" / "test_case_names"
# Each failing fixture with the offence the check must print for it.
OFFENCES = {
    "semicolon": "has a ';', which cuts it in two",
    "comment_between_literals": "has a ';', which cuts it in two",
    "escaped_semicolon": "has a ';', which cuts it in two",
    "delimited_escape": "has a ';', which cuts it in two",
    "undecodable_escape": "an escape that stands for no character",
    "line_break": "has a line break",
    "brackets": "has unbalanced brackets",
    "close_bracket": "has unbalanced brackets",
    "backslash": "joins it with the next name",
    "backslash_inside": "doctest's filter reads as an escape",
    "not_literal": "is not written as string literals",
    "literal_and_macro": "is not written as string literals",
    "scenario": "has a ';', which cuts it in two",
    "raw_semicolon": "has a ';', which cuts it in two",
    "empty": "is empty, which the discovery drops",
    "doctest_header": "reads like a line of doctest's list output, which the discovery skips",
    "non_ascii": "has a character outside ASCII",
}
CLEAN = "clean"
CLEAN_NAMES = 12


def run_check(fixture):
    # PYTHONSAFEPATH (Python 3.11+) keeps the script's directory off sys.path; the check must still find cpp_text.
    environment = dict(os.environ, PYTHONSAFEPATH="1")
    return subprocess.run([sys.executable, str(CHECK), str(FIXTURES / fixture)], capture_output=True, text=True,
                          env=environment)


class CheckTestCaseNamesTests(unittest.TestCase):
    def test_reports_each_offence_and_fails(self):
        for fixture, offence in OFFENCES.items():
            with self.subTest(fixture):
                result = run_check(fixture)
                self.assertIn(offence, result.stdout, result.stderr)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)

    def test_passes_clean_names(self):
        result = run_check(CLEAN)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue(result.stdout.startswith(f"{CLEAN_NAMES} test case names reach CTest whole"), result.stdout)

    def test_runs_every_fixture(self):
        fixtures = {path.name for path in FIXTURES.iterdir() if path.is_dir()}
        self.assertEqual(fixtures, set(OFFENCES) | {CLEAN})


if __name__ == "__main__":
    unittest.main()
