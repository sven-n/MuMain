"""Prepare painted terrain for repeat sampling and save a packed Blender review scene."""

import json
from pathlib import Path
import sys
import traceback

import bpy
import numpy as np

ROOT = Path(__file__).resolve().parent
TEXTURE_SIZE = 512
SEAM_BAND = 24
GRID_COLUMNS = 5
PATCH_SIZE = 3.0
PATCH_GAP = .25


def condition_axis(pixels, axis):
    """Match the two boundary samples with a smooth, local correction band."""
    image = np.swapaxes(pixels, 0, axis)
    left, right = image[0].copy(), image[-1].copy()
    target = (left + right) * .5
    for index in range(SEAM_BAND):
        t = index / (SEAM_BAND - 1)
        strength = 1 - t * t * (3 - 2 * t)
        image[index] += (target - left) * strength
        image[-1 - index] += (target - right) * strength


def prepare_image(path):
    image = bpy.data.images.load(str(path))
    image.scale(TEXTURE_SIZE, TEXTURE_SIZE)
    pixels = np.empty(TEXTURE_SIZE * TEXTURE_SIZE * 4, dtype=np.float32)
    image.pixels.foreach_get(pixels)
    pixels = pixels.reshape(TEXTURE_SIZE, TEXTURE_SIZE, 4)
    condition_axis(pixels, 0)
    condition_axis(pixels, 1)
    np.clip(pixels, 0, 1, out=pixels)
    pixels[:, :, 3] = 1
    image.pixels.foreach_set(pixels.ravel())
    image.update()
    image.file_format = "PNG"
    image.filepath_raw = str(ROOT / "textures" / (path.stem + ".png"))
    image.save()
    image.pack()
    return image


def review_material(image):
    material = bpy.data.materials.new(image.name)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    nodes.clear()
    texture = nodes.new("ShaderNodeTexImage")
    texture.image = image
    texture.interpolation = "Closest"
    texture.extension = "REPEAT"
    emission = nodes.new("ShaderNodeEmission")
    output = nodes.new("ShaderNodeOutputMaterial")
    material.node_tree.links.new(texture.outputs["Color"], emission.inputs["Color"])
    material.node_tree.links.new(emission.outputs[0], output.inputs[0])
    return material


def add_repeat_patch(index, image):
    x = (index % GRID_COLUMNS) * (PATCH_SIZE + PATCH_GAP)
    y = -(index // GRID_COLUMNS) * (PATCH_SIZE + PATCH_GAP)
    mesh = bpy.data.meshes.new(image.name)
    mesh.from_pydata([(x, y, 0), (x + PATCH_SIZE, y, 0),
                      (x + PATCH_SIZE, y + PATCH_SIZE, 0), (x, y + PATCH_SIZE, 0)], [], [(0, 1, 2, 3)])
    obj = bpy.data.objects.new(image.name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    layer = mesh.uv_layers.new()
    for loop, uv in zip(mesh.polygons[0].loop_indices, [(0, 0), (3, 0), (3, 3), (0, 3)]):
        layer.data[loop].uv = uv
    mesh.materials.append(review_material(image))
    obj["review_only"] = "3×3 repeat; not an engine model"


def review_camera():
    scene = bpy.context.scene
    camera = bpy.data.objects.new("TileReviewCamera", bpy.data.cameras.new("TileReviewCamera"))
    scene.collection.objects.link(camera)
    camera.location = (8, -3.375, 25)
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 17.2
    scene.camera = camera
    scene.render.resolution_x, scene.render.resolution_y = 1600, 1280
    scene.render.resolution_percentage = 100
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type == "VIEW_3D":
                area.spaces.active.region_3d.view_perspective = "CAMERA"
                area.spaces.active.shading.type = "MATERIAL"


def main():
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    (ROOT / "textures").mkdir(exist_ok=True)
    names = []
    for index, path in enumerate(sorted((ROOT / "paintings").glob("Tile*.png"))):
        image = prepare_image(path)
        add_repeat_patch(index, image)
        names.append(path.stem)
    assert len(names) == 17
    review_camera()
    bpy.context.scene["readme"] = "Packed 512px diffuse masters; planes display each material repeated 3×3. Not an in-game screenshot. Raw paintings and prompts are retained separately."
    bpy.context.scene["tile_order"] = names
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT / "source.blend"))
    print(json.dumps(dict(textures=names, size=TEXTURE_SIZE, seam_band=SEAM_BAND)))


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
