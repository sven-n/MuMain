#!/usr/bin/env python3
"""scripts/stamp_release_version.sh on archives laid out like the release builds' (ci.yml): the release tag lands
next to the client, and every entry the build wrote stays."""

import io
import os
import shutil
import subprocess
import tarfile
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts" / "stamp_release_version.sh"


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


def write_archive(path, files):
    with tarfile.open(path, "w:gz") as archive:
        for name, data in files.items():
            entry = tarfile.TarInfo(name)
            entry.size = len(data)
            archive.addfile(entry, io.BytesIO(data))


def read_archive(path):
    with tarfile.open(path, "r:gz") as archive:
        return {entry.name: archive.extractfile(entry).read() for entry in archive.getmembers() if entry.isfile()}


@unittest.skipUnless(BASH, "bash is required")
class StampReleaseVersionTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.folder = Path(temporary.name)

    def stamp(self, tag, *archives):
        return subprocess.run([BASH, SCRIPT.as_posix(), tag, *archives], cwd=self.folder, capture_output=True,
                              text=True)

    def test_the_tag_lands_next_to_the_client(self):
        # Windows and Linux archive their Release folder's content (tar -C <Release> .), macOS its Main.app.
        write_archive(self.folder / "windows.tar.gz", {"./Main.exe": b"exe", "./Data/a.txt": b"a"})
        write_archive(self.folder / "macos.tar.gz", {"Main.app/Contents/MacOS/Main": b"bin",
                                                     "Main.app/Contents/Info.plist": b"plist"})

        result = self.stamp("v1.37.0", "windows.tar.gz", "macos.tar.gz")
        self.assertEqual(result.returncode, 0, result.stderr)

        self.assertEqual(read_archive(self.folder / "windows.tar.gz"), {
            "./Main.exe": b"exe", "./Data/a.txt": b"a", "./release-version.txt": b"v1.37.0\n"})
        self.assertEqual(read_archive(self.folder / "macos.tar.gz"), {
            "Main.app/Contents/MacOS/Main": b"bin", "Main.app/Contents/Info.plist": b"plist",
            "Main.app/Contents/MacOS/release-version.txt": b"v1.37.0\n"})

    def test_a_missing_archive_fails(self):
        result = self.stamp("v1.37.0", "missing.tar.gz")
        self.assertNotEqual(result.returncode, 0)


if __name__ == "__main__":
    unittest.main()
