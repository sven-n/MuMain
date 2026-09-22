"""Disconnected components and UV role footprints for authored prop selection."""
import json
from pathlib import Path
import sys
sys.dont_write_bytecode=True
import bpy
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from config import PRODUCTION_ASSETS


def components(obj):
    remaining=set(range(len(obj.data.vertices)))
    adjacent={i:set() for i in remaining}
    for edge in obj.data.edges:
        a,b=edge.vertices;adjacent[a].add(b);adjacent[b].add(a)
    result=[]
    while remaining:
        pending=[min(remaining)];indices=set()
        while pending:
            index=pending.pop()
            if index in indices:continue
            indices.add(index);pending.extend(adjacent[index]-indices)
        remaining-=indices
        faces=[p for p in obj.data.polygons if p.vertices[0] in indices]
        points=[obj.matrix_world@obj.data.vertices[v].co for v in indices]
        uvs=[obj.data.uv_layers[0].data[i].uv for p in faces for i in p.loop_indices]
        result.append(dict(vertices=sorted(indices),faces=[p.index for p in faces],triangles=len(faces),
                           bounds=[[min(p[a] for p in points) for a in range(3)],[max(p[a] for p in points) for a in range(3)]],
                           uv_bounds=[[min(p[a] for p in uvs) for a in range(2)],[max(p[a] for p in uvs) for a in range(2)]]))
    return result


for name in PRODUCTION_ASSETS:
    bpy.ops.wm.open_mainfile(filepath=str(ROOT/name/'original/source.blend'))
    obj=next(o for o in bpy.context.scene.objects if o.type=='MESH' and not o.get('mu_helper'))
    result=components(obj)
    (ROOT/name/'original/components.json').write_text(json.dumps(result,indent=2)+'\n')
    print(name,[(p['triangles'],p['bounds'],p['uv_bounds']) for p in result])
