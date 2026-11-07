#!/usr/bin/env python3

import io
import os
import sys
import tempfile
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest import mock


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import ci_apt_cache  # noqa: E402
from ci_apt_cache import gather, image, key, listed_files  # noqa: E402

LISTING = (
    "'http://azure.archive.ubuntu.com/ubuntu/pool/main/c/cmake/cmake_4.2.3-2ubuntu2_amd64.deb' "
    "cmake_4.2.3-2ubuntu2_amd64.deb 12345678 SHA512:aa11\n"
    "'http://azure.archive.ubuntu.com/ubuntu/pool/main/p/pulseaudio/libpulse0_1%3a17.0-1_amd64.deb' "
    "libpulse0_1%3a17.0-1_amd64.deb 345678 SHA512:bb22\n"
)


class ListedFilesTests(unittest.TestCase):
    def test_names_each_listed_file(self):
        self.assertEqual(listed_files(LISTING),
                         ["cmake_4.2.3-2ubuntu2_amd64.deb", "libpulse0_1%3a17.0-1_amd64.deb"])

    def test_lines_that_list_no_file_are_left_out(self):
        text = "W: a message of a newer apt\n\n'' empty 1 x\n" + LISTING + "N: 2 files\n"
        self.assertEqual(len(listed_files(text)), 2)


class KeyTests(unittest.TestCase):
    def test_names_the_kind_the_image_and_a_hash(self):
        self.assertRegex(key("apt-linux-x64-", "ubuntu26_20261103.298.1", LISTING),
                         r"^apt-linux-x64-ubuntu26_20261103\.298\.1-[0-9a-f]{16}$")

    def test_another_mirror_for_the_same_files_keeps_the_key(self):
        elsewhere = LISTING.replace("http://azure.archive.ubuntu.com/ubuntu", "http://archive.ubuntu.com/ubuntu")
        self.assertEqual(key("k-", "i", LISTING), key("k-", "i", elsewhere))

    def test_the_order_of_the_files_does_not_count(self):
        reversed_listing = "\n".join(reversed(LISTING.splitlines())) + "\n"
        self.assertEqual(key("k-", "i", LISTING), key("k-", "i", reversed_listing))

    def test_a_new_version_size_or_hash_gives_a_new_key(self):
        for old, new in (("4.2.3-2ubuntu2_amd64.deb 1", "4.2.4-1_amd64.deb 1"), ("12345678", "12345679"),
                         ("SHA512:aa11", "SHA512:aa12")):
            self.assertNotEqual(key("k-", "i", LISTING), key("k-", "i", LISTING.replace(old, new)), old)

    def test_lines_that_list_no_file_keep_the_key(self):
        self.assertEqual(key("k-", "i", LISTING), key("k-", "i", "W: a message\n" + LISTING))

    def test_the_image_comes_from_the_runner(self):
        self.assertEqual(image({"ImageOS": "ubuntu26", "ImageVersion": "20261103.298.1"}), "ubuntu26_20261103.298.1")

    def test_the_image_has_no_dash(self):
        # The prune reads a key's image up to the next "-" (ci_cache_prune.group), as it reads a compiler.
        self.assertNotIn("-", image({"ImageOS": "ubuntu-26", "ImageVersion": "2026-11-03"}))

    def test_without_an_image_the_key_still_has_one(self):
        self.assertEqual(image({}), "unknown")

    def test_name_writes_the_image_and_the_key(self):
        with tempfile.TemporaryDirectory() as folder:
            listing = Path(folder) / "listing.txt"
            listing.write_text(LISTING, encoding="utf-8")
            outputs = Path(folder) / "outputs.txt"
            environment = {"GITHUB_OUTPUT": str(outputs), "ImageOS": "ubuntu26", "ImageVersion": "1"}
            with redirect_stdout(io.StringIO()), mock.patch.dict(os.environ, environment):
                ci_apt_cache.main(["name", "apt-linux-x64-", str(listing)])
            written = outputs.read_text(encoding="utf-8")
        self.assertEqual(written, f"image=ubuntu26_1\nkey={key('apt-linux-x64-', 'ubuntu26_1', LISTING)}\n")


