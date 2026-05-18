#!/usr/bin/env python3
"""
Recursively rename all files and directories to snake_case.

Usage:
    python normalize.py <directory>              # dry-run (preview changes)
    python normalize.py <directory> --apply      # apply renames

Conversion rules:
    - CamelCase      -> camel_case
    - letter + digit -> letter_digit  (Object40 -> object_40)
    - UPPER ext      -> lower ext     (.OZJ -> .ozj)
    - spaces/hyphens -> underscores
    - special chars  -> removed (! ( ))
    - multiple ___   -> single _
"""
import argparse
import os
import re
import sys


def to_snake_case(name: str) -> str:
    # Remove special characters
    name = name.replace('!', '')
    name = name.replace('(', '_')
    name = name.replace(')', '_')

    # Replace spaces and hyphens with underscores
    name = name.replace(' ', '_')
    name = name.replace('-', '_')

    # Insert _ before uppercase after lowercase:  partCharge -> part_Charge
    name = re.sub(r'([a-z])([A-Z])', r'\1_\2', name)

    # Insert _ before uppercase+lowercase after uppercase run:  IGSStorage -> IGS_Storage
    name = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)

    # Insert _ between letter and digit:  Object40 -> Object_40
    name = re.sub(r'([a-zA-Z])(\d)', r'\1_\2', name)

    # Lowercase
    name = name.lower()

    # Collapse multiple underscores and strip edges
    name = re.sub(r'_+', '_', name)
    name = name.strip('_')

    return name


def convert_name(name: str, is_dir: bool = False) -> str:
    if is_dir:
        return to_snake_case(name)

    last_dot = name.rfind('.')
    if last_dot > 0:
        stem = name[:last_dot]
        ext = name[last_dot:]
        return to_snake_case(stem) + ext.lower()
    return to_snake_case(name)


def add_suffix(filepath: str, suffix: int) -> str:
    dirpart = os.path.dirname(filepath)
    basename = os.path.basename(filepath)
    last_dot = basename.rfind('.')
    if last_dot > 0:
        stem = basename[:last_dot]
        ext = basename[last_dot:]
        return os.path.join(dirpart, f"{stem}_{suffix}{ext}")
    return os.path.join(dirpart, f"{basename}_{suffix}")


def collect_renames(base_dir: str) -> list[tuple[str, str]]:
    renames = []

    for dirpath, _, filenames in os.walk(base_dir, topdown=False):
        for filename in filenames:
            new_name = convert_name(filename, is_dir=False)
            if new_name != filename:
                old_path = os.path.join(dirpath, filename)
                new_path = os.path.join(dirpath, new_name)
                renames.append((old_path, new_path))

        if dirpath != base_dir:
            dirname = os.path.basename(dirpath)
            new_dirname = convert_name(dirname, is_dir=True)
            if new_dirname != dirname:
                parent = os.path.dirname(dirpath)
                renames.append((dirpath, os.path.join(parent, new_dirname)))

    return renames


def resolve_conflicts(renames: list[tuple[str, str]]) -> list[tuple[str, str]]:
    targets: dict[str, str] = {}
    resolved = []

    for old, new in renames:
        if new in targets:
            n = 2
            candidate = add_suffix(new, n)
            while candidate in targets:
                n += 1
                candidate = add_suffix(new, n)
            resolved.append((old, candidate))
            targets[candidate] = old
        else:
            resolved.append((old, new))
            targets[new] = old

    return resolved


def main():
    parser = argparse.ArgumentParser(
        description="Recursively rename files and directories to snake_case."
    )
    parser.add_argument("directory", help="Target directory to normalize")
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Apply renames (default is dry-run preview)",
    )
    args = parser.parse_args()

    base_dir = os.path.abspath(args.directory)
    if not os.path.isdir(base_dir):
        print(f"Error: '{base_dir}' is not a directory.", file=sys.stderr)
        sys.exit(1)

    renames = collect_renames(base_dir)

    if not renames:
        print("No renames needed.")
        return

    renames = resolve_conflicts(renames)

    # Verify uniqueness
    seen: dict[str, str] = {}
    for old, new in renames:
        assert new not in seen, f"Unresolved conflict: {old} and {seen[new]} -> {new}"
        seen[new] = old

    print(f"Total renames: {len(renames)}")

    # Report conflict resolutions
    conflict_items = [
        (o, n) for o, n in renames
        if '_2.' in os.path.basename(n) or os.path.basename(n).endswith('_2')
    ]
    if conflict_items:
        print(f"\nConflict resolutions ({len(conflict_items)} files get _2 suffix):")
        for old, new in conflict_items:
            print(f"  {os.path.relpath(old, base_dir)} -> {os.path.relpath(new, base_dir)}")

    if not args.apply:
        print(f"\n[DRY RUN] Sample of renames:")
        for old, new in renames[:60]:
            print(f"  {os.path.relpath(old, base_dir)} -> {os.path.relpath(new, base_dir)}")
        if len(renames) > 60:
            print(f"  ... and {len(renames) - 60} more")
        print(f"\nRe-run with --apply to execute.")
        return

    applied = 0
    skipped = 0
    errors = 0

    for old, new in renames:
        try:
            if not os.path.exists(old):
                skipped += 1
                continue
            if os.path.exists(new) and old.lower() != new.lower():
                print(f"  SKIP (target exists): {os.path.basename(old)} -> {os.path.basename(new)}")
                skipped += 1
                continue
            os.rename(old, new)
            applied += 1
        except Exception as e:
            print(f"  ERROR: {old} -> {new}: {e}")
            errors += 1

    print(f"\nDone. {applied} applied, {skipped} skipped, {errors} errors.")


if __name__ == "__main__":
    main()
