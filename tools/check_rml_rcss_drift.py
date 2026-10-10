#!/usr/bin/env python3
"""Check literal C++/RML contracts for every runtime theme variant.

Discover ThemedView declarations in headers and implementations, constructor-
initialized reusable views, per-instance social models, aliased make_unique views,
and literal direct loads. Refuse zero coverage or an undiscovered literal path.

Required literal GetElementById lookups are checked per document and theme. Model
callbacks are checked per view and theme (a view may own several documents).
Fields are checked across the model's document/theme variants so alternative
readouts remain legal. Deliberately unused offered fields need a source comment:
    // rml-contract-unused: field_name: reason
Comments and string constants in expressions do not satisfy a contract.

This is a source guard, not a C++ or RML interpreter. Dynamic ids, selector-based
lookups, struct member expressions and arbitrary registration helpers are outside
its scope; use runtime interaction checks for those. --review lists the contracts
and unused-field exceptions so discovery is inspectable.

Usage: python tools/check_rml_rcss_drift.py [--source-root DIR] [--asset-root DIR] [--review]
Exit code 0 = checked and clean, 1 = drift, incomplete discovery or missing assets.
"""
import argparse
import pathlib
import re
import sys

sys.dont_write_bytecode = True
from rml_contract_sources import discover_contracts


REPO_ROOT = pathlib.Path(__file__).resolve().parent.parent
TEMPLATE_LINK_RE = re.compile(r'<link\b[^>]*\btype\s*=\s*"text/template"[^>]*\bhref\s*=\s*"([^"]+)"')
COMMENT_RE = re.compile(r'<!--.*?-->', re.S)
ID_RE = re.compile(r'(?<![\w-])id\s*=\s*"([^"]+)"')
DATA_ATTRIBUTE_RE = re.compile(r'\b(data-[\w-]+)\s*=\s*"([^"]*)"')
INTERPOLATION_RE = re.compile(r'\{\{(.*?)\}\}', re.S)
STRING_RE = re.compile(r"'(?:\\.|[^'\\])*'")
IDENTIFIER_RE = re.compile(r'(?<![\w.])[A-Za-z_]\w*(?:\.[A-Za-z_]\w*)*')
UNUSED_RE = re.compile(r'//\s*rml-contract-unused:\s*(\w+)\s*:\s*([^\r\n]+)')
OPTIONAL_RE = re.compile(r'<!--\s*rml-contract-optional-(id|callback):\s*(\w+)\s*:\s*(.*?)-->', re.S)


def linked_documents(entry, resolve_dir):
    seen, texts = set(), []
    queue = [(entry, resolve_dir)]
    while queue:
        path, directory = queue.pop()
        if path in seen:
            continue
        seen.add(path)
        if not path.is_file():
            raise ValueError(f"Missing document/template: {path}")
        text = path.read_text(encoding="utf-8")
        texts.append(text)
        for href in TEMPLATE_LINK_RE.findall(COMMENT_RE.sub("", text)):
            linked = (directory / href).resolve()
            queue.append((linked, linked.parent))
    return "\n".join(texts), seen


def theme_documents(asset_root, document):
    themes = asset_root / "themes"
    directories = sorted(p for p in themes.iterdir() if p.is_dir()) if themes.is_dir() else []
    if not directories:
        directories = [asset_root]
    variants = {}
    for directory in directories:
        entry = directory / f"{document}.rml"
        if not entry.is_file():
            entry = asset_root / f"{document}.rml"
        variants[directory.name] = linked_documents(entry, directory)
    return variants


def markup_contract(text):
    optional = {"id": {}, "callback": {}}
    for kind, name, reason in OPTIONAL_RE.findall(text):
        if not reason.strip():
            raise ValueError(f"Optional {kind} '{name}' needs a reason")
        optional[kind][name] = reason.strip()
    text = COMMENT_RE.sub("", text)
    expressions = INTERPOLATION_RE.findall(text)
    callbacks = set()
    for attribute, value in DATA_ATTRIBUTE_RE.findall(text):
        if attribute == "data-model":
            continue
        expressions.append(value)
        if attribute.startswith("data-event-"):
            callback = re.match(r'\s*(\w+)', value)
            if callback:
                callbacks.add(callback[1])
    symbols = {name.split(".")[0] for name in IDENTIFIER_RE.findall(STRING_RE.sub("", "\n".join(expressions)))}
    return {"ids": set(ID_RE.findall(text)), "symbols": symbols, "callbacks": callbacks, "optional": optional}


def unused_fields(source_root):
    exceptions = {}
    for path in sorted(source_root.rglob("*")):
        if path.suffix not in (".cpp", ".h"):
            continue
        for field, reason in UNUSED_RE.findall(path.read_text(encoding="utf-8", errors="ignore")):
            if not reason.strip():
                raise ValueError(f"{path}: unused field '{field}' needs a reason")
            exceptions.setdefault(path, {})[field] = reason.strip()
    return exceptions


def contract_exceptions(contract, exceptions):
    paths = {contract["path"], contract["path"].with_suffix(".cpp")}
    model = contract["model"].split("::")[-1]
    paths.update(path for path in exceptions if path.stem == model)
    return {field: reason for path in paths for field, reason in exceptions.get(path, {}).items()}


