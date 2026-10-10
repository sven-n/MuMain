"""Run the rollout-2 diagnostic against real markup and translated resources.

This supplies presentation state without opening networked game windows. Fixed
event and shop notice layouts must pass geometry checks; remaining title/helper defects
are reported.
"""

import argparse
import copy
import os
from pathlib import Path
import re
import subprocess
import xml.etree.ElementTree as ET


LOCALES = ("en", "de", "es", "pl", "ru")
THEMES = ("legacy", "modern")
WINDOWS = ("blood_castle_enter", "devil_square_enter", "my_shop",
           "purchase_shop", "mu_helper_config")


def resources(path):
    return {entry.attrib["name"]: entry.findtext("value", "")
            for entry in ET.parse(path).getroot().findall("data")}


def symbol_keys(header):
    return dict(re.findall(r"extern const wchar_t\* (\w+);\s*// ([^\r\n]*)", header))


def source_bindings(source):
    bindings = dict(re.findall(r'c\.Bind\("([\w_]+)", &model\.(\w+)\)', source))
    assignments = dict(re.findall(
        r'model\.(\w+) = [\w:]+\(I18N::Game::(\w+)\)', source))
    result = {name: assignments[member] for name, member in bindings.items()
              if member in assignments}
    for name, symbol in re.findall(
            r'syncWide\([^,]+, "([\w_]+)", I18N::Game::(\w+)\)', source):
        result[name] = symbol
    members = dict(re.findall(
        r'labels.RegisterMember\("([\w_]+)", &MuHelperConfigLabels::(\w+)\)', source))
    values = dict(re.findall(r'l\.(\w+) = Narrow\(I18N::Game::(\w+)\)', source))
    result.update({"labels." + name: values[member] for name, member in members.items()
                   if member in values})
    return result


def translation(symbol, keys, localized):
    key = keys[symbol]
    if key not in localized:
        raise ValueError(f"Missing translation: {key}")
    value = localized[key]
    if not value.strip():
        raise ValueError(f"Empty translation: {key}")
    return value


def event_state(window, source, keys, localized):
    title = "MessengerOfArchangel" if window == "blood_castle_enter" else "DevilSquare"
    lines = re.search(r'const std::vector<std::wstring> lines = \{(.*?)\};', source, re.S)
    symbols = re.findall(r'I18N::Game::(\w+)', lines[1]) if lines else ["YourWillToHelpTheArchangel"]
    description = [translation(symbol, keys, localized) for symbol in symbols]
    blood = window == "blood_castle_enter"
    pattern = "CastleDLevelDD" if blood else "TheDSquareDDLevel"
    master = "CastleNoDMasterLevel" if blood else "SquareNoDMasterLevel"
    levels = re.findall(r'm_i\w+LimitLevel\[(\d+)\]\[(\d+)\] = (\d+)', source)
    limits = {(int(index), int(bound)): int(value) for index, bound, value in levels}
    count = 8 if blood else 7
    buttons = [translation(pattern, keys, localized) % (i + 1, limits[i, 0], limits[i, 1])
               for i in range(count - 1)]
    buttons.append(translation(master, keys, localized) % count)
    return {"title_text": translation(title, keys, localized)}, description, buttons


def expand_repeat(root, attribute, texts):
    for parent in root.iter():
        for element in list(parent):
            if element.attrib.get("data-for", "") != attribute:
                continue
            index = list(parent).index(element)
            parent.remove(element)
            for offset, text in enumerate(texts):
                clone = copy.deepcopy(element)
                clone.attrib.pop("data-for")
                target = clone if attribute == "line : lines" else clone.find("span")
                target.text = text
                target.set("data-audit-source", text)
                if attribute == "b, i : buttons" and offset == 1:
                    clone.set("class", clone.attrib.get("class", "") + " enabled")
                parent.insert(index + offset, clone)
            return
    raise ValueError(f"Missing repeated template: {attribute}")


