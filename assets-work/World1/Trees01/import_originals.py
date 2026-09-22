"""Import the four immutable originals with official tools into packed Blender projects."""
from pathlib import Path
import os
import subprocess

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
ASSETS = ('Tree01', 'Tree02', 'Tree11', 'Tree06')


def main():
    for name in ASSETS:
        folder = ROOT / name / 'original'
        command = [BLENDER, '-b', '--python', str(REPOSITORY / 'tools/blender/mu_bmd_import.py'), '--',
                   '--bmd', str(folder / f'{name}.bmd'), '--out', str(folder / 'source.blend'),
                   '--textures', str(ROOT / 'textures/original'), '--bmdconv', CONVERTER]
        result = subprocess.run(command, cwd=REPOSITORY, capture_output=True, text=True)
        (folder / 'import.log').write_text(result.stdout + result.stderr)
        if result.returncode:
            raise RuntimeError(result.stdout + result.stderr)
        print(name, 'imported', flush=True)


if __name__ == '__main__':
    main()
