#!/usr/bin/env python3

import io
import sys
import unittest
import urllib.error
from contextlib import redirect_stdout
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from ci_cache_prune import KINDS, PER_COMPILER, READ_BY, SYSTEMS, delete_entry, prune, reader, timestamp

NOW = datetime(2026, 11, 6, 12, 0, tzinfo=timezone.utc)


def entry(entry_id, key, created_at, size=100, last_accessed_at=None):
    return {"id": entry_id, "key": key, "created_at": created_at, "size_in_bytes": size,
            "last_accessed_at": last_accessed_at or created_at}


class PruneTests(unittest.TestCase):
    def run_prune(self, entries_by_kind, kinds=("deps-",), read_at=None):
        deleted = []
        with redirect_stdout(io.StringIO()):
            freed = prune(kinds, lambda kind: entries_by_kind.get(kind, []), deleted.append, NOW, read_at)
        return [item["id"] for item in deleted], freed

    def test_keeps_the_newest_entry_of_each_kind(self):
        entries = {
            "deps-": [entry(1, "deps-a", "2026-11-03T10:00:00Z", 300), entry(2, "deps-b", "2026-11-05T10:00:00Z"),
                      entry(3, "deps-c", "2026-11-04T10:00:00Z", 200)],
            "ccache-": [entry(4, "ccache-x", "2026-11-01T10:00:00Z"), entry(5, "ccache-y", "2026-11-02T10:00:00Z")],
        }
        deleted, freed = self.run_prune(entries, kinds=("deps-", "ccache-"))
        self.assertEqual(sorted(deleted), [1, 3, 4])
        self.assertEqual(freed, 300 + 200 + 100)

    def test_a_single_entry_or_none_stays(self):
        self.assertEqual(self.run_prune({"deps-": [entry(1, "deps-a", "2026-11-03T10:00:00Z")]}), ([], 0))
        self.assertEqual(self.run_prune({}), ([], 0))

    def test_entries_saved_in_the_same_second_keep_the_later_id(self):
        same_time = "2026-11-05T10:00:00Z"
        deleted, _ = self.run_prune({"deps-": [entry(8, "deps-a", same_time), entry(9, "deps-b", same_time)]})
        self.assertEqual(deleted, [8])

    def test_times_compare_as_times(self):
        # GitHub's fractions of a second vary in length; as text, "44.1Z" would sort after "44.123456Z".
        deleted, _ = self.run_prune({"deps-": [entry(1, "deps-a", "2026-11-05T10:00:44.1Z"),
                                               entry(2, "deps-b", "2026-11-05T10:00:44.123456Z")]})
        self.assertEqual(deleted, [1])

    def test_an_entry_read_lately_waits_for_a_later_run(self):
        # A job that found it may still be downloading it.
        entries = {"deps-": [entry(1, "deps-a", "2026-11-03T10:00:00Z", last_accessed_at="2026-11-06T11:50:00Z"),
                             entry(2, "deps-b", "2026-11-04T10:00:00Z", last_accessed_at="2026-11-06T11:40:00Z"),
                             entry(3, "deps-c", "2026-11-05T10:00:00Z")]}
        self.assertEqual(self.run_prune(entries), ([2], 100))

    def test_this_runs_own_read_does_not_keep_an_entry(self):
        # The run read the entry its build started from, then saved a newer one.
        restores_ended = datetime(2026, 11, 6, 11, 52, tzinfo=timezone.utc)
        entries = {"linux-deps-": [entry(1, "linux-deps-a", "2026-11-05T10:00:00Z",
                                         last_accessed_at="2026-11-06T11:51:30Z"),
                                   entry(2, "linux-deps-b", "2026-11-06T11:58:00Z")]}
        kinds = ("linux-deps-",)
        self.assertEqual(self.run_prune(entries, kinds, {"linux": restores_ended}), ([1], 100))
        # A read after this run's restores ended is another job's, which may still be downloading.
        entries["linux-deps-"][0]["last_accessed_at"] = "2026-11-06T11:53:00Z"
        self.assertEqual(self.run_prune(entries, kinds, {"linux": restores_ended}), ([], 0))
        # Another system's restores say nothing about this kind's reads.
        self.assertEqual(self.run_prune(entries, kinds, {"windows": NOW}), ([], 0))

    def test_every_kind_names_its_system(self):
        for kind in KINDS:
            self.assertEqual(sum(name in kind for name in SYSTEMS), 1, kind)
        self.assertEqual(reader("vcpkg-windows-x64-v2-"), "windows")
        self.assertEqual(reader("apt-linux-x64-"), "linux")

    def test_quality_gates_reads_its_cppcheck(self):
        # Quality Gates restores the cppcheck cache, before the Linux build starts.
        kind = "apt-cppcheck-linux-x64-"
        self.assertEqual(reader(kind), "quality")
        self.assertTrue(set(READ_BY) <= set(KINDS))
        # Two entries of one image (the part up to the next "-"), as the prune keeps the newest of each.
        entries = {kind: [entry(1, f"{kind}image-hash1", "2026-11-05T10:00:00Z",
                                last_accessed_at="2026-11-06T11:51:30Z"),
                          entry(2, f"{kind}image-hash2", "2026-11-06T11:58:00Z")]}
        restores_ended = datetime(2026, 11, 6, 11, 52, tzinfo=timezone.utc)
        self.assertEqual(self.run_prune(entries, (kind,), {"quality": restores_ended}), ([1], 100))
        # The Linux build's restores say nothing about it: another run's Quality Gates may have read it since.
        entries[kind][0]["last_accessed_at"] = "2026-11-06T11:53:00Z"
        self.assertEqual(self.run_prune(entries, (kind,), {"linux": NOW, "quality": restores_ended}), ([], 0))

    def test_a_read_in_the_second_the_restores_ended_is_the_runs_own(self):
        # Quality Gates' one small restore can end within the second its read began: its note keeps the fraction
        # (ci.yml), so the read still comes before it.
        kind = "apt-cppcheck-linux-x64-"
        entries = {kind: [entry(1, f"{kind}image-hash1", "2026-11-05T10:00:00Z",
                                last_accessed_at="2026-11-06T11:52:00.3Z"),
                          entry(2, f"{kind}image-hash2", "2026-11-06T11:58:00Z")]}
        restores_ended = timestamp("2026-11-06T11:52:00.812345Z")
        self.assertEqual(self.run_prune(entries, (kind,), {"quality": restores_ended}), ([1], 100))
        # Cut to the second, the note would come before the read.
        self.assertEqual(self.run_prune(entries, (kind,), {"quality": timestamp("2026-11-06T11:52:00Z")}), ([], 0))

    def test_only_keys_of_the_kind_count(self):
        # GitHub matches the key as a prefix, but an entry outside the kind must never be deleted for it.
        entries = {"deps-": [entry(1, "deps-a", "2026-11-03T10:00:00Z"), entry(2, "other", "2026-11-04T10:00:00Z")]}
        self.assertEqual(self.run_prune(entries), ([], 0))

    def test_compiler_caches_keep_the_newest_entry_of_each_compiler(self):
        # A runner image with another compiler restores only entries of its own (ci.yml's restore-keys).
        kind = "ccache-linux-x64-"
        entries = {kind: [entry(1, f"{kind}aaaa-commit1", "2026-11-03T10:00:00Z"),
                          entry(2, f"{kind}aaaa-commit2", "2026-11-04T10:00:00Z"),
                          entry(3, f"{kind}bbbb-commit3", "2026-11-05T10:00:00Z")]}
        deleted, _ = self.run_prune(entries, kinds=(kind,))
        self.assertEqual(deleted, [1])

    def test_per_compiler_kinds_are_kinds(self):
        for kind in PER_COMPILER:
            self.assertIn(kind, KINDS)

    def test_kinds_do_not_contain_each_other(self):
        for kind in KINDS:
            for other in KINDS:
                if kind != other:
                    self.assertFalse(other.startswith(kind), f"{kind} would also prune {other}")


