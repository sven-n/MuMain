"""Collects the effect, particle, joint and sprite types the client code uses, and generates the
compiled symbol lists and the catalogue files of the effect catalogue (FX1.1).

    python tools/effect-catalogue/effect_types.py collect <work dir>
        Scans src/source for the type expressions of each kind and writes
        <work dir>/candidates.json and <work dir>/probe.cpp. Compile and run the probe with the
        include folder src/source (any C++20 compiler) and save its output as
        <work dir>/values.txt.

    python tools/effect-catalogue/effect_types.py generate <work dir>
        Reads candidates.json and values.txt, merges expressions with the same value, and writes
        src/source/Data/GameData/EffectData/EffectTypeSymbols.cpp and the catalogue files in
        src/bin/Data/Effects/. Names already in the catalogue files are kept; new types get a
        generated name (see make_name), which is then reviewed by hand.

    python tools/effect-catalogue/effect_types.py unused <work dir>
        Lists the types that the code of a kind handles but no call creates.

Where a kind's types come from:
- the case labels of the switches on the type in that kind's code (Create/Move/Render),
- the registry rows (effects),
- the first argument of every call that creates that kind (BASE + rand() % n is expanded).
"""
import json
import os
import re
import sys
from collections import defaultdict

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = os.path.join(ROOT, 'src', 'source')
KINDS = ('effect', 'particle', 'joint', 'sprite')

# The code of each kind: its switches on the type are that kind's types.
KIND_FILES = {
    'effect': ['Render/Effects/ZzzEffect.cpp', 'Render/Effects/Behaviors/MoveHandlers.cpp',
               'Render/Effects/Behaviors/EffectBehaviors.cpp'],
    'particle': ['Render/Effects/ZzzEffectParticle.cpp'],
    'joint': ['Render/Effects/ZzzEffectJoint.cpp'],
    'sprite': ['Render/Effects/zzzeffectsprite.cpp'],
}
TYPE_SWITCH_CONDITIONS = {'Type', 'o->Type'}

# The functions that create each kind; member functions with the same names are left out.
CREATE_FUNCTIONS = {
    'CreateEffect': 'effect', 'CreateEffectFpsChecked': 'effect',
    'CreateParticle': 'particle', 'CreateParticleFpsChecked': 'particle',
    'CreateJoint': 'joint', 'CreateJointFpsChecked': 'joint',
    'CreateSprite': 'sprite',
}

# Functions that create a kind with a type passed in as an argument: (kind, argument index).
WRAPPER_FUNCTIONS = {
    'RenderBrightEffect': ('sprite', 1),
    'RenderLight': ('sprite', 1),
}

# Types that code computes in a way the scan cannot follow.
EXTRA_TYPES = {
    # frame animations: BASE + a frame number; an effect that draws sprites of its own type
    'sprite': [f'BITMAP_LIGHTNING_MEGA{i}' for i in (1, 2, 3)] + [f'BITMAP_FIRECRACKER{i:04d}' for i in range(1, 8)]
    + ['BITMAP_LIGHT_MARKS'],
    # effects create particles of their own type (CreateParticle(o->Type, ...))
    'particle': ['BITMAP_ORORA', 'BITMAP_SHINY+6', 'BITMAP_SPARK+2', 'BITMAP_PIN_LIGHT'],
    # the New Year event keeps the effect type in m_iAnimation (Event.cpp: BEKSULKI + 0..5, or PIG); the red
    # hot pepper is the green one that changes its type (ZzzEffect.cpp)
    'effect': ['MODEL_NEWYEARSDAY_EVENT_BEKSULKI', 'MODEL_NEWYEARSDAY_EVENT_CANDY',
               'MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN', 'MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED',
               'MODEL_NEWYEARSDAY_EVENT_PIG', 'MODEL_NEWYEARSDAY_EVENT_YUT'],
}

