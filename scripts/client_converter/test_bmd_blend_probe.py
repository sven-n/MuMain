#!/usr/bin/env python3
import json
import tempfile
import unittest
from pathlib import Path
import sys


SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

import bmd_converter as converter


def _triangle_mesh(texture_index: int, texture_name: str) -> converter.BmdMesh:
    vertices = [
        converter.BmdVertex(node=0, position=(0.0, 0.0, 0.0)),
        converter.BmdVertex(node=0, position=(1.0, 0.0, 0.0)),
        converter.BmdVertex(node=0, position=(0.0, 1.0, 0.0)),
    ]
    normals = [
        converter.BmdNormal(node=0, normal=(0.0, 0.0, 1.0), bind_vertex=0),
        converter.BmdNormal(node=0, normal=(0.0, 0.0, 1.0), bind_vertex=1),
        converter.BmdNormal(node=0, normal=(0.0, 0.0, 1.0), bind_vertex=2),
    ]
    texcoords = [
        converter.BmdTexCoord(u=0.0, v=0.0),
        converter.BmdTexCoord(u=1.0, v=0.0),
        converter.BmdTexCoord(u=0.0, v=1.0),
    ]
    triangles = [
        converter.BmdTriangle(
            polygon=3,
            vertex_index=(0, 1, 2, 0),
            normal_index=(0, 1, 2, 0),
            texcoord_index=(0, 1, 2, 0),
        )
    ]
    return converter.BmdMesh(
        num_vertices=3,
        num_normals=3,
        num_texcoords=3,
        num_triangles=1,
        texture=texture_index,
        vertices=vertices,
        normals=normals,
        texcoords=texcoords,
        triangles=triangles,
        texture_name=texture_name,
    )


def _build_test_model(texture_index: int = 1) -> converter.BmdModel:
    textured_mesh = _triangle_mesh(texture_index=texture_index, texture_name="Glow.jpg")
    texture_reference_mesh = converter.BmdMesh(
        num_vertices=0,
        num_normals=0,
        num_texcoords=0,
        num_triangles=0,
        texture=0,
        vertices=[],
        normals=[],
        texcoords=[],
        triangles=[],
        texture_name="GlowReference.jpg",
    )
    return converter.BmdModel(
        name="ProbeModel",
        version=0x0A,
        num_meshs=2,
        num_bones=0,
        num_actions=0,
        meshs=[textured_mesh, texture_reference_mesh],
        actions=[],
        bones=[],
    )