def project_attributes(element):
    for name, value in list(element.attrib.items()):
        if name == "data-class-hidden":
            element.set("data-audit-hidden", value)
        elif name == "data-class-summoner":
            element.set("data-audit-summoner", "true")
        elif name == "data-style---text-px":
            element.set("data-audit-native-root", "true")
        elif name == "data-style-font-size":
            role = "bold" if "bold_text_px" in value else "normal"
            element.set("data-audit-font", role)
        if name.startswith("data-") and not name.startswith("data-audit-"):
            element.attrib.pop(name)


def project_markup(root, state):
    leftovers = {"labels.seconds": "s", "shop_owner_text": "Shop owner",
                 "title": "Personal Store", "c.count": "", "it.name": "",
                 "hunt_range": "6", "pick_range": "6"}
    for element in root.iter():
        if element.text and "{{" in element.text:
            expression = re.fullmatch(r'\s*\{\{([\w.]+)\}\}\s*', element.text)
            if not expression:
                raise ValueError(f"Unrecognised template: {element.text}")
            key = expression[1]
            if key not in state and key not in leftovers:
                raise ValueError(f"Unpopulated text: {key}")
            element.text = state.get(key, leftovers.get(key))
        if "data-value" in element.attrib:
            element.set("value", "10" if element.attrib["data-value"] != "item_name" else "")
        project_attributes(element)


def prepare_document(repo, theme, window, keys, localized):
    assets = repo / "src/bin/Data/Interface/RmlUi"
    themed = assets / "themes" / theme
    path = themed / (window + ".rml")
    markup = (path if path.exists() else assets / (window + ".rml")).read_text(encoding="utf-8")
    root = ET.fromstring(re.sub(r'<!--.*?-->', '', markup, flags=re.S))
    sources = {"blood_castle_enter": "Events/BloodCastleEnter.cpp",
               "devil_square_enter": "Events/EnterDevilSquare.cpp",
               "my_shop": "Inventory/MyShopInventory.cpp",
               "purchase_shop": "Inventory/PurchaseShopInventory.cpp",
               "mu_helper_config": "MuHelper/MuHelperConfigWindow.cpp"}
    source = (repo / "src/source/UI" / sources[window]).read_text(encoding="utf-8")
    state = {name: translation(symbol, keys, localized)
             for name, symbol in source_bindings(source).items()}
    state.setdefault("title", translation("PersonalStore", keys, localized))
    if window.endswith("_enter"):
        state, description, buttons = event_state(window, source, keys, localized)
        expand_repeat(root, "line : lines", description)
        expand_repeat(root, "b, i : buttons", buttons)
    project_markup(root, state)
    for link in root.findall("./head/link"):
        path = themed / link.attrib["href"]
        link.set("href", (path if path.exists() else assets / link.attrib["href"]).as_posix())
    return ET.tostring(root, encoding="unicode")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, required=True)
    parser.add_argument("--generated", type=Path, required=True)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.repo = args.repo.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    keys = symbol_keys((args.generated / "I18N/Game.h").read_text(encoding="utf-8"))
    manifest = []
    english = resources(args.repo / "src/Localization/Game.en.resx")
    for locale in LOCALES:
        # ResxGen also falls back to English when a locale omits a resource key.
        localized = english | resources(args.repo / "src/Localization" / f"Game.{locale}.resx")
        for theme in THEMES:
            for window in WINDOWS:
                path = args.output / f"{theme}-{window}-{locale}.rml"
                path.write_text(prepare_document(args.repo, theme, window, keys, localized), encoding="utf-8")
                manifest.append("\t".join((theme, window, locale, str(path.resolve()))))
    (args.output / "manifest.tsv").write_text("\n".join(manifest), encoding="utf-8")
    env = dict(os.environ, MU_RML_TEXT_LAYOUT_CASES=str(args.output.resolve()))
    return subprocess.call([str(args.executable.resolve())], env=env)


if __name__ == "__main__":
    raise SystemExit(main())