SYMBOL = r'[A-Z_][A-Z0-9_]*'


def read_source(relative):
    """The file without comments and without #if 0 blocks; strings and line breaks are kept."""
    text = open(os.path.join(SRC, relative), encoding='utf-8', errors='replace').read()
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith('//', i):
            j = text.find('\n', i)
            i = n if j < 0 else j
        elif text.startswith('/*', i):
            j = text.find('*/', i + 2)
            block = text[i:(n if j < 0 else j + 2)]
            out.append('\n' * block.count('\n'))
            i = n if j < 0 else j + 2
        elif c in '"\'':
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            out.append(text[i:j + 1])
            i = j + 1
        else:
            out.append(c)
            i += 1
    return '\n'.join(active_lines(''.join(out).split('\n')))


# Macros the compiler or the build defines; the project never #defines them.
PLATFORM_MACROS = {'_WIN32', '_WIN64', '_MSC_VER', '__linux__', '__APPLE__', '__GNUC__', '__clang__', '_EDITOR',
                   '_DEBUG', 'NDEBUG', 'UNICODE', '_UNICODE', '__cplusplus'}
_project_macros = None


def project_macros():
    """Every macro that some file of the project #defines."""
    global _project_macros
    if _project_macros is None:
        _project_macros = set(PLATFORM_MACROS)
        for relative in all_source_files():
            for line in open(os.path.join(SRC, relative), encoding='utf-8', errors='replace'):
                m = re.match(r'\s*#\s*define\s+(\w+)', line)
                if m:
                    _project_macros.add(m.group(1))
    return _project_macros


def active_lines(lines):
    """The lines with the code of switched-off blocks blanked: #if 0, and #ifdef/#ifndef of feature
    macros that no file defines (e.g. PBG_ADD_CHARACTERSLOT). Other conditions count as true."""
    result, frames = [], []  # frames: [active, taken]
    for line in lines:
        stripped = line.strip()
        directive = re.match(r'#\s*(if|ifdef|ifndef|elif|else|endif)\b\s*(.*)', stripped)
        if directive:
            word, rest = directive.group(1), directive.group(2).strip()
            if word in ('if', 'ifdef', 'ifndef'):
                if word == 'if':
                    active = not re.match(r'0\b', rest)
                else:
                    defined = rest.split()[0] in project_macros() if rest else True
                    active = defined if word == 'ifdef' else not defined
                frames.append([active, active])
            elif word == 'elif' and frames:
                frames[-1] = [not frames[-1][1], True] if not frames[-1][1] else [False, True]
            elif word == 'else' and frames:
                frames[-1] = [not frames[-1][1], True]
            elif word == 'endif' and frames:
                frames.pop()
            result.append('')
            continue
        result.append(line if all(frame[0] for frame in frames) else '')
    return result


def line_of(text, index):
    return text.count('\n', 0, index) + 1


def matching(text, open_index, open_char, close_char):
    """The index of the bracket that closes the one at open_index (strings were kept intact)."""
    depth = 0
    i = open_index
    while i < len(text):
        c = text[i]
        if c in '"\'':
            j = i + 1
            while j < len(text) and text[j] != c:
                j += 2 if text[j] == '\\' else 1
            i = j + 1
            continue
        if c == open_char:
            depth += 1
        elif c == close_char:
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def normalize(expression):
    e = re.sub(r'\s+', '', expression)
    e = re.sub(r'^\((.*)\)$', r'\1', e)
    e = re.sub(r'^\(int\)', '', e)
    e = re.sub(r'static_cast<int>\((.*)\)', r'\1', e)
    return e


