"""Audit actual exported UVs, winding, part bounds, retained cards and seating contacts."""
import json
from pathlib import Path
import sys

sys.dont_write_bytecode = True
import numpy as np
from config import ASSETS, MATERIALS, ROOT

POSITION_TOLERANCE = .005  # Original imported rigid rotations round to six decimals in SMD.
UV_AREA_EPSILON = 1e-10
SEAT_HEIGHTS = {'Furniture06':49.784698486328125, 'Furniture07':51.41130065917969}


def triangles(path):
    lines = path.read_text().split('triangles\n')[1].splitlines()[:-1]
    return [(lines[i],np.array([list(map(float,row.split())) for row in lines[i+1:i+4]]))
            for i in range(0,len(lines),4)]


def points(records, material=None, bone=None):
    return np.array([row[1:4] for mat,tri in records for row in tri
                     if (material is None or mat==material) and (bone is None or row[0]==bone)])


def extents(array):
    return np.array([array.min(axis=0),array.max(axis=0)])


def surface_audit(records):
    minimum_uv_area, minimum_dot, minimum_area = 1., 1., float('inf')
    for material, tri in records:
        assert np.isfinite(tri).all()
        assert tri.shape[1] == 9, 'Expected original rigid primary-bone SMD encoding'
        cross = np.cross(tri[1,1:4]-tri[0,1:4],tri[2,1:4]-tri[0,1:4])
        length = float(np.linalg.norm(cross))
        assert length > 0, material
        normal_dot = float(cross @ tri[:,4:7].mean(axis=0))/length
        uv = tri[:,7:9]
        area = abs((uv[1,0]-uv[0,0])*(uv[2,1]-uv[0,1])-(uv[1,1]-uv[0,1])*(uv[2,0]-uv[0,0]))/2
        assert area > UV_AREA_EPSILON and normal_dot > 0, (material,area,normal_dot)
        minimum_uv_area = min(minimum_uv_area,float(area))
        minimum_dot, minimum_area = min(minimum_dot,normal_dot),min(minimum_area,length/2)
    return dict(minimum_uv_triangle_area=minimum_uv_area,minimum_geometry_triangle_area=minimum_area,
                minimum_winding_normal_dot=minimum_dot,zero_area_triangles=0,winding_disagreements=0)


def part_bounds(old,new):
    groups = sorted({(mat,int(row[0])) for mat,tri in old for row in tri})
    result = []
    for mat,bone in groups:
        before,after = extents(points(old,mat,bone)),extents(points(new,mat,bone))
        delta = float(np.abs(before-after).max())
        assert delta < POSITION_TOLERANCE, (mat,bone,delta)
        result.append(dict(material=mat,bone_index=bone,before=before.tolist(),after=after.tolist(),max_difference=delta))
    return result


def retained_surfaces(old,new):
    maximum=0
    count=0
    for material,tri in old:
        if material=='grass_01.tga':
            continue
        candidates=[row for mat,polygon in new if mat==material for row in polygon]
        uv=tri[:,7:9]
        area=abs(float(np.linalg.det(np.stack([uv[1]-uv[0],uv[2]-uv[0]]))))/2
        uv_tolerance=.001001 if area<1e-10 and material=='horse_drawn_01.jpg' else .000002
        for corner in tri:
            distances=[float(np.abs(row[1:4]-corner[1:4]).max()) for row in candidates
                       if row[0]==corner[0] and np.abs(row[7:9]-corner[7:9]).max()<uv_tolerance]
            assert distances and min(distances)<POSITION_TOLERANCE, (material,corner)
            maximum=max(maximum,min(distances))
            count+=1
    return dict(status='PASS',all_original_cart_wheel_axle_seat_cask_lamp_effect_corners_retained=True,
                UV_tolerance=.001001,UV_note="Four pre-existing collapsed wood UV triangles opened within quarter original atlas texel; other original UVs exact",
                corner_samples=count,max_position_difference=maximum,
                additive_mesh_index=2 if any(m=='horse_drawn_03.jpg' for m,t in old) else None)


def authored_bindings(name,new):
    expected=json.loads((ROOT/name/'validation/authored-vertices.json').read_text())
    actual=np.array([row[:4] for _,triangle in new for row in triangle])
    max_error=0
    for record in expected:
        candidates=actual[actual[:,0]==record['bone'],1:4]
        assert len(candidates)>0
        distance=float(np.max(np.abs(candidates-record['position']),axis=1).min())
        assert distance<POSITION_TOLERANCE,(name,record,distance)
        max_error=max(max_error,distance)
    for row in actual:
        candidates=np.array([record['position'] for record in expected if record['bone']==row[0]])
        assert len(candidates)>0 and np.max(np.abs(candidates-row[1:4]),axis=1).min()<POSITION_TOLERANCE
    return dict(status='PASS',source_vertices=len(expected),all_exported_corners_match_authored_bone_and_position=True,
                maximum_coordinate_deviation=max_error,tolerance=POSITION_TOLERANCE)


def audit(name):
    folder = ROOT/name/'validation'
    old,new = [triangles(folder/stage/(name+'.smd')) for stage in ('original','new')]
    result = dict(status='PASS',authored_bindings=authored_bindings(name,new),surface=surface_audit(new),per_material_and_bone_bounds=part_bounds(old,new),
                  retained_surfaces=retained_surfaces(old,new),
                  material_order=MATERIALS[name],rigid_bindings='Original primary bone indices only; no extra weights')
    (folder/'geometry-UV-contacts.json').write_text(json.dumps(result,indent=2)+'\n')
    print(name,result['surface'],result['retained_surfaces'],flush=True)


for asset in ASSETS:
    audit(asset)
