"""Record actual component topology and modular connection anchors before modeling."""
import json
from pathlib import Path
import sys
import bpy
sys.dont_write_bytecode=True
sys.path.insert(0,str(Path(__file__).resolve().parent))
from config import PROPS,ROOT


def components(obj):
    vertices=obj.data.vertices
    remaining=set(range(len(vertices)))
    neighbors={index:set() for index in remaining}
    for edge in obj.data.edges:
        a,b=edge.vertices
        neighbors[a].add(b)
        neighbors[b].add(a)
    records=[]
    while remaining:
        visited=set()
        todo=[next(iter(remaining))]
        while todo:
            index=todo.pop()
            if index in visited:continue
            visited.add(index)
            todo.extend(neighbors[index]-visited)
        remaining-=visited
        faces=[face for face in obj.data.polygons if face.vertices[0] in visited]
        points=[obj.matrix_world@vertices[index].co for index in visited]
        records.append({'vertices':len(visited),'faces':len(faces),
                        'materials':sorted(set(obj.data.materials[face.material_index].name for face in faces)),
                        'bounds':[[fn(point[a] for point in points) for a in range(3)] for fn in (min,max)],
                        'bones':sorted(set(obj.vertex_groups[group.group].name for index in visited for group in vertices[index].groups))})
    return records


for name in PROPS:
    bpy.ops.wm.open_mainfile(filepath=str(ROOT/name/'original/source.blend'))
    records=[{'name':obj.name,'components':components(obj)} for obj in bpy.context.scene.objects if obj.type=='MESH' and not obj.get('mu_helper')]
    (ROOT/name/'original/geometry.json').write_text(json.dumps(records,indent=2)+'\n')
    print(name,json.dumps(records))