def switch_case_labels(text):
    """(label, line) of the switches on the type, without the labels of switches nested in them."""
    labels = []
    for m in re.finditer(r'\bswitch\s*\(', text):
        open_paren = m.end() - 1
        close_paren = matching(text, open_paren, '(', ')')
        condition = normalize(text[open_paren + 1:close_paren])
        if condition not in TYPE_SWITCH_CONDITIONS:
            continue
        open_brace = text.find('{', close_paren)
        close_brace = matching(text, open_brace, '{', '}')
        body = text[open_brace + 1:close_brace]
        # blank out nested switch blocks
        masked = list(body)
        for nested in re.finditer(r'\bswitch\s*\(', body):
            p = nested.end() - 1
            q = matching(body, p, '(', ')')
            b = body.find('{', q)
            e = matching(body, b, '{', '}')
            for k in range(b, e + 1):
                if masked[k] != '\n':
                    masked[k] = ' '
        masked = ''.join(masked)
        for case in re.finditer(r'\bcase\s+([^:;]+?)\s*:(?!:)', masked):
            labels.append((normalize(case.group(1)), line_of(text, open_brace + 1 + case.start())))
    return labels


def nth_argument(text, open_paren, index):
    depth = 0
    start = open_paren + 1
    current = 0
    for i in range(open_paren + 1, len(text)):
        c = text[i]
        if c in '([{':
            depth += 1
        elif c in ')]}':
            if depth == 0:
                return text[start:i] if current == index else ''
            depth -= 1
        elif c == ',' and depth == 0:
            if current == index:
                return text[start:i]
            current += 1
            start = i + 1
    return ''


def resolve_variable(text, call_index, argument):
    """The expressions a variable argument stands for, from the code shortly before the call: a loop
    counter over BASE + i, or a variable set to one of some types."""
    e = normalize(argument)
    window = text[max(0, call_index - 4000):call_index]
    # BASE + i, BASE + (i % k) in a counted loop
    m = re.fullmatch(rf'({SYMBOL})\+\(?([a-z_]\w*)(?:%(\d+))?\)?', e)
    if m:
        base, counter, modulo = m.group(1), m.group(2), m.group(3)
        loops = re.findall(rf'for\s*\(\s*int\s+{counter}\s*=\s*0\s*;\s*{counter}\s*<\s*(\d+)', window)
        if loops:
            count = int(loops[-1]) if not modulo else min(int(loops[-1]), int(modulo))
            return [base if k == 0 else f'{base}+{k}' for k in range(count)]
        return []
    # a variable, maybe plus a constant
    m = re.fullmatch(r'([A-Za-z_]\w*)(?:\+(\d+))?', e)
    if not m or re.fullmatch(SYMBOL, m.group(1)):
        return []
    variable, offset = m.group(1), int(m.group(2) or 0)
    results = []
    for assignment in re.finditer(rf'\b{variable}\s*=(?!=)\s*([^;]+);', window):
        right = assignment.group(1)
        branches = re.split(r'[?:]', right)[1:] if '?' in right else [right]
        for branch in branches:
            for expression in expand(branch):
                if offset:
                    base, _, n = expression.partition('+')
                    expression = f'{base}+{int(n or 0) + offset}'
                results.append(expression)
    return results


def expand(expression):
    """The type expressions an argument stands for: one, a range for BASE + rand() % n, or none."""
    e = normalize(expression)
    if re.fullmatch(SYMBOL + r'(\+\d+)?', e) or re.fullmatch(r'\d+', e):
        return [e]
    for pattern in (rf'({SYMBOL})\+\(?rand\(\)%(\d+)\)?', rf'\(?rand\(\)%(\d+)\)?\+({SYMBOL})'):
        m = re.fullmatch(pattern, e)
        if m:
            base, count = (m.group(1), int(m.group(2))) if not m.group(1).isdigit() else (m.group(2), int(m.group(1)))
            return [base if k == 0 else f'{base}+{k}' for k in range(count)]
    for pattern in (rf'({SYMBOL})\+Random::RangeInt\(0,(\d+)\)', rf'Random::RangeInt\(0,(\d+)\)\+({SYMBOL})'):
        m = re.fullmatch(pattern, e)
        if m:
            base, last = (m.group(1), int(m.group(2))) if not m.group(1).isdigit() else (m.group(2), int(m.group(1)))
            return [base if k == 0 else f'{base}+{k}' for k in range(last + 1)]
    m = re.fullmatch(rf'({SYMBOL})\+(\d+)\+\(?rand\(\)%(\d+)\)?', e)
    if m:
        base, offset, count = m.group(1), int(m.group(2)), int(m.group(3))
        return [f'{base}+{offset + k}' for k in range(count)]
    return []


