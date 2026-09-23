"""Read-only full Data BMD texture field inventory for the exact Object2 atlas."""
from pathlib import Path
import hashlib
import json
import struct
import os
import subprocess
import sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent
REPO=ROOT.parents[2]
sys.path.insert(0,str(REPO/'assets-work/World1/Architecture03'))
from raw_bindings import payload


def names(path):
    data=payload(path)
    count=struct.unpack_from('<h',data,32)[0]
    assert 0<=count<1000
    cursor=38;result=[]
    for index in range(count):
        nv,nn,nu,nt,texture=struct.unpack_from('<5h',data,cursor)
        assert min(nv,nn,nu,nt)>=0
        cursor+=10+nv*16+nn*20+nu*8+nt*64
        name=data[cursor:cursor+32].split(b'\0')[0].decode('ascii')
        assert name and len(data)>=cursor+32
        result.append((index,name));cursor+=32
    return result


def main():
    paths=subprocess.check_output(['rg','--files','--no-ignore','src/bin/Data'],cwd=REPO).decode().splitlines()
    consumers=[];others=[];unsupported=[];models=0
    target=(REPO/'src/bin/Data/Object2/deep_wall03.OZJ').resolve()
    for relative in paths:
        if Path(relative).suffix.lower()!='.bmd':continue
        path=REPO/relative
        try:records=names(path)
        except Exception as error:
            item=dict(path=relative,error=type(error).__name__,header_hex=path.read_bytes()[:4].hex())
            if path.read_bytes().startswith(b'BMD'):
                converter=os.environ['MU_BMDCONV']
                check=subprocess.run([converter,'info',str(path)],capture_output=True)
                assert check.returncode!=0
                item.update(converter_exit=check.returncode,converter_message=(check.stdout+check.stderr).decode('ascii','backslashreplace'))
            else:
                item['classification']='Not a BMD model header'
            unsupported.append(item)
            continue
        models+=1
        for mesh,name in records:
            if Path(name.replace('\\','/')).name.lower()!='deep_wall03.jpg':continue
            resolved=(path.parent/Path(name.replace('\\','/')).with_suffix('.OZJ')).resolve()
            item=dict(path=relative,mesh=mesh,texture=name,resolved=str(resolved.relative_to(REPO)),sha256=hashlib.sha256(path.read_bytes()).hexdigest())
            (consumers if resolved==target else others).append(item)
    (ROOT/'texture-consumers.json').write_text(json.dumps(dict(parsed_models=models,exact_consumers=consumers,
        same_name_other_paths=others,nonmodel_or_unsupported_files=unsupported,
        target_sha256=hashlib.sha256(target.read_bytes()).hexdigest(),
        resolution='Standard per-model texture folder, exact Object2 path; all same-name references reported'),indent=2))
    print(models,consumers,'other-path references',len(others),'unparsed',len(unsupported))


if __name__=='__main__':main()
