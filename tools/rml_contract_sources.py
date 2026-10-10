"""Discover the repository's literal RML contracts without a C++ parser dependency.

Balanced delimiters keep lambdas and multiple views separate. Unsupported dynamic
document declarations are errors rather than silently disappearing from coverage.
"""
import pathlib
import re


CPP_TOKEN_RE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
DOCUMENT_RE = re.compile(r'"Data/Interface/RmlUi/([\w]+)\.rml"')
VIEW_RE = re.compile(r'\bThemedView\s*<([^<>]*)>\s+(\w+)\s*([\{;])')
VIEW_ALIAS_RE = re.compile(r'\busing\s+(\w+)\s*=\s*(?:[\w]+::)*ThemedView\s*<([^<>]*)>\s*;')
CLASS_RE = re.compile(r'\b(?:class|struct)\s+(\w+)[^;{]*\{')
FUNCTION_RE = re.compile(r'\b([\w:~]+)\s*\(')
BIND_RE = re.compile(r'\.Bind\(\s*"([^"]+)"\s*,')
CALLBACK_RE = re.compile(r'\.BindEventCallback\(\s*"([^"]+)"\s*,')
DIRECT_LOAD_RE = re.compile(r'\b(?:LoadThemedDocument|LoadDocument)\(\s*(?:[^,;{}]*,\s*)?"Data/Interface/RmlUi/(\w+)\.rml"\s*\)')


def strip_comments(text):
    return CPP_TOKEN_RE.sub(lambda m: " " * len(m[0]) if m[0].startswith("/") else m[0], text)


def code_only(text):
    return CPP_TOKEN_RE.sub(lambda m: " " * len(m[0]), text)


def closing_delimiter(text, start):
    pairs = {"{": "}", "(": ")", "[": "]", "<": ">"}
    opening, closing = text[start], pairs[text[start]]
    depth, position = 1, start + 1
    tokens = {m.start(): m.end() for m in CPP_TOKEN_RE.finditer(text, position)}
    while position < len(text):
        if position in tokens:
            position = tokens[position]
            continue
        if text[position] == opening:
            depth += 1
        elif text[position] == closing:
            depth -= 1
            if depth == 0:
                return position
        position += 1
    raise ValueError("Unclosed C++ delimiter near " + text[start:start + 80])


def class_at(text, position):
    owners = [(m.start(), m[1]) for m in CLASS_RE.finditer(code_only(text))
              if m.end() <= position < closing_delimiter(text, m.end() - 1)]
    return max(owners, default=(0, ""))[1]


def functions(text):
    """Yield name, parameters and body for definitions, excluding call sites."""
    position = 0
    for match in FUNCTION_RE.finditer(code_only(text)):
        if match.start() < position:
            continue
        start = match.end() - 1
        end = closing_delimiter(text, start)
        tail = re.match(r'\s*(?:const\s*|override\s*|noexcept\s*)*\{', text[end + 1:])
        if tail is None:
            continue
        body_start = end + tail.end()
        body_end = closing_delimiter(text, body_start)
        yield match[1], text[start + 1:end], text[body_start + 1:body_end]
        position = body_end + 1


def resolve_model(text, model, position=None):
    if position is not None:
        for match in reversed(list(CLASS_RE.finditer(code_only(text), 0, position))):
            if position < closing_delimiter(text, match.end() - 1):
                text = text[match.end():closing_delimiter(text, match.end() - 1)]
                break
    alias = re.search(r'\busing\s+' + re.escape(model) + r'\s*=\s*([\w:]+)\s*;', text)
    return alias[1] if alias else model


def declared_views(path, text):
    for match in VIEW_RE.finditer(code_only(text)):
        model, variable, delimiter = match.groups()
        owner = class_at(text, match.start())
        if delimiter == ";":
            yield {"path": path, "owner": owner, "model": resolve_model(text, model.strip(), match.start()),
                   "variable": variable, "documents": (), "deferred": True}
            continue
        end = closing_delimiter(text, match.end() - 1)
        documents = tuple(DOCUMENT_RE.findall(text[match.end():end]))
        yield {"path": path, "owner": owner, "model": resolve_model(text, model.strip(), match.start()),
               "variable": variable, "documents": documents, "deferred": False,
               "initializer": text[match.end():end]}
    for alias in VIEW_ALIAS_RE.finditer(text):
        pattern = re.compile(r'\bmake_unique\s*<' + re.escape(alias[1]) + r'>\s*\(')
        for match in pattern.finditer(text):
            end = closing_delimiter(text, match.end() - 1)
            yield {"path": path, "owner": "", "model": resolve_model(text, alias[2].strip()),
                   "variable": "", "documents": tuple(DOCUMENT_RE.findall(text[match.end():end])),
                   "deferred": False}


def constructor_documents(view, texts):
    text = texts.get(view["path"].with_suffix(".cpp"), "")
    initializer = re.search(r'\b' + re.escape(view["variable"]) + r'\s*\(', text)
    if initializer is None:
        raise ValueError(f'{view["path"]}: cannot discover initializer for {view["variable"]}')
    end = closing_delimiter(text, initializer.end() - 1)
    view["documents"] = tuple(DOCUMENT_RE.findall(text[initializer.end():end]))
    return bool(view["documents"])


