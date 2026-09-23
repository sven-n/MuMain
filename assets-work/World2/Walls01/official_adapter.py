"""Metadata-only CP949 name transport around unchanged official Blender tools."""
from pathlib import Path
import json,os,subprocess,sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent
REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'tools/blender'))
sys.path.insert(0,str(REPO/'assets-work/World1/Architecture03'))
from raw_bindings import payload

def action_lines(path):
    result=[]
    for raw in Path(path).read_bytes().splitlines():
        if not raw.startswith(b'action '):continue
        words=raw.decode('ascii').split();record={'index':int(words[1])}
        for word in words[2:]:
            key,_,value=word.partition('=');record[key]=value
        result.append(record)
    return result


def export():
    import mu_bmd_export as official
    import mu_blender_common as common
    name=os.environ['WALL_NAME'];folder=ROOT/name
    field=payload(folder/'baseline'/f'{name}.bmd')[:32]
    raw_name=field.split(b'\0',1)[0]
    assert field==raw_name.ljust(32,b'\0') and len(raw_name)<32
    def run(executable,*arguments):
        values=list(arguments)
        if values[0]=='smd2bmd':values[values.index('--name')+1]=raw_name
        args=[v if isinstance(v,bytes) else os.fsencode(v) for v in [executable,*values]]
        result=subprocess.run(args,capture_output=True,check=True)
        text=result.stdout.decode('utf8','backslashreplace')
        print(text,end='');print(result.stderr.decode('utf8','backslashreplace'),end='',file=sys.stderr)
        return text
    common.run_bmdconv=run
    official.main()
    final=folder/'exports'/f'{name}.bmd'
    assert payload(final)[:32]==field
    (folder/'validation/name-field.json').write_text(json.dumps(dict(status='PASS',raw32_hex=field.hex(),cp949_display=raw_name.decode('cp949'),exact_original_name_bytes=True),indent=2)+'\n')


if os.environ.get('WALL_MODE')=='export':export()
else:
    import mu_bmd_import as official
    official.read_manifest=action_lines
    official.main()