def group_contracts(contracts):
    groups = {}
    for contract in contracts:
        key = (contract["path"], contract["owner"], contract["model"])
        if not contract["model"]:
            key += (contract["variable"],)
        group = groups.setdefault(key, {"fields": set(), "callbacks": set(), "ids": {}, "contracts": []})
        group["fields"].update(contract["fields"])
        group["callbacks"].update(contract["callbacks"])
        group["contracts"].append(contract)
        for document, ids in contract["ids"].items():
            group["ids"].setdefault(document, set()).update(ids)
    return list(groups.values())


def check_ids(theme, document, ids, markup):
    errors = [f"{theme}/{document}: missing required id '{name}'"
              for name in sorted(ids - markup["ids"] - markup["optional"]["id"].keys())]
    for name in markup["optional"]["id"]:
        if name not in ids or name in markup["ids"]:
            errors.append(f"{theme}/{document}: stale optional-id exception '{name}'")
    return errors


def check_fields(group, all_symbols, exceptions, label):
    errors = []
    allowed = {field: reason for contract in group["contracts"]
               for field, reason in contract_exceptions(contract, exceptions).items()}
    for name in sorted(group["fields"] - all_symbols - allowed.keys()):
        errors.append(f"({label}): field '{name}' is referenced by no theme; document intentional non-use in its source")
    for name in sorted(allowed.keys() - group["fields"]):
        errors.append(f"({label}): unused-field exception '{name}' has no discovered binding")
    for name in sorted(allowed.keys() & all_symbols):
        errors.append(f"({label}): unused-field exception '{name}' is stale (markup uses it)")
    return errors


def check_group(group, documents, exceptions):
    errors = []
    label = ", ".join(sorted(group["ids"]))
    all_symbols = set()
    themes = next(iter(documents.values())).keys()
    for theme in themes:
        theme_callbacks = set()
        for document, ids in group["ids"].items():
            markup = documents[document][theme]
            all_symbols.update(markup["symbols"])
            theme_callbacks.update(markup["callbacks"])
            errors.extend(check_ids(theme, document, ids, markup))
        optional_callbacks = {name for document in group["ids"]
                              for name in documents[document][theme]["optional"]["callback"]}
        for name in sorted(group["callbacks"] - theme_callbacks - optional_callbacks):
            errors.append(f"{theme}/({label}): missing required callback '{name}'")
        for name in sorted(optional_callbacks - group["callbacks"] | optional_callbacks & theme_callbacks):
            errors.append(f"{theme}/({label}): stale optional-callback exception '{name}'")
    errors.extend(check_fields(group, all_symbols, exceptions, label))
    return errors


def review_group(group, exceptions):
    print("  " + ", ".join(sorted(group["ids"])))
    print("    fields: " + ", ".join(sorted(group["fields"])))
    print("    callbacks: " + ", ".join(sorted(group["callbacks"])))
    for document, ids in sorted(group["ids"].items()):
        print(f"    {document} ids: " + ", ".join(sorted(ids)))
    for contract in group["contracts"]:
        for field, reason in sorted(contract_exceptions(contract, exceptions).items()):
            print(f"    unused {field}: {reason}")


def check_contracts(source_root, asset_root, review=False):
    contracts = discover_contracts(source_root)
    if not contracts:
        raise ValueError("No documents discovered; refusing a successful check with zero coverage")
    groups = group_contracts(contracts)
    names = sorted({document for group in groups for document in group["ids"]})
    documents, files, variants = {}, set(), 0
    for name in names:
        documents[name] = {}
        for theme, (text, paths) in theme_documents(asset_root, name).items():
            documents[name][theme] = markup_contract(text)
            if review:
                for kind, optional_names in documents[name][theme]["optional"].items():
                    for symbol, reason in optional_names.items():
                        print(f"  {theme}/{name} optional {kind} {symbol}: {reason}")
            files.update(paths)
            variants += 1
    exceptions = unused_fields(source_root)
    errors = []
    for group in groups:
        errors.extend(check_group(group, documents, exceptions))
        if review:
            review_group(group, exceptions)
    return errors, len(names), variants, len(files)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--source-root", type=pathlib.Path, default=REPO_ROOT / "src/source")
    parser.add_argument("--asset-root", type=pathlib.Path, default=REPO_ROOT / "src/bin/Data/Interface/RmlUi")
    parser.add_argument("--review", action="store_true")
    args = parser.parse_args()
    if not args.source_root.is_dir() or not args.asset_root.is_dir():
        print("RML contract check: source and asset roots must exist", file=sys.stderr)
        return 1
    try:
        errors, documents, variants, files = check_contracts(args.source_root, args.asset_root, args.review)
    except (OSError, ValueError) as error:
        print(f"RML contract check failed: {error}", file=sys.stderr)
        return 1
    coverage = f"{documents} documents, {variants} theme variants, {files} files checked"
    if errors:
        print(f"RML contract check failed ({coverage}):", file=sys.stderr)
        for error in errors:
            print("  " + error, file=sys.stderr)
        return 1
    print(f"RML/RCSS theme-fork drift check: OK ({coverage})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