def all_source_files():
    for folder, _, files in os.walk(SRC):
        if 'ThirdParty' in folder:
            continue
        for f in files:
            if f.endswith(('.cpp', '.h')):
                yield os.path.relpath(os.path.join(folder, f), SRC).replace('\\', '/')


def collect(work):
    candidates = {kind: defaultdict(list) for kind in KINDS}
    unresolved = []

    def add(kind, expression, where):
        if re.fullmatch(r'(MODEL_|BITMAP_|BATTLE_CASTLE_WALL)[A-Z0-9_]*(\+\d+)?|\d+', expression):
            candidates[kind][expression].append(where)

    for kind, files in KIND_FILES.items():
        for relative in files:
            text = read_source(relative)
            for label, line in switch_case_labels(text):
                if not label.isdigit():
                    add(kind, label, f'{relative}:{line} case')

    registry = read_source('Render/Effects/EffectRegistry.cpp')
    for m in re.finditer(r'\badd\s*\(\s*\{([^}]*)\}', registry):
        for symbol in m.group(1).split(','):
            add('effect', normalize(symbol), f'Render/Effects/EffectRegistry.cpp:{line_of(registry, m.start())} registry')
    handlers = read_source('Render/Effects/Behaviors/MoveHandlers.cpp')
    for m in re.finditer(rf'\{{\s*({SYMBOL}(?:\s*\+\s*\d+)?)\s*,\s*&Move_', handlers):
        add('effect', normalize(m.group(1)), f'Render/Effects/Behaviors/MoveHandlers.cpp:{line_of(handlers, m.start())} registry')

    functions = {name: (kind, 0) for name, kind in CREATE_FUNCTIONS.items()}
    functions.update(WRAPPER_FUNCTIONS)
    names = '|'.join(functions)
    for relative in all_source_files():
        text = read_source(relative)
        for m in re.finditer(rf'(?<![\w.>:])({names})\s*\(', text):
            before = text[max(0, m.start() - 40):m.start()]
            if re.search(r'(void|int|BOOL|bool)\s*$', before):
                continue  # a declaration or definition
            kind, index = functions[m.group(1)]
            argument = nth_argument(text, m.end() - 1, index)
            if re.match(r'\s*(int|DWORD|WORD|short|BMD|OBJECT)\b', argument):
                continue
            where = f'{relative}:{line_of(text, m.start())} call'
            expressions = expand(argument) or resolve_variable(text, m.start(), argument)
            if not expressions:
                unresolved.append({'kind': kind, 'where': where, 'argument': normalize(argument)})
            for e in expressions:
                add(kind, e, where)

    for kind, symbols in EXTRA_TYPES.items():
        for symbol in symbols:
            add(kind, symbol, 'computed (effect_types.py EXTRA_TYPES)')

    os.makedirs(work, exist_ok=True)
    json.dump({'candidates': {k: dict(v) for k, v in candidates.items()}, 'unresolved': unresolved},
              open(os.path.join(work, 'candidates.json'), 'w', encoding='utf-8'), indent=1)
    lines = ['// Generated by tools/effect-catalogue/effect_types.py collect: prints the value of every',
             '// candidate type expression.',
             '#ifdef _WIN32', '#include <windows.h>', '#else',
             'typedef unsigned char BYTE;', 'typedef unsigned short WORD;', 'typedef unsigned int DWORD;',
             'typedef int BOOL;', '#endif',
             '#include "Core/Globals/Defined_Global.h"', '#include "Core/Globals/_define.h"',
             '#include "Core/Globals/_enum.h"', '#include "Core/Globals/_TextureIndex.h"',
             '#include <cstdio>', 'int main()', '{']
    for kind in KINDS:
        for expression in sorted(candidates[kind]):
            lines.append(f'    std::printf("{kind}\\t{expression}\\t%d\\n", static_cast<int>({expression}));')
    lines += ['    return 0;', '}', '']
    open(os.path.join(work, 'probe.cpp'), 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))
    for kind in KINDS:
        print(f'{kind}: {len(candidates[kind])} expressions')
    print(f'unresolved call arguments: {len(unresolved)} (see candidates.json)')


