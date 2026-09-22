"""Import a MU Online BMD model into Blender as a .blend file.

Run inside Blender:

    blender -b --python tools/blender/mu_bmd_import.py -- --bmd Data/Monster/Monster01.bmd --out work/Monster01.blend

Options:
    --textures DIR   folder with unwrapped .jpg/.tga textures (default: unwrap the .OZJ/.OZT
                     files that sit next to the model)
    --data DIR       game Data folder to search for textures when they are not next to the model
    --no-anims       import the reference mesh and skeleton only
    --bmdconv PATH   bmdconv executable (default: auto-detect in out/build/*)

What it does: bmdconv turns the .bmd into Valve SMD files, Blender Source Tools imports them
(mesh + armature + one action per animation), textures are unwrapped and wired into the
materials, the images are packed, and the result is saved. Each action carries the custom
properties ``mu_index`` and ``mu_lock_positions`` so mu_bmd_export.py can restore the game's
action order and lock flags.
"""

from __future__ import annotations

import argparse
import os
import sys
import tempfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import bpy  # noqa: E402

import mu_blender_common as common  # noqa: E402
import mu_texture  # noqa: E402

TEXTURE_CONTAINERS = {".jpg": ".ozj", ".tga": ".ozt"}


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--bmd", required=True)
    parser.add_argument("--out", required=True)
    parser.add_argument("--textures")
    parser.add_argument("--data")
    parser.add_argument("--no-anims", action="store_true")
    parser.add_argument("--bmdconv")
    return parser.parse_args(argv)


def read_manifest(path: str) -> list[dict]:
    actions = []
    with open(path, encoding="utf-8") as manifest:
        for line in manifest:
            parts = line.split()
            if not parts or parts[0] != "action":
                continue
            entry = {"index": int(parts[1])}
            for token in parts[2:]:
                key, _, value = token.partition("=")
                entry[key] = value
            actions.append(entry)
    return actions


def find_file_case_insensitive(directory: str, wanted: str) -> str | None:
    if not directory or not os.path.isdir(directory):
        return None
    lowered = wanted.lower()
    for root, _dirs, files in os.walk(directory):
        for name in files:
            if name.lower() == lowered:
                return os.path.join(root, name)
    return None


def resolve_texture(material_name: str, search_dirs: list[str], data_dirs: list[str], unwrap_dir: str) -> str | None:
    """Find an editable image for a texture name such as ``body_01.jpg``."""
    stem, extension = os.path.splitext(material_name)
    extension = extension.lower()
    candidates = [stem + extension] + [stem + other for other in TEXTURE_CONTAINERS if other != extension]
    for directory in search_dirs:
        for candidate in candidates:
            found = find_file_case_insensitive(directory, candidate)
            if found:
                return found
    for directory in data_dirs:
        for candidate in candidates:
            container = stem + TEXTURE_CONTAINERS[os.path.splitext(candidate)[1]]
            found = find_file_case_insensitive(directory, container)
            if found:
                target = os.path.join(unwrap_dir, candidate)
                mu_texture.unwrap_file(__import__("pathlib").Path(found), __import__("pathlib").Path(target))
                return target
    return None


def wire_texture(material, image_path: str) -> None:
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = next((node for node in nodes if node.type == "BSDF_PRINCIPLED"), None)
    if principled is None:
        principled = nodes.new("ShaderNodeBsdfPrincipled")
    image_node = nodes.new("ShaderNodeTexImage")
    image_node.image = bpy.data.images.load(image_path)
    image_node.location = (principled.location.x - 400, principled.location.y)
    links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
    if image_path.lower().endswith(".tga"):
        links.new(image_node.outputs["Alpha"], principled.inputs["Alpha"])
    material["mu_texture"] = os.path.basename(image_path)


def clear_scene() -> None:
    """Empty the startup scene without reloading preferences (that would unload add-ons)."""
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    for collection_name in ("meshes", "armatures", "materials", "images", "actions", "cameras", "lights"):
        collection = getattr(bpy.data, collection_name)
        for block in list(collection):
            collection.remove(block)


