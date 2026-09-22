"""Export the current Blender scene as a MU Online BMD model.

Run inside Blender on a .blend produced by mu_bmd_import.py (or built by hand following
docs/agents/ASSET_REGENERATION_PLAN.md):

    blender -b work/Monster01.blend --python tools/blender/mu_bmd_export.py -- --out out/Monster01.bmd

Options:
    --name TEXT      value for the model's 31-character name field (default: <out stem>.smd)
    --lock NAMES     comma separated action names whose root motion is locked (in addition to
                     actions tagged with the ``mu_lock_positions`` custom property)
    --no-anims       export the mesh with a single one-frame action (static object)
    --flip           reverse triangle winding
    --bmdconv PATH   bmdconv executable (default: auto-detect in out/build/*)
    --keep-temp      keep the intermediate SMD files and print their folder

Rules the export enforces so the game accepts the result:
    * all mesh objects are joined into one temporary object (one BMD = one model)
    * every vertex keeps only its heaviest vertex group (the engine binds one bone per vertex)
    * every bone is exported in armature order (attachments are addressed by bone index)
    * actions are ordered by their ``mu_index`` property, else by a leading number in the
      action name (``a00_stand``), else alphabetically
Material names must be the texture file names (``body_01.jpg`` / ``wings.tga``).
"""

from __future__ import annotations

import argparse
import os
import re
import shutil
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import bpy  # noqa: E402

import mu_blender_common as common  # noqa: E402

LEADING_NUMBER = re.compile(r"^[A-Za-z_]*?(\d+)")


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", required=True)
    parser.add_argument("--name")
    parser.add_argument("--lock", default="")
    parser.add_argument("--no-anims", action="store_true")
    parser.add_argument("--flip", action="store_true")
    parser.add_argument("--bmdconv")
    parser.add_argument("--keep-temp", action="store_true")
    return parser.parse_args(argv)


BONE_SHAPE_OBJECT = "smd_bone_vis"