class BlendProbeTests(unittest.TestCase):
    def test_parse_magick_gray_histogram_accepts_graya(self) -> None:
        total, bins = converter._parse_magick_gray_histogram(
            "\n".join(
                [
                    "2426: (0,0,0,255) #000000FF graya(0,1)",
                    "407: (1,1,1,255) #010101FF graya(1,1)",
                    "10: (255,255,255,255) #FFFFFFFF graya(255,1)",
                ]
            )
        )
        self.assertEqual(total, 2843)
        self.assertEqual(bins.get(0), 2426)
        self.assertEqual(bins.get(1), 407)
        self.assertEqual(bins.get(255), 10)

    def test_legacy_object_identity_detects_object4_object40(self) -> None:
        path = Path("/tmp/Data/Object4/Object40.bmd")
        identity = converter._legacy_object_identity_from_source_path(path)
        self.assertEqual(identity, (4, 40))

    def test_legacy_model_key_detects_object1_posebox01(self) -> None:
        path = Path("/tmp/DataNorm/object_1/pose_box_01.bmd")
        key = converter._legacy_model_key_from_source_path(path)
        self.assertEqual(key, "object1/posebox01")

    def test_legacy_model_key_detects_object4_object39(self) -> None:
        path = Path("/tmp/DataNorm/object_4/object_39.bmd")
        key = converter._legacy_model_key_from_source_path(path)
        self.assertEqual(key, "object4/object39")

    def test_legacy_model_key_detects_object1_housewall02(self) -> None:
        path = Path("/tmp/DataNorm/object_1/house_wall_02.bmd")
        key = converter._legacy_model_key_from_source_path(path)
        self.assertEqual(key, "object1/housewall02")

    def test_legacy_model_key_detects_skill_fire01(self) -> None:
        path = Path("/tmp/Data/Skill/Fire01.bmd")
        key = converter._legacy_model_key_from_source_path(path)
        self.assertEqual(key, "skill/fire01")

    def test_bmd_to_glb_collects_additive_probe_for_object4_object40(self) -> None:
        model = _build_test_model(texture_index=1)
        records = []
        glb = converter.bmd_to_glb(
            model,
            texture_resolver=None,
            source_path=Path("/tmp/Data/Object4/Object40.bmd"),
            blend_probe_records=records,
            force_player_inplace=True,
        )

        self.assertIsNotNone(glb)
        self.assertGreater(len(records), 0)
        first = records[0]
        self.assertEqual(first["legacy_blend_mode"], "additive")
        self.assertEqual(first["material_kind"], "additive_emissive")
        self.assertEqual(first["material_decision_source"], "legacy_blend_texture_index")
        self.assertEqual(first["material_inference_mode"], "additive")
        self.assertEqual(first["material_inference_source"], "legacy_blend_texture_index")
        self.assertEqual(first["object_dir"], 4)
        self.assertEqual(first["object_model"], 40)

    def test_bmd_to_glb_collects_additive_probe_for_model_key_posebox(self) -> None:
        model = _build_test_model(texture_index=0)
        records = []
        glb = converter.bmd_to_glb(
            model,
            texture_resolver=None,
            source_path=Path("/tmp/DataNorm/object_1/pose_box_01.bmd"),
            blend_probe_records=records,
            force_player_inplace=True,
        )

        self.assertIsNotNone(glb)
        self.assertGreater(len(records), 0)
        first = records[0]
        self.assertEqual(first["legacy_blend_mode"], "additive")
        self.assertEqual(first["material_kind"], "additive_emissive")
        self.assertEqual(first["material_decision_source"], "legacy_blend_texture_index")
        self.assertEqual(first["material_inference_mode"], "additive")
        self.assertEqual(first["material_inference_source"], "legacy_blend_texture_index")
        self.assertIsNone(first["object_dir"])
        self.assertIsNone(first["object_model"])

    def test_bmd_to_glb_collects_additive_probe_for_model_key_object4_object39(self) -> None:
        model = _build_test_model(texture_index=0)
        records = []
        glb = converter.bmd_to_glb(
            model,
            texture_resolver=None,
            source_path=Path("/tmp/DataNorm/object_4/object_39.bmd"),
            blend_probe_records=records,
            force_player_inplace=True,
        )

        self.assertIsNotNone(glb)
        self.assertGreater(len(records), 0)
        first = records[0]
        self.assertEqual(first["legacy_blend_mode"], "additive")
        self.assertEqual(first["material_kind"], "additive_emissive")
        self.assertEqual(first["material_decision_source"], "legacy_blend_texture_index")
        self.assertEqual(first["material_inference_mode"], "additive")
        self.assertEqual(first["material_inference_source"], "legacy_blend_texture_index")
        self.assertIsNone(first["object_dir"])
        self.assertIsNone(first["object_model"])

    def test_bmd_to_glb_collects_additive_probe_for_model_key_housewall02(self) -> None:
        model = _build_test_model(texture_index=4)
        records = []
        glb = converter.bmd_to_glb(
            model,
            texture_resolver=None,
            source_path=Path("/tmp/DataNorm/object_1/house_wall_02.bmd"),
            blend_probe_records=records,
            force_player_inplace=True,
        )

        self.assertIsNotNone(glb)
        self.assertGreater(len(records), 0)
        first = records[0]
        self.assertEqual(first["legacy_blend_mode"], "additive")
        self.assertEqual(first["material_kind"], "additive_emissive")
        self.assertEqual(first["material_decision_source"], "legacy_blend_texture_index")
        self.assertEqual(first["material_inference_mode"], "additive")
        self.assertEqual(first["material_inference_source"], "legacy_blend_texture_index")
        self.assertIsNone(first["object_dir"])
        self.assertIsNone(first["object_model"])

    def test_bmd_to_glb_collects_additive_probe_for_model_key_skill_fire(self) -> None:
        model = _build_test_model(texture_index=1)
        records = []
        glb = converter.bmd_to_glb(
            model,
            texture_resolver=None,
            source_path=Path("/tmp/Data/Skill/Fire01.bmd"),
            blend_probe_records=records,
            force_player_inplace=True,
        )

        self.assertIsNotNone(glb)
        self.assertGreater(len(records), 0)
        first = records[0]
        self.assertEqual(first["legacy_blend_mode"], "additive")
        self.assertEqual(first["material_kind"], "additive_emissive")
        self.assertEqual(first["material_decision_source"], "legacy_blend_texture_index")
        self.assertEqual(first["material_inference_mode"], "additive")
        self.assertEqual(first["material_inference_source"], "legacy_blend_texture_index")
        self.assertIsNone(first["object_dir"])
        self.assertIsNone(first["object_model"])

    def test_write_blend_probe_report_serializes_summary(self) -> None:
        entries = [
            {
                "material_decision_source": "legacy_blend_texture_index",
                "material_kind": "additive_emissive",
                "material_alpha_mode": "OPAQUE",
                "material_inference_mode": "additive",
                "material_inference_source": "legacy_blend_texture_index",
                "object_dir": 4,
                "object_model": 40,
            },
            {
                "material_decision_source": "default",
                "material_kind": "textured_pbr",
                "material_alpha_mode": "OPAQUE",
                "material_inference_mode": "opaque",
                "material_inference_source": "default",
                "object_dir": None,
                "object_model": None,
            },
        ]

        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = Path(temp_dir) / "blend_probe.json"
            converter.write_blend_probe_report(entries, output_path)
            payload = json.loads(output_path.read_text())

        self.assertEqual(payload["entry_count"], 2)
        self.assertEqual(
            payload["summary"]["material_decision_source_counts"][
                "legacy_blend_texture_index"
            ],
            1,
        )
        self.assertEqual(payload["summary"]["material_inference_mode_counts"]["additive"], 1)
        self.assertEqual(payload["summary"]["material_inference_source_counts"]["default"], 1)
        self.assertEqual(payload["summary"]["legacy_object_counts"]["object4/object40"], 1)

    def test_material_decision_uses_alpha_profile_for_blend(self) -> None:
        decision = converter._material_decision_from_inputs(
            texture_uri="effect.png",
            legacy_blend_mode=None,
            texture_signal_profile_by_uri={
                "effect.png": (True, True, 0.20, 0.10, 0.01, 0.02, 0.30, 0.40, 0.22, 0.31),
            },
        )
        self.assertEqual(decision["material_kind"], "textured_pbr")
        self.assertEqual(decision["alpha_mode"], "BLEND")
        self.assertEqual(decision["decision_source"], "alpha_profile")
        self.assertEqual(decision["inference_mode"], "blend")

    def test_material_decision_infers_additive_from_rgb_key_signal(self) -> None:
        decision = converter._material_decision_from_inputs(
            texture_uri="wing.jpg",
            legacy_blend_mode=None,
            texture_signal_profile_by_uri={
                "wing.jpg": (False, False, 0.0, 1.0, 0.80, 0.82, 0.20, 0.18, 0.30, 0.92),
            },
        )
        self.assertEqual(decision["material_kind"], "additive_emissive")
        self.assertEqual(decision["decision_source"], "legacy_rgb_key")
        self.assertEqual(decision["inference_mode"], "additive")
        self.assertAlmostEqual(float(decision["additive_intensity"]), 1.2, places=3)

    def test_material_decision_infers_additive_from_laser_hint_and_signal(self) -> None:
        decision = converter._material_decision_from_inputs(
            texture_uri="dragon_02.png",
            legacy_blend_mode=None,
            texture_signal_profile_by_uri={
                "dragon_02.png": (
                    False,
                    False,
                    0.0,
                    1.0,
                    0.378,
                    0.401,
                    0.138,
                    0.336,
                    0.324,
                    0.804,
                ),
            },
            material_hint_text="/tmp/DataNorm/skill/laser_01.bmd dragon_02.png",
        )
        self.assertEqual(decision["material_kind"], "additive_emissive")
        self.assertEqual(decision["decision_source"], "legacy_rgb_key")
        self.assertEqual(decision["inference_mode"], "additive")

    def test_material_decision_keeps_dark_texture_opaque_without_energy_signal(self) -> None:
        decision = converter._material_decision_from_inputs(
            texture_uri="stone_dark.png",
            legacy_blend_mode=None,
            texture_signal_profile_by_uri={
                "stone_dark.png": (
                    False,
                    False,
                    0.0,
                    1.0,
                    0.42,
                    0.68,
                    0.001,
                    0.21,
                    0.10,
                    0.05,
                ),
            },
        )
        self.assertEqual(decision["material_kind"], "textured_pbr")
        self.assertEqual(decision["alpha_mode"], "OPAQUE")
        self.assertEqual(decision["decision_source"], "default")

    def test_material_decision_disables_additive_for_png_in_remaster(self) -> None:
        decision = converter._material_decision_from_inputs(
            texture_uri="wing.png",
            legacy_blend_mode="additive",
            texture_signal_profile_by_uri={
                "wing.png": (False, False, 0.0, 1.0, 0.80, 0.82, 0.20, 0.18, 0.30, 0.92),
            },
            disable_additive_for_png=True,
        )
        self.assertEqual(decision["material_kind"], "textured_pbr")
        self.assertEqual(decision["alpha_mode"], "OPAQUE")
        self.assertEqual(decision["decision_source"], "remaster_png_override")

    def test_material_decision_keeps_additive_for_jpeg_in_remaster(self) -> None:
        decision = converter._material_decision_from_inputs(
            texture_uri="wing.jpg",
            legacy_blend_mode="additive",
            texture_signal_profile_by_uri={
                "wing.jpg": (False, False, 0.0, 1.0, 0.80, 0.82, 0.20, 0.18, 0.30, 0.92),
            },
            disable_additive_for_png=True,
        )
        self.assertEqual(decision["material_kind"], "additive_emissive")
        self.assertEqual(decision["inference_mode"], "additive")

    def test_texture_resolver_remaster_prefers_png_before_jpeg(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            texture_dir = temp_root / "ready"
            texture_dir.mkdir(parents=True, exist_ok=True)
            (texture_dir / "dragon_02.png").write_bytes(b"\x89PNG\r\n\x1a\n")
            (texture_dir / "dragon_02.jpg").write_bytes(b"\xff\xd8\xff\xd9")

            resolver = converter.TextureResolver(
                texture_dir=texture_dir,
                embed_textures=False,
                fallback_root=None,
                legacy_texture_dir=None,
                prefer_remaster_texture_order=True,
            )
            resolved = resolver.resolve("dragon_02.jpg")

            self.assertIsNotNone(resolved)
            assert resolved is not None
            self.assertEqual(resolved.uri.lower(), "dragon_02.png")
            self.assertTrue(resolved.found_on_disk)

    def test_texture_resolver_remaster_falls_back_to_jpeg_when_png_missing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            texture_dir = temp_root / "ready"
            texture_dir.mkdir(parents=True, exist_ok=True)
            (texture_dir / "dragon_02.jpg").write_bytes(b"\xff\xd8\xff\xd9")

            resolver = converter.TextureResolver(
                texture_dir=texture_dir,
                embed_textures=False,
                fallback_root=None,
                legacy_texture_dir=None,
                prefer_remaster_texture_order=True,
            )
            resolved = resolver.resolve("dragon_02.jpg")

            self.assertIsNotNone(resolved)
            assert resolved is not None
            self.assertEqual(resolved.uri.lower(), "dragon_02.jpg")
            self.assertTrue(resolved.found_on_disk)

    def test_model_references_remaster_ready_texture_accepts_jpeg_only(self) -> None:
        model = _build_test_model(texture_index=1)
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            (temp_root / "Glow.jpg").write_bytes(b"\xff\xd8\xff\xd9")
            self.assertTrue(converter._model_references_remaster_ready_texture(model, temp_root))

    def test_texture_resolver_matches_legacy_normalized_stem(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            texture_dir = temp_root / "textures"
            texture_dir.mkdir(parents=True, exist_ok=True)
            legacy_dir = temp_root / "legacy"
            legacy_dir.mkdir(parents=True, exist_ok=True)

            # 24-byte OZJ header + minimal JPEG payload marker bytes.
            (legacy_dir / "dragon_02.ozj").write_bytes(b"\x00" * 24 + b"\xff\xd8\xff\xd9")

            resolver = converter.TextureResolver(
                texture_dir=texture_dir,
                embed_textures=True,
                fallback_root=None,
                legacy_texture_dir=legacy_dir,
            )
            resolved = resolver.resolve("dragon02.jpg")

            self.assertIsNotNone(resolved)
            assert resolved is not None
            self.assertTrue(resolved.found_on_disk)
            self.assertIn("dragon_02", resolved.uri.lower())

    def test_alpha_profile_jpeg_payload_does_not_report_fake_alpha(self) -> None:
        has_alpha, has_partial, transparent_ratio, opaque_ratio = converter._png_alpha_profile(
            b"\xff\xd8\xff\xdb\x00\x00\x00"
        )
        self.assertFalse(has_alpha)
        self.assertFalse(has_partial)
        self.assertEqual(transparent_ratio, 0.0)
        self.assertEqual(opaque_ratio, 1.0)


if __name__ == "__main__":
    unittest.main()
