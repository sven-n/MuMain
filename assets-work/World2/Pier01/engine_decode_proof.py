"""Read-only engine-equivalent TurboJPEG FASTDCT pixel preservation check."""
from pathlib import Path
import ctypes as c,json,sys
import numpy as np
from PIL import Image
lib=c.CDLL('/opt/homebrew/lib/libturbojpeg.dylib');lib.tjInitDecompress.restype=c.c_void_p
lib.tjDecompress2.argtypes=[c.c_void_p,c.c_void_p,c.c_ulong,c.c_void_p,c.c_int,c.c_int,c.c_int,c.c_int,c.c_int];lib.tjDecompress2.restype=c.c_int;lib.tjDestroy.argtypes=[c.c_void_p]
def decode(path):
 raw=Path(path).read_bytes();src=c.create_string_buffer(raw);dst=c.create_string_buffer(64*128*3);handle=lib.tjInitDecompress();assert handle
 try:assert lib.tjDecompress2(handle,src,len(raw),dst,64,0,128,0,2048)==0
 finally:lib.tjDestroy(handle)
 return np.frombuffer(dst.raw,dtype=np.uint8).reshape(128,64,3).astype(int)
r=Path(sys.argv[1]);old=decode(r/'original.jpg');new=decode(r/'patched.jpg');diff=np.abs(old-new).max(axis=2)
mask=np.asarray(Image.open(Path(__file__).resolve().parent/'artwork/input-references/protected-plus2px-periodic-mask.png'))>0
selected=np.frombuffer((r/'allowed-blocks.bin').read_bytes(),dtype=np.uint8).reshape(16,8).astype(bool);editable=np.repeat(np.repeat(selected,8,axis=0),8,axis=1)
assert diff[mask].max()==0 and diff[~editable].max()==0
report=dict(status='PASS',decoder='libturbojpeg tjDecompress2 TJPF_RGB TJFLAG_FASTDCT(2048), matching GlobalBitmap.cpp',protected_filter_margin_max_rgb_error=int(diff[mask].max()),all_unselected_blocks_max_rgb_error=int(diff[~editable].max()),no_mipmaps='GlobalBitmap.cpp144 texInfo.num_levels=1; no resampling for64x128power-of-two',sampler='LoadData.h defaultGL_NEAREST, GlobalBitmap.cpp211–214 min/mag filter andnearest mip mode;2pxguard also covers fallbackbilinear')
(r/'engine-decode-proof.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
