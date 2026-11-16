#!/usr/bin/env python3
"""check_quit_paths.py against throwaway source trees."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parent / "check_quit_paths.py"


class CheckQuitPathsTests(unittest.TestCase):
    def run_check(self, files):
        with tempfile.TemporaryDirectory() as root:
            for name, text in files.items():
                path = Path(root, "src", "source", name)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text, encoding="utf-8")
            return subprocess.run([sys.executable, str(SCRIPT), root], capture_output=True, text=True)

    def test_quit_and_error_calls_pass(self):
        result = self.run_check({
            "Data/Load.cpp": "void f() { Core::Platform::ShowFatalError(L\"x\", L\"MessageBox(\"); }\n"
                             "// SendMessage(g_hWnd, WM_DESTROY, 0, 0);\n/* MessageBox(g_hWnd, t, NULL, MB_OK); */\n",
            "Core/Input/Ime.cpp": "void g() { ::SendMessage(hWnd, WM_IME_CONTROL, 0, 0); }\n",
            "Core/Platform/WinUser.cpp": "int MessageBoxW(HWND, LPCWSTR, LPCWSTR, UINT) { return 1; }\n"
                                         "void h() { PostQuitMessage(0); }\n",
        })
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_window_message_quits_and_message_boxes_fail(self):
        result = self.run_check({
            "Data/Load.cpp": "void f()\n{\n    MessageBox(g_hWnd, Text, NULL, MB_OK);\n"
                             "    SendMessage(g_hWnd,\n                WM_DESTROY, 0, 0);\n}\n",
            "UI/Menu.cpp": "void g() { ::PostMessage(g_hWnd, WM_CLOSE, 0, 0); ::MessageBoxW(nullptr, t, c, 0); }\n",
            "Render/Terrain.cpp": "void h() { PostQuitMessage(0); }\n",
            "Net/Thread.cpp": "void k() { PostThreadMessageW(id, WM_QUIT, 0, 0); MessageBoxExW(h, t, c, 0, 0); }\n",
        })
        self.assertEqual(result.returncode, 1)
        for expected in ("src/source/Data/Load.cpp:3: MessageBox", "src/source/Data/Load.cpp:4: SendMessage",
                         "src/source/UI/Menu.cpp:1: PostMessage", "src/source/UI/Menu.cpp:1: MessageBoxW",
                         "src/source/Render/Terrain.cpp:1: PostQuitMessage",
                         "src/source/Net/Thread.cpp:1: PostThreadMessageW", "src/source/Net/Thread.cpp:1: MessageBoxExW"):
            self.assertIn(expected, result.stdout)


if __name__ == "__main__":
    unittest.main()