# ---------------------------------------------------------------- generate

MARKER = re.compile(r'_(BEGIN|END|START|MAX|COUNT|FIRST|LAST)$')
KIND_FILE_NAMES = {'effect': 'EffectTypes.json', 'particle': 'ParticleTypes.json', 'joint': 'JointTypes.json',
                   'sprite': 'SpriteTypes.json'}
KIND_ARRAY_NAMES = {'effect': 'EffectSymbols', 'particle': 'ParticleSymbols', 'joint': 'JointSymbols',
                    'sprite': 'SpriteSymbols'}

# Words of the enum names that are written together or misspelled: word -> words. A word is
# looked up without its trailing digits (MAYASTONE1 -> MAYA_STONE1).
WORDS = {
    '2LINE': 'TWO_LINE', 'AUTOLOAD': 'AUTO_LOAD', 'BUFFSKILL': 'BUFF_SKILL', 'CHERRYBLOSSOM': 'CHERRY_BLOSSOM',
    'CLUD': 'CLOUD', 'CRACKEFFECT': 'CRACK_EFFECT', 'CUNDUN': 'KUNDUN', 'CURSEDLICH': 'CURSED_LICH',
    'CURSEDTEMPLE': 'CURSED_TEMPLE', 'DARKLORD': 'DARK_LORD', 'DARKSTINGER': 'DARK_STINGER', 'EFF': 'EFFECT',
    'EMPIREGUARDIAN': 'EMPIRE_GUARDIAN', 'EMPIREGUARDIANBOSS': 'EMPIRE_GUARDIAN_BOSS', 'EXPLOTION': 'EXPLOSION',
    'FIRECRACKERRISE': 'FIRECRACKER_RISE', 'FORCEPILLAR': 'FORCE_PILLAR', 'FRAMESTRIKE': 'FLAME_STRIKE',
    'GUARDIANDEFENDER': 'GUARDIAN_DEFENDER', 'HOLYITEM': 'HOLY_ITEM', 'HOTPEPPER': 'HOT_PEPPER',
    'ICEHEART': 'ICE_HEART', 'LACEARROW': 'LACE_ARROW', 'LAVAGIANT': 'LAVA_GIANT', 'LIGHTMARKS': 'LIGHT_MARKS',
    'MAGICPOWER': 'MAGIC_POWER', 'MAYAHANDSKILL': 'MAYA_HAND_SKILL', 'MAYASTAR': 'MAYA_STAR',
    'MAYASTONE': 'MAYA_STONE', 'MAYASTONEFIRE': 'MAYA_STONE_FIRE', 'MOONHARVEST': 'MOON_HARVEST',
    'NEWYEARSDAY': 'NEW_YEARS_DAY', 'NIFE': 'KNIFE', 'NIGHTWATER': 'NIGHT_WATER', 'PKFIELD': 'PK_FIELD',
    'PRODECTION': 'PROTECTION', 'PROTECTGUILD': 'PROTECT_GUILD', 'SCOLPION': 'SCORPION', 'SHOCKWAVE': 'SHOCK_WAVE',
    'SMOKELINE': 'SMOKE_LINE', 'SPEARSKILL': 'SPEAR_SKILL', 'STREAMBREATHFIRE': 'STREAM_BREATH_FIRE',
    'STREAMOFICEBREATH': 'STREAM_OF_ICE_BREATH', 'SWORDEFF': 'SWORD_EFFECT', 'SWORDLEFT': 'SWORD_LEFT',
    'SWORDMAIN': 'SWORD_MAIN', 'SWORDRIGHT': 'SWORD_RIGHT', 'TARGETMON': 'TARGET_MONSTER',
    'TARGETPOSITION': 'TARGET_POSITION', 'TOTEMGOLEM': 'TOTEM_GOLEM', 'TWINTAIL': 'TWIN_TAIL',
    'WINDFOCE': 'WIND_FORCE', 'WRISTRING': 'WRIST_RING',
}