def factory_views(factory, texts):
    """Reusable view constructors receive literal paths at their callers."""
    pattern = re.compile(r'\b' + re.escape(factory["owner"]) + r'\s+(\w+)\s*\{')
    for path, text in texts.items():
        for match in pattern.finditer(text):
            end = closing_delimiter(text, match.end() - 1)
            documents = tuple(DOCUMENT_RE.findall(text[match.end():end]))
            if documents:
                yield dict(factory, documents=documents, deferred=False)
        # A member initialized in the outer window's constructor (.cpp).
        for doc in DOCUMENT_RE.finditer(text):
            if path.suffix != ".cpp":
                continue
            before = text[:doc.start()]
            member = re.search(r'(\w+)\s*\([^()]*$', before)
            header = texts.get(path.with_suffix(".h"), "")
            if member and re.search(r'\b' + re.escape(factory["owner"]) + r'\s+' +
                                    re.escape(member[1]) + r'\s*;', header):
                yield dict(factory, documents=(doc[1],), deferred=False)


def discover_views(texts):
    views = []
    for path, text in texts.items():
        for view in declared_views(path, text):
            if view["deferred"] and not constructor_documents(view, texts):
                callers = list(factory_views(view, texts))
                if not callers:
                    raise ValueError(f'{path}: no literal callers for {view["owner"]}')
                views.extend(callers)
            else:
                if not view["documents"]:
                    raise ValueError(f'{path}: no literal documents for {view["variable"]}')
                views.append(view)
        for match in DIRECT_LOAD_RE.finditer(text):
            prefix = text[text.rfind("\n", 0, match.start()) + 1:match.start()]
            assignment = re.search(r'(\w+)\s*=.*$', prefix)
            views.append({"path": path, "owner": "", "model": "", "variable": assignment[1] if assignment else "",
                          "documents": (match[1],), "deferred": False, "direct": True})
    return views


def related_functions(view, definitions):
    paths = {view["path"], view["path"].with_suffix(".cpp")}
    model = view["model"].split("::")[-1]
    for path, name, parameters, body in definitions:
        owner = name.split("::")[-2] if "::" in name else ""
        same_owner = path in paths and (not owner or not view["owner"] or owner == view["owner"])
        model_binding = model and owner == model
        if same_owner or model_binding:
            yield name, parameters, body


def registration_bodies(view, definitions):
    if not view["model"] or view["model"] == "void":
        return
    yield view.get("initializer", "")
    model = view["model"].split("::")[-1]
    for name, parameters, body in related_functions(view, definitions):
        if "DataModelConstructor" in parameters:
            model_parameters = re.findall(r'([\w:]+)\s*&', parameters)
            if all(p.split("::")[-1] in ("DataModelConstructor", model, "Model") for p in model_parameters):
                yield body


def literal_ids(view, definitions):
    documents = view["documents"]
    ids = {doc: set() for doc in documents}
    access = re.escape(view["variable"]) + r'\.Document\(\s*(\d*)\s*\)'
    for name, parameters, body in related_functions(view, definitions):
        if view["variable"] and not re.search(access, body) and not view.get("direct"):
            continue
        aliases = {}
        for match in re.finditer(r'(\w+)\s*=\s*' + access, body):
            index = int(match[2] or 0)
            if index < len(documents):
                aliases[match[1]] = documents[index]
        for match in re.finditer(access + r'\s*->GetElementById\(\s*"([^"]+)"\s*\)', body):
            index = int(match[1] or 0)
            if index < len(documents):
                ids[documents[index]].add(match[2])
        for match in re.finditer(r'(\w+)->GetElementById\(\s*"([^"]+)"\s*\)', body):
            if match[1] in aliases:
                ids[aliases[match[1]]].add(match[2])
            elif len(documents) == 1:
                ids[documents[0]].add(match[2])
    return ids


def discover_contracts(source_root):
    paths = sorted(p for p in source_root.rglob("*") if p.suffix in (".cpp", ".h"))
    texts = {path: strip_comments(path.read_text(encoding="utf-8", errors="ignore")) for path in paths}
    views = discover_views(texts)
    discovered = {doc for view in views for doc in view["documents"]}
    referenced = {doc for text in texts.values() for doc in DOCUMENT_RE.findall(text)}
    if referenced - discovered:
        raise ValueError("Undiscovered document ownership: " + ", ".join(sorted(referenced - discovered)))
    definition_paths = {path for view in views for path in (view["path"], view["path"].with_suffix(".cpp"))}
    models = {view["model"].split("::")[-1] for view in views if view["model"]}
    definition_paths.update(path for path in paths if path.stem in models)
    definitions = [(path, *definition) for path in sorted(definition_paths) if path in texts
                   for definition in functions(texts[path])]
    contracts = []
    for view in views:
        bodies = "\n".join(registration_bodies(view, definitions))
        fields, callbacks = set(BIND_RE.findall(bodies)), set(CALLBACK_RE.findall(bodies))
        if view["model"] not in ("", "void") and not fields and not callbacks:
            raise ValueError(f'{view["path"]}: no literal bindings discovered for model {view["model"]}')
        contracts.append(dict(view, fields=fields, callbacks=callbacks, ids=literal_ids(view, definitions)))
    return contracts
