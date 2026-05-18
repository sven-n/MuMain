#!/usr/bin/env python3
import sys
import unittest
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import remaster_glb_v2 as remaster


class PreserveMaterialMetadataTests(unittest.TestCase):
    def test_preserves_source_material_semantics(self) -> None:
        source_payload = {
            "materials": [
                {
                    "pbrMetallicRoughness": {"baseColorTexture": {"index": 0}},
                    "alphaMode": "OPAQUE",
                    "doubleSided": True,
                    "emissiveFactor": [1.0, 1.0, 1.0],
                    "emissiveTexture": {"index": 0},
                    "extras": {
                        "legacy_blend_mode": "additive",
                        "mu_legacy_additive_intensity": 1.0,
                    },
                }
            ]
        }
        candidate_payload = {
            "materials": [
                {
                    "pbrMetallicRoughness": {"baseColorTexture": {"index": 0}},
                    "extras": {"candidate_note": "keep"},
                }
            ]
        }

        changed = remaster.preserve_source_material_render_metadata(
            source_payload, candidate_payload
        )

        self.assertTrue(changed)
        material = candidate_payload["materials"][0]
        self.assertEqual(material["alphaMode"], "OPAQUE")
        self.assertTrue(material["doubleSided"])
        self.assertEqual(material["emissiveFactor"], [1.0, 1.0, 1.0])
        self.assertEqual(material["emissiveTexture"], {"index": 0})
        self.assertEqual(material["extras"]["legacy_blend_mode"], "additive")
        self.assertEqual(material["extras"]["mu_legacy_additive_intensity"], 1.0)
        self.assertEqual(material["extras"]["candidate_note"], "keep")

    def test_returns_false_when_material_lists_are_missing(self) -> None:
        self.assertFalse(
            remaster.preserve_source_material_render_metadata({}, {"materials": []})
        )
        self.assertFalse(
            remaster.preserve_source_material_render_metadata({"materials": []}, {})
        )

    def test_ignores_non_overlapping_material_indices(self) -> None:
        source_payload = {"materials": [{"extras": {"legacy_blend_mode": "additive"}}, {}]}
        candidate_payload = {"materials": [{"pbrMetallicRoughness": {"baseColorFactor": [1, 1, 1, 1]}}]}

        changed = remaster.preserve_source_material_render_metadata(
            source_payload, candidate_payload
        )

        self.assertTrue(changed)
        self.assertEqual(
            candidate_payload["materials"][0]["extras"]["legacy_blend_mode"], "additive"
        )


if __name__ == "__main__":
    unittest.main()
