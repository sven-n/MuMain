#!/usr/bin/env python3
"""check_content_io.py against throwaway source trees."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parent / "check_content_io.py"


class CheckContentIoTests(unittest.TestCase):
    def run_check(self, files, baseline, *options):
        with tempfile.TemporaryDirectory() as root:
            for name, text in files.items():
                path = Path(root, "src", "source", name)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text, encoding="utf-8")
            Path(root, "tests").mkdir()
            baseline_file = Path(root, "tests", "check_content_io_baseline.txt")
            baseline_file.write_text(baseline, encoding="utf-8")
            result = subprocess.run([sys.executable, str(SCRIPT), root, *options], capture_output=True, text=True)
            result.baseline = baseline_file.read_text(encoding="utf-8")
            return result

    def test_a_call_the_baseline_allows_passes_and_is_still_to_move(self):
        result = self.run_check({"Data/Load.cpp": 'FILE* f = fopen("Data/x.bmd", "rb");\n'},
                                "src/source/Data/Load.cpp\tfopen\t1\n")
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(result.stdout, "still to move: src/source/Data/Load.cpp: 1 fopen call(s)\n")

    def test_a_call_with_a_reason_passes_quietly(self):
        result = self.run_check({"Data/Load.cpp": 'FILE* f = fopen("user.ini", "rb");\n'},
                                "src/source/Data/Load.cpp\tfopen\t1\tper-user file\n")
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(result.stdout, "")

    def test_update_keeps_the_reasons_of_lines_that_remain(self):
        files = {"Data/Load.cpp": 'fopen("a", "rb");\n', "Data/New.cpp": 'fopen("b", "rb");\n'}
        baseline = ("src/source/Data/Gone.cpp\tfopen\t1\tgone\n"
                    "src/source/Data/Load.cpp\tfopen\t2\tper-user file\n")
        result = self.run_check(files, baseline, "--update")
        self.assertEqual(result.returncode, 0, result.stdout)
        lines = [line for line in result.baseline.splitlines() if not line.startswith("#")]
        self.assertEqual(lines, ["src/source/Data/Load.cpp\tfopen\t1\tper-user file",
                                 "src/source/Data/New.cpp\tfopen\t1"])

    def test_a_new_call_fails_and_names_the_file(self):
        result = self.run_check({"Data/Load.cpp": 'std::ifstream file("Data/x.json");\n'}, "")
        self.assertEqual(result.returncode, 1)
        self.assertIn("src/source/Data/Load.cpp: 1 ifstream call(s), 0 allowed", result.stdout)

    def test_a_moved_call_fails_until_the_baseline_is_lowered(self):
        result = self.run_check({"Data/Load.cpp": "int nothing;\n"}, "src/source/Data/Load.cpp\tfopen\t1\n")
        self.assertEqual(result.returncode, 1)
        self.assertIn("lower it", result.stdout)

    def test_comments_strings_types_and_the_content_layer_are_not_calls(self):
        files = {
            "Data/Load.cpp": '// fopen(path)\nconst char* hint = "std::ifstream";\nauto m = std::ifstream::in;\n'
                             'void Read(std::ifstream& stream);\n',
            "Core/Content/ContentStore.cpp": "std::ifstream file(path);\n",
        }
        result = self.run_check(files, "")
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_each_kind_of_file_call_is_found(self):
        calls = [
            "_wfopen(p, L\"rb\");",
            "stream.open(p);",
            "std::fstream input(p, std::ios::in | std::ios::binary);",
            "GetFileAttributesW(p);",
            "std::filesystem::exists(p);",
            "std::filesystem::directory_iterator it(p);",
            "SDL_LoadPNG(p);",
            "TTF_OpenFont(p, 12);",
            "cgltf_parse_file(&o, p, &d);",
            "ma_sound_init_from_file(e, p, 0, 0, 0, &s);",
        ]
        result = self.run_check({"Audio/Load.inl": "\n".join(calls) + "\n"}, "")
        self.assertEqual(result.returncode, 1)
        self.assertEqual(len(result.stdout.splitlines()), len(calls), result.stdout)


if __name__ == "__main__":
    unittest.main()
