"""Official import/export with lossless legacy model-name metadata transport."""
from pathlib import Path
import os
import subprocess
import sys
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
sys.path.insert(0, str(REPOSITORY / 'assets-work/World1/Architecture03'))
import mu_blender_common as common
from raw_bindings import payload


def read_actions(path):
    result = []
    for line in Path(path).read_bytes().splitlines():
        if not line.startswith(b'action '):
            continue
        words = line.decode('ascii').split()
        record = {'index': int(words[1])}
        record.update(word.split('=', 1) for word in words[2:])
        result.append(record)
    return result


def main():
    args = sys.argv[sys.argv.index('--') + 1:]
    mode, name = args[:2]
    folder = ROOT / name
    original_name = payload(folder / 'baseline' / (name + '.bmd'))[:32].split(b'\0')[0]

    def run_converter(executable, *arguments):
        values = list(arguments)
        if values[0] == 'smd2bmd':
            values[values.index('--name') + 1] = original_name
        command = [v if isinstance(v, bytes) else os.fsencode(v) for v in [executable, *values]]
        result = subprocess.run(command, capture_output=True, check=True)
        output = result.stdout.decode('utf8', 'backslashreplace')
        print(output, end='')
        print(result.stderr.decode('utf8', 'backslashreplace'), end='', file=sys.stderr)
        return output

    common.run_bmdconv = run_converter
    converter = os.environ['MU_BMDCONV']
    if mode == 'import':
        import mu_bmd_import as official
        stage = args[2]
        target = folder / ('baseline/source.blend' if stage == 'baseline' else 'validation/reimported.blend')
        sys.argv = ['blender', '--', '--bmd', str(folder / stage / (name + '.bmd')),
                    '--textures', str(folder / 'textures'), '--out', str(target), '--bmdconv', converter]
        official.read_manifest = read_actions
    else:
        assert mode == 'export'
        import mu_bmd_export as official
        sys.argv = ['blender', '--', '--out', str(folder / 'exports' / (name + '.bmd')), '--bmdconv', converter]
    official.main()


if __name__ == '__main__':
    main()
