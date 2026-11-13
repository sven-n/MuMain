#!/usr/bin/env python3
"""check_platform_services.py against throwaway source trees."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parent / "check_platform_services.py"


class CheckPlatformServicesTests(unittest.TestCase):
    def run_check(self, files):
        with tempfile.TemporaryDirectory() as root:
            for name, text in files.items():
                path = Path(root, "src", "source", name)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text, encoding="utf-8")
            return subprocess.run([sys.executable, str(SCRIPT), root], capture_output=True, text=True)

    def test_platform_code_in_adapters_passes(self):
        result = self.run_check({
            "Core/Platform/Adapters/OpenSslCredentials.cpp": "#include <openssl/evp.h>\n#include \"Core/Platform/Dpapi.h\"\n",
            "GameShop/FileDownloader/Adapters/CurlShopDownload.cpp": "#include <curl/curl.h>\n",
            "Core/Utilities/Adapters/CpuUsagePosix.cpp": "int f() { rusage ru; return getrusage(RUSAGE_SELF, &ru); }\n",
            "Core/Platform/Dpapi.h": "inline BOOL CryptUnprotectData(DATA_BLOB*) { return TRUE; }\n",
        })
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_platform_code_outside_adapters_fails(self):
        result = self.run_check({
            "Data/GameConfig/GameConfig.cpp": "#include \"stdafx.h\"\n#include \"Core/Platform/Dpapi.h\"\n"
                                               "void f() { CryptUnprotectData(&in, 0, 0, 0, 0, 0, &out); }\n",
            "GameShop/Shop.h": "#pragma once\n#include <curl/curl.h>\n",
            "Core/Utilities/Cpu.cpp": "void g() { GetProcessTimes(h, &a, &b, &c, &d); }\n",
            "Core/Platform/Crypto.cpp": "#  include <openssl/rand.h>\n",
            "UI/Windows/LoginWin.cpp": "#ifdef _WIN32\n#include <dpapi.h>\n#endif\n",
        })
        self.assertEqual(result.returncode, 1)
        for expected in ("src/source/Data/GameConfig/GameConfig.cpp:2:", "src/source/Data/GameConfig/GameConfig.cpp:3:",
                         "src/source/GameShop/Shop.h:2:", "src/source/Core/Utilities/Cpu.cpp:1:",
                         "src/source/Core/Platform/Crypto.cpp:1:", "src/source/UI/Windows/LoginWin.cpp:2:"):
            self.assertIn(expected, result.stdout)

    def test_a_platform_branch_in_an_adapter_or_service_header_fails(self):
        result = self.run_check({
            "Core/Platform/Adapters/OpenSslCredentials.cpp": "#ifdef _WIN32\n#include <wincrypt.h>\n#endif\n",
            "Core/Utilities/CpuUsage.h": "#pragma once\n#if defined(__EMSCRIPTEN__)\nint none;\n#endif\n",
            "Core/Other.cpp": "#ifdef _WIN32\nint elsewhere;\n#endif\n",
        })
        self.assertEqual(result.returncode, 1)
        self.assertIn("src/source/Core/Platform/Adapters/OpenSslCredentials.cpp:1: #ifdef _WIN32", result.stdout)
        self.assertIn("src/source/Core/Utilities/CpuUsage.h:2: #if defined(__EMSCRIPTEN__)", result.stdout)
        self.assertNotIn("Core/Other.cpp", result.stdout)

    def test_commented_out_platform_code_passes(self):
        result = self.run_check({
            "Data/GameConfig/GameConfig.cpp": "// CryptUnprotectData(&in, ...) moved to the adapter\n"
                                               "/* #include <curl/curl.h> */\nint kept;\n",
        })
        self.assertEqual(result.returncode, 0, result.stdout)


if __name__ == "__main__":
    unittest.main()
