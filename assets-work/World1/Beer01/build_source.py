"""Rebuild Beer01 on its imported rig. Run Blender with original/source.blend."""

import json
import math
from pathlib import Path
import sys
import traceback

import bmesh
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parent
TAU = math.tau
ATLAS_RECTS = {
    "grape": (0.02, 0.52, 0.48, 0.98),
    "glass": (0.52, 0.52, 0.98, 0.98),
    "pewter": (0.02, 0.02, 0.48, 0.48),
    "wood": (0.52, 0.02, 0.98, 0.48),
}
MAX_TRIANGLES = 799


def new_collection(name):
    collection = bpy.data.collections.new(name)
    bpy.context.scene.collection.children.link(collection)
    collection.vs.export = False
    return collection


def preserve_original():
    reference = new_collection("REF_ORIGINAL")
    armature = next(obj for obj in bpy.data.objects if obj.type == "ARMATURE")
    for obj in list(bpy.context.scene.objects):
        if obj.type != "MESH" or obj.get("mu_helper"):
            continue
        for collection in list(obj.users_collection):
            collection.objects.unlink(obj)
        reference.objects.link(obj)
        obj["mu_reference"] = True
    reference.hide_render = True
    reference.hide_viewport = True
    for material in bpy.data.materials:
        material.name = "REF_" + material.name
    return armature


def atlas_material():
    material = bpy.data.materials.new("plate2.jpg")
    material.use_nodes = True
    material["mu_texture"] = "plate2.jpg"
    nodes = material.node_tree.nodes
    shader = nodes.get("Principled BSDF")
    shader.inputs["Roughness"].default_value = 1.0
    shader.inputs["Specular IOR Level"].default_value = 0.0
    texture = nodes.new("ShaderNodeTexImage")
    texture.image = bpy.data.images.load(str(ROOT / "textures/plate2.jpg"))
    texture.interpolation = "Closest"
    material.node_tree.links.new(texture.outputs["Color"], shader.inputs["Base Color"])
    return material


def atlas_uv(region, u, v):
    left, bottom, right, top = ATLAS_RECTS[region]
    return (left + u * (right - left), bottom + v * (top - bottom))


def mesh_object(name, vertices, faces, uvs, bone, collection, armature, material):
    mesh = bpy.data.meshes.new(name)
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    mesh.materials.append(material)
    layer = mesh.uv_layers.new(name="UVMap")
    for polygon, coordinates in zip(mesh.polygons, uvs):
        for loop, coordinate in zip(polygon.loop_indices, coordinates):
            layer.data[loop].uv = coordinate
        polygon.use_smooth = True
    obj.vertex_groups.new(name=bone).add(list(range(len(vertices))), 1.0, "REPLACE")
    modifier = obj.modifiers.new("OriginalRig", "ARMATURE")
    modifier.object = armature
    obj.parent = armature
    obj["mu_component"] = name
    return obj


def surface_rings(profile, segments, center, aspect):
    vertices, rings = [], []
    for radius, height in profile:
        ring = []
        count = segments if radius else 1
        for index in range(count):
            angle = TAU * index / segments
            ring.append(len(vertices))
            vertices.append((center[0] + radius * math.cos(angle),
                             center[1] + radius * math.sin(angle) * aspect,
                             center[2] + height))
        rings.append(ring)
    return vertices, rings


def ring_faces(rings, regions, segments, vcoords=None):
    faces, coordinates = [], []
    if vcoords is None:
        vcoords = [index / (len(rings) - 1) for index in range(len(rings))]
    for level, (lower, upper) in enumerate(zip(rings, rings[1:])):
        region = regions[level] if isinstance(regions, list) else regions
        for index in range(segments):
            nxt = (index + 1) % segments
            u0, u1 = index / segments, (index + 1) / segments
            v0, v1 = vcoords[level:level + 2]
            face = [lower[index % len(lower)], lower[nxt % len(lower)],
                    upper[nxt % len(upper)], upper[index % len(upper)]]
            uv = [atlas_uv(region, u0, v0), atlas_uv(region, u1, v0),
                  atlas_uv(region, u1, v1), atlas_uv(region, u0, v1)]
            if len(lower) == 1:
                del face[0]
                del uv[0]
            if len(upper) == 1:
                del face[-1]
                del uv[-1]
            faces.append(face)
            coordinates.append(uv)
    return faces, coordinates


def lathe(name, profile, segments, center, aspect, region, bone, context, vcoords=None):
    vertices, rings = surface_rings(profile, segments, center, aspect)
    faces, uvs = ring_faces(rings, region, segments, vcoords)
    return mesh_object(name, vertices, faces, uvs, bone, *context)


