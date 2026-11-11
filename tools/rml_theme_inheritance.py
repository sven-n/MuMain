"""Whole-file theme inheritance for the RML contract guard."""
import pathlib
import re


def base_name(directory):
    path = directory / "theme.ini"
    if not path.is_file():
        return ""
    section, base = "", ""
    for line in path.read_text(encoding="utf-8-sig").splitlines():
        line = line.strip()
        if not line or line.startswith((";", "#")):
            continue
        if line.startswith("[") and line.endswith("]"):
            section = line[1:-1].strip().lower()
        elif section == "theme" and "=" in line:
            key, value = line.split("=", 1)
            if key.strip().lower() == "base":
                base = value.strip()
                if len(base) >= 2 and base[0] == base[-1] and base[0] in "\"'":
                    base = base[1:-1]
    return base.lower()


def theme_chain(directory):
    root, seen, chain = directory.parent, set(), []
    while True:
        name = directory.name.lower()
        if not re.fullmatch(r"[a-z0-9_-]+", name) or not directory.is_dir():
            raise ValueError(f"Invalid or missing theme: {directory}")
        if name in seen:
            raise ValueError(f"Theme inheritance cycle at: {name}")
        seen.add(name)
        chain.append(directory)
        parent = base_name(directory)
        if not parent:
            return chain
        if not re.fullmatch(r"[a-z0-9_-]+", parent):
            raise ValueError(f"Invalid base theme: {parent}")
        directory = root / parent


def resolve_file(relative, chain):
    if relative.is_absolute() or ".." in relative.parts:
        return None
    return next((directory / relative for directory in chain
                 if (directory / relative).is_file()), None)


def resolve_virtual(path, virtual_root, chain):
    try:
        relative = path.relative_to(virtual_root)
    except ValueError:
        return path
    return resolve_file(relative, chain) or path