# Names chosen by hand: code -> name. For collisions the rules do not settle, file names that say
# nothing or are misspelled, and numbers without a name. Names already in the catalogue files are
# kept as they are.
NAME_OVERRIDES = {
    '9': 'kalimaFallingStone',  # GMHellas.cpp: draws model 9, a stone of the Kalima map objects
    'MODEL_SPEAR': 'lightSpear',  # the item model slot of the Light Spear; MODEL__SPEAR is "spear"
    'MODEL_PIERCING+1': 'piercingFire',  # no model; flies with a piercing and trails fire and smoke
    'BITMAP_SPARK+2': 'sparks',  # Spark.jpg, four clusters of sparks; BITMAP_SPARK is "spark"
    'BITMAP_BLOOD+1': 'blood2',  # blood.tga; BITMAP_BLOOD (blood01.tga) is "blood"
    'BITMAP_LIGHT+1': 'impact3',  # Impack03.jpg
    'BITMAP_LIGHT+2': 'glitter',  # cra_04.jpg
    'BITMAP_LIGHT+3': 'impact1',  # Impack01.jpg
    'BITMAP_EXPLOTION+1': 'explosion2',  # DinoE.jpg, four frames of an explosion
    'BITMAP_ADV_SMOKE+1': 'advSmoke2',  # fi02.tga
    'BITMAP_LIGHTMARKS': 'lightMarks2',  # lightmarks.jpg, as BITMAP_LIGHT_MARKS ("lightMarks")
}


def camel(words):
    parts = [p for p in re.split(r'_+', words) if p]
    if not parts:
        return ''
    name = parts[0].lower() + ''.join(p[:1].upper() + p[1:].lower() for p in parts[1:])
    return name


def symbol_name(symbol):
    """'MODEL_CURSEDTEMPLE_HOLYITEM' -> 'cursedTempleHolyItem', 'BITMAP_FLOWER01' -> 'flower1',
    'MODEL_1_STREAMBREATHFIRE' -> 'streamBreathFire1'."""
    words = []
    for word in re.sub(r'^(MODEL|BITMAP)_', '', symbol).split('_'):
        if word in WORDS:
            words.append(WORDS[word])
            continue
        m = re.match(r'([A-Z]+)(\d*)$', word)
        if m and m.group(1) in WORDS:
            words.append(WORDS[m.group(1)] + m.group(2))
        elif word:
            words.append(word)
    if words and words[0].isdigit():
        words = words[1:] + [words[0]]
    return re.sub(r'(?<=[A-Za-z])0+(\d+)$', r'\1', camel('_'.join(words)))


def stem_name(stem):
    """'EarthQuake01' -> 'earthQuake1', 'shiny05' -> 'shiny5', 'Fire_02' -> 'fire2'."""
    stem = re.sub(r'[^A-Za-z0-9]+', '_', stem).strip('_')
    if '_' in stem:
        name = camel(stem)
    else:
        name = stem[:1].lower() + stem[1:]
    return re.sub(r'(?<=[A-Za-z])0+(\d+)$', r'\1', name)


