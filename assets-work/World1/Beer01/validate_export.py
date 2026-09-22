"""Validate the final BMD and isolate skeleton/action equivalence from remodel differences."""

from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
CONVERTER = REPOSITORY / "out/build/macos-arm64/tools/bmdconv/Release/bmdconv"
REPORTS = ROOT / "validation"


def run(*arguments, check=True):
    result = subprocess.run([str(CONVERTER), *map(str, arguments)], capture_output=True, text=True)
    if check and result.returncode:
        raise RuntimeError(result.stdout + result.stderr)
    return result


def inspect_model(name, model):
    folder = REPORTS / name
    folder.mkdir(exist_ok=True)
    messages = [run("bmd2smd", model, folder).stdout]
    for path in sorted(folder.glob("Beer01*.smd")):
        options = ["--animation"] if "_a00" in path.name else []
        messages.append(run("validate", path, *options).stdout)
    # Keep node order, bind pose and the actual exported action. Only omit geometry.
    reference = (folder / "Beer01.smd").read_text()
    skeleton = reference.split("triangles\n")[0] + "triangles\nend\n"
    (folder / "skeleton.smd").write_text(skeleton)
    manifest = (folder / "Beer01.actions.txt").read_text()
    action_lines = [line for line in manifest.splitlines() if line.startswith("action ")]
    assert len(action_lines) == 1 and "keys=1 lock=0" in action_lines[0]
    converted = run("smd2bmd", folder / "skeleton.smd", folder / "skeleton.bmd",
                    "--anim", folder / "Beer01_a00.smd")
    messages.append(converted.stdout + converted.stderr)
    return messages


def main():
    original, new = ROOT / "original/Beer01.bmd", ROOT / "exports/Beer01.bmd"
    messages = inspect_model("original", original) + inspect_model("new", new)
    (REPORTS / "smd-validation.txt").write_text("\n".join(messages))
    (REPORTS / "info-after.txt").write_text(run("info", new).stdout)
    remodel = run("compare", original, new, check=False)
    assert "DIFFERENT" in remodel.stdout
    assert "max bone distance: 0.0000" in remodel.stdout and "differing bone names: 0" in remodel.stdout
    (REPORTS / "compare.txt").write_text(remodel.stdout)
    skeleton = run("compare", REPORTS / "original/skeleton.bmd", REPORTS / "new/skeleton.bmd")
    assert "EQUIVALENT" in skeleton.stdout
    (REPORTS / "skeleton-compare.txt").write_text(skeleton.stdout)
    print(remodel.stdout + "\nSkeleton/action-only comparison:\n" + skeleton.stdout)


if __name__ == "__main__":
    main()
