"""Check that the exit-status wrapper preserves an emulator command and arguments."""
import pathlib
import subprocess
import sys
import tempfile

wrapper = pathlib.Path(__file__).with_name("test_texture_failure_subprocess.py")
with tempfile.TemporaryDirectory(prefix="texture emulator ") as directory:
    emulator = pathlib.Path(directory) / "fake emulator.py"
    emulator.write_text(
        "import subprocess, sys\n"
        "assert sys.argv[1] == '--emulator-option'\n"
        "assert sys.argv[2] == 'argument with spaces'\n"
        "sys.exit(subprocess.run(sys.argv[3:], check=False).returncode)\n"
    )
    for expected, actual, wrapper_exit in ((0, 0, 0), (1, 1, 0), (1, 0, 1)):
        command = [
            sys.executable, str(wrapper), str(expected), sys.executable,
            str(emulator), "--emulator-option", "argument with spaces",
            sys.executable, "-c", f"raise SystemExit({actual})",
        ]
        result = subprocess.run(command, capture_output=True, text=True, check=False)
        if result.returncode != wrapper_exit:
            raise AssertionError(f"wrapper exit {result.returncode}: {result.stderr}")
