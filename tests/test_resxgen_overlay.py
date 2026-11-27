#!/usr/bin/env python3
"""ResxGen --overlay: entries of the overlay replace or add to the input ones.

    python tests/test_resxgen_overlay.py <repository root> <dotnet>
"""
import pathlib
import subprocess
import sys
import tempfile

RESX = '<?xml version="1.0" encoding="utf-8"?>\n<root>\n{}</root>\n'
ENTRY = '  <data name="{}"><value>{}</value></data>\n'


def write(folder, name, entries):
    folder.mkdir(parents=True, exist_ok=True)
    body = ''.join(ENTRY.format(key, value) for key, value in entries.items())
    (folder / name).write_text(RESX.format(body), encoding='utf-8')


def run(project, dotnet, source, out, overlay):
    command = [dotnet, 'run', '--project', str(project), '-c', 'Release', '--',
               '--input', str(source), '--output', str(out), '--overlay', str(overlay)]
    return subprocess.run(command, capture_output=True, text=True, encoding='utf-8', errors='replace')


def expect(condition, message):
    if not condition:
        raise SystemExit(f'FAIL: {message}')


def main():
    root, dotnet = pathlib.Path(sys.argv[1]), sys.argv[2]
    project = root / 'tools' / 'ResxGen' / 'ResxGen.csproj'
    with tempfile.TemporaryDirectory() as temp:
        temp = pathlib.Path(temp)
        source, overlay, out = temp / 'in', temp / 'overlay', temp / 'out'
        write(source, 'Probe.en.resx', {'Kept': 'Kept', 'Replaced': 'Replaced'})
        write(source, 'Probe.pt.resx', {'Kept': 'kept_original', 'Replaced': 'replaced_original'})
        write(overlay, 'Probe.pt.resx', {'Replaced': 'replaced_by_overlay', 'Unknown': 'unknown_key'})
        write(overlay, 'Probe.es.resx', {'Kept': 'added_locale'})

        result = run(project, dotnet, source, out, overlay)
        expect(result.returncode == 0, result.stderr or result.stdout)
        generated = (out / 'Probe.cpp').read_text(encoding='utf-8-sig')
        expect('kept_original' in generated, 'an entry the overlay does not name is kept')
        expect('replaced_by_overlay' in generated, 'an overlay entry is used')
        expect('replaced_original' not in generated, 'the overlaid entry is gone')
        expect('added_locale' in generated, 'a locale only the overlay has is added')
        expect('unknown_key' not in generated, 'a key the default locale lacks is skipped')
        expect("'Unknown'" in result.stderr, 'that key is warned about')

        write(overlay, 'Other.pt.resx', {'Kept': 'x'})
        result = run(project, dotnet, source, out, overlay)
        expect(result.returncode != 0 and "'Other'" in result.stderr, 'a group of the overlay alone is an error')
    print('ResxGen overlay: ok')


if __name__ == '__main__':
    main()