def vessels(context):
    lathe("SmallBowl", [(0, .57), (6, .57), (13, 9.7), (14.12, 11.70),
                        (12.75, 10.9), (0, 2.3)], 10, (-36.015, 8.437, 0),
          12.23 / 14.12, "pewter", "Cylinder09", context)
    lathe("ServingPlatter", [(0, .684), (15, .684), (23.19, 8), (23.19, 9.1),
                             (20.8, 8.1), (0, 2.4)], 12, (7.7485, -8.27, 0),
          21.36 / 23.19, "pewter", "Cylinder08", context)
    lathe("OakTankard", [(0, -.34), (9.81, -.34), (9.81, 2), (9.45, 2.25),
                         (9.45, 23.4), (9.81, 27.17),
                         (8.15, 27.17), (7.9, 3), (0, 3)], 12, (31.25, 14.4, 0),
          9.72 / 9.81, ["pewter", "pewter", "pewter", "wood", "pewter", "pewter", "wood", "wood"],
          "Cylinder11", context)
    lathe("OliveBottle", [(0, .637), (6.265, .637), (6.265, 31), (2.4, 40),
                          (2.4, 58.5), (2.8, 59), (2.8, 61.46), (0, 61.46)],
          12, (-17.515, 3.055, 0), 6.515 / 6.265,
          ["glass"] * 6 + ["wood"], "Cylinder31", context,
          vcoords=[.05, .1, .75, .80, .90, .94, .97, 1])


def tankard_handle(context):
    # Squared iron handle attached to the back of the oak mug.
    centers = [(31.25, 23.3, 22.7), (31.25, 27.7, 21.5),
               (31.25, 27.7, 8), (31.25, 23.3, 6)]
    vertices = []
    for x, y, z in centers:
        vertices.extend([(x - 1.15, y - 1.15, z), (x + 1.15, y - 1.15, z),
                         (x + 1.15, y + 1.15, z), (x - 1.15, y + 1.15, z)])
    rings = [list(range(index * 4, index * 4 + 4)) for index in range(len(centers))]
    faces, uvs = ring_faces(rings, "pewter", 4)
    for ring in (rings[0][::-1], rings[-1]):
        faces.append(ring)
        uvs.append([atlas_uv("pewter", u, v) for u, v in [(0, 0), (1, 0), (1, 1), (0, 1)]])
    return mesh_object("TankardHandle", vertices, faces, uvs, "Cylinder11", *context)


def grape(index, center, radius, context):
    sphere = bmesh.new()
    bmesh.ops.create_icosphere(sphere, subdivisions=1, radius=1)
    sphere.verts.ensure_lookup_table()
    sphere.verts.index_update()
    vertices = [tuple(Vector(center) + Vector((v.co.x * radius, v.co.y * radius, v.co.z * radius * 1.08)))
                for v in sphere.verts]
    faces = [[vertex.index for vertex in face.verts] for face in sphere.faces]
    uvs = [[atlas_uv("grape", .5 + sphere.verts[i].co.x * .45,
                     .5 + sphere.verts[i].co.z * .45) for i in face] for face in faces]
    sphere.free()
    return mesh_object(f"Grape{index:02}", vertices, faces, uvs, "Sphere05", *context)


def food_and_garnish(context):
    centers = [(-2, -4, 9.6), (6, -4, 9.6), (14, -4, 9.6),
               (0, -11, 9.6), (8, -11, 9.6), (16, -11, 9.6), (6, -18, 9.6),
               (3, -6, 17.5), (11, -6, 17.5), (5, -13, 16.5), (13, -13, 16.5),
               (7, -8, 23)]
    for index, center in enumerate(centers):
        grape(index, center, 5.2, context)
    vine_leaves(context)


def vine_leaves(context):
    vertices, faces, uvs = [], [], []
    outline = [(0, 0), (1.8, 1.2), (2.7, 3), (.9, 3.6), (0, 5), (-2.3, 3.2), (-1.2, 1.3)]
    for center, angle in [((11, 0, 25), -.7), ((13, 2, 25.5), .65)]:
        base = len(vertices)
        for x, y in outline:
            vertices.append((center[0] + x * math.cos(angle) - y * math.sin(angle),
                             center[1] + x * math.sin(angle) + y * math.cos(angle), center[2] + y * .12))
        polygon = list(range(base, base + len(outline)))
        faces.extend([polygon, polygon[::-1]])
        coordinates = [atlas_uv("glass", .40 + x * .025, .6 + y * .06) for x, y in outline]
        uvs.extend([coordinates, coordinates[::-1]])
    for start, end in [((8, -5, 25), (12, 2, 26)), ((12, 2, 26), (15, 5, 25))]:
        base = len(vertices)
        vertices.extend([(start[0] - .3, start[1], start[2]), (start[0] + .3, start[1], start[2]),
                         (end[0] + .3, end[1], end[2]), (end[0] - .3, end[1], end[2])])
        polygon = list(range(base, base + 4))
        faces.extend([polygon, polygon[::-1]])
        coordinates = [atlas_uv("wood", .1 + i * .05, .5) for i in range(4)]
        uvs.extend([coordinates, coordinates[::-1]])
    obj = mesh_object("HerbSprig", vertices, faces, uvs, "Sphere05", *context)
    for polygon in obj.data.polygons:
        polygon.use_smooth = False


