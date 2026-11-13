#!/usr/bin/env python3
"""check_network_bridge.py against throwaway source trees."""

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

SCRIPT = Path(__file__).resolve().parent / "check_network_bridge.py"

LISTS = ('constexpr ManagedExport kManagedExports_Connection[] = {\n    {"ConnectionManager_Send"},\n};\n'
         'inline constexpr const char* kManagedExports_Chat[] = {\n    "SendPublicChatMessage",\n    "SendKeepAlive",\n};\n')
LOOKUPS = ('auto send = LoadManagedSymbol<Send>("ConnectionManager_Send");\n'
           'static const auto chat = LoadManagedSymbol<::SendPublicChatMessage>("SendPublicChatMessage");\n'
           'static const auto keepAlive =\n    LoadManagedSymbol<::SendKeepAlive>("SendKeepAlive");\n')
HEADER = '#pragma once\n#include "ManagedSymbols.h"\nvoid InitializeDotNetBridge();\n'


class CheckNetworkBridgeTests(unittest.TestCase):
    def run_check(self, files):
        with tempfile.TemporaryDirectory() as root:
            for name, text in files.items():
                path = Path(root, "src", "source", name)
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text, encoding="utf-8")
            return subprocess.run([sys.executable, str(SCRIPT), root], capture_output=True, text=True)

    def test_lists_that_match_the_lookups_pass(self):
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": LOOKUPS,
                                 "Dotnet/Connection.h": HEADER})
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(result.stdout, "")

    def test_a_lookup_no_list_holds_fails(self):
        lists = LISTS.replace('    "SendPublicChatMessage",\n', "")
        result = self.run_check({"Dotnet/Lists.cpp": lists, "Dotnet/Senders.cpp": LOOKUPS})
        self.assertEqual(result.returncode, 1)
        self.assertIn("looks up SendPublicChatMessage, which no export list holds", result.stdout)

    def test_a_listed_export_nothing_looks_up_fails(self):
        result = self.run_check({"Dotnet/Lists.cpp": LISTS + 'constexpr const char* kManagedExports_X[] = {"SendGone"};\n',
                                 "Dotnet/Senders.cpp": LOOKUPS})
        self.assertEqual(result.returncode, 1)
        self.assertIn("lists SendGone, which nothing looks up", result.stdout)

    def test_a_lookup_in_a_comment_is_not_a_lookup(self):
        senders = LOOKUPS + '// LoadManagedSymbol<Old>("SendOld") is gone\n'
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": senders})
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_platform_conditional_in_a_bridge_header_fails(self):
        header = HEADER + "#ifdef _WIN32\nint windowsOnly;\n#endif\n"
        including = '#pragma once\n#include "Dotnet/Connection.h"\n#if defined(__APPLE__)\nint appleOnly;\n#endif\n'
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": LOOKUPS,
                                 "Dotnet/Connection.h": header, "Network/Client.h": including})
        self.assertEqual(result.returncode, 1)
        self.assertIn("src/source/Dotnet/Connection.h:4: platform conditional", result.stdout)
        self.assertIn("src/source/Network/Client.h:3: platform conditional", result.stdout)

    def test_library_loading_in_a_bridge_header_fails(self):
        header = HEADER + '#include "Core/Platform/PlatformLibrary.h"\ninline void* Load() { return dlopen("x", 0); }\n'
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": LOOKUPS,
                                 "Dotnet/Connection.h": header})
        self.assertEqual(result.returncode, 1)
        self.assertEqual(result.stdout.count("loads a library"), 2, result.stdout)

    def test_commented_out_platform_code_passes(self):
        header = (HEADER + "/* Before the adapters:\n#ifdef _WIN32\n    return LoadLibraryW(path);\n#endif\n*/\n"
                  "// #if defined(__APPLE__)\nint kept; // dlopen(\"x\", 0) was here\n")
        commented_include = '#pragma once\n/* #include "Connection.h" */\n#ifdef _WIN32\nint windowsOnly;\n#endif\n'
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": LOOKUPS,
                                 "Dotnet/Connection.h": header, "Network/Client.h": commented_include})
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_conditional_after_a_block_comment_still_fails(self):
        header = HEADER + "/* a note */\n#ifdef _WIN32\nint windowsOnly;\n#endif\n"
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": LOOKUPS,
                                 "Dotnet/Connection.h": header})
        self.assertEqual(result.returncode, 1)
        self.assertIn("src/source/Dotnet/Connection.h:5: platform conditional", result.stdout)

    def test_feature_conditionals_and_other_headers_pass(self):
        including = '#pragma once\n#include "Connection.h"\n#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM\nint shop;\n#endif\n'
        unrelated = "#pragma once\n#ifdef _WIN32\nint windowsOnly;\n#endif\n"
        result = self.run_check({"Dotnet/Lists.cpp": LISTS, "Dotnet/Senders.cpp": LOOKUPS,
                                 "Network/Client.h": including, "Core/Platform/Other.h": unrelated})
        self.assertEqual(result.returncode, 0, result.stdout)


if __name__ == "__main__":
    unittest.main()