class DeleteEntryTests(unittest.TestCase):
    def fake_url(self, failures):
        calls = []

        class Response:
            def __enter__(self):
                return self

            def __exit__(self, *args):
                return False

        def open_url(request, timeout):
            calls.append(request.get_method())
            if failures:
                raise failures.pop(0)
            return Response()

        return calls, open_url

    def http_error(self, code):
        return urllib.error.HTTPError("https://api.github.com/x", code, "error", {}, None)

    def test_deletes_once_when_github_answers(self):
        calls, open_url = self.fake_url([])
        delete_entry("https://api.github.com/x", "token", open_url=open_url, sleep=lambda delay: None)
        self.assertEqual(calls, ["DELETE"])

    def test_an_entry_already_gone_counts_as_deleted(self):
        calls, open_url = self.fake_url([self.http_error(404)])
        delete_entry("https://api.github.com/x", "token", open_url=open_url, sleep=lambda delay: None)
        self.assertEqual(calls, ["DELETE"])

    def test_server_errors_are_sent_again(self):
        calls, open_url = self.fake_url([self.http_error(503), self.http_error(502)])
        with redirect_stdout(io.StringIO()):
            delete_entry("https://api.github.com/x", "token", open_url=open_url, sleep=lambda delay: None)
        self.assertEqual(calls, ["DELETE"] * 3)

    def test_a_missing_permission_fails_at_once(self):
        calls, open_url = self.fake_url([self.http_error(403)])
        with self.assertRaises(urllib.error.HTTPError):
            delete_entry("https://api.github.com/x", "token", open_url=open_url, sleep=lambda delay: None)
        self.assertEqual(calls, ["DELETE"])


if __name__ == "__main__":
    unittest.main()