def import_smd(filepath: str, append_mode: str) -> None:
    result = bpy.ops.import_scene.smd(
        filepath=filepath, doAnim=True, append=append_mode, upAxis="Z", rotMode="XYZ", boneMode="SPHERE",
        createCollections=False
    )
    if "FINISHED" not in result:
        raise SystemExit(f"Source Tools could not import {filepath}: {result}")


def clip_registry(armature) -> dict:
    """Name -> animation clip. Blender 4.4+ stores one slot per imported animation on a single
    action; older versions store one action per animation."""
    animation = armature.animation_data if armature else None
    if animation and animation.action and getattr(animation.action, "slots", None) and len(animation.action.slots):
        return {slot.name_display: slot for slot in animation.action.slots}
    return {action.name: action for action in bpy.data.actions}


def tag_actions(actions: list[dict], armature) -> int:
    """Record the game's action order and lock flags on the armature (``mu_action_meta``)."""
    registry = clip_registry(armature)
    meta = {}
    for entry in actions:
        stem = os.path.splitext(os.path.basename(entry["file"]))[0]
        if stem not in registry:
            print(f"warning: no animation clip named {stem} after import")
            continue
        meta[stem] = {"index": int(entry["index"]), "lock": entry.get("lock", "0") == "1"}
    if armature is not None:
        armature["mu_action_meta"] = meta
    for action in bpy.data.actions:
        action.use_fake_user = True
    return len(meta)


def scene_armature():
    armatures = [obj for obj in bpy.data.objects if obj.type == "ARMATURE"]
    return armatures[0] if armatures else None


def main() -> None:
    args = parse_args(common.script_args())
    bmdconv = common.find_bmdconv(args.bmdconv)
    common.ensure_source_tools()
    bmd_path = os.path.abspath(args.bmd)
    stem = os.path.splitext(os.path.basename(bmd_path))[0]
    work = tempfile.mkdtemp(prefix="mu_import_")
    unwrap_dir = os.path.join(work, "textures")
    os.makedirs(unwrap_dir, exist_ok=True)

    common.run_bmdconv(bmdconv, "bmd2smd", bmd_path, work)
    actions = read_manifest(os.path.join(work, stem + ".actions.txt"))

    clear_scene()
    reference_smd = os.path.join(work, stem + ".smd")
    import_smd(reference_smd, "NEW_ARMATURE")
    armature = scene_armature()
    if armature is not None:
        # Blender re-orders bones depth-first; the exporter restores this original order.
        armature["mu_bone_order"] = common.read_smd_node_names(reference_smd)
    if not args.no_anims:
        for entry in actions:
            import_smd(os.path.join(work, entry["file"]), "APPEND")
        tagged = tag_actions(actions, scene_armature())
        print(f"imported {tagged} of {len(actions)} actions")
    helper = bpy.data.objects.get("smd_bone_vis")
    if helper is not None:
        helper["mu_helper"] = True  # bone display shape from Source Tools, never part of the model

    search_dirs = [args.textures] if args.textures else []
    data_dirs = [os.path.dirname(bmd_path)] + ([args.data] if args.data else [])
    missing = []
    for material in bpy.data.materials:
        image = resolve_texture(material.name, search_dirs, data_dirs, unwrap_dir)
        if image:
            wire_texture(material, image)
        else:
            missing.append(material.name)
    if missing:
        print("warning: no texture found for materials: " + ", ".join(missing))

    for obj in bpy.data.objects:
        obj["mu_source_bmd"] = bmd_path
    bpy.ops.file.pack_all()
    os.makedirs(os.path.dirname(os.path.abspath(args.out)) or ".", exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.abspath(args.out))
    meshes = [obj for obj in bpy.data.objects if obj.type == "MESH" and not obj.get("mu_helper")]
    armature = scene_armature()
    bones = len(armature.data.bones) if armature else 0
    clips = len(clip_registry(armature)) if armature else 0
    print(f"saved {args.out}: {len(meshes)} mesh object(s), {bones} bones, {clips} animation clips, "
          f"{len(bpy.data.materials)} materials ({len(missing)} without texture)")


if __name__ == "__main__":
    try:
        main()
    except SystemExit:
        raise
    except Exception:  # noqa: BLE001 - Blender would otherwise exit 0 after printing the traceback
        import traceback

        traceback.print_exc()
        sys.exit(1)
