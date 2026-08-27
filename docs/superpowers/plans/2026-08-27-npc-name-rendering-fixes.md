# NPC Name Rendering Fixes Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render every shipped English, Spanish, and Portuguese NPC name as valid Unicode without overflowing runtime name buffers.

**Architecture:** Normalize the three checked-in NPC-name assets to UTF-8, enlarge the shared monster-name capacity, and route the three chat-label ID copies through one bounded terminating helper. Tests validate strict asset decoding, shipped-name length, accented/long translations, helper truncation, and termination.

**Tech Stack:** C++20, Python 3 stdlib, CMake/CTest, doctest.

**Spec:** `docs/superpowers/specs/2026-08-27-text-label-health-bar-ui-scaling-design.md`

## Global Constraints

- UTF-8 is the only runtime NPC-name encoding; add no locale fallback.
- Convert all three checked-in `NpcName_*.txt` assets without changing record order or visible text.
- Set `MAX_MONSTER_NAME` to 64.
- Store monster and chat-label names in `MAX_MONSTER_NAME + 1` wide characters.
- Every changed copy must be bounded and explicitly terminated.
- Add no dependency.

---

### Task 1: Asset encoding regression

**Files:**
- Create: `tests/ui/test_npc_name_assets.py`
- Modify: `tests/ui/CMakeLists.txt`
- Modify mechanically: `src/bin/Data/Local/Eng/NpcName_Eng.txt`
- Modify mechanically: `src/bin/Data/Local/Por/NpcName_Por.txt`
- Modify mechanically: `src/bin/Data/Local/Spn/NpcName_Spn.txt`

**Interfaces:**
- Consumes: repository root passed as `sys.argv[1]`.
- Produces: CTest `npc_name_assets`; six strict UTF-8 assets preserving 532 parsed records each.

- [ ] **Step 1: Write the failing asset test**

```python
from pathlib import Path
import re
import sys

ROOT = Path(sys.argv[1])
MAX_MONSTER_NAME = 64
FILES = (
    "src/bin/Data/Local/Eng/NpcName_Eng.txt",
    "src/bin/Data/Local/Por/NpcName_Por.txt",
    "src/bin/Data/Local/Spn/NpcName_Spn.txt",
)
NAME_PATTERN = re.compile(r'^\s*\d+\s+\d+\s+"([^"]*)"', re.MULTILINE)

parsed = {}
for relative in FILES:
    text = (ROOT / relative).read_text(encoding="utf-8", errors="strict")
    names = NAME_PATTERN.findall(text)
    assert len(names) == 532, f"{relative}: expected 532 names, got {len(names)}"
    assert max(map(len, names)) <= MAX_MONSTER_NAME, relative
    parsed[relative] = names

assert "Esqueleto Mágico" in parsed["src/bin/Data/Local/Spn/NpcName_Spn.txt"]
assert "Administrador da Guilda Mercenária Tercia" in parsed["src/bin/Data/Local/Por/NpcName_Por.txt"]
assert "Tercia de Encargados del Gremio Mercenario" in parsed["src/bin/Data/Local/Spn/NpcName_Spn.txt"]
```

Register it:

```cmake
add_test(
    NAME npc_name_assets
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/test_npc_name_assets.py"
            "${CMAKE_SOURCE_DIR}"
)
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `ctest --test-dir build_validation -R '^npc_name_assets$' --output-on-failure`

Expected: FAIL with `UnicodeDecodeError` on an ISO-8859 Spanish or Portuguese asset.

- [ ] **Step 3: Convert the three assets mechanically**

For each exact path above, decode the current bytes as ISO-8859-1, encode as UTF-8, preserve existing line endings, then replace the original file. Do not reorder or edit records.

- [ ] **Step 4: Run the asset test**

Run: `ctest --test-dir build_validation -R '^npc_name_assets$' --output-on-failure`

Expected: PASS.

- [ ] **Step 5: Verify conversion scope**

Run: `git diff --numstat -- src/bin/Data/Local/*/NpcName*.txt src/bin/Data/Local/*/Npcname*.txt`

Expected: exactly three modified assets; no deleted records.

### Task 2: Bounded wide-name storage

**Files:**
- Create: `src/source/Core/Text/WideString.h`
- Create: `src/source/Core/Text/WideString.cpp`
- Create: `tests/ui/test_npc_name_storage.cpp`
- Modify: `tests/ui/CMakeLists.txt`
- Modify: `src/source/Core/Globals/_define.h:350`
- Modify: `src/source/Core/Globals/_struct.h:247-253`
- Modify: `src/source/Engine/Object/ZzzInfomation.cpp:2360-2383`
- Modify: `src/source/UI/Chat/Chat.cpp:52-69,358-385,460-535`

**Interfaces:**
- Produces: `bool Core::Text::CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source)`; returns `true` only when the complete source fits, returns `false` for invalid input or truncation, always terminates every non-empty valid destination.
- Consumes: `MAX_MONSTER_NAME == 64`; strict UTF-8 assets from Task 1.

- [ ] **Step 1: Write the failing storage test**

```cpp
#include <doctest.h>

