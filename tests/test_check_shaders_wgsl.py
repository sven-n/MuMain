"""The binding split of scripts/check_shaders_wgsl.py, on a hand-written module: it needs no naga."""

import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "scripts"))
import check_shaders_wgsl as check


def module(*decorations):
    words = [check.SPIRV_MAGIC, 0x10000, 0, 100, 0]
    for variable, decoration, value in decorations:
        words += [(4 << 16) | check.OP_DECORATE, variable, decoration, value]
    return struct.pack(f"<{len(words)}I", *words)


def values(data):
    words = struct.unpack(f"<{len(data) // 4}I", data)[check.HEADER_WORDS:]
    return [words[i + 3] for i in range(0, len(words), 4)]


class SplitBindings(unittest.TestCase):
    def test_a_texture_and_sampler_on_one_binding_are_separated(self):
        shared = module((1, 34, 2), (1, 33, 0), (2, 34, 2), (2, 33, 0))
        self.assertEqual(values(check.split_bindings(shared)), [2, 0, 2, 16])

    def test_the_same_binding_in_another_set_stays(self):
        apart = module((1, 34, 2), (1, 33, 0), (2, 34, 3), (2, 33, 0))
        self.assertEqual(check.split_bindings(apart), apart)

    def test_a_file_that_is_not_spirv_is_refused(self):
        with self.assertRaises(ValueError):
            check.split_bindings(bytes(20))


if __name__ == "__main__":
    unittest.main()