def asset_stems():
    """The file loaded into a slot, by the slot expression as written: AccessModel and LoadBitmap."""
    stems = {}
    for relative in all_source_files():
        text = read_source(relative)
        for m in re.finditer(r'AccessModel\s*\(([^,]+),\s*L"[^"]*",\s*L"([^"]+)"(?:\s*,\s*(\d+))?\s*\)', text):
            number = m.group(3)
            stem = m.group(2) + (f'{int(number):02d}' if number is not None else '')
            stems.setdefault(normalize(m.group(1)), stem)
        for m in re.finditer(r'LoadBitmap\s*\(\s*L"([^"]+)"\s*,\s*([^,)]+)', text):
            path = m.group(1).replace('\\\\', '/').replace('\\', '/')
            stem = os.path.splitext(os.path.basename(path))[0]
            stems.setdefault(normalize(m.group(2)), stem)
    return stems


def read_values(work):
    values = {kind: {} for kind in KINDS}
    for line in open(os.path.join(work, 'values.txt'), encoding='utf-8'):
        kind, expression, value = line.rstrip('\n').split('\t')
        values[kind][expression] = int(value)
    return values


def canonical_types(candidates, values):
    """Per kind: (value, canonical expression, all expressions) sorted by value. A plain enum name that
    is not a block marker wins; then the expression the kind's code uses most."""
    result = {}
    for kind in KINDS:
        by_value = defaultdict(list)
        for expression, where in candidates[kind].items():
            if expression in values[kind]:
                by_value[values[kind][expression]].append((expression, len(where)))
        types = []
        for value, expressions in sorted(by_value.items()):
            def rank(item):
                expression, uses = item
                plain = '+' not in expression and not expression.isdigit()
                return (not (plain and not MARKER.search(expression)), -uses, len(expression), expression)
            ordered = sorted(expressions, key=rank)
            types.append((value, ordered[0][0], [e for e, _ in ordered]))
        result[kind] = types
    return result


def make_name(code, aliases, stems):
    """The enum name in camelCase; for BASE+n the file loaded into the slot, else the base name with
    n + 1 (BITMAP_SMOKE+1 -> smoke2)."""
    if code in NAME_OVERRIDES:
        return NAME_OVERRIDES[code]
    if code.isdigit():
        return f'slot{code}'
    if '+' not in code:
        return symbol_name(code)
    for expression in [code] + aliases:
        if expression in stems:
            return stem_name(stems[expression])
    base, _, offset = code.partition('+')
    return symbol_name(base) + str(int(offset) + 1)


def resolve_collisions(entries):
    """A model type and a texture type with the same name in one kind: the model type gets the
    suffix "Model", because texture numbers are types in several kinds and keep one name in all."""
    names = defaultdict(list)
    for name, code in entries:
        names[name].append(code)
    result = []
    for name, code in entries:
        codes = names[name]
        if len(codes) == 2 and code.startswith('MODEL_') and any(c.startswith('BITMAP_') for c in codes):
            name += 'Model'
        result.append((name, code))
    return result


def existing_names(kind):
    path = os.path.join(ROOT, 'src', 'bin', 'Data', 'Effects', KIND_FILE_NAMES[kind])
    if not os.path.exists(path):
        return {}
    return {entry['code']: entry['name'] for entry in json.load(open(path, encoding='utf-8'))['types']}