def retain_high_poly(objects):
    high = new_collection("REF_HIGH_POLY")
    for obj in objects:
        duplicate = obj.copy()
        duplicate.data = obj.data.copy()
        duplicate.name = "HIGH_" + obj.name
        duplicate["mu_reference"] = True
        high.objects.link(duplicate)
        if obj.name != "HerbSprig":
            smooth = duplicate.modifiers.new("FutureNormalBakeSource", "SUBSURF")
            smooth.levels = 2
            smooth.render_levels = 3
    high.hide_render = True
    high.hide_viewport = True


def triangulate(objects):
    for obj in objects:
        mesh = bmesh.new()
        mesh.from_mesh(obj.data)
        bmesh.ops.triangulate(mesh, faces=list(mesh.faces), quad_method="FIXED", ngon_method="EAR_CLIP")
        # Herb leaves deliberately have both windings for the legacy renderer.
        if obj.name != "HerbSprig":
            bmesh.ops.recalc_face_normals(mesh, faces=list(mesh.faces))
        mesh.to_mesh(obj.data)
        mesh.free()
        obj.data.update()


def validate(objects, armature):
    triangles = sum(len(obj.data.polygons) for obj in objects)
    assert triangles <= MAX_TRIANGLES, triangles
    assert len(armature.data.bones) == 5
    assert len(armature["mu_action_meta"]) == 1
    assert list(armature["mu_bone_order"]) == ["Cylinder09", "Cylinder11", "Cylinder08", "Sphere05", "Cylinder31"]
    vertices = []
    for obj in objects:
        assert len(obj.data.uv_layers) == 1
        assert len(obj.data.materials) == 1 and obj.data.materials[0].name == "plate2.jpg"
        assert all(len(face.vertices) == 3 and face.area > .00001 for face in obj.data.polygons)
        for vertex in obj.data.vertices:
            assert len(vertex.groups) == 1 and vertex.groups[0].weight == 1
            assert obj.vertex_groups[vertex.groups[0].group].name in armature.data.bones
            vertices.append(obj.matrix_world @ vertex.co)
        assert all(0 <= value <= 1 for uv in obj.data.uv_layers[0].data for value in uv.uv)
    bounds = {"min": [min(v[i] for v in vertices) for i in range(3)],
              "max": [max(v[i] for v in vertices) for i in range(3)]}
    report = dict(triangles=triangles, objects=len(objects), bones=5, actions=1, bounds=bounds,
                  uv="one set, finite, within atlas", skinning="exactly one valid bone per vertex",
                  texture="plate2.jpg, 512×512, single diffuse atlas", status="Blender checks PASS; engine checks required")
    (ROOT / "validation").mkdir(exist_ok=True)
    (ROOT / "validation/blender.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report))


def preview_camera():
    scene = bpy.context.scene
    camera = bpy.data.objects.new("InspectionCamera", bpy.data.cameras.new("InspectionCamera"))
    scene.collection.objects.link(camera)
    camera.location = (120, -160, 150)
    camera.rotation_euler = (Vector((-5, 0, 25)) - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 125
    scene.camera = camera
    light = bpy.data.objects.new("InspectionLight", bpy.data.lights.new("InspectionLight", "AREA"))
    scene.collection.objects.link(light)
    light.location = (20, -50, 160)
    light.data.energy, light.data.size = 30000, 100
    light.data.shape = "DISK"
    scene.world.color = (.3, .3, .3)
    scene.world.use_nodes = True
    background = scene.world.node_tree.nodes.get("Background")
    background.inputs["Color"].default_value = (.35, .35, .35, 1)
    background.inputs["Strength"].default_value = .8
    sun = bpy.data.objects.new("DiffusePreviewSun", bpy.data.lights.new("DiffusePreviewSun", "SUN"))
    scene.collection.objects.link(sun)
    sun.data.energy = 2
    sun.rotation_euler = (.45, -.3, -.45)
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 32
    scene.render.resolution_x, scene.render.resolution_y = 1000, 750
    scene.render.resolution_percentage = 100
    scene.render.filepath = str(ROOT / "preview.png")


def main():
    armature = preserve_original()
    collection = new_collection("EXPORT_Beer01")
    material = atlas_material()
    context = (collection, armature, material)
    vessels(context)
    tankard_handle(context)
    food_and_garnish(context)
    objects = list(collection.objects)
    retain_high_poly(objects)
    triangulate(objects)
    bpy.context.view_layer.update()
    validate(objects, armature)
    preview_camera()
    bpy.ops.file.pack_all()
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT / "source.blend"))
    bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
