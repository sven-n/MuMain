"""Inspect per-bone pottery geometry before choosing the remodeling contract."""
import bpy
import json
from pathlib import Path
P=Path(__file__).resolve().parent
result={}
for name in ('Well01','Well02','Well03','Well04'):
 bpy.ops.wm.open_mainfile(filepath=str(P/name/'original/source.blend'))
 rows=[]
 for obj in bpy.data.objects:
  if obj.type!='MESH' or obj.get('mu_helper'):continue
  for group in obj.vertex_groups:
   vertices=[v for v in obj.data.vertices if any(g.group==group.index and g.weight>.99 for g in v.groups)]
   if not vertices:continue
   rows.append({'bone':group.name,'vertices':len(vertices),'bounds':[[min(v.co[k] for v in vertices) for k in range(3)],[max(v.co[k] for v in vertices) for k in range(3)]],'levels':sorted(set(round(v.co.z,2) for v in vertices)),'materials':[m.get('mu_texture') for m in obj.data.materials]})
 result[name]=rows
(P/'part-inspection.json').write_text(json.dumps(result,indent=2)+'\n')
