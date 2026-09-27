import pathlib
import subprocess
import sys

expected = int(sys.argv[1])
command = sys.argv[2:]
result = subprocess.run(command, check=False)
if result.returncode != expected:
    print(f"{command[-1]}: expected exit {expected}, got {result.returncode}", file=sys.stderr)
    sys.exit(1)

mode = command[-1]
if mode.startswith("dispatch-"):
    trace = pathlib.Path.cwd() / f"texture_failure_{mode}.txt"
    expected_trace = "dialog\n" if mode == "dispatch-continue" else "dialog\nconnection\nsound\nwindow\n"
    if not trace.exists() or trace.read_text() != expected_trace:
        print(f"{mode}: cleanup trace mismatch", file=sys.stderr)
        sys.exit(1)
    trace.unlink()