def bone_shape_objects() -> set:
    shapes = set()
    for armature in (obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"):
        for bone in armature.pose.bones:
            if bone.custom_shape:
                shapes.add(bone.custom_shape.name)
    shapes.add(BONE_SHAPE_OBJECT)
    return shapes


def mesh_objects() -> list:
    excluded = bone_shape_objects()
    return [
        obj for obj in bpy.context.scene.objects
        if obj.type == "MESH" and len(obj.data.polygons) > 0 and obj.name not in excluded and not obj.get("mu_helper")
    ]


def armature_of(meshes: list):
    for obj in meshes:
        for modifier in obj.modifiers:
            if modifier.type == "ARMATURE" and modifier.object:
                return modifier.object
        if obj.parent and obj.parent.type == "ARMATURE":
            return obj.parent
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    return armatures[0] if armatures else None


def join_meshes(meshes: list, name: str):
    """Duplicate and join every mesh object into one temporary object."""
    bpy.ops.object.mode_set(mode="OBJECT") if bpy.context.object and bpy.context.object.mode != "OBJECT" else None
    bpy.ops.object.select_all(action="DESELECT")
    copies = []
    for obj in meshes:
        copy = obj.copy()
        copy.data = obj.data.copy()
        bpy.context.scene.collection.objects.link(copy)
        copy.select_set(True)
        copies.append(copy)
    bpy.context.view_layer.objects.active = copies[0]
    if len(copies) > 1:
        bpy.ops.object.join()
    joined = bpy.context.view_layer.objects.active
    joined.name = name
    joined.data.name = name
    return joined


def keep_dominant_weight(obj) -> int:
    """Leave exactly one vertex group (weight 1.0) per vertex; returns how many were trimmed."""
    trimmed = 0
    groups = obj.vertex_groups
    for vertex in obj.data.vertices:
        if len(vertex.groups) <= 1:
            continue
        best = max(vertex.groups, key=lambda entry: entry.weight)
        for entry in list(vertex.groups):
            if entry.group != best.group:
                groups[entry.group].remove([vertex.index])
                trimmed += 1
        groups[best.group].add([vertex.index], 1.0, "REPLACE")
    return trimmed


def clip_names(armature) -> list[str]:
    """Animation clips: action slots (Blender 4.4+) or actions (older Blender)."""
    animation = armature.animation_data if armature else None
    if animation and animation.action and getattr(animation.action, "slots", None) and len(animation.action.slots):
        return [slot.name_display for slot in animation.action.slots]
    return [action.name for action in bpy.data.actions if action.users or action.use_fake_user]


def ordered_clips(armature, explicit_locks: set) -> list[tuple]:
    """(sort key, clip name, lock flag) using mu_action_meta, then a leading number, then the name."""
    meta = armature.get("mu_action_meta") if armature else None
    meta = meta.to_dict() if meta is not None and hasattr(meta, "to_dict") else (dict(meta) if meta else {})
    entries = []
    for name in clip_names(armature):
        info = meta.get(name, {})
        if "index" in info:
            key = (0, int(info["index"]), name)
        else:
            match = LEADING_NUMBER.match(name)
            key = (1, int(match.group(1)), name) if match else (2, 0, name)
        lock = bool(info.get("lock", False)) or name in explicit_locks
        entries.append((key, name, lock))
    entries.sort(key=lambda entry: entry[0])
    return entries


def configure_source_tools(vsutils, export_dir: str, armature, export_mesh, export_actions: bool) -> None:
    scene = bpy.context.scene
    scene.vs.export_path = export_dir
    scene.vs.export_format = "SMD"
    scene.vs.smd_format = "GOLDSOURCE"  # one bone per vertex, no weight links
    scene.vs.up_axis = "Z"
    for obj in scene.objects:
        obj.vs.export = obj in (armature, export_mesh)
    export_mesh.vs.subdir = ""
    if armature:
        armature.vs.subdir = ""
        armature.data.vs.implicit_zero_bone = False
        for bone in armature.data.bones:
            bone.use_deform = True
        if not armature.animation_data:
            armature.animation_data_create()
        if export_actions:
            armature.data.vs.action_selection = "FILTERED"  # every slot (4.4+) or every action (older)
            armature.vs.action_filter = "*"
        else:
            armature.vs.export = False
    vsutils.State.update_scene(scene)


def exported_clip_file(export_dir: str, clip_name: str) -> str | None:
    """Source Tools writes animations into the armature's sub folder (``anims`` by default)."""
    wanted = common.sanitise_action_stem(clip_name) + ".smd"
    for root, _dirs, files in os.walk(export_dir):
        if wanted in files:
            return os.path.join(root, wanted)
    return None


def main() -> None:
    args = parse_args(common.script_args())
    bmdconv = common.find_bmdconv(args.bmdconv)
    vsutils = common.ensure_source_tools()

    meshes = mesh_objects()
    if not meshes:
        raise SystemExit("the scene has no mesh objects to export")
    armature = armature_of(meshes)
    export_dir = tempfile.mkdtemp(prefix="mu_export_")
    stem = os.path.splitext(os.path.basename(args.out))[0]

    export_mesh = join_meshes(meshes, f"{stem}_mu_export")
    trimmed = keep_dominant_weight(export_mesh)
    if trimmed:
        print(f"trimmed {trimmed} secondary vertex weights (engine uses one bone per vertex)")

    explicit_locks = {name.strip() for name in args.lock.split(",") if name.strip()}
    clips = [] if args.no_anims or armature is None else ordered_clips(armature, explicit_locks)
    for action in bpy.data.actions:
        action.use_fake_user = True

    # Source Tools rebuilds objects while exporting; keep plain names and lists, not object
    # references, for everything that is needed afterwards.
    export_name = export_mesh.name
    desired_order = list(armature.get("mu_bone_order", [])) if armature is not None else []
    configure_source_tools(vsutils, export_dir, armature, export_mesh, export_actions=bool(clips))
    result = bpy.ops.export_scene.smd(export_scene=True)
    if "FINISHED" not in result:
        raise SystemExit(f"Source Tools export failed: {result}")

    reference = os.path.join(export_dir, common.sanitise_action_stem(export_name) + ".smd")
    if not os.path.isfile(reference):
        raise SystemExit(f"Source Tools did not write the reference file {reference}; files: {os.listdir(export_dir)}")

    clip_files = []
    for _key, name, lock in clips:
        file = exported_clip_file(export_dir, name)
        if file is None:
            raise SystemExit(f"Source Tools did not export animation '{name}'; files: {os.listdir(export_dir)}")
        clip_files.append((file, lock))

    if desired_order:
        moved = sum(common.reorder_smd_bones(path, desired_order) for path in [reference] + [f for f, _ in clip_files])
        if moved:
            print(f"restored the original bone order ({moved} bone index changes across {1 + len(clip_files)} files)")

    arguments = ["smd2bmd", reference, os.path.abspath(args.out)]
    for file, lock in clip_files:
        arguments += ["--anim", file + (":lock" if lock else "")]
    if args.flip:
        arguments.append("--flip")
    arguments += ["--name", args.name or f"{stem}.smd"]
    common.run_bmdconv(bmdconv, *arguments)
    common.run_bmdconv(bmdconv, "info", os.path.abspath(args.out))

    if args.keep_temp:
        print(f"intermediate SMD files kept in {export_dir}")
    else:
        shutil.rmtree(export_dir, ignore_errors=True)


if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except Exception:  # noqa: BLE001 - Blender would otherwise exit 0 after printing the traceback
        import traceback

        traceback.print_exc()
        sys.exit(1)