def write_symbols(types):
    lines = ['#include "stdafx.h"', '', '#include "EffectTypeSymbols.h"', '',
             '#include "Core/Globals/_define.h"', '#include "Core/Globals/_enum.h"',
             '#include "Core/Globals/_TextureIndex.h"', '',
             '// Generated by tools/effect-catalogue/effect_types.py from the types the code uses, see',
             '// docs/effect-data.md. Each kind lists every number once, sorted by number.',
             'namespace Data::Effects', '{', 'namespace', '{', '// clang-format off']
    for kind in KINDS:
        lines.append(f'constexpr EffectTypeSymbol {KIND_ARRAY_NAMES[kind]}[] = {{')
        for value, code, _ in types[kind]:
            expression = code.replace('+', ' + ')
            lines.append(f'    {{"{code}", {expression}}},')
        lines.append('};')
        lines.append('')
    lines[-1:] = ['// clang-format on', '} // namespace', '',
                  'std::span<const EffectTypeSymbol> GetEffectTypeSymbols(EffectKind kind)', '{',
                  '    switch (kind)', '    {']
    for kind, enum in (('effect', 'Effect'), ('particle', 'Particle'), ('joint', 'Joint'), ('sprite', 'Sprite')):
        lines.append(f'    case EffectKind::{enum}:')
        lines.append(f'        return {KIND_ARRAY_NAMES[kind]};')
    lines += ['    }', '    return {};', '}', '} // namespace Data::Effects', '']
    path = os.path.join(SRC, 'Data', 'GameData', 'EffectData', 'EffectTypeSymbols.cpp')
    open(path, 'w', encoding='utf-8', newline='\n').write('\n'.join(lines))


def write_catalogue(kind, entries):
    document = {'formatVersion': 1, 'kind': kind,
                'types': [{'name': name, 'code': code} for name, code in sorted(entries)]}
    path = os.path.join(ROOT, 'src', 'bin', 'Data', 'Effects', KIND_FILE_NAMES[kind])
    open(path, 'w', encoding='utf-8', newline='\n').write(json.dumps(document, indent=2, ensure_ascii=False) + '\n')


def generate(work):
    data = json.load(open(os.path.join(work, 'candidates.json'), encoding='utf-8'))
    values = read_values(work)
    types = canonical_types(data['candidates'], values)
    stems = asset_stems()
    write_symbols(types)
    name_of_code = {}
    for kind in KINDS:
        kept = existing_names(kind)
        entries = resolve_collisions([(make_name(code, aliases[1:], stems), code) for _, code, aliases in types[kind]])
        entries = [(kept.get(code, name), code) for name, code in entries]
        names = defaultdict(list)
        for name, code in entries:
            names[name.lower()].append(code)
        for name, codes in names.items():
            if len(codes) > 1:
                print(f'{kind}: the name {name} is generated for {", ".join(codes)}; choose names in NAME_OVERRIDES')
        for name, code in entries:
            if name_of_code.setdefault(code, name) != name:
                print(f'{kind}: {code} is named {name}, but {name_of_code[code]} in another kind')
            if not re.fullmatch(r'[a-z][A-Za-z0-9]*', name):
                print(f'{kind}: {name} ({code}) is not a name of letters and digits starting with a small letter')
        write_catalogue(kind, entries)
        print(f'{kind}: {len(entries)} types')


def unused(work):
    """Prints the types that the code of a kind handles (case labels, registry rows) but no call creates."""
    data = json.load(open(os.path.join(work, 'candidates.json'), encoding='utf-8'))
    types = canonical_types(data['candidates'], read_values(work))
    for kind in KINDS:
        names = existing_names(kind)
        never = []
        for _, code, aliases in types[kind]:
            where = [w for e in aliases for w in data['candidates'][kind][e]]
            if all(w.endswith((' case', ' registry')) for w in where):
                never.append((names.get(code, '?'), code, where))
        print(f'{kind}: {len(never)} of {len(types[kind])} types are never created')
        for name, code, where in never:
            print(f'  {name} ({code}): {", ".join(where)}')


COMMANDS = {'collect': collect, 'generate': generate, 'unused': unused}

if __name__ == '__main__':
    if len(sys.argv) != 3 or sys.argv[1] not in COMMANDS:
        print(__doc__)
        sys.exit(1)
    COMMANDS[sys.argv[1]](sys.argv[2])