#include "Core/Globals/_define.h"
#include "Core/Text/WideString.h"

TEST_CASE("monster name capacity covers shipped translations [ui][npc_name]")
{
    CHECK(MAX_MONSTER_NAME == 64);
}

TEST_CASE("wide string copy truncates and terminates [ui][npc_name]")
{
    wchar_t destination[5] = {L'x', L'x', L'x', L'x', L'x'};

    CHECK_FALSE(Core::Text::CopyWideString(destination, 5, L"abcdef"));
    CHECK(destination[0] == L'a');
    CHECK(destination[3] == L'd');
    CHECK(destination[4] == L'\0');
}

TEST_CASE("wide string copy rejects zero capacity [ui][npc_name]")
{
    wchar_t destination = L'x';
    CHECK_FALSE(Core::Text::CopyWideString(&destination, 0, L"abc"));
    CHECK(destination == L'x');
}
```

Register it:

```cmake
mu_add_test(
    NAME npc_name_storage_tests
    SOURCES
        test_npc_name_storage.cpp
        ${CMAKE_SOURCE_DIR}/src/source/Core/Text/WideString.cpp
)
```

- [ ] **Step 2: Run the storage test to verify it fails**

Run: `cmake --build build_validation --target npc_name_storage_tests -j2 && ctest --test-dir build_validation -R 'npc_name_storage' --output-on-failure`

Expected: build FAIL because `Core/Text/WideString.h` does not exist.

- [ ] **Step 3: Implement the bounded helper**

```cpp
// Core/Text/WideString.h
#pragma once

#include <cstddef>

namespace Core::Text
{
    bool CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source);
}
```

```cpp
// Core/Text/WideString.cpp
#include "Core/Text/WideString.h"

#include <algorithm>
#include <cwchar>

bool Core::Text::CopyWideString(wchar_t* destination, std::size_t destinationCount, const wchar_t* source)
{
    if (destination == nullptr || destinationCount == 0)
        return false;

    if (source == nullptr)
    {
        destination[0] = L'\0';
        return false;
    }

    const std::size_t sourceLength = std::wcslen(source);
    const std::size_t copyLength = std::min(sourceLength, destinationCount - 1);
    std::wmemcpy(destination, source, copyLength);
    destination[copyLength] = L'\0';
    return copyLength == sourceLength;
}
```

- [ ] **Step 4: Enlarge monster and chat storage**

Apply these exact declarations:

```cpp
#define MAX_MONSTER_NAME    64
```

```cpp
wchar_t Name[MAX_MONSTER_NAME + 1];
```

```cpp
wchar_t ID[MAX_MONSTER_NAME + 1];
```

Decode each null-terminated token without truncating its UTF-8 bytes, then terminate defensively:

```cpp
CMultiLanguage::ConvertFromUtf8(m->Name, TokenString);
m->Name[MAX_MONSTER_NAME] = L'\0';
```

- [ ] **Step 5: Replace the three unsafe chat-label copies**

Include `Core/Text/WideString.h`, replace each `wcscpy(c->ID, character_name)` with:

```cpp
Core::Text::CopyWideString(c->ID, std::size(c->ID), character_name);
```

Replace the byte-literal clear with:

```cpp
memset(c->Text[0], 0, sizeof(c->Text[0]));
```

- [ ] **Step 6: Run focused tests**

Run: `cmake --build build_validation --target npc_name_storage_tests -j2 && ctest --test-dir build_validation -R 'npc_name_(assets|storage)' --output-on-failure`

Expected: all NPC-name tests PASS.

- [ ] **Step 7: Build affected production code**

Run: `cmake --build build_validation --target MuClient -j2`

Expected: target builds successfully.

- [ ] **Step 8: Commit**

```bash
git add tests/ui/CMakeLists.txt tests/ui/test_npc_name_assets.py tests/ui/test_npc_name_storage.cpp \
  src/source/Core/Text/WideString.h src/source/Core/Text/WideString.cpp \
  src/source/Core/Globals/_define.h src/source/Core/Globals/_struct.h \
  src/source/Engine/Object/ZzzInfomation.cpp src/source/UI/Chat/Chat.cpp \
  src/bin/Data/Local/Eng/NpcName_Eng.txt \
  src/bin/Data/Local/Por/NpcName_Por.txt \
  src/bin/Data/Local/Spn/NpcName_Spn.txt
git commit -m "fix(ui): preserve translated NPC names"
```
