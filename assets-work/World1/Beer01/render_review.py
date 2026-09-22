"""Offline comparison renders, explicitly not screenshots from the MU client."""

import importlib.util
from pathlib import Path
import sys
import traceback

import bpy

ROOT = Path(__file__).resolve().parent
sys.dont_write_bytecode = True


def load_builder():
    spec = importlib.util.spec_from_file_location("beer_builder", ROOT / "build_source.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def wire_material():
    material = bpy.data.materials.new("REVIEW_Wireframe")
    material.use_nodes = True
    nodes = material.node_tree.nodes
    nodes.clear()
    wire = nodes.new("ShaderNodeWireframe")
    wire.inputs["Size"].default_value = .10
    mix = nodes.new("ShaderNodeMixRGB")
    mix.inputs[1].default_value = (.33, .43, .43, 1)
    mix.inputs[2].default_value = (.005, .01, .01, 1)
    emission = nodes.new("ShaderNodeEmission")
    output = nodes.new("ShaderNodeOutputMaterial")
    links = material.node_tree.links
    links.new(wire.outputs[0], mix.inputs[0])
    links.new(mix.outputs[0], emission.inputs[0])
    links.new(emission.outputs[0], output.inputs[0])
    return material


def render(path):
    bpy.context.scene.render.filepath = str(path)
    bpy.ops.render.render(write_still=True)


def main():
    builder = load_builder()
    folder = ROOT / "review"
    folder.mkdir(exist_ok=True)
    bpy.ops.wm.open_mainfile(filepath=str(ROOT / "original/source.blend"))
    builder.preview_camera()
    render(folder / "before.png")
    bpy.ops.wm.open_mainfile(filepath=str(ROOT / "source.blend"))
    render(folder / "after.png")
    wire = wire_material()
    for obj in bpy.data.collections["EXPORT_Beer01"].objects:
        obj.data.materials.clear()
        obj.data.materials.append(wire)
    render(folder / "wireframe.png")


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