class GatherTests(unittest.TestCase):
    def setUp(self):
        folder = tempfile.TemporaryDirectory()
        self.addCleanup(folder.cleanup)
        self.archives = Path(folder.name) / "archives"
        self.destination = Path(folder.name) / "cache"
        self.archives.mkdir()

    def test_copies_the_listed_files_and_nothing_else(self):
        for name in ("a.deb", "b.deb", "unlisted.deb"):
            (self.archives / name).write_bytes(name.encode())
        self.destination.mkdir()
        (self.destination / "from-an-older-entry.deb").write_bytes(b"old")
        self.assertEqual(gather(["a.deb", "b.deb"], self.archives, self.destination), [])
        self.assertEqual(sorted(path.name for path in self.destination.iterdir()), ["a.deb", "b.deb"])
        self.assertEqual((self.destination / "a.deb").read_bytes(), b"a.deb")

    def test_returns_the_missing_files(self):
        (self.archives / "a.deb").write_bytes(b"a")
        self.assertEqual(gather(["a.deb", "gone.deb"], self.archives, self.destination), ["gone.deb"])

    def test_a_missing_file_gets_a_notice_that_names_it(self):
        listing = self.archives.parent / "listing.txt"
        listing.write_text(LISTING, encoding="utf-8")
        (self.archives / "cmake_4.2.3-2ubuntu2_amd64.deb").write_bytes(b"cmake")
        out = io.StringIO()
        outputs = self.archives.parent / "outputs.txt"
        with redirect_stdout(out), mock.patch.dict(os.environ, {"GITHUB_OUTPUT": str(outputs)}):
            ci_apt_cache.main(["gather", str(listing), str(self.archives), str(self.destination)])
        notice = next(line for line in out.getvalue().splitlines() if line.startswith("::notice "))
        self.assertIn("1 of 2 listed files", notice)
        self.assertIn("libpulse0_1%3a17.0-1_amd64.deb", notice)

    def complete_output(self, listing_text, present):
        """What main writes to GITHUB_OUTPUT for a listing when only the present files are in apt's folder."""
        listing = self.archives.parent / "listing.txt"
        listing.write_text(listing_text, encoding="utf-8")
        for name in present:
            (self.archives / name).write_bytes(name.encode())
        outputs = self.archives.parent / "outputs.txt"
        with redirect_stdout(io.StringIO()), mock.patch.dict(os.environ, {"GITHUB_OUTPUT": str(outputs)}):
            ci_apt_cache.main(["gather", str(listing), str(self.archives), str(self.destination)])
        return outputs.read_text(encoding="utf-8")

    def test_complete_only_with_every_listed_file(self):
        # Save apt packages stores an entry only when Gather apt packages says complete=true.
        self.assertEqual(self.complete_output(LISTING, listed_files(LISTING)), "complete=true\n")

    def test_a_missing_file_leaves_the_entry_unsaved(self):
        self.assertEqual(self.complete_output(LISTING, listed_files(LISTING)[:1]), "complete=false\n")

    def test_an_empty_listing_leaves_nothing_to_save(self):
        self.assertEqual(self.complete_output("", []), "complete=false\n")

    def test_a_long_notice_counts_the_rest(self):
        missing = [f"p{index}.deb" for index in range(ci_apt_cache.NAMED_MISSING + 3)]
        notice = ci_apt_cache.missing_notice(missing, len(missing), self.archives)
        self.assertIn("p0.deb", notice)
        self.assertNotIn(f"p{ci_apt_cache.NAMED_MISSING}.deb", notice)
        self.assertTrue(notice.endswith("and 3 more"))


if __name__ == "__main__":
    unittest.main()
